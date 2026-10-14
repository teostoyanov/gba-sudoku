#ifndef GAME_H
#define GAME_H

#include "puzzles_data.h"

typedef struct
{
    unsigned char value[GRID_N][GRID_N];    // 0 = empty, 1..6 = digit
    unsigned char given[GRID_N][GRID_N];    // 1 = fixed clue, cannot edit
    unsigned char conflict[GRID_N][GRID_N]; // 1 = duplicate in row/col/box
} Board;

extern Board board;
extern int g_puzzleIndex;
extern int g_cursorRow, g_cursorCol;
extern int g_selectedDigit; // 1..6, cycled with L/R
extern unsigned int g_frameCount;
extern unsigned int g_bestFrames[NUM_PUZZLES]; // 0 = no best time yet
extern int g_solved;
extern int g_newBest; // true if the just-finished solve set a new best

// Loads puzzle `idx` (wrapping into range) and resets cursor/timer/state.
void game_new_puzzle(int idx);

// Places/clears the given digit at (r,c). No-op on a given (clue) cell.
// Placing the same digit that's already there clears the cell.
void game_place_digit(int r, int c, int d);

// Fills the first wrong/empty cell with its correct solution digit.
void game_give_hint(void);

// Recomputes board.conflict for every cell from scratch.
void game_recompute_conflicts(void);

// True if the board is completely filled with no conflicts.
int game_is_solved(void);

#endif // GAME_H
