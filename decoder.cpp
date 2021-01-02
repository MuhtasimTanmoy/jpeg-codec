// Author: Muhtasim Ulfat Tanmoy
#include "jpg.h"
#include "markers.h"
#include <bits/stdc++.h>
using namespace std;

void readAPPN(ifstream& fs, Header* const header) {
    cout<<"Reading APPN"<<endl;
    uint length = (fs.get() << 8) + fs.get();
    for (uint i = 0; i < length-2; i++) {
        fs.get();
    }
}

void readComment(ifstream& fs, Header* const header) {
    cout<<"Reading Comment"<<endl;
    uint length = (fs.get() << 8) + fs.get();
    for (uint i = 0; i < length-2; i++) {
        fs.get();
    }
}

void readDQT(ifstream& fs, Header* const header) {
    cout<<"Quantization table"<<endl;
    int length = (fs.get() << 8) + fs.get();
    length-=2;
    while(length > 0) {
        uint bit_type = fs.get();
        length--;
        uint tableID = bit_type & 0x0F;
        if(tableID > 3) {
            cout<<"Invalid table ID "<<tableID<<endl;
            header->valid = false;
            return;
        }
        header->quantizationTable[tableID].set = true;
        uint is64 = bit_type >> 4;
        if (is64 > 0) {
            for (uint i = 0; i < 64; i++) {
                header->quantizationTable[tableID].table[zigzagMap[i]] = (fs.get() << 8) + fs.get();
            }
            length-=128;
        } else {
             for (uint i = 0; i < 64; i++) {
                header->quantizationTable[tableID].table[zigzagMap[i]] =  fs.get();
            }
            length-=64;
        }
    }

    if(length != 0) {
        cout<<"Marker invalid"<<endl;
        header->valid = false;
    }
}

void readStartOfScan(ifstream& f, Header* const header) {
    cout<<"Reading SOS markers\n";
    if(header->numOfComponents == 0) {
        cout<<"Error - SOS encountered before SOF\n";
        header->valid = false;
        return;
    }
    int length = (f.get() << 8) + f.get();
    for (int i = 0; i < header->numOfComponents; i++){
        header->colorComponents[i].used = false;
    }

    bytebits numberOfComponents = f.get();
    for (int i = 0; i < numberOfComponents; i++){
        bytebits componentID = f.get();
        if (header->zeroBased) {
            componentID += 1;
        }
        if (componentID > numberOfComponents) {
            cout<<"Error : Invalid component ID: "<<componentID<<endl;
            header->valid = false;
            return;
        }

        ColorComponent* colorComponent = &header->colorComponents[componentID-1];
        if (colorComponent->used) {
            cout<<"Duplicate color component\n";
            header->valid = false;
            return;
        }
        colorComponent->used = true;

        bytebits huffmanTableIDs = f.get();
        colorComponent->huffmanACTableID  = huffmanTableIDs & 0x0F;
        colorComponent->huffmanDCTableID = huffmanTableIDs >> 4;

        if (colorComponent->huffmanACTableID > 3) {
            cout<<"Huffman AC Table id invalid\n";
            header->valid = false;
            return;
        }

        if (colorComponent->huffmanDCTableID > 3) {
            cout<<"Huffman DC Table id invalid\n";
            header->valid = false;
            return;
        }

    }

    header->startOfSelection = f.get();
    header->endOfSelection = f.get();
    bytebits successiveAprrox = f.get();
    header->succesiveAppoximationHigh= successiveAprrox >> 4;
    header->succesiveAppoximationHigh= successiveAprrox & 0x0F;

    // Baseline JPEG does not use spectral selection or successive approximation
    if(header->startOfSelection != 0 && header->endOfSelection!=63) {
        cout<<"Invalid spectral selection\n";
        header->valid = false;
        return;
    }

    if(header->succesiveAppoximationHigh != 0 && header->succesiveAppoximationLow!=0) {
        cout<<"Invalid succesive appoximation\n";
        header->valid = false;
        return;
    }
    
    if(length - 2 - 1 - numberOfComponents*2 - 3) {
        cout<<"Erro SOS length\n";
        header->valid = false;
        return;
    }
}


// Used to syncronize with stream
void readRestartInterval(ifstream& f, Header* const header) {
    cout<<"Reading Restart Interavl markers\n";
    int length = (f.get()<<8) + f.get();
    header->restartInterval = (f.get()<<8) + f.get();
    if(length - 4 != 0) {
        cout<<"Invalid marker"<<endl;
        header->valid =false;
    }
}

void readHuffmanTable(ifstream& f, Header* const header) {
    cout<<"Reading Huffman table\n";
    int length = (f.get() << 8) + f.get();
    length-=2;

    while (length > 0)
    {
        bytebits tableInfo = f.get();
        bool acTable = tableInfo >> 4;
        uint tableID = tableInfo & 0x0F;

        if(tableID > 3) {
            cout<<"Invalid huffman table\n";
            header->valid = false;
            return;
        }

        HuffmanTable* hTable; 
        if(acTable) {
            cout<<"AC Table\n";
            hTable = &header->huffmanACTables[tableID];
        }
        else {
            cout<<"DC Table\n";
            hTable = &header->huffmanDCTables[tableID];
        }

        if(hTable->set) {
            cout<<"Duplicate huffman table encounter\n";
            header->valid = false;
            return;
        } 
        hTable->set = true; 

        hTable->offset[0] = 0;
        uint allSymbols = 0;
        for (int i = 1; i <= 16; i++) {
            allSymbols += f.get();
            hTable->offset[i] = allSymbols;
        }

        if(allSymbols > 162) {
            cout<<"Symbols count error\n";
            header->valid = false;
            return;
        }

        for (int i = 0; i < allSymbols; i++) {
            hTable->symbols[i] = f.get(); 
        }
        length = length - 1 - 16 - allSymbols;
    }

    if (length != 0) {
        cout<<"Huffman table length error\n";
        header->valid = false;
        return;
    }
}

void readFrame(ifstream& f, Header* const header) {
    cout<<"Reading SOF markers\n";
    if(header->numOfComponents != 0) {
        cout<<"Error multiple SOF  header detected";
        header->valid = false;
        return;
    }
    int length = (f.get() << 8) + f.get();
    int precision = f.get();
    if(precision != 8) {
        cout<<"Error in precision"<<endl;
        header->valid = false;
        return;
    }

    header->height = (f.get() << 8) + f.get();
    header->width = (f.get() << 8) + f.get();
    if(header->height == 0 || header->width == 0) {
        cout<<"Invalid JPEG\n";
        header->valid = false;
        return;
    }

    header->numOfComponents = f.get();
    if(header->numOfComponents == 0) {
        cout<<"Invalid number of components\n";
        header->valid = false;
        return;
    }
    else if(header->numOfComponents == 4) {
        cout<<"CMYK color component\n";
        header->valid = false;
        return;
    }

    for (uint i = 0; i < header->numOfComponents; i++) {
        bytebits componentID = f.get();
        if(componentID == 0) {
            header->zeroBased = true;
        }

        if(header->zeroBased) {
            componentID++;
        }

        if (componentID == 4 || componentID == 5) {
            cout<<"YIQ color mode not supported\n";
            header->valid = false;
            return;
        }
        if (componentID == 0 || componentID > 3) {
            cout<<"Invalid color componentID\n";
            header->valid = false;
            return;
        }
        ColorComponent* colorComponent = &header->colorComponents[componentID-1];
        if (colorComponent->used) {
            cout<<"Duplicate color component\n";
            header->valid = false;
            return; 
        }
        colorComponent->used = true;
        bytebits samplingFactor = f.get();
        colorComponent->horizaontalSamplingFactor = samplingFactor >> 4;
        colorComponent->verticalSamplingFactor = samplingFactor & 0x0F;

        if (colorComponent->horizaontalSamplingFactor !=1 || colorComponent->verticalSamplingFactor != 1) {
            cout<<"Scaling factor error\n";
            header->valid = false;
            return;
        }

        colorComponent->quantizationTableID = f.get();
        if(colorComponent->quantizationTableID > 3) {
            cout<<"Invalid table ID";
            header->valid = false;
            return;
        }

        if (length - 2 - 1 - 4 - 1 - header->numOfComponents * 3 != 0) {
            cout<<"Invalid length\n";
            header->valid = false;
            return;
        }
    }
}

void printHeader(const Header* const header) {
    if (header == nullptr) return;
    cout<<"\n";
    cout<<"DQT============="<<endl;
    for (int i = 0; i < 4; i++) {
        if (header->quantizationTable[i].set) {
            cout<<"\n";
            cout<<"Table ID "<< i <<endl;
            cout<<"Table Data"<<endl;

            for (int j = 0; j < 64; j++) {
                if (j % 8 == 0) cout<<endl;
                cout<< header->quantizationTable[i].table[j] <<' ';
            }
            cout<<endl;
        }
    }

    cout<<"\n";
    cout<<"Frame Type "<<::hex<<(int)header->frameType<<::dec<<endl;
    cout<<"Height "<<header->height<<endl;
    cout<<"Width "<<header->width<<endl;
    cout<<"Number of Components "<<header->numOfComponents<<endl;

    cout<<"Components "<<endl;
    for (int i = 0; i < header->numOfComponents; i++) {
        cout<<endl;
        cout<<"Component ID "<<i<<" "<<"\n";
        cout<<"Horizaontal sampling factor: "<< (int)header->colorComponents[i].horizaontalSamplingFactor <<"\n";
        cout<<"Vertical sampling factor: "<< (int)header->colorComponents[i].verticalSamplingFactor <<"\n";
        cout<<"Quantization Table ID: "<< (int)header->colorComponents[i].quantizationTableID <<"\n";
        cout<<endl;
    }
    

    cout<<"DHT===============\n\n";

    cout<<"DC Tables\n";
    for (int i = 0; i < 4; i++){
        HuffmanTable dc = header->huffmanDCTables[i];
        if (dc.set) {
            cout<<"\nTable ID "<<i<<endl;
            for (int j = 0; j < 16; j++) {
                cout<<"Code length "<<j<<": ";
                for (int symbolIndex = dc.offset[j]; 
                    symbolIndex < dc.offset[j+1]; 
                    symbolIndex++) {
                    cout<<::hex<<(uint)dc.symbols[symbolIndex]<<::dec<<" ";
                }
                cout<<"\n";
            }
        }
    }


    cout<<"\nAC Tables\n";
    for (int i = 0; i < 4; i++){
        HuffmanTable ac = header->huffmanACTables[i];
        if (ac.set) {
            cout<<"\nTable ID "<<i<<endl;
            for (int j = 0; j < 16; j++){
                cout<<"Code length "<<j<<": ";
                for (int symbolIndex = ac.offset[j]; 
                    symbolIndex < ac.offset[j+1]; 
                    symbolIndex++) {
                    cout<<::hex<<(uint)ac.symbols[symbolIndex]<<::dec<<" ";
                }
                cout<<"\n";
            }
        }
    }


    cout << "SOS=============\n";
    cout << "Start of Selection: " << (uint)header->startOfSelection << '\n';
    cout << "End of Selection: " << (uint)header->endOfSelection << '\n';
    cout << "Successive Approximation High: " << (uint)header->succesiveAppoximationHigh << '\n';
    cout << "Successive Approximation Low: " << (uint)header->succesiveAppoximationLow << '\n';
    cout << "Color Components:\n";
    for (uint i = 0; i < header->numOfComponents; ++i) {
        cout << "Component ID: " << (i + 1) << '\n';
        cout << "Huffman DC Table ID: " << (uint)header->colorComponents[i].huffmanDCTableID << '\n';
        cout << "Huffman AC Table ID: " << (uint)header->colorComponents[i].huffmanACTableID << '\n';
    }
    cout << "Length of Huffman Data: " << header->huffmanData.size() << '\n';
    cout << "DRI=============\n";
    cout << "Restart Interval: " << header->restartInterval << '\n';
}

Header* readJPG(const string& filename ) {
    // Read file
    ifstream inFile = ifstream(filename, ios::in | ios::binary);
    if (!inFile.is_open()) {
        cout<<"Error opening file\n"<<endl;
        return nullptr;
    }

    Header* header = new (nothrow) Header;
    if (header == nullptr) {
        cout<<"Memory error\n";
        inFile.close();
        return nullptr;
    }
    bytebits left = (bytebits)inFile.get();
    bytebits right = (bytebits)inFile.get();

    if (left != 0xFF && right != SOI) {
        header->valid = false;
        inFile.close();
        return header;
    }
    left = (bytebits)inFile.get();
    right = (bytebits)inFile.get();

    while(header->valid) {
        if(!inFile) {
            cout<<"Error in file"<<endl;
            header->valid = false;
            inFile.close();
            return header;
        }

        if(left != 0xFF) {
            cout<<"Not a marker"<<endl;
            header->valid = false;
            inFile.close();
            return header;
        }

        if (right == DQT) {
            readDQT(inFile, header);
        }
        else if (right == SOF0) {
            cout<<"Baseline DCT\n";
            header->frameType = SOF0;
            readFrame(inFile, header);
        }
        else if (right == SOF2) { 
            cout<<"Progressive DCT\n";
            header->frameType = SOF2;
            readFrame(inFile, header);
        }
        else if(right == COM) {
            readComment(inFile, header);
        }
        else if(right == SOS) {
            readStartOfScan(inFile, header);
            break;
        }
        else if(right == DHT) {
            readHuffmanTable(inFile, header);
        }
        else if (right >= APP0 && right <= APP15) {
            readAPPN(inFile, header);
        }
        else if(right == DRI) {
            readRestartInterval(inFile, header);
        }
        else if((right >= JPEG0 && right<=JPEG13) ||
                right == DNL || 
                right == DHP ||
                right == EXP) {
            readComment(inFile, header);
        }
        else if (right == TEM) {
            // TEM has no size
        }
        else if (right == 0xFF) {
            right = inFile.get();
            continue;
        }
        else if (right == SOI) {
            cout<<"Error: Start of image marker should not be encountered\n";
            header->valid = false;
            inFile.close();
            return header;
        }
        else if (right == EOI) {
            cout<<"Error: EOI should be only encountered in SOS\n";
            header->valid = false;
            inFile.close();
            return header;
        }
        else if (right == DAC) {
            cout<<"Error: Arithmatic coding not supported\n";
            header->valid = false;
            inFile.close();
            return header;
        }
        else if (right >= SOF0 || right <=SOF15) {
            cout<<"Error: Wrong SOF\n";
            header->valid = false;
            inFile.close();
            return header;
        }
        else if (right >= RST0 || right <= RST5) {
            cout<<"Error: Restart markers should not be seen outside SOS \n";
            header->valid = false;
            inFile.close();
            return header;
        }
        else {
            cout<<"Unknown marker\n";
            header->valid = false;
            inFile.close();
            return header;
        }

        left = (bytebits)inFile.get();
        right = (bytebits)inFile.get();
    }

    if (header->valid) {
        right = inFile.get();
        // read compressed image data
        while (true) {
            if (!inFile) {
                std::cout << "Error - File ended prematurely\n";
                header->valid = false;
                inFile.close();
                return header;
            }

            left = right;
            right = inFile.get();
            // if marker is found
            if (left == 0xFF) {
                // end of image
                if (right == EOI) {
                    break;
                }
                // 0xFF00 means put a literal 0xFF in image data and ignore 0x00
                else if (right == 0x00) {
                    header->huffmanData.push_back(right);
                    // overwrite 0x00 with next byte
                    right = inFile.get();
                }
                // restart marker
                else if (right >= RST0 && right <= RST7) {
                    // overwrite marker with next byte
                    right = inFile.get();
                }
                // ignore multiple 0xFF's in a row
                else if (right == 0xFF) {
                    // do nothing
                    continue;
                }
                else {
                    cout << "Error - Invalid marker during compressed data scan: 0x" << ::hex << (uint)right << ::dec << '\n';
                    header->valid = false;
                    inFile.close();
                    return header;
                }
            }
            else {
                header->huffmanData.push_back(right);
            }
        }
    }

      // validate header info
    if (header->numOfComponents != 1 && header->numOfComponents != 3) {
        std::cout << "Error - " << (uint)header->numOfComponents << " color components given (1 or 3 required)\n";
        header->valid = false;
        inFile.close();
        return header;
    }

    for (uint i = 0; i < header->numOfComponents; ++i) {
        if (header->quantizationTable[header->colorComponents[i].quantizationTableID].set == false) {
            std::cout << "Error - Color component using uninitialized quantization table\n";
            header->valid = false;
            inFile.close();
            return header;
        }
        if (header->huffmanDCTables[header->colorComponents[i].huffmanDCTableID].set == false) {
            std::cout << "Error - Color component using uninitialized Huffman DC table\n";
            header->valid = false;
            inFile.close();
            return header;
        }
        if (header->huffmanACTables[header->colorComponents[i].huffmanACTableID].set == false) {
            std::cout << "Error - Color component using uninitialized Huffman AC table\n";
            header->valid = false;
            inFile.close();
            return header;
        }
    }

    inFile.close();
    
    return header;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "Error - Invalid Arguments" << endl;
        return 0;
    }
    for (int i = 1; i < argc; i++) {
        const string fileName(argv[i]);
        Header* header = readJPG(fileName);
        if (header == nullptr) {
            continue;
        }
        printHeader(header);
        if (header->valid == false) {
            cout<<"Header not valid"<<endl;
            continue;
        }
        // Process huffman coded bitstream
        delete header;
    }
    return 0;
}