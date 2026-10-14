#include <stdio.h>
#include <tonc.h>
#include "text.h"
#include "game.h"

// Box layout (must match tools/gen_puzzles.py and game.c).
#define BOX_H 2
#define BOX_W 3

// Palette banks used for BG0 text tiles.
enum
{
    BANK_BORDER = 0,
    BANK_BORDER_BOX = 1,
    BANK_GIVEN = 2,
    BANK_USER = 3,
    BANK_CONFLICT = 4,
    BANK_CURSOR = 5,
    BANK_SIDEBAR = 6,
    BANK_SOLVED = 7,
};

// NOTE: on real GBA hardware, pixel/palette index 0 of a BG tile is always
// transparent and falls through to the single global backdrop color
// (pal_bg_mem[0]) - a palette bank cannot give a tile its own independent
// *background* fill. So every bank below only controls its ink (text)
// color; CLR_BG is passed as every bank's "paper" purely because
// text_set_bank_colors() takes one, but only bank 0's copy of it is ever
// actually visible (as the backdrop).
#define CLR_BG RGB15(1, 1, 3)
#define CLR_BORDER RGB15(10, 10, 14)
#define CLR_BORDER_BOX RGB15(20, 20, 26)
#define CLR_GIVEN RGB15(14, 18, 31)
#define CLR_USER RGB15(31, 31, 31)
#define CLR_CONFLICT RGB15(31, 6, 6)
#define CLR_CURSOR RGB15(31, 29, 4)
#define CLR_SIDEBAR RGB15(20, 20, 24)
#define CLR_SOLVED RGB15(10, 31, 10)

// Sudoku grid occupies a 13x13 tile block starting here.
#define GRID_COL0 1
#define GRID_ROW0 2
#define SIDE_COL 16
#define SIDE_ROW 2

static void draw_grid_borders(void)
{
    for (int j = 0; j <= GRID_N; j++)
    {
        int rowBoundary = (j % BOX_H == 0);
        int row = GRID_ROW0 + 2 * j;
        for (int i = 0; i <= GRID_N; i++)
        {
            int col = GRID_COL0 + 2 * i;
            int colBoundary = (i % BOX_W == 0);
            int bank = (rowBoundary || colBoundary) ? BANK_BORDER_BOX : BANK_BORDER;
            text_put_char(col, row, '+', bank);

            if (i < GRID_N)
            {
                int hBank = rowBoundary ? BANK_BORDER_BOX : BANK_BORDER;
                text_put_char(col + 1, row, rowBoundary ? '=' : '-', hBank);
            }
        }
    }

    for (int i = 0; i <= GRID_N; i++)
    {
        int colBoundary = (i % BOX_W == 0);
        int col = GRID_COL0 + 2 * i;
        for (int j = 0; j < GRID_N; j++)
        {
            int row = GRID_ROW0 + 2 * j;
            int vBank = colBoundary ? BANK_BORDER_BOX : BANK_BORDER;
            text_put_char(col, row + 1, '|', vBank);
        }
    }
}

static void draw_cells(void)
{
    for (int r = 0; r < GRID_N; r++)
    {
        for (int c = 0; c < GRID_N; c++)
        {
            int col = GRID_COL0 + 1 + 2 * c;
            int row = GRID_ROW0 + 1 + 2 * r;

            unsigned char v = board.value[r][c];
            int isCursor = (r == g_cursorRow && c == g_cursorCol);

            char ch;
            int bank;
            if (isCursor)
            {
                // On an empty cell, preview the currently-loaded digit so
                // the player can see what A/B would place.
                ch = (char)('0' + (v ? v : g_selectedDigit));
                bank = BANK_CURSOR;
            }
            else if (!v)
            {
                ch = ' ';
                bank = BANK_USER;
            }
            else if (board.given[r][c])
            {
                ch = (char)('0' + v);
                bank = BANK_GIVEN;
            }
            else if (board.conflict[r][c])
            {
                ch = (char)('0' + v);
                bank = BANK_CONFLICT;
            }
            else
            {
                ch = (char)('0' + v);
                bank = BANK_USER;
            }

            text_put_char(col, row, ch, bank);
        }
    }
}

static void draw_sidebar(void)
{
    char buf[24];

    // Font glyphs fill almost the full 8px tile height, so stacked text
    // lines need a blank tile row between them or they visually touch.
    unsigned int secs = g_frameCount / 60;
    sprintf(buf, "TIME  %02u:%02u", secs / 60, secs % 60);
    text_put_str(SIDE_COL, SIDE_ROW, buf, BANK_SIDEBAR);

    unsigned int best = g_bestFrames[g_puzzleIndex];
    if (best == 0)
        sprintf(buf, "BEST  --:--");
    else
    {
        unsigned int bsecs = best / 60;
        sprintf(buf, "BEST  %02u:%02u", bsecs / 60, bsecs % 60);
    }
    text_put_str(SIDE_COL, SIDE_ROW + 2, buf, BANK_SIDEBAR);

    if (g_solved)
    {
        text_put_str(SIDE_COL, SIDE_ROW + 4, g_newBest ? "SOLVED!" : "SOLVED! ", BANK_SOLVED);
        text_put_str(SIDE_COL, SIDE_ROW + 6, g_newBest ? "NEW BEST" : "        ", BANK_SOLVED);
    }
    else
    {
        text_put_str(SIDE_COL, SIDE_ROW + 4, "        ", BANK_SIDEBAR);
        text_put_str(SIDE_COL, SIDE_ROW + 6, "        ", BANK_SIDEBAR);
    }
}

static void draw_all(void)
{
    draw_grid_borders();
    draw_cells();
    draw_sidebar();
}

static void init_palette(void)
{
    text_set_bank_colors(BANK_BORDER, CLR_BORDER, CLR_BG);
    text_set_bank_colors(BANK_BORDER_BOX, CLR_BORDER_BOX, CLR_BG);
    text_set_bank_colors(BANK_GIVEN, CLR_GIVEN, CLR_BG);
    text_set_bank_colors(BANK_USER, CLR_USER, CLR_BG);
    text_set_bank_colors(BANK_CONFLICT, CLR_CONFLICT, CLR_BG);
    text_set_bank_colors(BANK_CURSOR, CLR_CURSOR, CLR_BG);
    text_set_bank_colors(BANK_SIDEBAR, CLR_SIDEBAR, CLR_BG);
    text_set_bank_colors(BANK_SOLVED, CLR_SOLVED, CLR_BG);
}

int main(void)
{
    irq_init(NULL);
    irq_enable(II_VBLANK);
    REG_DISPSTAT |= DSTAT_VBL_IRQ;
    REG_DISPCNT = DCNT_MODE0 | DCNT_BG0;

    text_init();
    init_palette();

    game_new_puzzle(0);
    draw_all();

    while (1)
    {
        VBlankIntrWait();
        key_poll();

        if (!g_solved)
        {
            if (key_hit(KEY_UP) && g_cursorRow > 0)
                g_cursorRow--;
            if (key_hit(KEY_DOWN) && g_cursorRow < GRID_N - 1)
                g_cursorRow++;
            if (key_hit(KEY_LEFT) && g_cursorCol > 0)
                g_cursorCol--;
            if (key_hit(KEY_RIGHT) && g_cursorCol < GRID_N - 1)
                g_cursorCol++;

            if (key_hit(KEY_L))
                g_selectedDigit = (g_selectedDigit == 1) ? GRID_N : g_selectedDigit - 1;
            if (key_hit(KEY_R))
                g_selectedDigit = (g_selectedDigit == GRID_N) ? 1 : g_selectedDigit + 1;

            if (key_hit(KEY_A) || key_hit(KEY_B))
                game_place_digit(g_cursorRow, g_cursorCol, g_selectedDigit);

            if (key_hit(KEY_SELECT))
                game_give_hint();

            g_frameCount++;
        }

        if (key_hit(KEY_START))
            game_new_puzzle(g_puzzleIndex + 1);

        draw_all();
    }
}
