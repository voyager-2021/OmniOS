/* ============================================================
   OmniOS - Package implementations + PKG_Init
   ============================================================ */
#include "cmd_helpers.h"

/* ---- neofetch ---- */
static void neofetch_run(ShellCmd *cl)
{
    (void)cl;
    uint32_t ticks=g_tick_count,secs=ticks/18,mins=secs/60,hrs=mins/60;
    int installed=0;
    for(int i=0;i<g_package_count;i++) if(g_packages[i].installed) installed++;

    static const char *logo[]={"     .---.      ","    /     \\     ","   |  O O  |    ",
        "   |  ___  |    ","    \\_____/     ","   /|     |\\    ","  / |     | \\   ",
        " /  |     |  \\  ","    |     |     ","    |_   _|     ","    | | | |     ","    |_| |_|     "};
    static const char *keys[]={"OS","Version","Build","Arch","CPU Mode","Shell",
        "Display","Keyboard","Uptime","Packages","Memory","Disk"};

    char ubuf[32]; snprintf(ubuf,32,"%02u:%02u:%02u",hrs,mins%60,secs%60);
    char pbuf[32]; snprintf(pbuf,32,"%d/%d installed",installed,g_package_count);

    static const char *vs[]={"OmniOS",OMNIOS_VERSION,OMNIOS_BUILD,"x86 (i686) 32-bit",
        "Protected Mode","omni-shell 1.0","VGA Text 80x25","PS/2 IRQ1",
        NULL,NULL,"Extended (see meminfo)","FAT32 250MB"};
    const char *vals[12]; for(int i=0;i<12;i++) vals[i]=vs[i]; vals[8]=ubuf; vals[9]=pbuf;

    puts("\n");
    for(int i=0;i<12;i++){
        VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_CYAN,VGA_COLOR_BLACK));
        puts("  "); puts(logo[i]);
        VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN,VGA_COLOR_BLACK));
        puts(keys[i]); int pad=12-cmd_strlen(keys[i]); for(int j=0;j<pad;j++) putc(' ');
        COLOR_RESET(); puts(": "); puts(vals[i]); putc('\n');
    }
    puts("  "); puts("                 ");
    for(int c=0;c<8;c++){VGA_SetColor(VGA_MAKE_COLOR(c,c));puts("   ");}
    COLOR_RESET(); puts("\n  "); puts("                 ");
    for(int c=8;c<16;c++){VGA_SetColor(VGA_MAKE_COLOR(c,c));puts("   ");}
    COLOR_RESET(); puts("\n\n");
}

/* ---- fortune ---- */
static void fortune_run(ShellCmd *cl)
{
    (void)cl;
    static const char *f[]={"The best way to predict the future is to invent it. - Alan Kay",
        "Talk is cheap. Show me the code. - Linus Torvalds",
        "Any sufficiently advanced technology is indistinguishable from magic.",
        "First, solve the problem. Then, write the code.",
        "Make it work, make it right, make it fast. - Kent Beck",
        "It works on my machine. - Every developer ever",
        "99 bugs in the code. Take one down, patch it... 127 bugs in the code.",
        "OmniOS: Because every OS starts with a single putc()."};
    int n=(int)(sizeof(f)/sizeof(f[0]));
    VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_CYAN,VGA_COLOR_BLACK));
    puts("\n  \""); puts(f[pkg_rand()%n]); puts("\"\n\n"); COLOR_RESET();
}

/* ---- cowsay ---- */
static void cowsay_run(ShellCmd *cl)
{
    if(cl->argc<2){cprintln("  Usage: cowsay <message>",VGA_COL_WARNING);return;}
    static char msg[200]; int pos=0;
    for(int i=1;i<cl->argc&&pos<198;i++){for(int j=0;cl->argv[i][j]&&pos<198;j++)msg[pos++]=cl->argv[i][j];if(i<cl->argc-1&&pos<198)msg[pos++]=' ';}
    msg[pos]='\0';
    puts("   "); for(int i=0;i<pos+2;i++)putc('_');
    puts("\n  < "); puts(msg); puts(" >\n   "); for(int i=0;i<pos+2;i++)putc('-');
    puts("\n          \\   ^__^\n           \\  (oo)\\_______\n              (__)\\       )\\/\\\n                  ||----w |\n                  ||     ||\n");
}

/* ---- matrix ---- */
static void matrix_run(ShellCmd *cl)
{
    (void)cl;
    VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN,VGA_COLOR_BLACK));
    puts("  Matrix rain... press any key to stop.\n\n");
    for(int f=0;f<80;f++){for(int c=0;c<VGA_WIDTH;c++){int r=pkg_rand();if(r%3==0){VGA_SetColor(VGA_MAKE_COLOR((r%5==0)?VGA_COLOR_WHITE:VGA_COLOR_LIGHT_GREEN,VGA_COLOR_BLACK));putc('!'+(r%94));}else putc(' ');}pkg_delay(1);if(KB_KeyAvailable()){KB_TryGetChar();break;}}
    COLOR_RESET(); puts("\n\n");
}

/* ---- figlet ---- */
static void figlet_run(ShellCmd *cl)
{
    if(cl->argc<2){cprintln("  Usage: figlet <text>",VGA_COL_WARNING);return;}
    COLOR_SET(VGA_COL_HEADER); puts("\n");
    for(int row=0;row<5;row++){puts("  ");for(int i=1;i<cl->argc;i++){for(int j=0;cl->argv[i][j];j++){char c=cl->argv[i][j];if(c>='a'&&c<='z')c-=32;if((c>='A'&&c<='Z')||(c>='0'&&c<='9')){switch(row){case 0:case 4:putc('#');putc('#');putc('#');break;case 1:case 3:putc('#');putc(' ');putc('#');break;case 2:if(c=='A'||c=='B'||c=='E'||c=='F'||c=='H'||c=='P'||c=='R'||c=='S'||c=='8'||c=='0'){putc('#');putc('#');putc('#');}else{putc('#');putc(' ');putc('#');}break;}}else if(c==' ')puts("   ");else{putc(c);puts("  ");}putc(' ');}putc(' ');}putc('\n');}
    puts("\n"); COLOR_RESET();
}

/* ---- snake ---- */
static void snake_run(ShellCmd *cl)
{
    (void)cl; cprintln("  Snake v1.0 - WASD move, Q quit\n",VGA_COL_WARNING);
    #define FW 20
    #define FH 10
    #define MS 50
    int sx[MS],sy[MS],slen=3,dx=1,dy=0,fx,fy,score=0; bool running=true;
    for(int i=0;i<slen;i++){sx[i]=5-i;sy[i]=5;} fx=pkg_rand()%FW; fy=pkg_rand()%FH;
    while(running){puts("  ");for(int i=0;i<FW+2;i++)putc('-');putc('\n');
        for(int y=0;y<FH;y++){puts("  |");for(int x=0;x<FW;x++){bool is=false;for(int s=0;s<slen;s++)if(sx[s]==x&&sy[s]==y){is=true;break;}
            if(x==sx[0]&&y==sy[0]){VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_WHITE,VGA_COLOR_BLACK));putc('@');}
            else if(is){VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN,VGA_COLOR_BLACK));putc('o');}
            else if(x==fx&&y==fy){VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_RED,VGA_COLOR_BLACK));putc('*');}
            else{COLOR_RESET();putc(' ');}}COLOR_RESET();puts("|\n");}
        puts("  ");for(int i=0;i<FW+2;i++)putc('-');putc('\n');
        COLOR_SET(VGA_COL_INFO);printf("  Score: %d  Len: %d\n",score,slen);COLOR_RESET();
        uint32_t dl=g_tick_count+4;while(g_tick_count<dl){char k=KB_TryGetChar();if(k=='w'||k=='W'){if(dy!=1){dx=0;dy=-1;}break;}if(k=='s'||k=='S'){if(dy!=-1){dx=0;dy=1;}break;}if(k=='a'||k=='A'){if(dx!=1){dx=-1;dy=0;}break;}if(k=='d'||k=='D'){if(dx!=-1){dx=1;dy=0;}break;}if(k=='q'||k=='Q'){running=false;break;}__asm__ volatile("hlt");}
        if(!running)break; int nx=sx[0]+dx,ny=sy[0]+dy;
        if(nx<0||nx>=FW||ny<0||ny>=FH){COLOR_SET(VGA_COL_ERROR);printf("  Game Over! Score: %d\n",score);COLOR_RESET();break;}
        for(int s=0;s<slen;s++)if(sx[s]==nx&&sy[s]==ny){COLOR_SET(VGA_COL_ERROR);printf("  Game Over! Score: %d\n",score);COLOR_RESET();running=false;break;}
        if(!running)break; bool ate=(nx==fx&&ny==fy);
        if(!ate){for(int s=slen-1;s>0;s--){sx[s]=sx[s-1];sy[s]=sy[s-1];}}
        else{if(slen<MS){for(int s=slen;s>0;s--){sx[s]=sx[s-1];sy[s]=sy[s-1];}slen++;}score+=10;fx=pkg_rand()%FW;fy=pkg_rand()%FH;}
        sx[0]=nx;sy[0]=ny;}
    #undef FW
    #undef FH
    #undef MS
}

/* ---- sysmon ---- */
static void sysmon_run(ShellCmd *cl){(void)cl;cprintln("  System Monitor - Q to exit\n",VGA_COL_HEADER);bool r=true;while(r){uint32_t t=g_tick_count,s=t/18,m=s/60,h=m/60;COLOR_SET(VGA_COL_INFO);printf("  Uptime: %02u:%02u:%02u | Ticks: %-10u\r",h,m%60,s%60,t);uint32_t dl=g_tick_count+9;while(g_tick_count<dl){char k=KB_TryGetChar();if(k=='q'||k=='Q'){r=false;break;}__asm__ volatile("hlt");}}puts("\n\n");COLOR_RESET();}

/* ---- editor ---- */
static void editor_run(ShellCmd *cl){(void)cl;cprintln("  OmniOS Editor v1.0 - ESC to exit",VGA_COL_HEADER);VGA_DrawHLine('-',VGA_WIDTH,VGA_COL_NORMAL);COLOR_RESET();while(1){char c=KB_GetChar();if((unsigned char)c==0x95)break;if(c=='\b')VGA_PutChar('\b');else if(c=='\n'||c=='\r')putc('\n');else if((unsigned char)c>=0x20&&(unsigned char)c<0x80)putc(c);}puts("\n");VGA_DrawHLine('-',VGA_WIDTH,VGA_COL_NORMAL);cprintln("  Editor closed.",VGA_COL_INFO);}

/* ---- beep ---- */
static void beep_run(ShellCmd *cl){int f=1000,d=3;if(cl->argc>=2)f=cmd_atoi(cl->argv[1]);if(cl->argc>=3)d=cmd_atoi(cl->argv[2]);if(f<20)f=20;if(f>20000)f=20000;if(d<1)d=1;if(d>36)d=36;COLOR_SET(VGA_COL_INFO);printf("  Beep: %d Hz\n",f);COLOR_RESET();spk_on(f);pkg_delay(d);spk_off();}

/* ---- piano ---- */
static void piano_run(ShellCmd *cl){(void)cl;cprintln("  Piano - Q quit, ASDFGHJK = notes\n",VGA_COL_HEADER);
    static const struct{char k;int f;const char*n;}notes[]={{'a',262,"C4"},{'w',277,"C#"},{'s',294,"D4"},{'e',311,"D#"},{'d',330,"E4"},{'f',349,"F4"},{'t',370,"F#"},{'g',392,"G4"},{'y',415,"G#"},{'h',440,"A4"},{'u',466,"A#"},{'j',494,"B4"},{'k',523,"C5"}};
    int num=13;while(1){char c=KB_GetChar();if(c=='q'||c=='Q')break;if(c>='A'&&c<='Z')c+=32;for(int i=0;i<num;i++)if(c==notes[i].k){COLOR_SET(VGA_COL_SUCCESS);printf("  %s (%d Hz)  \r",notes[i].n,notes[i].f);COLOR_RESET();spk_on(notes[i].f);pkg_delay(3);spk_off();break;}}spk_off();puts("\n  Piano closed.\n");}

/* ---- hangman ---- */
static void hangman_run(ShellCmd *cl){(void)cl;static const char*words[]={"kernel","memory","driver","shell","timer","stack","queue","interrupt","register","binary","compile","linker","buffer","cache","thread","process","segment","bootloader","pixel","keyboard"};
    int wc=20;const char*word=words[pkg_rand()%wc];int wlen=cmd_strlen(word);char guessed[32]={0};int gc=0;char wrong[16]={0};int wn=0;int mx=6;
    cprintln("  Hangman - guess the word!\n",VGA_COL_HEADER);
    while(wn<mx){puts("   +---+\n   |   ");putc(wn>=1?'O':' ');puts("\n   |  ");putc(wn>=3?'/':' ');putc(wn>=2?'|':' ');putc(wn>=4?'\\':' ');puts("\n   |  ");putc(wn>=5?'/':' ');putc(' ');putc(wn>=6?'\\':' ');puts("\n   +===+\n\n");
        COLOR_SET(VGA_COL_SUCCESS);puts("  Word: ");bool complete=true;for(int i=0;i<wlen;i++){bool f=false;for(int g=0;g<gc;g++)if(guessed[g]==word[i]){f=true;break;}if(f){putc(word[i]);putc(' ');}else{putc('_');putc(' ');complete=false;}}puts("\n");
        if(complete){VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN,VGA_COLOR_BLACK));printf("\n  You won! Word: %s\n\n",word);COLOR_RESET();return;}
        if(wn>0){COLOR_SET(VGA_COL_ERROR);puts("  Wrong: ");for(int i=0;i<wn;i++){putc(wrong[i]);putc(' ');}puts("\n");}
        COLOR_SET(VGA_COL_INFO);printf("  Tries: %d  Guess: ",mx-wn);COLOR_RESET();char c=KB_GetChar();if(c>='A'&&c<='Z')c+=32;if(c<'a'||c>'z')continue;putc(c);puts("\n\n");
        bool dup=false;for(int g=0;g<gc;g++)if(guessed[g]==c){dup=true;break;}for(int w=0;w<wn;w++)if(wrong[w]==c){dup=true;break;}if(dup){cprintln("  Already guessed!\n",VGA_COL_WARNING);continue;}
        bool in=false;for(int i=0;i<wlen;i++)if(word[i]==c){in=true;break;}if(in){if(gc<31)guessed[gc++]=c;}else{if(wn<15)wrong[wn++]=c;}}
    COLOR_SET(VGA_COL_ERROR);printf("\n  You lost! Word: %s\n\n",word);COLOR_RESET();}

/* ---- tictactoe ---- */
static void tictactoe_run(ShellCmd *cl){(void)cl;char b[9]={' ',' ',' ',' ',' ',' ',' ',' ',' '};bool pt=true;int mv=0;
    cprintln("  Tic-Tac-Toe - You=X, CPU=O. 1-9:\n",VGA_COL_HEADER);
    #define CW(b,c) ((b[0]==c&&b[1]==c&&b[2]==c)||(b[3]==c&&b[4]==c&&b[5]==c)||(b[6]==c&&b[7]==c&&b[8]==c)||(b[0]==c&&b[3]==c&&b[6]==c)||(b[1]==c&&b[4]==c&&b[7]==c)||(b[2]==c&&b[5]==c&&b[8]==c)||(b[0]==c&&b[4]==c&&b[8]==c)||(b[2]==c&&b[4]==c&&b[6]==c))
    while(mv<9){for(int r=0;r<3;r++){puts("   ");for(int c=0;c<3;c++){char ch=b[r*3+c];if(ch=='X')VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN,VGA_COLOR_BLACK));else if(ch=='O')VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_RED,VGA_COLOR_BLACK));else COLOR_RESET();putc(' ');putc(ch);putc(' ');COLOR_RESET();if(c<2)putc('|');}putc('\n');if(r<2)puts("  ---+---+---\n");}puts("\n");
        if(pt){COLOR_SET(VGA_COL_INFO);puts("  Move (1-9): ");COLOR_RESET();char k;int p;while(1){k=KB_GetChar();if(k>='1'&&k<='9'){p=k-'1';if(b[p]==' ')break;}}putc(k);putc('\n');b[p]='X';}
        else{int p=-1;for(int i=0;i<9&&p<0;i++){if(b[i]==' '){b[i]='O';if(CW(b,'O'))p=i;b[i]=' ';}}for(int i=0;i<9&&p<0;i++){if(b[i]==' '){b[i]='X';if(CW(b,'X'))p=i;b[i]=' ';}}if(p<0&&b[4]==' ')p=4;int cn[]={0,2,6,8};for(int i=0;i<4&&p<0;i++)if(b[cn[i]]==' ')p=cn[i];for(int i=0;i<9&&p<0;i++)if(b[i]==' ')p=i;b[p]='O';COLOR_SET(VGA_COL_INFO);printf("  CPU plays %d\n\n",p+1);COLOR_RESET();}
        mv++;if(CW(b,'X')){cprintln("  You win!\n",VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN,VGA_COLOR_BLACK));return;}if(CW(b,'O')){cprintln("  CPU wins!\n",VGA_COL_ERROR);return;}pt=!pt;}
    cprintln("  Draw!\n",VGA_COL_WARNING);
    #undef CW
}

/* ---- mandelbrot ---- */
static void mandelbrot_run(ShellCmd *cl){(void)cl;cprintln("  Mandelbrot Set\n",VGA_COL_HEADER);
    #define FS 12
    #define FO (1<<FS)
    #define FM(a,b) (((a)*(b))>>FS)
    static const char sh[]=" .:-=+*#%@";uint8_t co[]={0,1,2,3,4,5,6,7,15,14};int mi=10;
    for(int r=0;r<22;r++){for(int c=0;c<78;c++){int cr=-2*FO+(c*3*FO)/78,ci=-1*FO+(r*2*FO)/22,zr=0,zi=0,it=0;while(it<mi){int z2=FM(zr,zr),i2=FM(zi,zi);if(z2+i2>4*FO)break;int nr=z2-i2+cr;zi=2*FM(zr,zi)+ci;zr=nr;it++;}int si=(it*9)/mi;if(si>9)si=9;VGA_SetColor(VGA_MAKE_COLOR(co[si],VGA_COLOR_BLACK));putc(sh[si]);}putc('\n');}
    #undef FS
    #undef FO
    #undef FM
    puts("\n");COLOR_RESET();}

/* ---- clock ---- */
static void clock_run(ShellCmd *cl){(void)cl;cprintln("  Clock - Q to exit\n",VGA_COL_HEADER);
    static const char*d[10][5]={{"###","# #","# #","# #","###"},{"  #","  #","  #","  #","  #"},{"###","  #","###","#  ","###"},{"###","  #","###","  #","###"},{"# #","# #","###","  #","  #"},{"###","#  ","###","  #","###"},{"###","#  ","###","# #","###"},{"###","  #","  #","  #","  #"},{"###","# #","###","# #","###"},{"###","# #","###","  #","###"}};
    while(1){uint32_t s=g_tick_count/18,m=s/60,h=m/60;int td[6]={(h/10)%10,h%10,(m%60)/10,(m%60)%10,(s%60)/10,(s%60)%10};for(int r=0;r<5;r++){puts("     ");for(int i=0;i<6;i++){VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN,VGA_COLOR_BLACK));puts(d[td[i]][r]);COLOR_RESET();if(i==1||i==3){if(r==1||r==3)puts(" : ");else puts("   ");}else putc(' ');}putc('\n');}puts("\n");
        uint32_t dl=g_tick_count+18;bool q=false;while(g_tick_count<dl){char k=KB_TryGetChar();if(k=='q'||k=='Q'){q=true;break;}__asm__ volatile("hlt");}if(q)break;}
    COLOR_RESET();puts("  Clock stopped.\n");}

/* ---- passwords ---- */
static void passwords_run(ShellCmd *cl){int l=16,c=5;if(cl->argc>=2)l=cmd_atoi(cl->argv[1]);if(cl->argc>=3)c=cmd_atoi(cl->argv[2]);if(l<4)l=4;if(l>64)l=64;if(c<1)c=1;if(c>20)c=20;
    static const char cs[]="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+";int cl2=cmd_strlen(cs);
    COLOR_SET(VGA_COL_HEADER);printf("  %d passwords (len %d):\n\n",c,l);for(int p=0;p<c;p++){COLOR_SET(VGA_COL_INFO);printf("  %2d: ",p+1);COLOR_SET(VGA_COL_SUCCESS);for(int i=0;i<l;i++)putc(cs[pkg_rand()%cl2]);putc('\n');}puts("\n");COLOR_RESET();}

/* ---- rot13 ---- */
static void rot13_run(ShellCmd *cl){if(cl->argc<2){cprintln("  Usage: rot13 <text>",VGA_COL_WARNING);return;}COLOR_SET(VGA_COL_SUCCESS);puts("  ");for(int i=1;i<cl->argc;i++){for(int j=0;cl->argv[i][j];j++){char c=cl->argv[i][j];if(c>='a'&&c<='z')c='a'+(c-'a'+13)%26;else if(c>='A'&&c<='Z')c='A'+(c-'A'+13)%26;putc(c);}if(i<cl->argc-1)putc(' ');}puts("\n");COLOR_RESET();}

/* ---- wc ---- */
static void wc_run(ShellCmd *cl){if(cl->argc<2){cprintln("  Usage: wc <text>",VGA_COL_WARNING);return;}int ch=0,w=cl->argc-1;for(int i=1;i<cl->argc;i++){for(int j=0;cl->argv[i][j];j++)ch++;if(i<cl->argc-1)ch++;}COLOR_SET(VGA_COL_INFO);printf("  Words: %d  Chars: %d\n",w,ch);COLOR_RESET();}

/* ---- rev ---- */
static void rev_run(ShellCmd *cl){if(cl->argc<2){cprintln("  Usage: rev <text>",VGA_COL_WARNING);return;}static char buf[256];int pos=0;for(int i=1;i<cl->argc&&pos<254;i++){for(int j=0;cl->argv[i][j]&&pos<254;j++)buf[pos++]=cl->argv[i][j];if(i<cl->argc-1&&pos<254)buf[pos++]=' ';}buf[pos]='\0';COLOR_SET(VGA_COL_SUCCESS);puts("  ");for(int i=pos-1;i>=0;i--)putc(buf[i]);puts("\n");COLOR_RESET();}

/* ---- rand ---- */
static void rand_run(ShellCmd *cl){int lo=1,hi=100;if(cl->argc>=2)hi=cmd_atoi(cl->argv[1]);if(cl->argc>=3){lo=cmd_atoi(cl->argv[1]);hi=cmd_atoi(cl->argv[2]);}if(lo>hi){int t=lo;lo=hi;hi=t;}COLOR_SET(VGA_COL_SUCCESS);printf("  Random [%d-%d]: %d\n",lo,hi,lo+(int)(pkg_rand()%(hi-lo+1)));COLOR_RESET();}

/* ---- sleep ---- */
static void sleep_run(ShellCmd *cl){int s=1;if(cl->argc>=2)s=cmd_atoi(cl->argv[1]);if(s<1)s=1;if(s>60)s=60;COLOR_SET(VGA_COL_INFO);printf("  Sleeping %d sec...\n",s);COLOR_RESET();pkg_delay(s*18);cprintln("  Done.",VGA_COL_SUCCESS);}

/* ============================================================
   PKG_Init + registry
   ============================================================ */
Package g_packages[PKG_MAX_PACKAGES];
int     g_package_count = 0;

static void pkg_add(const char *n, const char *d, const char *v, void(*r)(ShellCmd*))
{
    if (g_package_count >= PKG_MAX_PACKAGES) return;
    Package *p = &g_packages[g_package_count++];
    cmd_strcpy(p->name, n);
    cmd_strcpy(p->description, d);
    cmd_strcpy(p->version, v);
    p->installed = false;
    p->on_install = 0;
    p->on_run = r;
}

void PKG_Init(void)
{
    g_package_count = 0;
    pkg_add("neofetch",   "System info with ASCII art",    "1.0", neofetch_run);
    pkg_add("fortune",    "Random programming quotes",     "1.0", fortune_run);
    pkg_add("cowsay",     "ASCII cow says your message",   "1.0", cowsay_run);
    pkg_add("figlet",     "Big block text renderer",       "1.0", figlet_run);
    pkg_add("rot13",      "ROT13 cipher encode/decode",    "1.0", rot13_run);
    pkg_add("wc",         "Word and character count",       "1.0", wc_run);
    pkg_add("rev",        "Reverse text",                   "1.0", rev_run);
    pkg_add("rand",       "Random number generator",        "1.0", rand_run);
    pkg_add("sleep",      "Sleep for N seconds",            "1.0", sleep_run);
    pkg_add("passwords",  "Generate random passwords",      "1.0", passwords_run);
    pkg_add("editor",     "Simple text editor (ESC quit)",  "1.0", editor_run);
    pkg_add("sysmon",     "Live system monitor (Q quit)",   "1.0", sysmon_run);
    pkg_add("clock",      "Large digital clock (Q quit)",   "1.0", clock_run);
    pkg_add("beep",       "PC speaker beep [freq] [ticks]", "1.0", beep_run);
    pkg_add("piano",      "Musical keyboard (Q quit)",      "1.0", piano_run);
    pkg_add("matrix",     "Matrix digital rain effect",     "1.0", matrix_run);
    pkg_add("mandelbrot", "ASCII Mandelbrot fractal",       "1.0", mandelbrot_run);
    pkg_add("snake",      "Classic snake game (WASD+Q)",    "1.0", snake_run);
    pkg_add("hangman",    "Word guessing game",             "1.0", hangman_run);
    pkg_add("tictactoe",  "Tic-Tac-Toe vs CPU",            "1.0", tictactoe_run);
}

int PKG_Find(const char *name){for(int i=0;i<g_package_count;i++)if(cmd_strcmp(g_packages[i].name,name)==0)return i;return -1;}
bool PKG_Install(const char *name){int i=PKG_Find(name);if(i<0)return false;g_packages[i].installed=true;if(g_packages[i].on_install)g_packages[i].on_install();return true;}
bool PKG_Uninstall(const char *name){int i=PKG_Find(name);if(i<0)return false;g_packages[i].installed=false;return true;}
bool PKG_IsInstalled(const char *name){int i=PKG_Find(name);if(i<0)return false;return g_packages[i].installed;}
bool PKG_Run(const char *name, ShellCmd *cl){int i=PKG_Find(name);if(i<0||!g_packages[i].installed||!g_packages[i].on_run)return false;g_packages[i].on_run(cl);return true;}