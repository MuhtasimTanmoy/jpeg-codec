#include "markers.h"
#ifndef JPG_H
#define JPG_H

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

struct HuffmanTable 
{
    bytebits symbols[162] = { 0 };
    bytebits offset[17] = { 0 };
    bool set = false; 
};

struct Header
{
    QuantizationTable quantizationTable[4];
    HuffmanTable huffmanDCTables[4];
    HuffmanTable huffmanACTables[4];

    uint height = 0;
    uint width = 0;
    uint numOfComponents = 0;
    bytebits frameType = 0;
    ColorComponent colorComponents[3];
    bool valid = true;

    bool zeroBased = false;

    uint restartInterval = 0;
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
