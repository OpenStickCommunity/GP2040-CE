/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 OpenStickCommunity (gp2040-ce.info)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef _XGIP_PROTOCOL_
#define _XGIP_PROTOCOL_

//
// XGIP Protocol
// XGIP10 or Xbox Game Input Protocol (1.0?)
//  Used in Xbox 360 and Xbox One controller
//  communication and reporting.
//  
//  Documentation comes from various sources
//  including Santroller (https://github.com/Santroller/Santroller),
//  GIMX (https://github.com/matlo/GIMX), and other
//  open-source Github projects.
//
//  This is a free, open-source, guess-work
//  based interpretation of the protocol for
//  the GP2040-CE controller platform and in
//  no way reflects any commercial software or
//  protocols. This implementation is provided
//  as-is and we are not responsible for
//  this interpretation corrupting data or sending
//  invalid packets.
//
//  !!!USE AT YOUR OWN RISK!!!
//

#include <stdint.h>

#include "gamepad/descriptors/XBOneDescriptors.h"

// All max chunks are this size
#define GIP_MAX_CHUNK_SIZE 0x3A

class XGIPProtocol {
public:
    XGIPProtocol();
    ~XGIPProtocol();
    void reset();                               // Reset packet information
    bool parse(const uint8_t * buffer, uint16_t len); // Parse incoming packet
    bool validate();                            // is valid packet?
    bool endOfChunk();                          // Is this the end of the chunk?
    void setAttributes(uint8_t cmd, uint8_t seq, uint8_t internal, uint8_t isChunked, uint8_t needsAck);   // Set attributes for next output packet
    void incrementSequence();                   // Add 1 to sequence
    bool setData(const uint8_t* data, uint16_t len); // Set data (buf and length)
    uint8_t * generatePacket();                 // Generate output packet (chunk will generate on-going packet)
    uint8_t * generateAckPacket();              // Generate an ack for the last received packet
    bool validateAck(XGIPProtocol & ackPacket); // Validate an incoming ack packet against 
    uint8_t getCommand();                       // Get command of a parsed packet
    uint8_t getSequence();                      // Get sequence of a parsed packet
    uint8_t getChunked();                       // Is this packet chunked?
    uint8_t getPacketAck();                     // Did the packet require an ACK?
    uint8_t getPacketLength();                  // Get packet length of our last output
    uint8_t * getData();                        // Get data from a packet or packet-chunk
    uint16_t getDataLength();                   // Get length of a packet or packet-chunk
    bool getChunkData(XGIPProtocol & packet);   // Get chunk data from incoming packet
    bool ackRequired();                         // Did our last parsed packet require an ack?
private:
    GipHeader_t header;             // On-going GIP header
    uint16_t totalChunkLength;      // How big is the chunk?
    uint16_t actualDataReceived;    // How much actual data have we received?
    uint16_t totalChunkReceived;    // How much have we received in chunk mode length? (length | 0x80)
    uint16_t totalChunkSent;        // How much have we sent?
    uint16_t totalDataSent;         // How much actual data have we sent?
    uint16_t numberOfChunksSent;    // How many actual chunks have we sent?
    bool chunkEnded;                // did we hit the end of the chunk successfully?
    uint8_t packet[64];             // for output packets
    uint16_t packetLength;          // LAST SENT packet length
    uint8_t * data;                 // Total data in this packet
    uint16_t dataLength;            // actual length of data
    bool isValidPacket;             // is this a valid packet or did we get an error?
};

#endif