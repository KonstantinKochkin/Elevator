
#include "msoftcon.h"
#include "elevator_app.h"
HANDLE hConsole;         //поддержка консольного режима 
char fill_char;          //символ заполнения
//---------------------------------------------------------
void init_graphics(int consX,int consY)
{
	//system("mode con cols=100 lines=40");
	COORD console_size = { consX, consY };
	if (consY > 44) consY = 44;
	System::Console::SetWindowSize(consX, consY);

	//открыть канал ввода/вывода на консоль 
	hConsole = CreateFile(L"CONOUT$", GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		0L, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0L);

	//установить размер экрана  
	SetConsoleScreenBufferSize(hConsole, console_size);
	//текст белым по черному 
	SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));

	fill_char = '\xDB';  //заполнение по умолчанию
	clear_screen();
}
//---------------------------------------------------------
void set_color(color foreground, color background)
{
	SetConsoleTextAttribute(hConsole,
		(WORD)((background << 4) | foreground));
}  //конец setcolor()

/* 0  Черный          8  Темно-серый
   1  Темно-синий     9  Синий
   2  Темно-зеленый   10 Зеленый
   3  Темно-голубой   11 Алый
   4  Темно-красный   12 красный
   5  Темно-алый      13 алый
   6  Коричневый      14 Желтый
   7  Светло-серый    15 Белый
*/
//---------------------------------------------------------
void set_cursor_pos(int x, int y)
   {
   COORD cursor_pos;            //Начало в верхнем левом 
   cursor_pos.X = x - 1;        //Windows начинает с (0, 0)
   cursor_pos.Y = y - 1;        //мы начнем с (1, 1)
   SetConsoleCursorPosition(hConsole, cursor_pos);
   }
//---------------------------------------------------------
void clear_screen()
   {
   set_cursor_pos(1, 25);
   for(int j=0; j<25; j++)
      _putch('\n');
   set_cursor_pos(1, 1);
   }
//---------------------------------------------------------
void wait(int milliseconds)
   {
   Sleep(milliseconds);
   }
//---------------------------------------------------------
void clear_line()                 //очистка до конца строки
   {                              //60 пробелов
   //.....1234567890123456789012345678901234567890
   //.....0........1.........2.........3.........4 
   _cputs("                              ");
   _cputs("                              ");
   }