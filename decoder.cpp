#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>

struct Cell {
    int row;
    int col;
    bool operator==(const Cell& other) const {
        return (row == other.row && col == other.col);
    }
};

int gridRows, gridCols, lineCount;
std::vector<Cell> startPoints, endPoints;

int makeVarId(int r, int c, int lineIndex) {
    return (r * gridCols + c) * lineCount + lineIndex + 1;
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "Usage: ./decoder <filename_without_extension>" << std::endl;
        return 1;
    }

    std::string baseName = argv[1];

    std::ifstream satFile(baseName + ".satoutput");
    if (!satFile.is_open()) {
        std::cerr << "Error: Couldn't open .satoutput file." << std::endl;
        return 1;
    }

    std::string outcome;
    satFile >> outcome;

    if (outcome == "UNSAT") {
        std::ofstream outFile(baseName + ".metromap");
        outFile << 0 << std::endl;
        return 0;
    }

    std::vector<bool> truthTable;
    int literal;
    while (satFile >> literal && literal != 0) {
        if ((int)truthTable.size() <= std::abs(literal))
            truthTable.resize(std::abs(literal) + 1);
        truthTable[std::abs(literal)] = (literal > 0);
    }
    satFile.close();

    std::ifstream cityFile(baseName + ".city");
    if (!cityFile.is_open()) {
        std::cerr << "Error: Couldn't open .city file." << std::endl;
        return 1;
    }

    int scenarioType, junctionCount, penaltyValue;
    cityFile >> scenarioType;
    cityFile >> gridCols >> gridRows >> lineCount >> junctionCount;
    if (scenarioType == 2)
        cityFile >> penaltyValue;

    startPoints.resize(lineCount);
    endPoints.resize(lineCount);

    for (int k = 0; k < lineCount; ++k) {
        cityFile >> startPoints[k].col >> startPoints[k].row >> endPoints[k].col >> endPoints[k].row;
    }

    cityFile.close();

    std::ofstream mapFile(baseName + ".metromap");
    if (!mapFile.is_open()) {
        std::cerr << "Error: Couldn't create .metromap file." << std::endl;
        return 1;
    }

    // Movements: Up, Down, Left, Right
    const int rowMove[4] = {-1, 1, 0, 0};
    const int colMove[4] = {0, 0, -1, 1};
    const char moveChar[4] = {'U', 'D', 'L', 'R'};

    for (int lineIdx = 0; lineIdx < lineCount; ++lineIdx) {
        Cell current = startPoints[lineIdx];
        Cell previous = {-1, -1};  

        while (!(current == endPoints[lineIdx])) {
            bool foundNext = false;

            for (int dir = 0; dir < 4; ++dir) {
                Cell next = {current.row + rowMove[dir], current.col + colMove[dir]};

                // Stay within bounds
                if (next.row < 0 || next.row >= gridRows || next.col < 0 || next.col >= gridCols)
                    continue;

                if (next == previous) continue;

                int id = makeVarId(next.row, next.col, lineIdx);

                if (id < (int)truthTable.size() && truthTable[id]) {
                    mapFile << moveChar[dir] << " ";
                    
                    previous = current;
                    current = next;
                    foundNext = true;
                    break;
                }
            }

            if (!foundNext) {
                mapFile << "? "; 
                break;
            }
        }
        
        mapFile << "0" << std::endl;
    }

    mapFile.close();
    return 0;
}