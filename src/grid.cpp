#include "grid.h"

Grid::Grid() {
    // Initialize all cells to empty
    for (auto& row : cells) {
        for (auto& cell : row) {
            cell.type = 0;
        }
    }
}

bool Grid::isValidPosition(int x, int y) const {
    return x >= 0 && x < SIZE && y >= 0 && y < SIZE;
}

void Grid::setCell(int x, int y, int type) {
    if (isValidPosition(x, y)) {
        cells[y][x].type = type;
    }
}

int Grid::getCell(int x, int y) const {
    if (isValidPosition(x, y)) {
        return cells[y][x].type;
    }
    return -1; // Invalid
}