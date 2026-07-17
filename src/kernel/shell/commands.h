#pragma once
#include "shell.h"

/* ============================================================
   OmniOS v1.0.0 - Command declarations
   Each cmd_*.c file implements one function.
   ============================================================ */

void Commands_Dispatch(ShellCmd *cl);

/* Individual commands */
void cmd_help(ShellCmd *cl);
void cmd_clear(ShellCmd *cl);
void cmd_echo(ShellCmd *cl);
void cmd_version(ShellCmd *cl);
void cmd_sysinfo(ShellCmd *cl);
void cmd_meminfo(ShellCmd *cl);
void cmd_cpuinfo(ShellCmd *cl);
void cmd_history(ShellCmd *cl);
void cmd_color(ShellCmd *cl);
void cmd_calc(ShellCmd *cl);
void cmd_ascii(ShellCmd *cl);
void cmd_hex(ShellCmd *cl);
void cmd_peek(ShellCmd *cl);
void cmd_poke(ShellCmd *cl);
void cmd_uptime(ShellCmd *cl);
void cmd_date(ShellCmd *cl);
void cmd_whoami(ShellCmd *cl);
void cmd_hostname(ShellCmd *cl);
void cmd_banner(ShellCmd *cl);
void cmd_packages(ShellCmd *cl);
void cmd_install(ShellCmd *cl);
void cmd_uninstall(ShellCmd *cl);
void cmd_ls(ShellCmd *cl);
void cmd_cat(ShellCmd *cl);
void cmd_disk(ShellCmd *cl);
void cmd_reboot(ShellCmd *cl);
void cmd_halt(ShellCmd *cl);
void cmd_panic(ShellCmd *cl);