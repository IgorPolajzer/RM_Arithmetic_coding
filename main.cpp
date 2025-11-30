#include <complex>
#include <iostream>
#include <fstream>
#include <cmath>
#include <tsl/ordered_map.h>

using namespace std;
using namespace tsl;

struct TableEntry {
    char character;
    mutable int frequency;
    mutable int lowerBound;
    mutable int upperBound;
};

ordered_map<char, TableEntry> initalizeTable(const string& fileName) {
    char c;
    ordered_map<char, TableEntry> table;
    ifstream file(fileName, ios::binary | ios::in);
    int lowerBound = 0;

    while (file.get(c))
    {
        if(!table.contains(c)) {
            int frequency = 1;
            int upperBound = lowerBound + frequency;

            table[c] = TableEntry(c, frequency, lowerBound, upperBound);
            lowerBound = upperBound;

        } else {
            table[c].frequency++;
            table[c].upperBound++;
            lowerBound = table[c].upperBound;
        }
    }

    return table;
}

int getComulativeFrequency(const ordered_map<char, TableEntry>& table) {
    int frequency = 0;

    for (const auto & [ key, value ] : table) {
        frequency += value.frequency;
    }

    return frequency;
}

void compress(const string& inputFile, const string& outputFile) {
    int bitSize;
    ifstream input(inputFile, ios::binary);
    ofstream output(outputFile, ios::binary);

    cout << "Specify the coder bitsize: ";
    cin >> bitSize;

    int lowerBound = 0;
    int upperBound = pow(2, bitSize - 1) - 1;
    int secondQuarter = (upperBound + 1) / 2;
    int firstQuarter = secondQuarter / 2;
    int thirdQuarter = firstQuarter * 3;

    ordered_map<char, TableEntry> table = initalizeTable(inputFile);
    int cummulativeFrequency = getComulativeFrequency(table);

    if (input.is_open() && output.is_open()) {
        // Write file header.
        output << bitSize << ",";
        for (auto & it : table) {output << it.first << ",";}
        for (auto & it : table) {output << it.second.frequency << ",";}

        char c;
        int E3_counter = 0;

        while (input.get(c)) {
            int step = (upperBound - lowerBound + 1) / cummulativeFrequency;
            upperBound = lowerBound + step * table[c].upperBound - 1;
            lowerBound = lowerBound + step * table[c].lowerBound;

            while (upperBound < secondQuarter || lowerBound >= secondQuarter) {
                if (upperBound < secondQuarter) {
                    lowerBound *= 2;
                    upperBound = upperBound * 2 + 1;
                    output << 0;
                    for (int i = 0; i < E3_counter; i++) {
                        output << 1;
                    }
                    E3_counter = 0;
                } else if (lowerBound >= secondQuarter) {
                    lowerBound = 2 * (lowerBound - secondQuarter);
                    upperBound = 2 * (upperBound - secondQuarter) + 1;
                    output << 1;
                    for (int i = 0; i < E3_counter; i++) {
                        output << 0;
                    }
                    E3_counter = 0;
                }
            }

            while (lowerBound >= firstQuarter && upperBound < thirdQuarter) {
                lowerBound = 2 * (lowerBound - firstQuarter);
                upperBound = 2 * (upperBound - firstQuarter) + 1;
                E3_counter++;
            }
        }

        if (lowerBound < firstQuarter) {
            output << 0 << 1;
            for (int i = 0; i < E3_counter; i++) {
                output << 1;
            }
        } else {
            output << 1 << 0;
            for (int i = 0; i < E3_counter; i++) {
                output << 0;
            }
        }
    } else {
        cerr << "Error opening one of the files: " << inputFile << ", " << outputFile << endl;
    }
}

ordered_map<char, TableEntry> parseHeader(ifstream& input, int &bitSize) {
    char c;
    string stringBitSize;
    ordered_map<char, TableEntry> table;

    // Parse bit size (digits until first non-digit)
    while (isdigit(input.peek())) {
        input.get(c);
        stringBitSize += c;
    }
    bitSize = stoi(stringBitSize);

    // Parse used characters (all non-digits)
    while (true) {
        if (!isalpha(input.peek()) && input.peek() != ',') break; // stop at first non-letter/non-comma
        input.get(c);
        if (c == ',') continue;
        table[c] = TableEntry{c, -1, -1, -1};
    }

    // Parse character frequencies.
    int previousUpper = -1;
    for (auto & it : table) {
        string freqStr;
        while (input.get(c) && c != ',') {
            freqStr += c;
        }

        int currentFreq = stoi(freqStr);
        int lowerBound = previousUpper == -1 ? 0 : previousUpper;
        int upperBound = lowerBound + currentFreq;

        it.second.frequency = currentFreq;
        it.second.lowerBound = lowerBound;
        it.second.upperBound = upperBound;

        previousUpper = upperBound;
    }

    return table;
}

char getCharFromTable(ordered_map<char, TableEntry>& table, int v) {
    for (auto & it : table) {
        if (it.second.lowerBound <= v  && it.second.upperBound > v) {
            return it.first;
        }
    }
}

int readNextBit(ifstream& input) {
    char bit_char;
    if (input.get(bit_char)) {
        if (bit_char == '0') return 0;
        if (bit_char == '1') return 1;
    }
    return 0;
}

void decompress(const string& inputFile, const string& outputFile) {
    ifstream input(inputFile, ios::binary);
    ofstream output(outputFile, ios::binary);

    if (input.is_open() && output.is_open()) {
        // Parse the file header.
        int bitSize;
        ordered_map<char, TableEntry> table = parseHeader(input, bitSize);
        int cummulativeFrequency = getComulativeFrequency(table);

        int field = 0;
        for (int i = 0; i < bitSize - 1; i++ ) field = ( field << 1 ) | readNextBit(input);

        int lowerBound = 0;
        int upperBound = pow(2, bitSize - 1) - 1;
        int secondQuarter = (upperBound + 1) / 2;
        int firstQuarter = secondQuarter / 2;
        int thirdQuarter = firstQuarter * 3;

        for (int i = 0; i < cummulativeFrequency; i++) {
            int step = (upperBound - lowerBound + 1) / cummulativeFrequency;
            int v = (field - lowerBound) / step;
            char currentChar = getCharFromTable(table, v);
            output << currentChar;

            upperBound = lowerBound + step * table[currentChar].upperBound - 1;
            lowerBound = lowerBound + step * table[currentChar].lowerBound;

            while (upperBound < secondQuarter || lowerBound >= secondQuarter) {
                if (upperBound < secondQuarter) {
                    lowerBound *= 2;
                    upperBound = upperBound * 2 + 1;

                    field = 2 * field + readNextBit(input);
                } else if (lowerBound >= secondQuarter) {
                    lowerBound = 2 * (lowerBound - secondQuarter);
                    upperBound = 2 * (upperBound - secondQuarter) + 1;

                    field = 2 * (field - secondQuarter) + readNextBit(input);
                }
            }

            while (lowerBound >= firstQuarter && upperBound < thirdQuarter) {
                lowerBound = 2 * (lowerBound - firstQuarter);
                upperBound = 2 * (upperBound - firstQuarter) + 1;

                field = 2 * (field - firstQuarter) + readNextBit(input);
            }
        }
    } else {
        cerr << "Error opening one of the files: " << inputFile << ", " << outputFile << endl;
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

    if (operation == "c") {
        compress(inputFile, outputFile);
    } else if (operation == "d") {
        decompress(inputFile, outputFile);
    }

    return 0;
}