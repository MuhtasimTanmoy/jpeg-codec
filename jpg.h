#ifndef JPG_H
#define JPG_H

typedef unsigned char bytebits;
typedef unsigned int uint;

/*
JPEG Marker codes
Start of Frame markers, non-differential, Huffman coding
*/

// Nondifferential Huffman-coding frame

const bytebits SOF0 = (bytebits)0xc0; // baseline dct
const bytebits SOF1 = (bytebits)0xc1; // extended dct
const bytebits SOF2 = (bytebits)0xc2; // progressive dct
const bytebits SOF3 = (bytebits)0xc3; //  Lossless (Sequential)

// Differential Huffman-coding frame

const bytebits SOF5 = (bytebits)0xc5; // Sequential DCT
const bytebits SOF6 = (bytebits)0xc6; // Progressive DCT
const bytebits SOF7 = (bytebits)0xc7; // lossless

// Nondifferential Arithmetic-coding frame

const bytebits SOF9 = (bytebits)0xc9;  // extended dct
const bytebits SOF10 = (bytebits)0xca; // progressive dct
const bytebits SOF11 = (bytebits)0xcb; // lossless

// Differential Arithmetic-coding frame

const bytebits SOF13 = (bytebits)0xcd; // sequential dct
const bytebits SOF14 = (bytebits)0xce; // progressive dct
const bytebits SOF15 = (bytebits)0xcf; // lossless

/// Huffman Table
const bytebits DHT = (bytebits)0xc4;

/// Quantization Table
const bytebits DQT = (bytebits)0xdb;

/// Start of Scan
const bytebits SOS = (bytebits)0xda;

/// Define Restart Interval
const bytebits DRI = (bytebits)0xdd;

/// Comment
const bytebits COM = (bytebits)0xfe;

/// Start of Image
const bytebits SOI = (bytebits)0xd8;

/// End of Image
const bytebits EOI = (bytebits)0xd9;

/// Define Number of Lines
const bytebits DNL = (bytebits)0xdc;

// JFIF identifiers
const bytebits JFIF_J = (bytebits)0x4a;
const bytebits JFIF_F = (bytebits)0x46;
const bytebits JFIF_I = (bytebits)0x49;
const bytebits JFIF_X = (bytebits)0x46;

// Application Reserved Keywords
const bytebits APP0 = (bytebits)0xe0;
const bytebits APP1 = (bytebits)0xe1;
const bytebits APP2 = (bytebits)0xe2;
const bytebits APP3 = (bytebits)0xe3;
const bytebits APP4 = (bytebits)0xe4;
const bytebits APP5 = (bytebits)0xe5;
const bytebits APP6 = (bytebits)0xe6;
const bytebits APP7 = (bytebits)0xe7;
const bytebits APP8 = (bytebits)0xe8;
const bytebits APP9 = (bytebits)0xe9;
const bytebits APP10 = (bytebits)0xea;
const bytebits APP11 = (bytebits)0xeb;
const bytebits APP12 = (bytebits)0xec;
const bytebits APP13 = (bytebits)0xed;
const bytebits APP14 = (bytebits)0xee;
const bytebits APP15 = (bytebits)0xef;

const bytebits RST0 = (bytebits)0xd0;
const bytebits RST1 = (bytebits)0xd1;
const bytebits RST2 = (bytebits)0xd2;
const bytebits RST3 = (bytebits)0xd3;
const bytebits RST4 = (bytebits)0xd4;
const bytebits RST5 = (bytebits)0xd5;
const bytebits RST6 = (bytebits)0xd6;
const bytebits RST7 = (bytebits)0xd7;

struct QuantizationTable
{
    uint table[64] = {0};
    bool set = false;
};

struct ColorComponent
{
    bytebits horizaontalSamplingFactor = 1;
    bytebits verticalSamplingFactor = 1;
    bytebits quantizationTableID = 0;
    bool used = false;
};

struct Header
{
    QuantizationTable quantizationTable[4];

    uint height = 0;
    uint width = 0;
    uint numOfComponents = 0;
    bytebits frameType = 0;
    ColorComponent colorComponents[3];
    bool valid = true;
};

const bytebits zigzagMap[] = {
    0, 1, 5, 6, 14, 15, 27, 28,
    2, 4, 7, 13, 16, 26, 29, 42,
    3, 8, 12, 17, 25, 30, 41, 43,
    9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

#endif
