#include "jpg.h"
#include <bits/stdc++.h>
using namespace std;

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

    bytebits last = (bytebits)inFile.get();
    bytebits current = (bytebits)inFile.get();

    if (last != 0xFF && current != SOI) {
        header->valid = false;
        inFile.close();
        return header;
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

        if (header->valid == false) {
            cout<<"Header not valid"<<endl;
            continue;
        }

        // Process huffman coded bitstream
        delete header;
    }
    
    return 0;
}