# VT52 Control Codes

Screen is 80x24, top left is (1,1)

Parameters are encoded as CHR(x+31)

ASCII code | Description
:----------|:--------------------------
CR | Move to start of line
LF | Move to next line, scroll screen up on last line.
BS | Move left 1 char (ESC D)
TAB | Move right to 8*N+1

## Basic escapes

|ESC + | Description
|:-----|:--------------------------------
|A | Move up 1 line, no scroll
|B | Move down 1 line, no scroll
|C | Move right 1 char, no scroll
|D | Move left 1 char, no scroll
|F | Graphics char set
|G | Regular char set
|H | Cursor to 1,1
|I | Move up 1 line, scroll screen down on first line.
|J | Clear from cursor to end of screen
|K | Clear from cursor to end of line
|L | Insert line at cursor (scroll down from this line down)
|M | Remove line at cursor (scroll up from line down)
|Y r c | Move to c,r (encoded)
|Z | ID by sending "ESC / K" back.
|= | Alternate Keypad
| > | Normal Keypad

## Expanded ESC set, Atari ST

ESC + | Type | Description
:-----|:----:|:-------------------------
E | Atari ST | Same as sequence "ESC H ESC J" (clear screen)
b # | Atari ST | Change to forecolor #
c # | Atari ST | Change to backcolor #

## RW Pocket Term

RW Pocket Term uses more ESC commands.

Parameters are Base85 encoded

### Extended codes

ESC + | Description
:-----|:--------------------
N | 
O |
P | Toggle paint mode
Q |
R | Respond with "ESC / R" ("ESC / P" if still in paint mode)
S | Respond with "ESC X rr cc" with rr=max rows, cc=max columns 
T |
U |
V |
W |
X rr cc | Move to cc,rr

### Paint Mode

TBD
