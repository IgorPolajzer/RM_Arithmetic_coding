#include <complex>
#include <iostream>
#include <fstream>
#include <map>
#include <cmath>
#include <vector>

using namespace std;

struct TableEntry {
    char character;
    int frequency;
    int lowerBound;
    int upperBound;
};

map<char, TableEntry> initalizeTable(const string& fileName) {
    char c;
    map<char, TableEntry> table;
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

int getComulativeFrequency(const map<char, TableEntry>& table) {
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

    map<char, TableEntry> table = initalizeTable(inputFile);
    int cummulativeFrequency = getComulativeFrequency(table);

    if (input.is_open() && output.is_open()) {
        // Write file header.
        output << bitSize;
        for (auto & it : table) {output << it.first;}
        for (auto & it : table) {output << it.second.frequency;}

        char c;
        int E3_counter = 0;

        while (input.get(c))
        {
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
                }

                if (lowerBound >= secondQuarter) {
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

map<char, TableEntry> parseHeader(ifstream& input, int &bitSize) {
    char c;
    string usedChars, stringBitSize;
    map<char, TableEntry> table;

    // Parse bit size (digits until first non-digit)
    while (isdigit(input.peek())) {
        input.get(c);
        stringBitSize += c;
    }
    bitSize = stoi(stringBitSize);

    // Parse used characters (all non-digits)
    while (isalpha(input.peek())) {
        input.get(c);
        usedChars += c;
    }

    // Parse character frequencies.
    int previousUpper = -1;
    for (char ch : usedChars) {
        input.get(c);
        string freq(1, c);

        int currentFreq = stoi(freq);
        int lowerBound = previousUpper == - 1 ? 0 : previousUpper;
        int upperBound = lowerBound * currentFreq;
        table[ch] = TableEntry{ch, currentFreq, lowerBound, upperBound};
        previousUpper = upperBound;
    }

    return table;
}

void decompress(const string& inputFile, const string& outputFile) {
    ifstream input(inputFile, ios::binary);
    ofstream output(outputFile, ios::binary);

    if (input.is_open() && output.is_open()) {
        // Parse the file header.
        int field;
        map<char, TableEntry> table = parseHeader(input, field);

        char c;

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