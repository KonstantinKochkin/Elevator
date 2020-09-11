//ќбъ€влени€ функций консольной графики от Lafore
//используютс€ консольные функции Windows 

#ifndef _INC_WCONSOLE    //этот файл не должен включатьс€
#define _INC_WCONSOLE    //дважды в тот же исходный файл

#include <windows.h>     //дл€ консольных функций Windows
#include <conio.h>       //дл€ kbhit(), getche()

enum color {
   cBLACK=0,     cDARK_BLUE=1,  cDARK_GREEN=2, DARK_CYAN=3, 
   cDARK_RED=4,  cDARK_MAGENTA=5, cBROWN=6,  cLIGHT_GRAY=7,
   cDARK_GRAY=8, cBLUE=9,         cGREEN=10,     cCYAN=11, 
   cRED=12,      cMAGENTA=13,     cYELLOW=14,    cWHITE=15 };
//---------------------------------------------------------
void init_graphics(int consX=80, int consY=25);
void set_color(color fg, color bg = cBLACK);
void set_cursor_pos(int x, int y);
void clear_screen();
void wait(int milliseconds);
void clear_line();
#endif /* _INC_WCONSOLE */