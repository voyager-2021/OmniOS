#pragma once
#include "shell.h"

/* ============================================================
   OmniOS - Command Dispatcher Header
   ============================================================ */

/* Dispatch a parsed command line to the correct handler */
void Commands_Dispatch(ShellCmd *cl);

/* Individual command handlers (can be called directly) */
void Cmd_Help(ShellCmd *cl);
void Cmd_Clear(ShellCmd *cl);
void Cmd_Echo(ShellCmd *cl);
void Cmd_Version(ShellCmd *cl);
void Cmd_SysInfo(ShellCmd *cl);
void Cmd_MemInfo(ShellCmd *cl);
void Cmd_CpuInfo(ShellCmd *cl);
void Cmd_History(ShellCmd *cl);
void Cmd_Color(ShellCmd *cl);
void Cmd_Calc(ShellCmd *cl);
void Cmd_Ascii(ShellCmd *cl);
void Cmd_Uptime(ShellCmd *cl);
void Cmd_Banner(ShellCmd *cl);
void Cmd_Hex(ShellCmd *cl);
void Cmd_Peek(ShellCmd *cl);
void Cmd_Poke(ShellCmd *cl);
void Cmd_Cls(ShellCmd *cl);
void Cmd_Reboot(ShellCmd *cl);
void Cmd_Halt(ShellCmd *cl);
void Cmd_Panic(ShellCmd *cl);