//#pragma once
#include "msoftcon.h".
#include"elevator_app.h"
#include <iostream>
#include <conio.h>
#include <iomanip>
#include <fstream>
#include<list>
#include <ctime>
using namespace std;

//два вида кнопок на этажах: вверх и вниз
enum buttonDirectionType { bUp, bDown };
//направления движения для кабинок лифта
enum movingDirectionType { up, down, no };
//все возможные состояния кабинок лифта
enum carStateType {
	startsUp, movingUp, stopsUp, doorOpening, stopped,
	doorClosing, startsDown, movingDown, stopsDown,
	notUsed, slowMovingUp, slowMovingDown
};

//структура для кнопок на этажах`
struct CallButton
{
	int floorNumber;
	buttonDirectionType direction;
	CallButton() :floorNumber(-1) {}
	CallButton(int fn, buttonDirectionType d) 
		:floorNumber(fn), direction(d) {}
	bool operator==(const CallButton cb2)const
	{
		return (floorNumber == cb2.floorNumber 
			&& direction == cb2.direction);
	}
};

class ElevatorError: public exception
{
private:
	int id;
public:
	ElevatorError(int id, const string error);
	int getErrorId()const;
};



class ElevatorCar;

class Elevator
{
private:
	bool* callsDown;	// кнопки вниз на этажах
	bool* callsUp;		//кнопки вверх на этажах
protected:
	ElevatorCar** cars;	//массив с ссылками на все кабинки лифтов
public:
	const int floorCount;	//число этажей
	const int carCount;		//число кабинок лифта

private:
	//является ли кнопка целью другой кабинки? 
	bool theCallIsTargetOfAnotherElevators(const CallButton cb)const; //new

public:
	Elevator(bool key = 1);  //key=0  для наследования
	virtual ~Elevator();

	//обработать вызов с этажа
	//если вызов может быть обслужен в этот же момент, вернуть false
	virtual bool SetTheCall(const CallButton);

	//включена ли кнопка на этаже?
	bool theCallButtonIsOn(const CallButton)const;

	//получить кабинку лифта по его номеру
	ElevatorCar*const GetCar(const int carNum)const; //new

	//найти самую ближайшую задачу для кабинки лифта,
	//с учетом всех остальных кабинок
	CallButton FindTask(const ElevatorCar*const theCar)const;

	//выключить кнопку с этажа
	virtual void CallButtonOff(const CallButton);//new

	// перевести систему в следующее состояние
	//factor - коэффициент времени задержки, относительно TIME_INTERVAL 
	void SystemTick(double factor = 1.0);

	//найти кабинку, которая уже приехала на вызов и вернуть ее номер
	int FindReadyCar(const CallButton cb)const;

	void CheckButton(const CallButton cb)const;
};

class ElevatorCar //кабинка лифта
{
private:
	bool* orderFloors;			//массив с кнопками заказа этажей 
	int timer;			//время нахождения в текущем состоянии

protected:
	const int carNumber;				//номер кабинки
	Elevator*const parentElev;			//родительский лифт
	const int floorCount;				//общее число этажей
	int currentFloor;					//текущий этаж
	movingDirectionType movingDirection; //направление движения
	carStateType state;					//состояние
	int targetFloor;					// целевой этаж
	bool targetCallAvailability;		//наличие целевого вызова
	CallButton targetCall;				//целевой вызов

private:

	//рассчитать время обработки всех заказов выше этажа pos,
	//если вызов тоже выше этажа, то вернуть true
	const bool CalculateHigher
	(const CallButton cb, double& ariveTime, int& pos)const;

	//рассчитать время обработки всех заказов ниже этажа pos,
	//если вызов тоже ниже этажа, то вернуть true
	const bool CalculateLower
	(const CallButton cb, double& ariveTime, int& pos)const;

protected:
	//определить целевой этаж для кабинки,
	//если целевого этажа нет, вернуть false
	const bool DeterminateTargetFloor(); //new

	//вычислить минимальное число этажей, которое
	//требуется кабинки для торможения
	const int GetBrakingFloor()const;

public:
	ElevatorCar(int nElevator, Elevator* ptrElevSys);
	ElevatorCar(ElevatorCar& elev);
	virtual ~ElevatorCar();

	//получить номер кабинки
	const int GetCarNumber()const;
	//получить текущий этаж кабинки
	const int GetCurrentFloor()const;
	//получить направление движения кабинки
	const movingDirectionType GetMovingDirection()const;
	//получить состояние кабинки
	const carStateType GetState()const;
	//является ли этаж заказанным?
	const bool isOrderFloor(const int floor)const;
	//получить целевой этаж
	const int GetTargetFloor()const;
	//есть ли вызов,являющийся целью кабинки
	const bool GetTargetCallAvailability()const;
	//целевой вызов для кабинки
	const CallButton GetTargetCall()const;
	//свободна ли кабинка?
	const bool IsFree()const;

	//открыть двери
	void OpenDoor();

	//закрыть двери
	void CloseDoor();

	//установить заказ на этаж
	//если кабинка уже открыта на этом этаже, вернуть false
	virtual bool SetOrderFloor(const int nFloor);

	//установить целевой этаж
	void SetTargetFloor(CallButton cb);

	//выключить кнопку в кабинке или на этаже
	virtual void TurnOffButton();

	//первести кабинку в следующее состояние
	virtual void Tick(); //new

	//перевести кабинку в будущее состояние, в timer
	//записать время затраченное для прибытие на вызов 
	void MoveElevator(const CallButton cb, const double time);

	//вызов находится между кабинкой и целевым этажом?
	const bool CallIsBetweenCarAndTargetFloor(const CallButton cb)const; //new

	//эта кабинка между другой кабинкой и вызовом? 
	const bool IsBetweenCarAndCall
	(const ElevatorCar*const elev, const CallButton cb)const ;

	//целевой этаж находится раньше, чем вызов?
	const bool TargetIsEarlierThanCall(const CallButton cb)const;

	//расчитаь время прибытия кабинки на вызов с учетом, 
	//что она выполнит все свои заказы и вызовы по пути
	const double CalculateTheCallAriveTime(const CallButton cb)const;

	//расчитать время прибытия кабинки на вызов,
	//игнориря все другие вызовы и заказы
	const double MinCalculateTheCallAriveTime(const CallButton cb)const ;

	//получить вызов increment по счету от первого этажа,
	//на котором может остановиться кабинка.
	//Если increment больше чем всего кнопок на этажах
	//то будет возвращен пустой вызов
	const CallButton NextFloor(int increment)const;

	void CheckFloor(const int floor)const;
};

class ElevatorCarG;

//лифт с графическим интерфейсом
class ElevatorG : public Elevator
{
private:
	ElevatorCarG** carsG;	//массив с ссылками на все кабинки лифтов
	long long runningTime;	//время работы лифта
	string callLogName;		//имя журнала вызовов
	ofstream callLogFile;	//поток файла журнала вызовов
	bool stateFileLog;		//доступен или не доступен
	int callGenerationLevel;//интенсивность генерации пассажиров [0..9]
	int speedID;			//вариант скорости модуляции лифта
public:
	ElevatorG();
	~ElevatorG();

	//включить графику
	void GraphicsON();

	//отобразить на экране состояние лифта в текущий момент времени
	void SystemDisplay()const;

	// перевести систему в следующее состояние
	// возвращает false в случае необходимости завершить программу
	bool SystemTick();

	//получить задачу для лифта через диалог с пользователем,
	//если получено указание на завершение программы вернет false
	const bool GetDirections();

	//обработать вызов с этажа
	//если вызов может быть обслужен в этот же момент, вернуть false
	bool SetTheCall(const CallButton) override;

	//выключить кнопку с этажа
	void CallButtonOff(const CallButton) override;//new

	//записать в журнал заказ из кабинки
	void WriteCarOrder(int idCar, int orderFloor);

	//вывести уровень генеации пассажиров на экран
	void DisplayGenerationLevel()const;

	//получить уровень генерации пассажиров
	int GetGenerationLevel()const;

	//вывести скорость моделирования лифта
	void DisplaySpeed()const;

	//изменить доступность файла журнала вызовов
	void UpdateFileLogState(bool fstate);
};

//кабинка с графическим интерфейсом
class ElevatorCarG : public ElevatorCar
{
private:
	ElevatorG*const parentElevG;
	int lastfloor;	//этаж, на котором лифт находился
					//в прошлый момент времени
public:
	ElevatorCarG(int nElevator, ElevatorG* ptrElevSys);

	//установить заказ на этаж
	//если кабинка уже открыта на этом этаже, вернуть false
	bool SetOrderFloor(const int nFloor) override;

	//выключить кнопку в кабинке или на этаже
	void TurnOffButton() override;

	//первести кабинку в следующее состояние
	void Tick() override; 

	//отобразить кабинку на экране
	void CarDisplay()const;
};


namespace CallGeneration
{
	class Man	//Пассажир лифта
	{
	private:
		//состояния пассажиров
		enum manState { wait, entry, going };
		//wait - вызвал лифт и ждет прибития
		//entry - заходит в кабинку и нажимает кнопку
		//going - едет на свой этаж

		manState state;	//состояние пассажира
		CallButton cb;	//вызов, который сделал пассажир
		int carEntryTime;	//время нажатия на кнопку заказа
		int carNumber;	//номер кабинки, приехавшей на вызов
		int order;	//заказ этажа в лифте

		//лифт, с которым взаимодействует пассажир
		static ElevatorG* elev;
		static list<Man*> people; //список всех пассажиров
	public:
		Man();	//установить вызов
		void SetOrder(); //установить заказ

		//перевести пассажира в следующее состояние(если он
		//уже сделал заказ, то вернется false - можно удалять
		bool Tick(); 

		//подготовка списка пассажиров
		static void Initialization(ElevatorG* el);

		//перевести всех пассажиров в следующее состояние
		static void PeopleTick();

		//очистить список пассажиров
		static void RemovePeople();
	};
}