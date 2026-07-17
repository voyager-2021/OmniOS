#include "cmd_helpers.h"

void cmd_banner(ShellCmd *cl)
{
    (void)cl;
    VGA_ClearScreen();
    hr('*', VGA_WIDTH, VGA_COL_HEADER);
    COLOR_SET(VGA_COL_HEADER);
    puts("\n");
    puts("     ::::::::  :::   ::: ::::    ::: :::::::::  ::::::::  :::::\n");
    puts("    :+:    :+: :+:  :+: :+:+:   :+:  :+:    :+::+:    :+::+:  \n");
    puts("    +#+    +:+ +#++:+++ +#+ +:+ +#+  +#+    +:++#+    +:++#++:\n");
    puts("    #+#    #+# #+#  #+# #+#   #+#+#  #+#    #+##+#    #+#    #+#\n");
    puts("     ########  ###  ### ###    ####  #########  ########  :::::\n");
    puts("\n");
    COLOR_RESET();
    COLOR_SET(VGA_COL_INFO);
    printf("   Terminal OS  v%s  |  Build %s\n", OMNIOS_VERSION, OMNIOS_BUILD);
    COLOR_RESET();
    puts("\n");
    hr('*', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("  Type 'help' for commands.", VGA_COL_INFO);
}