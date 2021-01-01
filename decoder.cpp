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

void readRestartInterval(ifstream& f, Header* const header) {
    cout<<"Reading Restart Interavl markers\n";
    int length = (f.get()<<8) + f.get();
    header->restartInterval = (f.get()<<8) + f.get();
    if(length - 4 != 0) {
        cout<<"Invalid marker"<<endl;
        header->valid =false;
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

        if (length -2 - 1 - 4 - 1 - header->numOfComponents * 3 != 0) {
            cout<<"Invalid length\n";
            header->valid = false;
            return;
        }
    }
}

void printHeader(const Header* const header) {
    if (header == nullptr) return;
    cout<<"DQT"<<endl;
    for (int i = 0; i < 4; i++) {
        if (header->quantizationTable[i].set) {
            cout<<"Table ID "<< i <<endl;
            cout<<"Table Data"<<endl;

            for (int j = 0; j < 64; j++) {
                if (j % 8 == 0) cout<<endl;
                cout<< header->quantizationTable[i].table[j] <<' ';
            }
            cout<<endl;
        }
    }

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
    while(header->valid){
        if(!inFile){
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
        else if (right >= APP0 && right <= APP15) {
            readAPPN(inFile, header);
        }
        else if(right == DRI) {
            readRestartInterval(inFile, header);
        }
        left = (bytebits)inFile.get();
        right = (bytebits)inFile.get();
    }
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