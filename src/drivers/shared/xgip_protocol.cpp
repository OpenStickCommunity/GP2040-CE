/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
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

#include "drivers/xbone/XBOneDescriptors.h"
#include "drivers/shared/xgip_protocol.h"

// GIP lengths use unsigned LEB128, including zero-extension padding
static bool read_leb128(const uint8_t * buffer, uint16_t len, uint16_t & offset, uint32_t & value) {
    value = 0;
    for (uint8_t shift = 0; shift < 28 && offset < len; shift += 7) {
        uint8_t byte = buffer[offset++];
        value |= (uint32_t)(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) return true;
    }
    return false;
}

// Default Constructor
XGIPProtocol::XGIPProtocol() {
    reset();
}

// Default Destructor
XGIPProtocol::~XGIPProtocol() {
}

// Reset packet information
void XGIPProtocol::reset() {
    memset((void*)&header, 0, sizeof(GipHeader_t));
    actualDataReceived = 0;     // How much actual data have we received?
    totalDataSent = 0;          // How much actual data have we sent?
    numberOfChunksSent = 0;     // How many actual chunks have we sent?
    chunkEnded = false;         // Are we at the end of the chunk?
    isValidPacket = false;      // Is this a valid packet?
    memset(data, 0, 1024);
    dataLength = 0;             // Set data length to 0
    memset(packet, 0, sizeof(packet)); // Set our packet to 0
    packetLength = 0;           // Set packet length to 0
}

// Parse incoming packet
bool XGIPProtocol::parse(const uint8_t * buffer, uint16_t len) {
    isValidPacket = false;
    if (buffer == nullptr || len < sizeof(GipHeader_t) || len > sizeof(packet)) {
        reset();
        return false;
    }

    GipHeader_t newPacket;
    memcpy(&newPacket, buffer, sizeof(GipHeader_t));
    uint16_t payloadOffset = 3;
    uint32_t payloadLength, chunkOffset = 0;
    if (!read_leb128(buffer, len, payloadOffset, payloadLength) ||
        (newPacket.chunked && !read_leb128(buffer, len, payloadOffset, chunkOffset)) ||
        payloadLength > (uint32_t)(len - payloadOffset)) {
        return false;
    }

    if (newPacket.command == GIP_ACK_RESPONSE) {
        if (newPacket.internal != 1 || newPacket.chunked || payloadLength != 9 ||
            payloadOffset + payloadLength != len) {
            return false;
        }
        // ACKs must not discard an incoming fragmented message
        header = newPacket;
        packetLength = len;
        isValidPacket = true;
        return true;
    }

    if (newPacket.chunked) {
        if (newPacket.chunkStart) {
            if (payloadLength == 0 || chunkOffset > sizeof(data) || payloadLength > chunkOffset) {
                return false;
            }
            reset();
            dataLength = chunkOffset;
            chunkOffset = 0;
        } else {
            if (chunkOffset > actualDataReceived ||
                chunkOffset > dataLength || payloadLength > dataLength - chunkOffset) {
                return false;
            }
            if (payloadLength == 0) {
                // Some auth dongles pad a short terminal header with a zero byte
                if (payloadOffset == 5 && len == 6 && buffer[5] == 0) payloadOffset++;
                if (payloadOffset != len || chunkOffset != dataLength || actualDataReceived != dataLength) {
                    return false;
                }
                header = newPacket;
                packetLength = len;
                chunkEnded = true;
                isValidPacket = true;
                return true;
            }
            if (numberOfChunksSent == 0) return false;
        }
        // TLO is a byte offset; retransmitted fragments replace those bytes
        memcpy(&data[chunkOffset], &buffer[payloadOffset], payloadLength);
        actualDataReceived = chunkOffset + payloadLength;
        numberOfChunksSent++;
        chunkEnded = false;
    } else {
        reset();
        memcpy(data, &buffer[payloadOffset], payloadLength);
        actualDataReceived = payloadLength;
        dataLength = payloadLength;
    }
    header = newPacket;
    packetLength = len;
    isValidPacket = true;
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
    header.reserved = 0;
    header.command = cmd;
    header.sequence = seq;
    header.internal = internal;
    header.chunked = isChunked;
    header.needsAck = needsAck;
}

bool XGIPProtocol::setData(const uint8_t * buffer, uint16_t len) {
    if ((len && buffer == nullptr) || len > sizeof(data) ||
        (!header.chunked && len > sizeof(packet) - sizeof(GipHeader_t))) {
        return false;
    }
    if (len) memcpy(data, buffer, len);
    dataLength = len;
    return true;
}

// Generate XGIP Packet for output
uint8_t * XGIPProtocol::generatePacket() {
    if (header.chunked && dataLength < GIP_MAX_CHUNK_SIZE) {
        header.chunkStart = 0;
        header.chunked = 0;
        header.needsAck = 1;
    }
    if (header.chunked == 0) {
        header.length = dataLength;
        memcpy(packet, &header, sizeof(GipHeader_t));
        memcpy(&packet[4], data, dataLength);
        packetLength = sizeof(GipHeader_t) + dataLength;
    } else {
        uint16_t dataToSend = dataLength - totalDataSent;
        header.chunkStart = (numberOfChunksSent == 0);
        // ACK the first, every fifth, and final data fragment
        header.needsAck = (numberOfChunksSent == 0 || (numberOfChunksSent + 1) % 5 == 0 ||
            dataToSend <= GIP_MAX_CHUNK_SIZE);
        if (dataToSend > GIP_MAX_CHUNK_SIZE) dataToSend = GIP_MAX_CHUNK_SIZE;
        if (dataToSend == 0) {
            header.needsAck = 0;
            header.chunkStart = 0;
            chunkEnded = true;
        }

        uint16_t chunkOffset = header.chunkStart ? dataLength : totalDataSent;
        header.length = dataToSend;
        // Keep a six-byte header: pad Length below 128, otherwise encode TLO
        if (dataToSend != 0 && chunkOffset < 0x80) {
            header.length |= 0x80;
            memcpy(packet, &header, sizeof(GipHeader_t));
            packet[4] = 0;
            packet[5] = chunkOffset;
        } else {
            memcpy(packet, &header, sizeof(GipHeader_t));
            packet[4] = (chunkOffset & 0x7F) | 0x80;
            packet[5] = chunkOffset >> 7;
        }
        memcpy(&packet[6], &data[totalDataSent], dataToSend);
        packetLength = sizeof(GipHeader_t) + 2 + dataToSend;
        totalDataSent += dataToSend;
        if (dataToSend != 0) numberOfChunksSent++;
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

    // ACK progress counts payload bytes only, excluding all GIP headers.
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

// Get last generated output packet length
uint8_t XGIPProtocol::getPacketLength() {
    return packetLength;
}

// Get the header information if the packet needs an ACK
uint8_t XGIPProtocol::getPacketAck() {
    return header.needsAck;
}

// Get command of a parsed packet
uint8_t XGIPProtocol::getCommand() {
    return header.command;
}

// Is this packet chunked?
uint8_t XGIPProtocol::getChunked(){
    return header.chunked;
}

// Get seqeuence in the header
uint8_t XGIPProtocol::getSequence() {
    return header.sequence;
}

// Get data from a packet or packet-chunk
uint8_t * XGIPProtocol::getData() {
    return data;
}

// Get length of a packet or packet-chunk
uint16_t XGIPProtocol::getDataLength() {
    return dataLength;
}

// Get chunk data from incoming packet
bool XGIPProtocol::getChunkData(XGIPProtocol & packet) {
    return false;
}

// Last packet parsed needs an ACK
bool XGIPProtocol::ackRequired() {
    return header.needsAck;
}
