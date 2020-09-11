
#include "msoftcon.h"
#include "elevator_app.h"
HANDLE hConsole;         //��������� ����������� ������ 
char fill_char;          //������ ����������
//---------------------------------------------------------
void init_graphics(int consX,int consY)
{
	//system("mode con cols=100 lines=40");
	COORD console_size = { consX, consY };
	if (consY > 44) consY = 44;
	System::Console::SetWindowSize(consX, consY);

	//������� ����� �����/������ �� ������� 
	hConsole = CreateFile(L"CONOUT$", GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		0L, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0L);

	//���������� ������ ������  
	SetConsoleScreenBufferSize(hConsole, console_size);
	//����� ����� �� ������� 
	SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));

	fill_char = '\xDB';  //���������� �� ���������
	clear_screen();
}
//---------------------------------------------------------
void set_color(color foreground, color background)
{
	SetConsoleTextAttribute(hConsole,
		(WORD)((background << 4) | foreground));
}  //����� setcolor()

/* 0  ������          8  �����-�����
   1  �����-�����     9  �����
   2  �����-�������   10 �������
   3  �����-�������   11 ����
   4  �����-�������   12 �������
   5  �����-����      13 ����
   6  ����������      14 ������
   7  ������-�����    15 �����
*/
//---------------------------------------------------------
void set_cursor_pos(int x, int y)
   {
   COORD cursor_pos;            //������ � ������� ����� 
   cursor_pos.X = x - 1;        //Windows �������� � (0, 0)
   cursor_pos.Y = y - 1;        //�� ������ � (1, 1)
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
void clear_line()                 //������� �� ����� ������
   {                              //60 ��������
   //.....1234567890123456789012345678901234567890
   //.....0........1.........2.........3.........4 
   _cputs("                              ");
   _cputs("                              ");
   }