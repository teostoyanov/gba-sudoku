# Building a Sudoku Game for the Game Boy Advance

I own a Game Boy Advance Micro and an Everdrive GBA Pro flash cart, which means I can load and run my own homebrew games on real hardware. For this tiny project I wanted to see the whole pipeline through: write a small game in C, cross-compile it into an actual GBA ROM, and play it on the original hardware — no PC, no emulator, just the cartridge.

The game: a simple 6x6 Sudoku (digits 1-6, with boxes of 2 rows by 3 columns instead of the usual 9x9/3x3 layout), built on [devkitARM](https://devkitpro.org/) and [libtonc](https://gbadev.net/tonc/), the classic toolchain and tutorial that most GBA homebrew is built on.

## The gameplay

- D-pad moves the cursor around the grid
- L/R cycles which digit (1-6) is "loaded"
- A or B stamps the loaded digit into the current cell — press it again to clear the cell
- SELECT gives a hint, filling in one correct cell
- START moves on to the next puzzle
- Duplicate digits in a row, column, or box light up red immediately
- The sidebar tracks elapsed time and your best time for each puzzle

One small UX touch I liked: when the cursor sits on an empty cell, it shows a "ghost" preview of whichever digit is currently loaded, in a distinct color, so you can see exactly what A/B is about to place before you commit to it.

## Puzzles you can actually trust

Rather than generate puzzles on the GBA itself — which would mean writing a solver and generator that run fast enough on a 16MHz ARM7, with a real risk of shipping a broken or non-unique puzzle — I generated a fixed set of 12 puzzles offline in Python. The generator is a real backtracking solver: it builds a full solution, then removes clues one at a time, checking after every removal that the puzzle still has *exactly one* solution. Anything that would make the puzzle ambiguous gets put back. The puzzles are baked into the ROM as plain data, so there's no runtime generation risk at all — just a verified set of boards with increasing difficulty.

## A font with no guesswork

GBA tile rendering wants 8x8 pixel glyphs, and I didn't want to hand-type bitmap font data from memory — that's exactly the kind of thing that's easy to get subtly wrong (a mirrored row here, an off-by-one there) and hard to notice until it's on screen. Instead I rasterized Python/Pillow's built-in bitmap font, character by character, into 8x8 1-bit glyphs, and wrote a small generator script that spits out the result as a C header. The font is deterministic, reproducible, and never relied on me remembering what a pixel-perfect letter 'B' is supposed to look like.

## The hardware quirk that broke my first design

My original plan was to highlight the cursor and conflicting cells with a solid colored background block — yellow for the cursor, red for a conflict — using the GBA's palette-bank system, where each background tile can reference one of 16 color banks.

It didn't work, and the reason is a genuine GBA hardware quirk: palette index 0 in a background tile is *always* transparent and falls through to a single global backdrop color, no matter which palette bank the tile uses. You can't give sixteen different banks sixteen independent "paper" colors on one background layer — only the ink (foreground) color is actually independent per bank. I only caught this because I tested the ROM running (see below) and saw the cursor cell rendering with a plain dark background instead of bright yellow. The fix was simpler than the original plan anyway: color the *digit itself* instead of the cell background, which is what you see in the final version.

## Testing without a GBA plugged into the dev machine

This was all built on a headless Linux box with no display attached — so "run it and see" wasn't as simple as double-clicking a ROM. The workflow that ended up working well:

1. Build the ROM with `make`.
2. Spin up a virtual framebuffer (`Xvfb`) and launch the ROM in the [mGBA](https://mgba.io/) emulator inside it.
3. Drive the emulator with synthetic key presses (`python-xlib`'s XTEST extension) to move the cursor, place digits, trigger hints, and solve puzzles.
4. Grab X11 screenshots at each step and inspect them — including sampling exact pixel colors to verify palette banks were rendering the right colors, which is how the background-highlight bug above actually got caught.

Only once the emulator run looked right — borders, box boundaries, conflict highlighting, hints, the solved/best-time banner, puzzle cycling — did the ROM go onto the Everdrive's SD card for a real-hardware test.

## Toolchain bumps along the way

A few things broke that were worth noting for next time:

- devkitPro changed how they distribute their installer; the old GitHub `.deb` download is gone, replaced by a hosted install script behind Cloudflare (which blocked requests from the sandboxed dev environment but worked fine from an ordinary home connection).
- The Makefile template's `-mno-fpu` linker flag isn't recognized by the current GCC — a leftover from older ARM EABI toolchains.
- `make` really does not like spaces in directory paths (it treats them as list separators in `VPATH`, `-I`, `-L`, …), which meant renaming the project folder.
- `gbafix`, the tool that patches the ROM header with a title and game code, needs those values passed explicitly — otherwise the Everdrive's game list just shows a blank title.

None of these are exotic failures, but they're the kind of thing that eats an afternoon if you don't have the exact error message in front of you.

## Running it

```
make
cp GBA_Sudoku.gba /path/to/everdrive/sd/card/
```

Then it's just: SD card into the Everdrive, Everdrive into the GBA Micro, power on, pick it from the menu.

## What's next

Puzzle progress and best times currently reset whenever the game powers off — the Everdrive supports SRAM saves, so proper save/resume is the natural next step if this little project gets a second pass.
