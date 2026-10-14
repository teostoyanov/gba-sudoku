#include "text.h"
#include "font_data.h"

// Packs an 8x8 1bpp glyph (MSB = leftmost pixel) into a 4bpp GBA tile:
// two pixels per byte, low nibble = left pixel of the pair. "On" pixels
// get palette index 1 (the bank's ink color); "off" pixels get index 0
// (the bank's paper color).
static void build_tile(TILE *out, const unsigned char rows[8])
{
    for (int y = 0; y < 8; y++)
    {
        unsigned char r = rows[y];
        u32 word = 0;
        for (int x = 0; x < 8; x += 2)
        {
            unsigned px0 = (r >> (7 - x)) & 1;
            unsigned px1 = (r >> (7 - x - 1)) & 1;
            unsigned byte = px0 | (px1 << 4);
            word |= byte << (4 * x);
        }
        out->data[y] = word;
    }
}

void text_init(void)
{
    for (int i = 0; i < FONT_NUM_GLYPHS; i++)
    {
        TILE t;
        build_tile(&t, font8x8[i]);
        tile_mem[TEXT_CBB][i] = t;
    }

    for (int b = 0; b < 16; b++)
    {
        pal_bg_mem[b * 16 + 0] = RGB15(1, 1, 3);
        pal_bg_mem[b * 16 + 1] = RGB15(28, 28, 28);
    }

    REG_BG0CNT = BG_CBB(TEXT_CBB) | BG_SBB(TEXT_SBB) | BG_4BPP | BG_REG_32x32;
}

void text_set_bank_colors(int bank, u16 fg, u16 bg)
{
    pal_bg_mem[bank * 16 + 0] = bg;
    pal_bg_mem[bank * 16 + 1] = fg;
}

void text_put_char(int col, int row, char ch, int bank)
{
    if (col < 0 || col >= 32 || row < 0 || row >= 32)
        return;

    int idx = (unsigned char)ch - FONT_FIRST_CHAR;
    if (idx < 0 || idx >= FONT_NUM_GLYPHS)
        idx = 0; // out-of-range -> blank

    se_mem[TEXT_SBB][row * 32 + col] = idx | SE_PALBANK(bank);
}

void text_put_str(int col, int row, const char *s, int bank)
{
    int c = col;
    while (*s)
    {
        if (*s == '\n')
        {
            row++;
            c = col;
        }
        else
        {
            text_put_char(c, row, *s, bank);
            c++;
        }
        s++;
    }
}
