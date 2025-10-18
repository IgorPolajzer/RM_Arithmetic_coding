#include <complex>
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>

using namespace std;

struct TableEntry {
    char character;
    int frequency;
    int lowerBound;
    int upperBound;
};

int contains(vector<TableEntry> table, char character) {
    for (int i = 0; i < table.size(); i++) {
        if (table[i].character == character) return i;
    }

    return -1;
}

vector<TableEntry> initalizeTable(ifstream &file) {
    char c;
    vector<TableEntry> table;
    int lowerBound = 0;

    while (file.get(c))
    {
        int index = contains(table, c);
        if(index == -1) {
            int frequency = 1;
            int upperBound = lowerBound + frequency;

            table.push_back(TableEntry(c, frequency, lowerBound, upperBound));
            lowerBound = upperBound;

        } else {
            if (table.size() >= index) {
                table[index].frequency++;
                table[index].upperBound++;
                lowerBound = table[index].upperBound;
            } else {
                cerr << "Table size is too small" << endl;
            }
        }
    }

    return table;
}

void compress(const string& fileName) {
    int bitSize;
    vector<bool> binary;
    ifstream f(fileName, ios::binary | ios::in);

    cout << "Specify the coder bitsize: ";
    cin >> bitSize;

    int lowerBound = 0;
    int upperBound = pow(2, bitSize - 1) - 1;
    int secondQuarter = (upperBound + 1) / 2;
    int firstQuarter = secondQuarter / 2;
    int thirdQuarter = firstQuarter * 3;
    int cummulativeFrequency = 5; // TODO Number od characters in file

    if (f.is_open()) {
        char c;

        vector<TableEntry> table = initalizeTable(f);

        while (f.get(c)) // reads one char (1 byte) intro c.
        {
            // Extracts each bit from the byte.
            for (int i = 7; i >= 0; i--)
                // Shifts the byte to the right and masks with 1 to get the bit value.
                    cout << ((c >> i) & 1);
            cout << " ";
        }
    } else {
        cerr << "Error opening file " << fileName << endl;
    }
}

void writeBinaryFile(const string& fileName, const vector<bool>& bits) {
    ofstream f(fileName, ios::binary);
    if (!f) {
        cerr << "File '" << fileName << "' couldn't be opened." << endl;
        return;
    }

    unsigned char byte = 0;
    int bitCount = 0;
    for (bool bit : bits) {
        // Shift the existing bits left and add the new one
        byte = (byte << 1) | (bit ? 1 : 0);
        bitCount++;

        if (bitCount == 8) {
            f.put(static_cast<char>(byte));
            byte = 0;
            bitCount = 0;
        }
    }

    // Handle leftover bits (if not multiple of 8)
    if (bitCount > 0) {
        byte <<= (8 - bitCount); // pad with zeros on the right
        f.put(static_cast<char>(byte));
    }
}

vector<int> findBitset(vector<bool>& bits, string& bitSet) {
    vector<int> indexes;
    int firstBit = bitSet[0] - '0';

    for (int i = 0; i < bits.size(); i++) {
        if (bits[i] == firstBit && bits.size() - i >= bitSet.length()) {
            bool match = true;

            for (int j = 0; j < bitSet.length(); j++) {
                if (bits[i + j] != bitSet[j] - '0') {
                    match = false;
                    break;
                }
            }

            if (match) {
                indexes.push_back(i);
                i += bitSet.length() - 1;
            }
        }
    }

    return indexes;
}

void swapBitset(vector<bool>& bits, string& bitSet, string& bitSetReplacement) {
    if (bitSet.length() != bitSetReplacement.length()) {
        cerr << "Bitset length mismatch." << endl;
        return;
    }
    if (bits.size() < bitSet.length()) {
        cerr << "Bitset too large." << endl;
        return;
    }
    vector<int> indexes = findBitset(bits, bitSet);
    for (int index : indexes) {
        for (int i = index, j = 0; i < bitSetReplacement.length() + index; i++, j++) {
            bits[i] = bitSetReplacement[j] - '0';
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <operation> <inputFile> <outputFile>" << endl;
        return 1;
    }

    string operation = argv[1];
    string inputFile = argv[2];
    string outputFile = argv[3];

    // Bit search operation.
    if (operation == "c") {
        compress(inputFile);
    // Bit switch operation.
    } else if (operation == "d") {

        //writeBinaryFile(outputFile, binaryfile);
    }

    return 0;
}