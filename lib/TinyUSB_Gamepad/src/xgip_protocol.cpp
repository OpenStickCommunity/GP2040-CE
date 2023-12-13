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

    //printf("[XGIP_PROCOTOL] Data Length is currently: %04X\r\n", dataLength);

    // Do we have enough room for a header? No, this isn't valid
    if ( len < 4 ) {
        reset();
        printf("[XGIPProtocol::parse] Invalid due to bad length\r\n");
        isValidPacket = false;
        return false;
    }

    // Set packet length
    packetLength = len;

    // Use buffer as a raw packet without copying to our internal structure
    GipHeader_t * newPacket = (GipHeader_t*)buffer;
    if ( newPacket->command == GIP_ACK_RESPONSE ) {
        if ( len != 13 ||  newPacket->internal != 0x01 ||  newPacket->length != 0x09 ) {
            printf("[XGIP_PROCOTOL] Bad ACK Request (length is wrong)\r\n");
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
                //printf("[XGIP_PROCOTOL] End of CHUNK\r\n");
                //uint16_t endChunkSize = (buffer[4] | buffer[5] << 8);

                // Verify chunk is good
                /*
                if ( totalChunkLength != endChunkSize) {
                    printf("[XGIP_PROTOCOL] INVALID PACKET: Chunk Length Value (%u) != End Chunk Size Value (%u)\r\n", totalChunkLength, endChunkSize);
                    isValidPacket = false;
                    return false;
                }*/

                chunkEnded = true;
                //printf("[XGIP_PROTOCOL] [+] Successfully received the final chunk piece\r\n");
                isValidPacket = true;
                return true; // we're good!
            }
            
            if ( header.chunkStart == 1 ) { // START OF CHUNK
                //printf("[XGIP_PROCOTOL] Start of CHUNK\r\n");
                reset();
                memcpy((void*)&header, buffer, sizeof(GipHeader_t));

                // Get total chunk length in uint16
                if ( header.length > GIP_MAX_CHUNK_SIZE && buffer[4] == 0x00 ) { // if we see 0xBA and buf[4] == 0, single-byte mode
                    totalChunkLength = (uint16_t)buffer[5]; // byte is correct
                } else {
                    // we need to calculate the actual buffer length as this number is not right
                    totalChunkLength = ((uint16_t)buffer[4] | ((uint16_t)buffer[5] << 8)); // not the actual length but the chunked length (length | 0x80)
                }

                // We can go up to 0xFFFF in size? 65kb
                //printf("[XGIP_PROCOTOL] Got a chunk total size (not actual size): %04x\r\n", totalChunkLength);

                dataLength = totalChunkLength;
                if ( totalChunkLength > 0x100 ) {
                    dataLength = dataLength - 0x100;
                    dataLength = dataLength - ((dataLength / 0x100)*0x80);
                }

                //printf("[XGIP_PROCOTOL] Setting data length to: %04X\r\n", dataLength);
                data = new uint8_t[dataLength];
                actualDataReceived = 0; // haven't received anything yet
                totalChunkReceived = header.length; // 
            } else {
                //printf("[XGIP_PROCOTOL] MIDDLE of CHUNK\r\n");
                totalChunkReceived += header.length; // not actual data length, but chunk value
            }

            uint16_t copyLength = header.length;
            if ( header.length > GIP_MAX_CHUNK_SIZE ) { // if length is greater than 0x3A (bigger than 64 bytes), we know it is | 0x80 so we can ^ 0x80 and get the real length
                copyLength ^= 0x80;  // packet length is set to length | 0x80 (0xBA instead of 0x3A)
            }
            //printf("[XGIP_PROCOTOL] Header Length: %02X Copying chunk of data: %04X to data[%04X]\r\n", header.length, copyLength, actualDataReceived);
            memcpy(&data[actualDataReceived], &buffer[6], copyLength); // 

            actualDataReceived += copyLength;
            numberOfChunksSent++; // count our chunks for the ACK
            isValidPacket = true;
        } else {
            //printf("[XGIP_PROTOCOL] New Non-Chunked Packet\r\n");
            /*if ( newPacket->length != (len-4) ) {
                printf("[XGIP_Protocol] INVALID PACKET: Header length does not match our incoming packet\r\n");
                isValidPacket = false;
                return false;
            }*/
            reset();
            memcpy((void*)&header, buffer, sizeof(GipHeader_t));
            if ( header.length > 0 ) {
                //printf("[XGIP_PROCOTOL] Copying non-chunk of data size: %u\r\n", header.length);
                if (data != nullptr)
                    delete data;
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
        printf("[XGIP_PROTCOL] ERROR CANNOT SET DATA LENGTH : %u, too big!", len);
        return false;
    }
    if ( data != nullptr )
        delete data;
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
            //printf("[XGIP_PROTOCOL] GENERATE FINAL CHUNK PACKET\r\n");
            header.needsAck = 0;
            header.length = 0;
            memcpy(packet, &header, sizeof(GipHeader_t));
            // Save total chunk length is 1-byte mode if packet is < 0x100
            if ( totalChunkLength < GIP_MAX_CHUNK_SIZE ) {
                packet[4] = 0x00;
                packet[5] = (uint8_t) totalChunkLength;
            } else {
                uint16_t chunkLengthShort = totalChunkLength;
                packet[4] = chunkLengthShort & 0x00FF;
                packet[5] = (chunkLengthShort & 0xFF00) >> 8;
            } 
            packetLength = sizeof(GipHeader_t) + 2;
            //printf("[XGIP_PROTOCOL] Packet Length: %u\r\n", packetLength);
            chunkEnded = true;
        } else {
            //printf("[XGIP_PROTOCOL] Generating chunk\r\n");
            if ( numberOfChunksSent == 0 ) {
                header.chunkStart = 1;
               // printf("[XGIP_PROTOCOL] Generating size of chunks with odd behavior\r\n");


                // This is wrong:
                // We split the outgoing packets into 0x3A chunks
                // IF we've sent less than 0x80, we OR the length but don't do anything to chunk total
                // IF we've sent more than 0x80, add 0x100
                // IF our next additional packet length moves from 0x1xx to 0x2xx or 0x2xx to 0x3xx, OR the incoming additional length 0x80
                uint16_t i = dataLength;
                uint16_t j = 0;
                do {
                    if ( i < GIP_MAX_CHUNK_SIZE ) {
                        //printf("-> if ( i < GIP_MAX_CHUNK_SIZE ) { j=%u i=%u\r\n", j, i);
                        if ( (j / 0x100) != ((j + i) / 0x100)) { // if we go 0x100 to 0x200, or 0x200 to 0x300, | 0x80
                            j = j + (i | 0x80);
                        } else {
                            j = j + i;
                        }
                        i = 0;
                    } else {
                        if ( (j + GIP_MAX_CHUNK_SIZE > 0x80) && (j + GIP_MAX_CHUNK_SIZE < 0x100) ) {
                            j = j + GIP_MAX_CHUNK_SIZE + 0x100; // first 0x80 bytes, move up 0x100 if we get this far
                            //printf("-> if ( (j + GIP_MAX_CHUNK_SIZE > 0x80) && (j + GIP_MAX_CHUNK_SIZE < 0x100) ) { j=%u i=%u\r\n", j, i);
                        } else {
                            if ( (j / 0x100) != ((j + GIP_MAX_CHUNK_SIZE) / 0x100)) {
                                j = j + (GIP_MAX_CHUNK_SIZE | 0x80);
                                //printf("-> if ( (j / 0x100) != (j + GIP_MAX_CHUNK_SIZE / 0x100)) { j=%u i=%u\r\n", j, i);
                            } else {
                                j = j + GIP_MAX_CHUNK_SIZE;
                                //printf("->  } else { j=%u i=%u\r\n", j, i);
                            }
                        }
                        i = i - GIP_MAX_CHUNK_SIZE;
                    }
                } while( i != 0 );

                /*
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
                */
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

/*
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
*/
            uint16_t dataToSend = GIP_MAX_CHUNK_SIZE; // calculate
            if ( (dataLength - totalDataSent) < dataToSend ) {
                dataToSend = dataLength - totalDataSent;
                header.needsAck = 1;
            }
            if ( numberOfChunksSent > 0 && totalChunkSent < 0x100 ) {
                header.length = dataToSend | 0x80;
            } else if ( numberOfChunksSent == 0 && dataLength < 0x80 ) {
                header.length = dataToSend | 0x80; // data < 0x80 and first chunk, we |0x80
            } else {
                header.length = dataToSend;
            }

            packetLength = sizeof(GipHeader_t) + 2 + dataToSend;

            // Copy our header
            //printf("[XGIP_PROTOCOL] Data to send: %02x\r\n", dataToSend);
            memcpy(packet, &header, sizeof(GipHeader_t));
            memcpy((void*)&packet[6], &data[totalDataSent], dataToSend);

            // Set our packet length
            packetLength = sizeof(GipHeader_t) + 2 + dataToSend;
            //printf("[XGIP_PROTOCOL] Packet Length: %u\r\n", packetLength);

            // If total chunk < 0x100, split
            if ( numberOfChunksSent == 0 ) {
                if ( totalChunkLength < 0x100 ) {
                    packet[4] = 0x00;
                    packet[5] = (uint8_t) totalChunkLength;
                } else {
                    packet[4] = totalChunkLength & 0x00FF;
                    packet[5] = (totalChunkLength & 0xFF00) >> 8;
                }
            } else {
                if ( totalChunkSent < 0x100 ) {
                    packet[4] = 0x00;
                    packet[5] = (uint8_t) totalChunkSent;
                } else {
                    packet[4] = totalChunkSent & 0x00FF;
                    packet[5] = (totalChunkSent & 0xFF00) >> 8;
                }
            }

            // If we're sending over 0x80, add 0x100
            if ( totalChunkSent < 0x100 && (totalChunkSent + dataToSend) > 0x80) {
                totalChunkSent = totalChunkSent + dataToSend + 0x100;
            } else if ( ((totalChunkSent + dataToSend)/0x100) > (totalChunkSent/0x100)) {
                // if our next chunk sent will roll over the 3rd digit, | 0x80
                totalChunkSent = totalChunkSent + (dataToSend | 0x80);
            } else {
                // just add data sent
                totalChunkSent = totalChunkSent + dataToSend;
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