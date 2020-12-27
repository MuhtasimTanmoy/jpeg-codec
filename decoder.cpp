#include "jpg.h"
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
        else if (right == DQT) {
            readDQT(inFile, header);
        }
        else if (right >= APP0 && right <= APP15) {
            readAPPN(inFile, header);
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