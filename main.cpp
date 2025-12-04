#include <complex>
#include <iostream>
#include <fstream>
#include <cmath>
#include <tsl/ordered_map.h>

using namespace std;
using namespace tsl;

const char DELIMITER = '\0';

struct TableEntry {
    char character;
    mutable int frequency;
    mutable int lowerBound;
    mutable int upperBound;
};

uint64_t string_to_uint64(string str) {
    stringstream stream(str);
    uint64_t result;
    stream >> result;
    return result;
}

ordered_map<char, TableEntry> initalizeTable(const string& fileName) {
    char c;
    ordered_map<char, TableEntry> table;
    ifstream file(fileName, ios::binary | ios::in);

    // Count frequencies
    while (file.get(c)) {
        if(!table.contains(c)) {
            table[c] = TableEntry{c, 1, 0, 0};
        } else {
            table[c].frequency++;
        }
    }

    // Calculate bounds
    int cumulativeFreq = 0;
    for (auto & it : table) {
        it.second.lowerBound = cumulativeFreq;
        cumulativeFreq += it.second.frequency;
        it.second.upperBound = cumulativeFreq;
    }

    return table;
}

uint64_t getComulativeFrequency(const ordered_map<char, TableEntry>& table) {
    uint64_t frequency = 0;

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

    uint64_t lowerBound = 0;
    uint64_t upperBound = (1ULL << (bitSize - 1)) - 1; //pow(2, bitSize - 1) - 1;
    uint64_t secondQuarter = (upperBound + 1) / 2;
    uint64_t firstQuarter = secondQuarter / 2;
    uint64_t thirdQuarter = firstQuarter * 3;

    ordered_map<char, TableEntry> table = initalizeTable(inputFile);
    uint64_t cummulativeFrequency = getComulativeFrequency(table);

    if (input.is_open() && output.is_open()) {
        // Write file header.
        output << bitSize;
        output.write(&DELIMITER, 1);
        for (auto & it : table) {
            output.write(&it.first, 1);
            output.write(&DELIMITER, 1);
        }
        output.write(&DELIMITER, 1); // To mark the beginning of frequencies.
        for (auto & it : table) {
            output << it.second.frequency;
            output.write(&DELIMITER, 1);
        }
        output.write(&DELIMITER, 1); // To mark beginning of body.

        char c;
        int E3_counter = 0;

        while (input.get(c)) {
            uint64_t step = (upperBound - lowerBound + 1) / cummulativeFrequency;
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

    // Parse used characters
    while (true) {
        input.get(c);
        if (c == DELIMITER) {
            if (input.peek() == DELIMITER) {
                input.get();
                break;
            }
            continue;
        }
        table[c] = TableEntry{c, -1, -1, -1};
    }

    // Parse character frequencies.
    int previousUpper = -1;
    for (auto & it : table) {
        string freqStr;
        // Parse and concat character frequencies.
        while (true) {
            input.get(c);
            if (c == DELIMITER) {
                break;
            }
            freqStr += c;
        }

        uint16_t currentFreq = string_to_uint64(freqStr);
        uint64_t lowerBound = previousUpper == -1 ? 0 : previousUpper;
        uint64_t upperBound = lowerBound + currentFreq;

        it.second.frequency = currentFreq;
        it.second.lowerBound = lowerBound;
        it.second.upperBound = upperBound;

        previousUpper = upperBound;
    }
    input.get(); // Consume delimiter.

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
        uint64_t cummulativeFrequency = getComulativeFrequency(table);

        uint64_t field = 0;
        for (int i = 0; i < bitSize - 1; i++ ) field = ( field << 1 ) | readNextBit(input);

        uint64_t lowerBound = 0;
        uint64_t upperBound = (1ULL << (bitSize - 1)) - 1; //pow(2, bitSize - 1) - 1;
        uint64_t secondQuarter = (upperBound + 1) / 2;
        uint64_t firstQuarter = secondQuarter / 2;
        uint64_t thirdQuarter = firstQuarter * 3;

        for (uint64_t i = 0; i < cummulativeFrequency; i++) {
            uint64_t step = (upperBound - lowerBound + 1) / cummulativeFrequency;
            uint64_t v = (field - lowerBound) / step;
            char currentChar = getCharFromTable(table, v);
            output.write(&currentChar, 1);

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