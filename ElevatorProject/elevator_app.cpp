#include"elevator.h"
using namespace CallGeneration;

int main()
{
	const int ERROR_LIMIT = 10;

	ElevatorG theElevatorSystem;

	//обнулить рандом и дать ссылку на лифт
	Man::Initialization(&theElevatorSystem);

	int ErrorCounter = 0;
	bool result = true;
	do {
		try {
			//отобразить лифт
			theElevatorSystem.SystemDisplay();

			//перевести пассажиров в следующий момент времени
			Man::PeopleTick();

			//перевести лифт в следующее состояние
			//в случае неудачи закончить программу
			result = theElevatorSystem.SystemTick();
		}
		catch (const ElevatorError& e)
		{
			cout << e.what() << endl;
			if (e.getErrorId() < 3) {
				ErrorCounter++;
			}
			else {
				ErrorCounter = ERROR_LIMIT;
			}
		}
		catch (const exception& e) {
			cout << e.what() << endl;
			ErrorCounter = ERROR_LIMIT;
		}
		catch (...) {
			ErrorCounter = ERROR_LIMIT;
			cout << "Error";
		}
	} while (result && ErrorCounter<ERROR_LIMIT);

	return 0;
}






//не пробьет ли крышу???
//можно сделать обработку исключений
// например если целью стал несуществующий этаж

//добавить проверку на существание этажа и 
//направления в функции принятия заказов и вызовов


//сделать отображение подвальных этажей

//технические (запретные)0

//идея с сохранением состояния в файл

//ускорение времени