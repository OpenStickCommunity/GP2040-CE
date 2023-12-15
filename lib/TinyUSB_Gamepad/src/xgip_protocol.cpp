#include "xgip_protocol.h"

#include "gamepad/descriptors/XBOneDescriptors.h"

#include <cstring>

#include <stdio.h>

// Default Constructor
XGIPProtocol::XGIPProtocol() {
    reset();
}

// Default Destructor
XGIPProtocol::~XGIPProtocol() {
    if ( data != nullptr ) {
        delete data;
    }
}

// Reset packet information
void XGIPProtocol::reset() {
    //printf("[XGIPProtocol::reset]\r\n");
    memset((void*)&header, 0, sizeof(GipHeader_t));
    totalChunkLength = 0;      // How big is the chunk?
    totalLengthReceived = 0;   // How much have we received?
    //uint16_t actualDataLength;      // What is the actual calculated data length? (useds in ACKs)
    actualDataReceived = 0;    // How much actual data have we received?
    totalChunkReceived = 0;    // How much have we received in chunk mode length? (length | 0x80)
    totalChunkSent = 0;        // How much chunk-data (not real total) have we sent?
    totalDataSent = 0;         // How much actual data have we sent?
    numberOfChunksSent = 0;    // How many actual chunks have we sent?
    chunkEnded = false;
    isValidPacket = false;
    if ( data != nullptr ) {
        delete [] data;
    }
    data = nullptr;
    dataLength = 0;
    memset(packet, 0, sizeof(packet));
}

//
// Example of chunk size < 0x100 start:  06,f0,06,ba, 00,4a, 00,c2,00,08,00,44,08,01,00,40,9c,f1,e0,58,83,94,6b,c0,a9,ab,4c,0d,63,85,bd,8e,4d,64,42,bd,86,a6,ed,a9,19,b2,c8,54,03,82,f0,aa,c3,65,77,84,fd,44,d3,3b,56,14,cf,af,37,c1,58,b7
// 0xba = length is (0x3A) | 0x80
// total size = 0x00 0x4a (or 0x004a bytes)
//
// Example of chunk size >= 0x100 start: 06,f0,04,3a, 92,02, 00,41,00,05,01,04,05,01,01,00,00,a7,0b,ec,b6,0f,98,97,05,99,d3,eb,d1,62,b1,0d,10,44,bb,20,19,fe,7a,a6,91,28,c0,6c,ce,8f,44,49,4b,f4,4b,bb,23,be,18,16,2f,0c,e5,79,1a,4a,e3,8c
// 0x3a = length is (0x3a)
// total size = 0x29 0x02 (or 0x0292 bytes)
//
// Parse incoming packet
// TRANSCEIVER: RECEIVE PACKET (RX)
bool XGIPProtocol::parse(const uint8_t * buffer, uint16_t len) { // Parse incoming packet
/*    printf("[XGIP_PROCOTOL] Packet (%u): ", len);
    for(uint16_t i = 0; i < len; i++) { 
        printf("%02x ", buffer[i]);
    }
    printf("\r\n");
*/

    //printf("[XGIP_PROCOTOL] Data Length is currently: %04X\r\n", dataLength);

    // Do we have enough room for a header? No, this isn't valid
    if ( len < 4 ) {
        reset();
        //printf("[XGIPProtocol::parse] Invalid due to bad length\r\n");
        isValidPacket = false;
        return false;
    }

    // Set packet length
    packetLength = len;

    // Use buffer as a raw packet without copying to our internal structure
    GipHeader_t * newPacket = (GipHeader_t*)buffer;
    if ( newPacket->command == GIP_ACK_RESPONSE ) {
        if ( len != 13 ||  newPacket->internal != 0x01 ||  newPacket->length != 0x09 ) {
            //printf("[XGIP_PROCOTOL] Bad ACK Request (length is wrong)\r\n");
            reset();
            isValidPacket = false;
            return false; // something malformed in this packet
        }

        memcpy((void*)&header, buffer, sizeof(GipHeader_t));

        // don't do anything with ack packets for now
        isValidPacket = true;
        return true;
    } else { // Non-ACK
        // Continue parsing chunked data
        if ( newPacket->chunked == true ) {
            // Always copy to header buffer
            memcpy((void*)&header, buffer, sizeof(GipHeader_t));

            if ( header.length == 0 ) { // END OF CHUNK
                uint16_t endChunkSize = (buffer[4] | buffer[5] << 8);

                // Verify chunk is good
                if ( totalChunkLength != endChunkSize) {
                    //printf("[XGIP_PROTOCOL] INVALID PACKET: Chunk Length Value (%u) != End Chunk Size Value (%u)\r\n", totalChunkLength, endChunkSize);
                    isValidPacket = false;
                    return false;
                }

                chunkEnded = true;
                //printf("[XGIP_PROTOCOL] [+] Successfully received the final chunk piece\r\n");
                isValidPacket = true;
                return true; // we're good!
            }
            
            if ( header.chunkStart == 1 ) { // START OF CHUNK
                reset();
                memcpy((void*)&header, buffer, sizeof(GipHeader_t));

                // Get total chunk length in uint16
                if ( header.length > GIP_MAX_CHUNK_SIZE && buffer[4] == 0x00 ) { // if we see 0xBA and buf[4] == 0, single-byte mode
                    totalChunkLength = (uint16_t)buffer[5]; // byte is correct
                } else {
                    // we need to calculate the actual buffer length as this number is not right
                    totalChunkLength = ((uint16_t)buffer[4] | ((uint16_t)buffer[5] << 8)); // not the actual length but the chunked length (length | 0x80)
                }

                // Real data length = chunk length > 0x100? (chunk length - 0x100) - ((chunk length / 0x100)*0x80)
                dataLength = totalChunkLength;
                if ( totalChunkLength > 0x100 ) {
                    dataLength = dataLength - 0x100;
                    dataLength = dataLength - ((dataLength / 0x100)*0x80);
                }

                // Ensure we clear data if its set to something else
                if ( data != nullptr )
                    delete [] data;
                data = new uint8_t[dataLength];
                actualDataReceived = 0; // haven't received anything yet
                totalChunkReceived = header.length; // 
            } else {
                totalChunkReceived += header.length; // not actual data length, but chunk value
            }

            uint16_t copyLength = header.length;
            if ( header.length > GIP_MAX_CHUNK_SIZE ) { // if length is greater than 0x3A (bigger than 64 bytes), we know it is | 0x80 so we can ^ 0x80 and get the real length
                copyLength ^= 0x80;  // packet length is set to length | 0x80 (0xBA instead of 0x3A)
            }
            
            memcpy(&data[actualDataReceived], &buffer[6], copyLength); // 

            actualDataReceived += copyLength;
            numberOfChunksSent++; // count our chunks for the ACK
            isValidPacket = true;
        } else {
            reset();
            memcpy((void*)&header, buffer, sizeof(GipHeader_t));
            if ( header.length > 0 ) {
                if (data != nullptr)
                    delete [] data;
                data = new uint8_t[header.length];
                memcpy(data, &buffer[4], header.length); // copy incoming data
            }
            actualDataReceived = header.length;
            dataLength = actualDataReceived;
            isValidPacket = true;
        }
    }

    return false;
}

bool XGIPProtocol::endOfChunk() {
    return chunkEnded;
}

bool XGIPProtocol::validate() { // is valid packet?
    return isValidPacket;
}

void XGIPProtocol::incrementSequence() {
    header.sequence++;
    if ( header.sequence == 0 )
        header.sequence = 1;
}

void XGIPProtocol::setAttributes(uint8_t cmd, uint8_t seq, uint8_t internal, uint8_t isChunked, uint8_t needsAck) { // Set attributes for next output packet
    header.command = cmd;
    header.sequence = seq;
    header.internal = internal;
    header.chunked = isChunked;
    header.needsAck = needsAck;
}

bool XGIPProtocol::setData(const uint8_t * buffer, uint16_t len) {
    if ( len > 0x3000) { // arbitrary but this should cover us if something bad happens
        //printf("[XGIP_PROTCOL] ERROR CANNOT SET DATA LENGTH : %u, too big!", len);
        return false;
    }
    if ( data != nullptr )
        delete [] data;
    data = new uint8_t[len];
    memcpy(data, buffer, len);
    dataLength = len;
    return true;
}

// TRANSCEIVER: SEND PACKET (TX)
uint8_t * XGIPProtocol::generatePacket() { // Generate output packet
    // Are we a simple data packet?
    if ( header.chunked == 0 ) {
        header.length = (uint8_t)dataLength;
        memcpy(packet, &header, sizeof(GipHeader_t));
        memcpy((void*)&packet[4], data, dataLength);
        packetLength = sizeof(GipHeader_t) + dataLength;
    } else { // Are we a chunk?
        if ( numberOfChunksSent > 0 && totalDataSent == dataLength ) { // General Final Chunk Packet (End-Packet)
            header.needsAck = 0;
            header.length = 0;
            memcpy(packet, &header, sizeof(GipHeader_t));
            packet[4] = totalChunkLength & 0x00FF;
            packet[5] = (totalChunkLength & 0xFF00) >> 8;
            packetLength = sizeof(GipHeader_t) + 2;
            chunkEnded = true;
        } else {
            if ( numberOfChunksSent == 0 ) {
                if ( dataLength < GIP_MAX_CHUNK_SIZE ) {
                    // In the rare case the chunked packet is < max chunk size
                    // we set the chunk flags to 0, set our actual data length
                    // BUT we still require an ACK and have to reply to it
                    totalChunkLength = dataLength;
                    header.chunkStart = 0;
                    header.chunked = 0;
                } else {
                    header.chunkStart = 1;

                    // Clean this
                    uint16_t i = dataLength;
                    uint16_t j = 0;
                    do {
                        if ( i < GIP_MAX_CHUNK_SIZE ) {
                            if ( (j / 0x100) != ((j + i) / 0x100)) { // if we go 0x100 to 0x200, or 0x200 to 0x300, | 0x80
                                j = j + (i | 0x80);
                            } else {
                                j = j + i;
                            }
                            i = 0;
                        } else {
                            if ( (j + GIP_MAX_CHUNK_SIZE > 0x80) && (j + GIP_MAX_CHUNK_SIZE < 0x100) ) {
                                j = j + GIP_MAX_CHUNK_SIZE + 0x100; // first 0x80 bytes, move up 0x100 if we get this far
                            } else {
                                if ( (j / 0x100) != ((j + GIP_MAX_CHUNK_SIZE) / 0x100)) {
                                    j = j + (GIP_MAX_CHUNK_SIZE | 0x80);
                                } else {
                                    j = j + GIP_MAX_CHUNK_SIZE;
                                }
                            }
                            i = i - GIP_MAX_CHUNK_SIZE;
                        }
                    } while( i != 0 );

                    totalChunkLength = j;
                }
            } else {
                header.chunkStart = 0;
            }

            // Ack on 1st and every 5 interval (Mayflash)
            //    Note: this will send on (0 chunks sent) 1st, (4 chunks sent) 5th, (5 chunks sent) 10th, (5 chunks sent) 15th. this is correct
            if ( numberOfChunksSent == 0 || (numberOfChunksSent+1)%5 == 0 ) {
                header.needsAck = 1;
            } else {
                header.needsAck = 0;
            }

            // Assume we're sending the maximum chunk size
            uint16_t dataToSend = GIP_MAX_CHUNK_SIZE;

            // If we're at the end, reduce data to send and set the ack flag
            if ( (dataLength - totalDataSent) < dataToSend ) {
                dataToSend = dataLength - totalDataSent;
                header.needsAck = 1;
            }

            // If we've sent our first chunk already and total chunks sent is < 0x100, | 0x80
            if ( numberOfChunksSent > 0 && totalChunkSent < 0x100 ) {
                header.length = dataToSend | 0x80;
            // we haven't sent any chunks and data length total < 0x80
            } else if ( numberOfChunksSent == 0 && dataLength > GIP_MAX_CHUNK_SIZE && dataLength < 0x80 ) {
                header.length = dataToSend | 0x80; // data < 0x80 and first chunk, we |0x80
            } else {
                header.length = dataToSend; // length is actual data to send
            }

            // Copy our header and data to the packet
            memcpy(packet, &header, sizeof(GipHeader_t));
            memcpy((void*)&packet[6], &data[totalDataSent], dataToSend);

            // Set our packet length
            packetLength = sizeof(GipHeader_t) + 2 + dataToSend;

            // If first packet, chunk value in [4][5] is total chunk length
            uint16_t chunkValue;
            if ( numberOfChunksSent == 0 ) {
                chunkValue = totalChunkLength;
            // else, chunk value is total chunk sent
            } else {
                chunkValue = totalChunkSent;
            }

            // Place value in right-byte if our chunk value is < 0x100
            if ( chunkValue < 0x100 ) {
                packet[4] = 0x00;
                packet[5] = (uint8_t) chunkValue;
            // Split appropriately
            } else {
                packet[4] = chunkValue & 0x00FF;
                packet[5] = (chunkValue & 0xFF00) >> 8;
            }

            // XGIP Hashing: If we're sending over 0x80, + ( data to send + 0x100 )
            if ( totalChunkSent < 0x100 && (totalChunkSent + dataToSend) > 0x80) {
                totalChunkSent = totalChunkSent + dataToSend + 0x100;
            // else if our next chunk sent will roll over the 3rd digit e.g. 0x200 to 0x300,  + ( data to send | 0x80 )
            } else if ( ((totalChunkSent + dataToSend)/0x100) > (totalChunkSent/0x100)) {
                totalChunkSent = totalChunkSent + (dataToSend | 0x80);
            // else + ( data to send )
            } else {
                totalChunkSent = totalChunkSent + dataToSend;
            }
            totalDataSent += dataToSend; // Total Data Sent in bytes
            numberOfChunksSent++;        // Number of Chunks sent so far

            //printf("[XGIP_PROTOCOL] Chunk length: %04X data length: %04X  total data sent: %04X  data to send: %04X  number of chunks sent: %u\r\n", totalChunkLength, dataLength, totalChunkSent, totalDataSent, numberOfChunksSent);
        }
    }
    return packet;
}

uint8_t * XGIPProtocol::generateAckPacket() { // Generate output packet
    packet[0] = 0x01;
    packet[1] = 0x20;
    packet[2] = header.sequence;
    packet[3] = 0x09;
    packet[4] = 0x00;
    packet[5] = header.command;
    packet[6] = 0x20;

    // we have to keep track of # of chunks because data received for ACK is +2 for size of chunk
    uint16_t dataReceived = actualDataReceived;
    packet[7] = dataReceived & 0x00FF;
    packet[8] = (dataReceived & 0xFF00) >> 8;
    packet[9] = 0x00;
    packet[10] = 0x00;
    if ( header.chunked == true ) { // Are we a chunk?
        uint16_t left = dataLength - dataReceived;
        packet[11] = left & 0x00FF;
        packet[12] = (left & 0xFF00) >> 8;
    } else {
        packet[11] = 0;
        packet[12] = 0;
    }
    packetLength = 13;

    return packet;
}

uint8_t XGIPProtocol::getPacketLength() { // Get last generated output packet length
    return packetLength;
}

uint8_t XGIPProtocol::getPacketAck() {
    return header.needsAck;
}

// Get command of a parsed packet
uint8_t XGIPProtocol::getCommand() {
    return header.command;
}

uint8_t XGIPProtocol::getChunked(){
    return header.chunked;
}

uint8_t XGIPProtocol::getSequence() {
    return header.sequence;
}

uint8_t * XGIPProtocol::getData() {   // Get data from a packet or packet-chunk
    return data;
}

uint16_t XGIPProtocol::getDataLength() { // Get length of a packet or packet-chunk
    return dataLength;
}

bool XGIPProtocol::getChunkData(XGIPProtocol & packet) { // get chunk data from incoming packet
    return false;
}

// Last packet parsed needs an ACK
bool XGIPProtocol::ackRequired() {
    return header.needsAck;
}