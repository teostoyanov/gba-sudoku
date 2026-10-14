#include <string.h>
#include "game.h"

// Box layout matches tools/gen_puzzles.py: 2 rows x 3 cols per box.
#define BOX_H 2
#define BOX_W 3

Board board;
int g_puzzleIndex = 0;
int g_cursorRow = 0, g_cursorCol = 0;
int g_selectedDigit = 1;
unsigned int g_frameCount = 0;
unsigned int g_bestFrames[NUM_PUZZLES];
int g_solved = 0;
int g_newBest = 0;

void game_new_puzzle(int idx)
{
    idx %= NUM_PUZZLES;
    if (idx < 0)
        idx += NUM_PUZZLES;
    g_puzzleIndex = idx;

    for (int r = 0; r < GRID_N; r++)
    {
        for (int c = 0; c < GRID_N; c++)
        {
            unsigned char v = puzzle_givens[idx][r][c];
            board.value[r][c] = v;
            board.given[r][c] = (v != 0);
            board.conflict[r][c] = 0;
        }
    }

    g_cursorRow = 0;
    g_cursorCol = 0;
    g_selectedDigit = 1;
    g_frameCount = 0;
    g_solved = 0;
    g_newBest = 0;
}

static int has_duplicate(int r, int c, int v)
{
    for (int i = 0; i < GRID_N; i++)
    {
        if (i != c && board.value[r][i] == v)
            return 1;
        if (i != r && board.value[i][c] == v)
            return 1;
    }
    int br = (r / BOX_H) * BOX_H;
    int bc = (c / BOX_W) * BOX_W;
    for (int i = br; i < br + BOX_H; i++)
    {
        for (int j = bc; j < bc + BOX_W; j++)
        {
            if ((i != r || j != c) && board.value[i][j] == v)
                return 1;
        }
    }
    return 0;
}

void game_recompute_conflicts(void)
{
    for (int r = 0; r < GRID_N; r++)
    {
        for (int c = 0; c < GRID_N; c++)
        {
            unsigned char v = board.value[r][c];
            board.conflict[r][c] = (v != 0) && has_duplicate(r, c, v);
        }
    }
}

int game_is_solved(void)
{
    for (int r = 0; r < GRID_N; r++)
    {
        for (int c = 0; c < GRID_N; c++)
        {
            if (board.value[r][c] == 0 || board.conflict[r][c])
                return 0;
        }
    }
    return 1;
}

static void check_solved(void)
{
    g_newBest = 0;
    if (!game_is_solved())
        return;

    g_solved = 1;
    unsigned int best = g_bestFrames[g_puzzleIndex];
    if (best == 0 || g_frameCount < best)
    {
        g_bestFrames[g_puzzleIndex] = g_frameCount;
        g_newBest = 1;
    }
}

void game_place_digit(int r, int c, int d)
{
    if (board.given[r][c])
        return;

    board.value[r][c] = (board.value[r][c] == d) ? 0 : (unsigned char)d;
    game_recompute_conflicts();
    check_solved();
}

void game_give_hint(void)
{
    if (g_solved)
        return;

    for (int r = 0; r < GRID_N; r++)
    {
        for (int c = 0; c < GRID_N; c++)
        {
            if (board.value[r][c] != puzzle_solutions[g_puzzleIndex][r][c])
            {
                board.value[r][c] = puzzle_solutions[g_puzzleIndex][r][c];
                game_recompute_conflicts();
                check_solved();
                return;
            }
        }
    }
}
