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

// All max chunks are this size
#define GIP_MAX_CHUNK_SIZE 0x3A
#define GIP_SHORT_PACKET_LIMIT 64

// Reset packet information
void XGIPProtocol::reset() {
    memset((void*)&header, 0, sizeof(GipHeader_t));
    memset((void*)&ackHeader, 0, sizeof(GipHeader_t));
    sequence = 0;
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
        delete data;
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

    // Do we have enough room for a header? No, this isn't valid
    if ( len < 4 ) {
        reset();
        isValidPacket = false;
        return false;
    }

    // Use buffer as a raw packet without copying to our internal structure
    GipHeader_t * newPacket = (GipHeader_t*)buffer;
    if ( newPacket->command == GIP_ACK_RESPONSE ) {
//        printf("[XGIP_PROCOTOL] ACK RESPONSE!\r\n");

        // validate this is the correct ACK
        if ( len != 13 ||  newPacket->internal != 0x01 ||  newPacket->length != 0x09 ) {
 //           printf("[XGIP_PROCOTOL] Bad ACK Request (length is wrong)\r\n");
            reset();
            isValidPacket = false;
            return false; // something malformed in this packet
        }

        memcpy((void*)&header, buffer, sizeof(GipHeader_t));
        memcpy((void*)&ackHeader, &buffer[5], sizeof(GipHeader_t));
        if ( ackHeader.length != 0 || ackHeader.internal != 1 ) {
//            printf("[XGIP_PROCOTOL] Bad ACK Request (internal)\r\n");
            reset();
            isValidPacket = false;
            return false; // something malformed in the ACK
        }

        // Don't validate this yet        
        /*uint16_t ackReceived = *((uint16_t*)&buffer[11]);
        if ( ackReceived != totalChunkReceived ) {
            printf("[XGIP_PROCOTOL] ACK received data length does not match how much we've sent\r\n");
            reset();
            isValidPacket = false;
            return false;
        }*/

        // we're good
        isValidPacket = true;
        return true;
    } else { // Non-ACK
        // Continue parsing chunked data
        if ( newPacket->chunked == true ) {
            // START OF CHUNK
            if ( newPacket->chunkStart == 1 ) { // START OF CHUNK
                printf("[XGIP_PROCOTOL] Start of CHUNK\r\n");
                reset();
            }
            
            // Always copy to header buffer
            memcpy((void*)&header, buffer, sizeof(GipHeader_t));
            
            if ( header.chunkStart == 1 ) { // START OF CHUNK
                sequence = header.sequence;

                // Get total chunk length in uint16
                if ( header.length > GIP_MAX_CHUNK_SIZE && buffer[4] == 0x00 ) { // if we see 0xBA and buf[4] == 0, single-byte mode
                    totalChunkLength = (uint16_t)buffer[5]; // byte is correct
                } else {
                    // we need to calculate the actual buffer length as this number is not right
                    uint16_t chunkedPartition = (buffer[4] | buffer[5] << 8);
                    totalChunkLength = chunkedPartition; // not the actual length but the chunked length (length | 0x80)
                }

                // We can go up to 0xFFFF in size? 65kb
                printf("[XGIP_PROCOTOL] Got a chunk total size (not actual size): %u\r\n", totalChunkLength);

                // Calculate the actual length of our packet which is NOT the total chunk length value
                if ( header.length == (GIP_MAX_CHUNK_SIZE | 0x80) ) { // if first chunk packet length is 0x3A|0x80, then total chunk length is buf[5]
                    dataLength = totalChunkLength;
                } else {
                    // this might not work if length is right at 0x100, verify
                    dataLength = totalChunkLength - (0x80*2);
                }

                printf("[XGIP_PROCOTOL] Allocating calculated buffer of size: %u\r\n", dataLength);
                data = new uint8_t[dataLength];
                actualDataReceived = 0; // haven't received anything yet
                totalChunkReceived = header.length; // 
            } else if ( header.length == 0 ) { // END OF CHUNK
                printf("[XGIP_PROCOTOL] End of CHUNK\r\n");
                uint16_t endChunkSize = (buffer[4] | buffer[5] << 4);

                // Verify chunk is good
                if ( totalChunkLength != endChunkSize) {
                    printf("[XGIP_PROTOCOL] Chunk Length Value != End Chunk Size Value\r\n");
                    isValidPacket = false;
                    return false;
                } 

                chunkEnded = true;
                printf("[XGIP_PROTOCOL] [+] Successfully received the final chunk piece\r\n");
                isValidPacket = true;
                return true; // we're good!
            }

            totalChunkReceived += header.length; // not actual data length, but chunk value
            uint16_t copyLength = header.length;
            if ( header.length > GIP_MAX_CHUNK_SIZE ) { // if length is greater than 0x3A (bigger than 64 bytes), we know it is | 0x80 so we can ^ 0x80 and get the real length
                copyLength ^= 0x80;  // packet length is set to length | 0x80 (0xBA instead of 0x3A)
            }
            copyLength -= 2; // get rid of two bytes used for chunk total OR chunk total sent bytes
            printf("[XGIP_PROCOTOL] Copying chunk of data: %u\r\n", copyLength);
            memcpy(&data[actualDataReceived], &buffer[6], copyLength); // 
            actualDataReceived += copyLength;
            numberOfChunksSent++; // count our chunks for the ACK
            isValidPacket = true;
        } else {
            printf("[XGIP_PROTOCOL] New Non-Chunked Packet\r\n");
            if ( newPacket->length != (len-4) ) {
                printf("[XGIP_Protocol] ERROR! Header length does not match our incoming packet\r\n");
                isValidPacket = false;
                return false;
            }
            reset();
            memcpy((void*)&header, buffer, sizeof(GipHeader_t));
            if ( header.length == 0 ) {
                printf("[XGIP_PROCOTOL] No data, just a command\r\n", header.length);
            } else {
                data = new uint8_t[header.length];
                printf("[XGIP_PROCOTOL] Copying non-chunk of data size: %u\r\n", header.length);
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
    sequence++;
    if ( sequence == 0 )
        sequence = 1;
}

bool XGIPProtocol::setAttributes(uint8_t cmd, uint8_t seq, uint8_t internal, bool isChunked) { // Set attributes for next output packet
    printf("[XGIP_PROTCOL] Setting attributes:\r\n   header.command = %02x\r\n   header.sequence = %u\r\n   header.internal = %u\r\n   header.chunked = %u\r\n", 
                cmd, seq, internal, isChunked);
    header.command = cmd;
    header.sequence = seq;
    header.internal = internal;
    header.chunked = isChunked;
    return true;
}

bool XGIPProtocol::setData(const uint8_t * buffer, uint16_t len) {
    if ( len > 0x3000) { // arbitrary but this should cover us if something bad happens
        printf("[XGIP_PROTCOL] ERROR CANNOT SET DATA LENGTH : %u, too big!", len);
        return false;
    }
    if ( data != nullptr )
        delete data;
    printf("[XGIP_PROTOCOL] Setting data of total length : %u\r\n", len);
    data = new uint8_t[len];
    memcpy(data, buffer, len);
    dataLength = len;
    return true;
}

// TRANSCEIVER: SEND PACKET (TX)
uint8_t * XGIPProtocol::generatePacket() { // Generate output packet
    // Are we a simple data packet?
    if ( header.chunked == 0 ) {
        header.needsAck = 0;
        header.length = (uint8_t)dataLength;
        memcpy(packet, &header, sizeof(GipHeader_t));
        memcpy((void*)&packet[4], data, dataLength);
        packetLength = sizeof(GipHeader_t) + dataLength;
    } else { // Are we a chunk?
        if ( numberOfChunksSent > 0 && totalDataSent == dataLength ) { // General Final Chunk Packet (End-Packet)
            //printf("[XGIP_PROTOCOL] GENERATE FINAL CHUNK PACKET\r\n");
            header.needsAck = 0;
            header.length = 0;
            memcpy(packet, &header, sizeof(GipHeader_t));
            // Save total chunk length is 1-byte mode if packet is < 0x100
            if ( totalChunkLength < 0x100 ) {
                packet[4] = 0x00;
                packet[5] = (uint8_t) totalChunkLength;
            } else {
                packet[4] = totalChunkLength & 0x00FF;
                packet[5] = (totalChunkLength & 0xFF00) >> 8;
            } 
            packetLength = sizeof(GipHeader_t) + 2;
            //printf("[XGIP_PROTOCOL] Packet Length: %u\r\n", packetLength);
            chunkEnded = true;
        } else {
            //printf("[XGIP_PROTOCOL] Generating chunk\r\n");
            if ( numberOfChunksSent == 0 ) {
                header.chunkStart = 1;
               // printf("[XGIP_PROTOCOL] Generating size of chunks with odd behavior\r\n");

                // CLEAN THIS UP
                uint8_t i = dataLength;
                uint16_t j = 0;
                do {
                    if ( dataLength < GIP_SHORT_PACKET_LIMIT ) { // GIP_SHORT_PACKET_LIMIT is wrong but it seems like there's a short-burst packet size
                        if ( i < GIP_MAX_CHUNK_SIZE ) {
                            j += (i | 0x80);
                            i = 0;
                        } else {
                            j = j + (GIP_MAX_CHUNK_SIZE | 0x80);
                            i = i - GIP_MAX_CHUNK_SIZE;
                        }
                    } else {
                        if ( i < GIP_MAX_CHUNK_SIZE ) {
                            j += i;
                            i = 0;
                            //printf("-> if ( i < GIP_MAX_CHUNK_SIZE ) j=%u i=%u\r\n", j, i);
                        } else if ( j != 0 && (dataLength - i < 0x100)) {
                            j = j + (GIP_MAX_CHUNK_SIZE | 0x80);
                            i = i - GIP_MAX_CHUNK_SIZE;
                            //printf("-> else if ( j != 0 && (dataLength - i < 0x100)) j=%u i=%u\r\n", j, i);
                        } else {
                            j = j + (GIP_MAX_CHUNK_SIZE);
                            i = i - GIP_MAX_CHUNK_SIZE;
                            //printf("-> else j=%u i=%u\r\n", j, i);
                        }
                    }
                } while( i != 0 );
                totalChunkLength = j;
                //printf("[XGIP_PROTOCOL] Calculated total chunk length: %04x\r\n", totalChunkLength);
            } else {
                header.chunkStart = 0;
            }

            // Ack on 1st and every 5 interval (Mayflash)
            //    Note: this will send on (0 chunks sent) 1st, (4 chunks sent) 5th, (5 chunks sent) 10th, (5 chunks sent) 15th. this is correct
            if ( numberOfChunksSent == 0 || (numberOfChunksSent)%5 == 0 ) {
                //printf("[XGIP_PROTOCOL] Setting ACK on chunk %u\r\n", numberOfChunksSent+1);
                header.needsAck = 1;
            } else {
                header.needsAck = 0;
            }

            // we also request an ack if this is the last piece of data, then we need to send the final command
            uint16_t dataLeft = dataLength - totalDataSent;
            uint16_t dataToSend = 0; // calculate
            if ( dataLeft < GIP_MAX_CHUNK_SIZE ) {
                dataToSend = dataLeft;
                header.needsAck = 1;
                header.length = (uint8_t)dataToSend;
                if ( dataLength < GIP_SHORT_PACKET_LIMIT ) { // VERY ODD: on last packet, if our total data is < 0x100, we have to |=0x80
                    //printf("[XGIP_PROTOCOL] BIT OR 0x80 because total data length < 0x100\r\n");
                    header.length |= 0x80;  // odd XGIP behavior
                }
            } else {
                dataToSend = GIP_MAX_CHUNK_SIZE;
                header.length = (uint8_t)dataToSend;
                // if total data >= 0x100:
                //   first packet is ONLY length
                //   packets before we get to two-byte total are length |= 0x80
                //   packets after we get to two-byte total are length only
                if ( dataLength >= GIP_SHORT_PACKET_LIMIT ) {
                    if ( totalDataSent > 0 && totalDataSent < 0x100 ) {
                        //printf("Data length > 0x100, Total Data Sent > 0 and < 0x100: %u\r\n", totalDataSent);
                        header.length |= 0x80;
                    }
                // else if totalData < 0x100:
                } else if ( dataLength < GIP_SHORT_PACKET_LIMIT ) {
                    //printf("[XGIP_PROTOCOL] Total data length (%u) less than 0x100, OR 0x80\r\n", dataLength);
                    header.length |= 0x80;
                }

                // Set our packet length
                packetLength = sizeof(GipHeader_t) + 2 + dataToSend;
            }

            // Copy our header
            //printf("[XGIP_PROTOCOL] Data to send: %02x\r\n", dataToSend);
            memcpy(packet, &header, sizeof(GipHeader_t));
            memcpy((void*)&packet[6], &data[totalDataSent], dataToSend);

            // Set our packet length
            packetLength = sizeof(GipHeader_t) + 2 + dataToSend;
            //printf("[XGIP_PROTOCOL] Packet Length: %u\r\n", packetLength);

            // Bytes 5 and 6 are always the ACTUAL bytes sent, not the chunked |0x80
            if ( numberOfChunksSent == 0 ) { // FIRST PACKET
                //printf("[XGIP_PROTOCOL] FIRST PACKET\r\n");
                if ( totalChunkLength < 0x100 ) {
                    packet[4] = 0x00;
                    packet[5] = (uint8_t) totalChunkLength;
                } else {
                    packet[4] = totalChunkLength & 0x00FF;
                    packet[5] = (totalChunkLength & 0xFF00) >> 8;
                }
            } else {
                if ( dataToSend == GIP_MAX_CHUNK_SIZE ) { // MIDDLE PACKET
                    if ( totalDataSent < 0x100 ) {
                        packet[4] = 0x00;
                        packet[5] = (uint8_t) totalDataSent;
                    } else {
                        packet[4] = totalDataSent & 0x00FF;
                        packet[5] = (totalDataSent & 0xFF00) >> 8;
                    }
                } else {  // LAST PACKET (not chunk-end packet)
                    // VERY Odd behavior: very last packet is total chunk length - ACTUAL (not |=0x80) size of last packet
                    uint16_t lastChunkSize = totalChunkLength - dataToSend;
                    if ( lastChunkSize < 0x100 ) {
                        packet[4] = 0x00;
                        packet[5] = (uint8_t) lastChunkSize;
                    } else {
                        packet[4] = lastChunkSize & 0x00FF;
                        packet[5] = (lastChunkSize & 0xFF00) >> 8;
                    }
                }
            }

            // Do not add total data sent until after we calculate our packet[4] packet[5]
            totalDataSent += dataToSend;
            numberOfChunksSent++; // chunk went up
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
    uint16_t dataReceived = actualDataReceived + numberOfChunksSent*2; // 2 * numberOfChunksReceived
    packet[7] = dataReceived & 0x00FF;
    packet[8] = (dataReceived & 0xFF00) << 4;
    packet[9] = 0x00;
    packet[10] = 0x00;
    if ( header.chunked == true ) { // Are we a chunk?
        uint16_t left = dataLength - dataReceived;
        packet[11] = left & 0x00FF;
        packet[12] = (left & 0xFF00) << 4;
    } else {
        packet[11] = 0x00;
        packet[12] = 0x00;
    }
    packetLength = 13;
    return packet;
}

uint8_t XGIPProtocol::getPacketLength() { // Get last generated output packet length
    return packetLength;
}

// Get command of a parsed packet
uint8_t XGIPProtocol::getCommand() {
    return header.command;
}

// Get command of a parsed ACK packet
uint8_t XGIPProtocol::getAckCommand() {
    return ackHeader.command;
}

uint8_t XGIPProtocol::getSequence() {
    return header.sequence;
}

uint8_t * XGIPProtocol::getData() {   // Get data from a packet or packet-chunk
    return data;
}

uint8_t XGIPProtocol::getDataLength() { // Get length of a packet or packet-chunk
    return dataLength;
}

bool XGIPProtocol::getChunkData(XGIPProtocol & packet) { // get chunk data from incoming packet
    return false;
}

// Last packet parsed needs an ACK
bool XGIPProtocol::ackRequired() {
    return header.needsAck;
}