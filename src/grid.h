#pragma once

#include <vector>
#include <array>

struct GridCell {
    // Placeholder for cell data, e.g., terrain type, occupancy
    int type = 0; // 0 = empty, 1 = occupied, etc.
};

class Grid {
public:
    static constexpr int SIZE = 8;
    std::array<std::array<GridCell, SIZE>, SIZE> cells;

    Grid();
    bool isValidPosition(int x, int y) const;
    void setCell(int x, int y, int type);
    int getCell(int x, int y) const;
};