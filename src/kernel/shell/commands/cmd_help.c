#include "cmd_helpers.h"

void cmd_help(ShellCmd *cl)
{
    (void)cl;
    typedef struct { const char *name; const char *desc; } E;
    static const E cmds[] = {
        {"help",             "Show this help message"},
        {"clear / cls",      "Clear the terminal screen"},
        {"echo <text>",      "Print text to terminal"},
        {"version",          "Show OmniOS version"},
        {"sysinfo",          "Display system information"},
        {"meminfo",          "Show memory layout"},
        {"cpuinfo",          "Show CPU / arch information"},
        {"history",          "List command history"},
        {"color <0-7>",      "Change colour scheme"},
        {"calc <expr>",      "Integer calculator (e.g. calc 10+3)"},
        {"ascii",            "Print ASCII table"},
        {"hex <addr> [len]", "Hex dump memory"},
        {"peek <addr>",      "Read byte from address"},
        {"poke <addr> <v>",  "Write byte to address"},
        {"uptime",           "Show tick counter"},
        {"date",             "Show system date/time"},
        {"whoami",           "Show current user"},
        {"hostname [name]",  "Show/set hostname"},
        {"ls [path]",        "List directory contents"},
        {"cat <file>",       "Print file contents"},
        {"disk",             "Show disk information"},
        {"banner",           "Redraw boot banner"},
        {"packages / pkg",   "List available packages"},
        {"install <pkg>",    "Install a package (or 'all')"},
        {"uninstall <pkg>",  "Uninstall a package (or 'all')"},
        {"reboot",           "Reboot the system"},
        {"halt / shutdown",  "Halt the CPU"},
        {"panic",            "Trigger kernel panic (test)"},
        {NULL, NULL}
    };
    hr('=', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("   OmniOS v" OMNIOS_VERSION " Command Reference", VGA_COL_HEADER);
    hr('=', VGA_WIDTH, VGA_COL_HEADER);
    for (int i = 0; cmds[i].name; i++) {
        COLOR_SET(VGA_COL_SUCCESS);
        puts("  "); puts(cmds[i].name);
        int pad = 26 - cmd_strlen(cmds[i].name);
        for (int j = 0; j < pad; j++) putc(' ');
        COLOR_RESET();
        puts(cmds[i].desc); putc('\n');
    }
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("  Installed packages also work as commands.", VGA_COL_INFO);
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
}