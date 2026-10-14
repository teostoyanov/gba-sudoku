#ifndef TEXT_H
#define TEXT_H

#include <tonc.h>

// BG0 is used as a plain 32x32 tiled text surface. The char base block
// holds our generated 8x8 font glyphs; the screen base block holds the
// character map. CBB0 occupies VRAM blocks 0-7 (4bpp, <=512 tiles), so
// the screen block must be 8 or higher.
#define TEXT_CBB 0
#define TEXT_SBB 28

// Initializes BG0, builds all font glyphs into the char base block, and
// sets a default color (light gray on near-black) for every palette bank.
void text_init(void);

// Sets the ink (fg) and paper (bg) color for one of the 16 BG palette
// banks (0-15). Characters drawn with that bank use these colors.
void text_set_bank_colors(int bank, u16 fg, u16 bg);

// Draws a single character at tile position (col,row) using the given
// palette bank. Characters outside the generated font range render blank.
void text_put_char(int col, int row, char ch, int bank);

// Draws a NUL-terminated string starting at (col,row); '\n' moves to the
// next row at the same starting column.
void text_put_str(int col, int row, const char *s, int bank);

#endif // TEXT_H
