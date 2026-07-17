#include "cmd_helpers.h"

void cmd_color(ShellCmd *cl)
{
    typedef struct { const char *name; uint8_t col; } S;
    static const S schemes[] = {
        {"Classic (grey/black)",  VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREY,  VGA_COLOR_BLACK)},
        {"Matrix  (green/black)", VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK)},
        {"Ocean   (cyan/black)",  VGA_MAKE_COLOR(VGA_COLOR_LIGHT_CYAN,  VGA_COLOR_BLACK)},
        {"Sunset  (red/black)",   VGA_MAKE_COLOR(VGA_COLOR_LIGHT_RED,   VGA_COLOR_BLACK)},
        {"Royal   (blue/black)",  VGA_MAKE_COLOR(VGA_COLOR_LIGHT_BLUE,  VGA_COLOR_BLACK)},
        {"Gold    (yellow/black)",VGA_MAKE_COLOR(VGA_COLOR_YELLOW,      VGA_COLOR_BLACK)},
        {"Mint    (white/blue)",  VGA_MAKE_COLOR(VGA_COLOR_WHITE,       VGA_COLOR_BLUE)},
        {"Alert   (white/red)",   VGA_MAKE_COLOR(VGA_COLOR_WHITE,       VGA_COLOR_RED)},
    };
    int num = (int)(sizeof(schemes) / sizeof(schemes[0]));
    if (cl->argc < 2) {
        cprintln("  Colour schemes:", VGA_COL_HEADER);
        for (int i = 0; i < num; i++) {
            COLOR_SET(schemes[i].col);
            printf("  %d  %s\n", i, schemes[i].name);
        }
        COLOR_RESET();
        cprintln("\n  Usage: color <0-7>", VGA_COL_WARNING);
        return;
    }
    int n = cmd_atoi(cl->argv[1]);
    if (n < 0 || n >= num) { cprintln("  Invalid scheme.", VGA_COL_ERROR); return; }
    VGA_SetColor(schemes[n].col);
    VGA_ClearScreen();
    printf("  Colour scheme %d applied.\n", n);
}