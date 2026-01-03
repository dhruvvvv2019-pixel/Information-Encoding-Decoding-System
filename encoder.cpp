#include <iostream>
#include <fstream>
#include <vector>
#include <string>

struct Node {
    int row;
    int col;
};

int COLS, ROWS, LINES, TURN_LIMIT, POPULAR_COUNT;
std::vector<Node> lineStart, lineEnd, mustVisit;
std::vector<std::vector<int>> cnfClauses;
int totalVars = 0;

inline int cellToVar(int r, int c, int lineIndex) {
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return 0;
    return (r * COLS + c) * LINES + lineIndex + 1;
}

inline void pushClause(const std::vector<int>& clause) {
    cnfClauses.push_back(clause);
}

void rule_unique_cells() {
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            for (int a = 0; a < LINES; ++a) {
                for (int b = a + 1; b < LINES; ++b) {
                    pushClause({-cellToVar(r, c, a), -cellToVar(r, c, b)});
                }
            }
        }
    }
}

void rule_path_consistency() {
    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    for (int k = 0; k < LINES; ++k) {
        pushClause({cellToVar(lineStart[k].row, lineStart[k].col, k)});
        pushClause({cellToVar(lineEnd[k].row, lineEnd[k].col, k)});

        for (int r = 0; r < ROWS; ++r) {
            for (int c = 0; c < COLS; ++c) {
                int currentVar = cellToVar(r, c, k);

                std::vector<int> neighbors;
                for (int d = 0; d < 4; ++d) {
                    int nr = r + dr[d], nc = c + dc[d];
                    if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS)
                        neighbors.push_back(cellToVar(nr, nc, k));
                }

                bool isStart = (r == lineStart[k].row && c == lineStart[k].col);
                bool isEnd = (r == lineEnd[k].row && c == lineEnd[k].col);

                if (isStart || isEnd) {
                    pushClause(neighbors); // at least one
                    for (size_t i = 0; i < neighbors.size(); ++i) {
                        for (size_t j = i + 1; j < neighbors.size(); ++j) {
                            pushClause({-neighbors[i], -neighbors[j]}); // at most one
                        }
                    }
                } else {

                    if (neighbors.size() >= 3) {
                        for (size_t i = 0; i < neighbors.size(); ++i) {
                            for (size_t j = i + 1; j < neighbors.size(); ++j) {
                                for (size_t l = j + 1; l < neighbors.size(); ++l) {
                                    pushClause({-currentVar, -neighbors[i], -neighbors[j], -neighbors[l]});
                                }
                            }
                        }
                    }

                    if (neighbors.size() >= 2) {
                        for (size_t i = 0; i < neighbors.size(); ++i) {
                            std::vector<int> tmp = {-currentVar};
                            for (size_t j = 0; j < neighbors.size(); ++j) {
                                if (i != j) tmp.push_back(neighbors[j]);
                            }
                            pushClause(tmp);
                        }
                    }
                }
            }
        }
    }
}

void rule_turn_limits() {
    if (TURN_LIMIT >= (ROWS + COLS)) return;

    int baseVarCount = ROWS * COLS * LINES;
    totalVars = baseVarCount;

    auto turnVar = [&](int r, int c, int k) {
        return baseVarCount + (r * COLS + c) * LINES + k + 1;
    };

    int counterBase = baseVarCount + ROWS * COLS * LINES;

    for (int k = 0; k < LINES; ++k) {
        std::vector<int> lineTurns;
        for (int r = 0; r < ROWS; ++r) {
            for (int c = 0; c < COLS; ++c) {
                bool isStart = (r == lineStart[k].row && c == lineStart[k].col);
                bool isEnd = (r == lineEnd[k].row && c == lineEnd[k].col);
                if (isStart || isEnd) continue;

                int cellVar = cellToVar(r, c, k);
                int tVar = turnVar(r, c, k);
                lineTurns.push_back(tVar);

                int up = cellToVar(r - 1, c, k);
                int down = cellToVar(r + 1, c, k);
                int left = cellToVar(r, c - 1, k);
                int right = cellToVar(r, c + 1, k);

                if (up && left) pushClause({-cellVar, -up, -left, tVar});
                if (up && right) pushClause({-cellVar, -up, -right, tVar});
                if (down && left) pushClause({-cellVar, -down, -left, tVar});
                if (down && right) pushClause({-cellVar, -down, -right, tVar});

                pushClause({-tVar, cellVar});
            }
        }

        if (lineTurns.empty()) continue;

        int turnCount = lineTurns.size();
        std::vector<std::vector<int>> s(turnCount + 1, std::vector<int>(TURN_LIMIT + 2, 0));

        int curVar = counterBase + k * turnCount * (TURN_LIMIT + 2);
        for (int i = 1; i <= turnCount; ++i) {
            for (int j = 1; j <= TURN_LIMIT + 1; ++j) {
                s[i][j] = ++curVar;
            }
        }

        totalVars = std::max(totalVars, curVar);

        pushClause({-lineTurns[0], s[1][1]});
        for (int j = 2; j <= TURN_LIMIT + 1; ++j)
            pushClause({-s[1][j]});

        for (int i = 2; i <= turnCount; ++i) {
            pushClause({-s[i - 1][1], s[i][1]});
            pushClause({-lineTurns[i - 1], s[i][1]});
            for (int j = 2; j <= TURN_LIMIT + 1; ++j) {
                pushClause({-s[i - 1][j], s[i][j]});
                pushClause({-lineTurns[i - 1], -s[i - 1][j - 1], s[i][j]});
            }
        }

        pushClause({-s[turnCount][TURN_LIMIT + 1]});
    }
}

void rule_popular_visit() {
    for (const auto& cell : mustVisit) {
        std::vector<int> clause;
        for (int k = 0; k < LINES; ++k)
            clause.push_back(cellToVar(cell.row, cell.col, k));
        pushClause(clause);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./encoder <input_filename>" << std::endl;
        return 1;
    }

    std::string fileBase = argv[1];
    std::ifstream fin(fileBase + ".city");
    if (!fin.is_open()) {
        std::cerr << "Error: Cannot open .city file" << std::endl;
        return 1;
    }

    int scenario;
    fin >> scenario;
    fin >> COLS >> ROWS >> LINES >> TURN_LIMIT;
    if (scenario == 2)
        fin >> POPULAR_COUNT;

    lineStart.resize(LINES);
    lineEnd.resize(LINES);
    for (int i = 0; i < LINES; ++i)
        fin >> lineStart[i].col >> lineStart[i].row >> lineEnd[i].col >> lineEnd[i].row;

    if (scenario == 2) {
        mustVisit.resize(POPULAR_COUNT);
        for (int p = 0; p < POPULAR_COUNT; ++p)
            fin >> mustVisit[p].col >> mustVisit[p].row;
    }

    fin.close();

    totalVars = COLS * ROWS * LINES;

    rule_unique_cells();
    rule_path_consistency();
    rule_turn_limits();
    if (scenario == 2) rule_popular_visit();

    std::ofstream fout(fileBase + ".satinput");
    fout << "p cnf " << totalVars << " " << cnfClauses.size() << "\n";
    for (const auto& clause : cnfClauses) {
        for (int lit : clause) fout << lit << " ";
        fout << "0\n";
    }

    fout.close();
    return 0;
}