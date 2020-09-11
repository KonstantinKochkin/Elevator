#include "elevator.h"

const int BUF_SIZE = 5;
const int NUMBER_OF_SPEEDS = 7;
const float SPEED_VALUES[NUMBER_OF_SPEEDS] = { 0.5,0.75,1,1.5,2,3,4 };


ElevatorError::ElevatorError(int id, const string error) 
	:exception(error.c_str()), id(id)
{}
int ElevatorError::getErrorId()const {
	return id;
}

//=================================================================================
//==============================Elevator===========================================

//создать лифт
Elevator::Elevator(bool key) :
	floorCount(NUM_FLOORS), carCount(NUM_CARS)
{
	if (key)	//при наследовании будут создаваться другие кабинки
	{
		//создание массива для кабинок лифта
		cars = new ElevatorCar*[carCount];
		//создание нужного числа кабинок лифта
		for (int j = 0; j < carCount; j++) {
			cars[j] = new ElevatorCar(j + 1, this);
		}
	}

	// нужного числа кнопок на этажах
	callsDown = new bool[floorCount];
	callsUp = new bool[floorCount];

	for (int j = 0; j < floorCount; j++) {
		//выключить кнопки на этаже
		callsDown[j] = false;
		callsUp[j] = false;
	}
}

//удалить лифт
Elevator::~Elevator()
{
	for (int j = 0; j < carCount; j++) {
		delete cars[j];
	}
	delete cars;
	delete callsDown;
	delete callsUp;
}

void Elevator::CheckButton(CallButton cb)const
{
	if (cb.floorNumber >= floorCount || cb.floorNumber < 0
		|| cb.floorNumber == floorCount - 1 && cb.direction == bUp
		|| cb.floorNumber == 0 && cb.direction == bDown)
	{
		char buff1[BUF_SIZE];
		string str1 = itoa(cb.floorNumber, buff1, 10);
		string str2 = (cb.direction == bUp ? "up" : "down");
		throw ElevatorError(0,"Impossible call ("+str1+str2+")");
	}
}

//обработать вызов с этажа
//если вызов может быть обслужен в этот же момент, вернуть false
bool Elevator::SetTheCall(const CallButton cb)
{
	CheckButton(cb); //проверить кнопку на исключение

	//если такая кнопка уже нажата
	if (cb.direction == bUp && callsUp[cb.floorNumber]
		|| cb.direction == bDown && callsDown[cb.floorNumber]) {
		return false;
	}

	//если есть кабинки на этаже кнопки, которые могут 
	//сразу принять вызов, то принять вызов
	for (int j = 0; j < carCount; j++)
	{
		if (cars[j]->GetCurrentFloor() == cb.floorNumber)
		{
			carStateType eState = cars[j]->GetState();
			movingDirectionType mDirection = cars[j]->GetMovingDirection();

			//если напрваление движения кабинки подходит
			if ((mDirection == up || mDirection == no) && cb.direction == bUp
				|| (mDirection == down || mDirection == no) && cb.direction == bDown)
			{
				//если двери открываются или уже открыты
				if (eState == doorOpening || eState == stopped) {
					return false; //ничего не делаем
				}
				//если двери закрываются или закрыты
				else if (eState == doorClosing || eState == notUsed)
				{
					cars[j]->OpenDoor(); //открыть двери
					return false;
				}
			}
		}
	}

	//установить вызов в массив
	if (cb.direction == bUp) {
		callsUp[cb.floorNumber] = true;
	}
	else if (cb.direction == bDown) {
		callsDown[cb.floorNumber] = true;
	}

	//если вызов является целью какой-то кабинки, то не обрабатывать его
	for (int j = 0; j < carCount; j++)
	{
		//если у кабинки есть целевой вызов
		if (cars[j]->GetTargetCallAvailability())
		{
			//если этот вызов совпадает с текущим
			if (cars[j]->GetTargetCall() == cb) {
				return true;
			}
		}
	}


	//из всех свободных кабинок и кабинок, которые по пути к целевому 
	//этажу могут обслужить вызов, выбрать ту, которая обслужит вызов первой
	double minArivTime = -1;
	ElevatorCar* theMostQuicklyCar;

	for (int j = 0; j < carCount; j++)
	{
		//если кабинка свободна или вызов находится между кабинкой и 
		//ее целевым этажом
		if (cars[j]->IsFree() || cars[j]->CallIsBetweenCarAndTargetFloor(cb))
		{
			//подсчитать время прибытия на вызов для кабинки,
			//если она проигнорирует свой целевой этаж
			double arivTime = cars[j]->MinCalculateTheCallAriveTime(cb);
			//если это первая подходящая кабинка
			if (minArivTime == -1)
			{
				minArivTime = arivTime;
				theMostQuicklyCar = cars[j];
			}
			else //2 или 3 или ... подходящая кабинка
				if (arivTime < minArivTime)
				{
					minArivTime = arivTime;
					theMostQuicklyCar = cars[j];
				}
		}
	}

	//если есть кабинки, которые ближе к вызову чем theMostQuicklyCar,
	//но их целевой этаж раньше вызова, то найти самый быстрый из них
	if (minArivTime > -1) {	//если ранее была найдена хотябы одна
							//кабинка, которая может обработать вызов
		double minArivTimeBusyElev = -1;
		//кабинка, которая после обслуживание всех своих целевых 
		//этажей, сможет обслужить вызов быстрее всех
		ElevatorCar* theMostQuicklyBusyElevator;

		for (int j = 0; j < carCount; j++)
		{
			//если кабинка и ее целевовй этаж находятся между
			//theMostQuicklyCar и вызовом
			if (cars[j]->IsBetweenCarAndCall(theMostQuicklyCar, cb))
			{
				//посчитать время прибытия на вызов,
				//обработав свои цели
				double arivTime = cars[j]->CalculateTheCallAriveTime(cb);
				//если это первая подходящая кабинка
				if (minArivTimeBusyElev == -1)
				{
					minArivTimeBusyElev = arivTime;
					theMostQuicklyBusyElevator = cars[j];
				}
				else
					//2 или 3 или ... подходящая кабинка
					if (arivTime < minArivTimeBusyElev)
					{
						minArivTimeBusyElev = arivTime;
						theMostQuicklyBusyElevator = cars[j];
					}
			}
		}

		//если не была найдена не одина подходящая кабинка или она была
		//найдена и время ее прибытия >= чем у theMostQuicklyCar 
		if (minArivTimeBusyElev == -1 || minArivTimeBusyElev >= minArivTime) {
			//установить целевой этаж для theMostQuicklyCar
			theMostQuicklyCar->SetTargetFloor(cb);
		}
		//иначе не принимать вызов сейчас, а принять позже
	}
	return true;
}

//включена ли кнопка на этаже?
bool Elevator::theCallButtonIsOn(const CallButton cb)const
{
	CheckButton(cb); ////проверить кнопку на исключение

	//если вызов вверх, то проверить в массиве кнопок вверх
	//если вызов вниз, - в массиве кнопок вниз
	if (cb.direction == bUp && callsUp[cb.floorNumber]
		|| cb.direction == bDown && callsDown[cb.floorNumber]) {
		return true;
	}
	return false;
}

//является ли кнопка целью другой кабинки? 
bool Elevator::theCallIsTargetOfAnotherElevators(const CallButton cb)const
{
	CheckButton(cb); //проверить кнопку на исключение

	for (int j = 0; j < carCount; j++)
	{
		//если у кабинки есть целевой вызов
		if (cars[j]->GetTargetCallAvailability())
		{
			//и если он совпадает с текущим
			if (cars[j]->GetTargetCall() == cb)
				return true;
		}
	}
	return false;
}

//получить кабинку лифта по его номеру
ElevatorCar*const Elevator::GetCar(const int elevNum)const
{
	for (int j = 0; j < carCount; j++)
	{
		if (cars[j]->GetCarNumber() == elevNum) {
			return cars[j];
		}
	}
	char buff[BUF_SIZE];
	string str = itoa(elevNum,buff,10);
	throw ElevatorError(2, "No car with number " + str);
}

//найти самую ближайшую задачу для кабинки лифта,
//с учетом всех остальных кабинок
CallButton Elevator::FindTask(const ElevatorCar*const theCar)const
{
	if (theCar == 0) {
		throw ElevatorError(3,"The link points to a non-existent car");
	}

	//создадим копию всех кабинок лифта
	ElevatorCar** newCars = new ElevatorCar*[carCount];
	for (int j = 0; j < carCount; j++)
	{
		newCars[j] = new ElevatorCar(*cars[j]);
	}
	//кабинки будут менять свое состояние для расчета состояний
	//всего лифта в будущем, для этого и нужны копии кабинок

	
	int j = 0;
	//первый этаж с направлением, на котором кабинка может остановиться
	CallButton cb = theCar->NextFloor(j++);

	//пока не будут рассморены все этажи с направлением по одному разу
	while (cb.floorNumber != -1)
	{
		//если этаж является заказом
		if (theCar->isOrderFloor(cb.floorNumber))
		{
			//удаление копии кабинок
			for (int i = 0; i < carCount; i++) {
				delete newCars[i];
			}
			delete newCars;

			return cb; //вернуть этот заказ
		}

		//если кнопка, соответствющая этажу и направлению включена
		if (theCallButtonIsOn(cb))
		{
			bool isServiced = false; //вызов уже обслуживается?
			for (int elev = 0; elev < carCount; elev++)
			{
				//если у кабинки есть целевой вызов и он равен текужщему
				if (newCars[elev]->GetTargetCallAvailability() &&
					newCars[elev]->GetTargetCall() == cb)
				{
					isServiced = true; //значит он обслуживается
					break;
				}
			}
			if (!isServiced) //если вызов не обслуживается
			{
				// найжем самую быструю кабинку, которая сможет
				//принять вызов

				double minArivTime = -1;
				ElevatorCar* theMostQuicklyCar;

				for (int j = 0; j < carCount; j++)
				{
					//если кабинка свободна или находится между кабинкой,
					//для которой определяется задача и вызовом и 
					//целевой этаж кабинки раньше, чем вызов
					if (newCars[j]->IsFree() ||
						(newCars[j]->IsBetweenCarAndCall(theCar, cb)
							&& newCars[j]->TargetIsEarlierThanCall(cb)))
					{
						//расчитать время прибытия кабинки на вызов
						double arivTime = newCars[j]->CalculateTheCallAriveTime(cb);
						//если это первая подходящая кабинка
						if (minArivTime == -1)
						{
							minArivTime = arivTime;
							theMostQuicklyCar = newCars[j];
						}
						else //2 или 3 или ... подходящая кабинка
							if (arivTime < minArivTime)
							{
								minArivTime = arivTime;
								theMostQuicklyCar = newCars[j];
							}
					}
				}
				// если время прибытия для текущей кабинки <= чем
				//для самой быстрой или подходящей кабинки не нашлось
				if (theCar->CalculateTheCallAriveTime(cb) <= minArivTime || minArivTime == -1)
				{
					//удаляем копии кабинок
					for (int i = 0; i < carCount; i++) {
						delete newCars[i];
					}
					delete newCars;

					return cb; //вернем вызов (этаж с направлением движения)
				}
				else { //если текущая кабинка не бысрее всех придет на вызов
					//то перевести состояние самый быстрой кабинки в 
					//состояние, когда она приехала на вызов, записав в 
					//timer затраченное на это время
					theMostQuicklyCar->MoveElevator(cb, minArivTime);
				}
			}
		}

		//очередной этаж с направлениием начиная с самого ближнего,
		//на котором может остановиться лифт (для него j=0)
		cb = theCar->NextFloor(j++);
	}

	//эта часть кода будет выполнятся только, если не нашлось
	//ни одной задачи для текущей кабинки

	//удаляем копии кабинок
	for (int j = 0; j < carCount; j++) {
		delete newCars[j];
	}
	delete newCars;

	return CallButton(); //вернем пустой вызов
}

//выключить кнопку с этажа
void Elevator::CallButtonOff(const CallButton cb)
{
	CheckButton(cb); //проверить кнопку на исключение

	//если кнопка вверх, то выключить ее в массиве кнопок вверх
	if (cb.direction == bUp) {
		callsUp[cb.floorNumber] = false;
	}
	//если кнопка вниз, то выключить ее в массиве кнопок вниз
	else if (cb.direction == bDown) {
		callsDown[cb.floorNumber] = false;
	}
}

// перевести систему в следующее состояние
//factor - коэффициент времени задержки, относительно TIME_INTERVAL
void Elevator::SystemTick(double factor)
{
	Sleep(1.0*TIME_INTERVAL/factor);	//задержка 

	for (int j = 0; j < carCount; j++)
	{
		//перевести кабинку в следующее состояние
		cars[j]->Tick();
	}
}

//найти кабинку, которая уже приехала на вызов и вернуть ее номер
int Elevator::FindReadyCar(CallButton cb)const
{
	CheckButton(cb); //проверить кнопку на исключение

	for (int j = 0; j < carCount; j++)
	{
		//Если кабинка на нужном этаже с нужным направлением и открыта
		if (cars[j]->GetState() == stopped && cars[j]->GetCurrentFloor() == cb.floorNumber
			&& (cars[j]->GetMovingDirection() == up && cb.direction == bUp
				|| cars[j]->GetMovingDirection() == down && cb.direction == bDown)) 
		{
			return cars[j]->GetCarNumber();
		}
	}
	return -1;
}


//===========================Elevator==============================================
//=================================================================================
//=========================ElevatorCar=============================================


//инициализация кабинки лифта
ElevatorCar::ElevatorCar(int nCar, Elevator* ptrElev) :
	carNumber(nCar), parentElev(ptrElev),floorCount(ptrElev->floorCount)
{
	targetFloor = currentFloor = rand()%floorCount; //случайный начальный этаж
	movingDirection = no;
	state = notUsed;

	//создание нужного числа выключенных кнопок внутри лифта
	orderFloors = new bool[floorCount];
	for (int j = 0; j < floorCount; j++)
	{
		orderFloors[j] = false;
	}
	targetCallAvailability = false;
	timer = 0;
}

//копирование кабинки лифта
ElevatorCar::ElevatorCar(ElevatorCar& elev):
	carNumber(elev.carNumber), parentElev(elev.parentElev),
	floorCount(elev.floorCount)
{
	currentFloor = elev.currentFloor;
	movingDirection = elev.movingDirection;
	state = elev.state;

	//создание копии массива кнопок
	orderFloors = new bool[floorCount];
	for (int j = 0; j < floorCount; j++)
	{
		orderFloors[j] = elev.orderFloors[j];
	}
	targetFloor = elev.targetFloor;
	targetCallAvailability = elev.targetCallAvailability;
	targetCall = elev.targetCall;
	timer = elev.timer;
}

//удаление кабинки лифта
ElevatorCar::~ElevatorCar()
{
	delete orderFloors;
}

void ElevatorCar::CheckFloor(const int floor)const
{
	if (floor < 0 || floor >= floorCount)
	{
		char buff[BUF_SIZE];
		string str1 = itoa(floor, buff, 10);
		throw ElevatorError(1,"Impossible Floor (" + str1 + ")");
	}
}

//получить номер кабинки
const int ElevatorCar::GetCarNumber()const
{
	return carNumber;
}

//получить текущий этаж кабинки
const int ElevatorCar::GetCurrentFloor()const
{
	return currentFloor;
}

//получить направление движения кабинки
const movingDirectionType ElevatorCar::GetMovingDirection()const
{
	return movingDirection;
}

//получить состояние кабинки
const carStateType ElevatorCar::GetState()const
{
	return state;
}

//является ли этаж заказанным?
const bool ElevatorCar::isOrderFloor(const int floor)const
{
	return orderFloors[floor];
}

//получить целевой этаж
const int ElevatorCar::GetTargetFloor()const
{
	return targetFloor;
}

//есть ли вызов,являющийся целью кабинки
const bool ElevatorCar::GetTargetCallAvailability()const
{
	return targetCallAvailability;
}

//целевой вызов для кабинки
const CallButton ElevatorCar::GetTargetCall()const
{
	return targetCall;
}

//свободна ли кабинка?
const bool ElevatorCar::IsFree()const
{
	return state == notUsed;
}

//открыть двери
void ElevatorCar::OpenDoor()
{
	if (state == doorClosing || state == notUsed) {
		state = doorOpening;
		timer = OPENING_DOOR_TIME;
	}
}

//закрыть двери
void ElevatorCar::CloseDoor()
{
	if (state == stopped)
	{
		state = doorClosing;
		timer = CLOSING_DOOR_TIME;
	}
}

//установить заказ на этаж
//если кабинка уже открыта на этом этаже, вернуть false
bool ElevatorCar::SetOrderFloor(const int nFloor)
{
	CheckFloor(nFloor); //проверить этаж на исключение

	//если заказан этаж, на котором открыта кабинка 
	//или заказан уже заказанный этаж
	if (nFloor == currentFloor && state == stopped || orderFloors[nFloor]) {
		return false;  //ничего не делать
	}

	//если заказан этаж, на котором кабинка закрывает двери или не используется
	if (nFloor == currentFloor && (state == doorClosing || state == notUsed)) {
		OpenDoor(); //открыть двери
		return false; //и больше ничего не делать
	}

	orderFloors[nFloor] = true; //установить заказанный этаж

	DeterminateTargetFloor(); //определить целевой этаж

	return true;
}

//установить целевой этаж
void ElevatorCar::SetTargetFloor(const CallButton cb)
{
	parentElev->CheckButton(cb); //проврка вызов на исключение

	targetCallAvailability = true; //цель кабинки - вызов с этажа
	targetCall = cb; //сам целевой вызов
	targetFloor = cb.floorNumber; //целевой этаж

	//если этаж вызова выше текущего этажа кабинки
	if (cb.floorNumber > currentFloor) {
		movingDirection = up; //направление движение - вверх
	}
	//если этаж вызова выше текущего этажа кабинки
	else if (cb.floorNumber < currentFloor) {
		movingDirection = down; //направление движение - вниз
	}
}

//выключить кнопку в кабинке или на этаже
void ElevatorCar::TurnOffButton()
{
	//выключить кнопку текущего этажа в кабинке
	orderFloors[currentFloor] = false;

	//если у кабинки был целью вызов с этажа
	if (targetCallAvailability)
	{
		//выключить соответствующую кнопку вызова на этаже
		parentElev->CallButtonOff(targetCall);

		//отключим цель кабинки, вызов с этажа
		targetCallAvailability = false;
	}
}

//определить целевой этаж для кабинки,
//если целевого этажа нет, вернуть false
const bool ElevatorCar::DeterminateTargetFloor()
{
	//если лифт тормозит, значит он неизбежно прибудет на целевой этаж
	//и определение следующей цели произойдет на нем
	if (state == stopsUp || state == stopsDown || state == slowMovingUp
		|| state == slowMovingDown) {
		return true; //целевой этаж остается прежним
	}

	//оставить прежде выбранный вызов
	targetCallAvailability = false;

	//получить задачу (заказ или вызов)
	CallButton taskCB = parentElev->FindTask(this);

	if (taskCB.floorNumber == -1) //если задачи нет
	{
		movingDirection = no; //кабинка свободна
		return false; //этаж не определен
	}
	else //если была найдена задача
	{
		CheckFloor(taskCB.floorNumber); //проверка этажа на исключение

		targetFloor = taskCB.floorNumber; //установить целевой этаж

		//если задача(этаж вызова или заказа) выше кабинки
		if (taskCB.floorNumber > currentFloor) {
			movingDirection = up; //направление движение вверх
		}
		//если задача(этаж вызова или заказа) ниже кабинки
		else if (taskCB.floorNumber < currentFloor) {
			movingDirection = down; //направление движение вниз
		}

		//если есть вызов с этажа, соответствующей задаче
		if (parentElev->theCallButtonIsOn(taskCB))
		{
			parentElev->CheckButton(taskCB); //проверка вызова на исклюючение

			targetCallAvailability = true; //цель - внешний вызов
			targetCall = taskCB; //целевой вызов равен задаче
		}

		return true; //целевой этаж определен
	}
}

//первести кабинку в следующее состояние
void ElevatorCar::Tick()
{
	CheckFloor(targetFloor); //проверка этажа на исключение
	if (targetCallAvailability) {
		parentElev->CheckButton(targetCall); //проверка вызова
	}


	if (timer > 0) //если время текущего состояния не закончилось
	{
		timer -= TIME_INTERVAL;	//уменьшить оставшееся время
		return;
	}

	//если кабинка уже движется вверх с высокой скоростью 
	if (state == movingUp || state == startsUp)
	{
		currentFloor++;
		//если нужно остановиться на следующем этаже
		if (targetFloor - currentFloor == 1)
		{
			state = stopsUp; //тормозим, двигаясь вверх
			timer = ACCELERATION_TIME - TIME_INTERVAL;
		}
		//если нужно остановить дальше по ходу движение кабинки
		else if (targetFloor - currentFloor > 1) 
		{
			state = movingUp; //двигаемся вверх дальше
			timer = MOVING_TIME - TIME_INTERVAL;
		}
		else    //если нужно остановиться на текущем этаже 
				//или этаже ниже
		{
			state = stopsUp; //этаж проехали, тормозим
			timer = ACCELERATION_TIME - TIME_INTERVAL;
		}
		return;
	}
	
	//если кабинка уже движется вниз с высокой скоростью
	if (state == movingDown || state == startsDown)
	{
		currentFloor--;
		//если нужно остановиться на следующем этаже
		if (currentFloor - targetFloor == 1)
		{
			state = stopsDown; //тормозим, двигаясь вниз
			timer = ACCELERATION_TIME - TIME_INTERVAL;
		}
		//если нужно остановить дальше по ходу движение кабинки
		else if (currentFloor - targetFloor > 1) 
		{
			state = movingDown; //двигаемся вниз дальше
			timer = MOVING_TIME - TIME_INTERVAL;
		}
		else	//если нужно остановиться на текущем этаже
				//или этаже выше
		{
			state = stopsDown; //этаж проехали, тормозим
			timer = ACCELERATION_TIME - TIME_INTERVAL;
		}
		return;
	}

	//если кабинка остановилась
	//(предыдущим состоянием было торможение)
	if (state == stopsUp || state == slowMovingUp
		|| state == stopsDown || state == slowMovingDown)
	{
		//кабинка тормозила двигаясь вверх
		if (state == stopsUp || state == slowMovingUp) {
			currentFloor++;
		}
		else { //кабинка тормозила двигаясь вниз
			currentFloor--;
		}

		state = doorOpening;

		//если у кабинки был целью вызов с этажа
		if (targetCallAvailability)
		{
			//в зависимости от направление движения вызова
			//установить направление движения кабинки
			movingDirection = (targetCall.direction == bUp ? up : down);
		}
		//выключить кнопку внутри лифта/на этаже
		TurnOffButton();

		timer = OPENING_DOOR_TIME - TIME_INTERVAL;
		return;
	}

	//если кабинка открывала двери
	if (state == doorOpening)
	{
		//начать посадку/высадку пассажиров
		state = stopped;
		timer = STANDING_TIME - TIME_INTERVAL;
		return;
	}

	//если пасадка/высадка завершена
	if (state == stopped)
	{
		//закрыть двери
		state = doorClosing;
		timer = CLOSING_DOOR_TIME - TIME_INTERVAL;
		return;
	}

	//если двери закрылись или лифт не использовался
	if (state == doorClosing || state == notUsed)
	{
		//если не установлен целью вызов с этажа
		if (!targetCallAvailability)
		{
			//если целевой этаж для кабинки не был определен
			if (!DeterminateTargetFloor())
			{
				//оставить кабинку в покое
				state = notUsed;
				return;
			}
		}

		//значит целью кабинки является вызов с этажа
		//или определен целевой этаж

		//если кабинка находится на нужном этаже
		if (targetFloor == currentFloor) 
		{
			state = doorOpening;
			//выключить кнопку текущего этажа внутри кабинки
			orderFloors[currentFloor] = false;

			//если целью кабинки является вызов с этажа
			if (targetCallAvailability) 
			{
				//установить направление движения кабинки
				movingDirection = (targetCall.direction == bUp ? up : down);
				//отключим цель кабинки, вызов с этажа
				targetCallAvailability = false;
				//выключить соответствующую кнопку вызова на этаже
				parentElev->CallButtonOff(targetCall);
			}
			timer = OPENING_DOOR_TIME - TIME_INTERVAL;
		}
		else 
		{
			//если этаж назначения на этаже выше
			if (targetFloor - currentFloor == 1)
			{
				//подъем на один этаж
				state = slowMovingUp;
				timer = SLOW_MOVING_TIME - TIME_INTERVAL;
			}
			//если этаж назначения на этаже ниже
			else if (targetFloor - currentFloor == -1)
			{
				//спуск на один этаж
				state = slowMovingDown;
				timer = SLOW_MOVING_TIME - TIME_INTERVAL;
			}
			else
			{
				//если этаж назначения находится сверху, кабинка
				//ускоряетя вверх, иначе ускоряется вниз
				state = (targetFloor > currentFloor) ? startsUp : startsDown;
				timer = ACCELERATION_TIME - TIME_INTERVAL;
			}
		}
		return;
	}
}

//вычислить минимальное число этажей, которое
//требуется кабинки для торможения
const int ElevatorCar::GetBrakingFloor()const
{
	//если кабинка лифта уже стоит
	if (state == doorOpening || state == stopped || state == doorClosing || state == notUsed) {
		return 0;
	}
	//если кабинка останавливается
	else if (state == stopsUp || state == stopsDown || state == slowMovingUp || state == slowMovingDown) {
		return 1;
	}
	else {	//движется быстро или ускоряется
		return 2;
	}
}

//перевести кабинку в будущее состояние, в timer
//записать время затраченное для прибытие на вызов 
void ElevatorCar::MoveElevator(CallButton cb, double time)
{
	parentElev->CheckButton(cb); //проверка вызова на исключение

	//минимальное число этажей для торможения
	int brakingFloor = GetBrakingFloor();

	//если кабинка движется вверх
	if (movingDirection == up)
	{
		//если вызов вверх выше кабинки
		if (cb.floorNumber >= currentFloor + brakingFloor && cb.direction == bUp)
		{
			for (int j = currentFloor + brakingFloor; j <= cb.floorNumber; j++)
			{
				//отключить все кнопки внутри кабинки соответствующие пройденным этажам
				orderFloors[j] = false; 
			}
		}
		//если вызов вниз
		else if (cb.direction == bDown)
		{
			//поднятся до самого верхнего этажа
			for (int j = currentFloor + brakingFloor; j < floorCount; j++)
			{
				orderFloors[j] = false; //отключить кнопки в кабинке
			}
			//спустится до вызова
			for (int j = currentFloor + brakingFloor; j >= cb.floorNumber; j--)
			{
				orderFloors[j] = false; //отключить кнопки в кабинке
			}
		}
		else //вызов вверх, но кабинка уже проехала его или не может затормозить
		{
			//кабинка пройдет все этажи
			for (int j = 0; j < floorCount; j++)
			{
				orderFloors[j] = false; //отключить кнопки в кабинке
			}
		}
	}
	//если кабинка движется вниз
	else if (movingDirection == down)
	{
		//если вызов вниз ниже кабинки
		if (cb.floorNumber <= currentFloor - brakingFloor && cb.direction == bDown)
		{
			for (int j = currentFloor - brakingFloor; j >= cb.floorNumber; j--)
			{
				//отключить все кнопки внутри кабинки соответствующие пройденным этажам
				orderFloors[j] = false;
			}
		}
		//если вызов вверх
		else if (cb.direction == bUp)
		{
			//опустится до самого нижнего этажа
			for (int j = currentFloor - brakingFloor; j >= 0; j--)
			{
				orderFloors[j] = false; //отключить кнопки в кабинке
			}
			//поднятся до вызова
			for (int j = currentFloor - brakingFloor; j <= cb.floorNumber; j++)
			{
				orderFloors[j] = false; //отключить кнопки в кабинке
			}
		}
		else //вызов вниз, но кабинка уже проехала его или не может затормозить
		{
			//кабинка пройдет все этажи
			for (int j = 0; j < floorCount; j++)
			{
				orderFloors[j] = false; //отключить кнопки в кабинке
			}
		}
	}
	//если кабинка не используется
	else if (movingDirection == no)
	{
		//если вызов выше текущего этажа
		if (cb.floorNumber >= currentFloor)
		{
			//поднятся до вызова
			for (int j = currentFloor; j <= cb.floorNumber; j++)
			{
				orderFloors[j] = false; //отключить кнопки в кабинке
			}
		}
		else {	//вызов ниже текущего этажа или совпадает с этажом
			//опустится до вызова
			for (int j = currentFloor; j >= cb.floorNumber; j--)
			{
				orderFloors[j] = false; //отключить кнопки в кабинке
			}
		}
	}

	//установить целевой этаж для кабинки и переместить ее туда
	currentFloor = targetFloor = cb.floorNumber;
	//выбрать будущее направление движения
	movingDirection = (cb.direction == bUp ? up : down);
	state = doorOpening;
	//записать затраченное на перемещение время в таймер 
	timer = time;
}

//вызов находится между кабинкой и целевым этажом?
const bool ElevatorCar::CallIsBetweenCarAndTargetFloor
(const CallButton cb)const
{
	parentElev->CheckButton(cb); //проверка вызова на исключение

	//минимальное число этажей для торможения
	int brakingFloor = GetBrakingFloor();

	//если кабинка движется вверх
	if (movingDirection == up)
	{
		//если цель кабинки не вызов или вызов вверх
		if (!(targetCallAvailability && targetCall.direction == down))
		{
			//если вызов между кабинкой и ее целевым этажом
			if (cb.direction == bUp && (cb.floorNumber >= currentFloor + brakingFloor)
				&& cb.floorNumber <= targetFloor) {
				return true;
			}
		}
		else	//цель кабинки - вызов вниз
		{
			//если вызов между кабинкой и ее целевым этажом
			if (cb.direction == bUp && (cb.floorNumber >= currentFloor + brakingFloor)
				|| cb.direction == bDown && cb.floorNumber >= targetFloor) {
				return true;
			}
		}
	}
	//если кабинка движется вниз
	else if (movingDirection == down)
	{
		//если цель кабинки не вызов или вызов вниз
		if (!(targetCallAvailability && targetCall.direction == up))
		{
			//если вызов между кабинкой и ее целевым этажом
			if (cb.direction == bDown && (cb.floorNumber <= currentFloor - brakingFloor)
				&& cb.floorNumber >= targetFloor) {
				return true;
			}
		}
		else	//цель кабинки - вызов вверх
		{
			//если вызов между кабинкой и ее целевым этажом
			if (cb.direction == bDown && (cb.floorNumber <= currentFloor - brakingFloor)
				|| cb.direction == bUp && cb.floorNumber <= targetFloor) {
				return true;
			}
		}
	}

	return false;
}

//эта кабинка между другой кабинкой и вызовом? 
const bool ElevatorCar::IsBetweenCarAndCall
(const ElevatorCar*const elev, const CallButton cb)const
{
	parentElev->CheckButton(cb); //проверка вызова на исключение
	if (elev == 0) {	//проверка кабинки лифта на исключение
		throw ElevatorError(3, "The link points to a non-existent car");
	}

	//минимальное число этажей для торможения
	int brakingFloor = GetBrakingFloor();

	//если сравниваемая кабинка движется вверх
	if (elev->movingDirection == up)
	{
		//если сравниваемая кабинка может остановиться на этаже вызова или ниже
		if (elev->currentFloor+elev->GetBrakingFloor() <= cb.floorNumber)
		{
			if (cb.direction == bUp)	//если вызов вверх
			{
				//если текущая кабинка находиться между сравниваемой кабинкой
				//и вызовом и движется в нужном направлении
				if (currentFloor >= elev->currentFloor && (movingDirection != down)
					&& currentFloor + brakingFloor <= cb.floorNumber) {
					return true;
				}
			}
			else if (cb.direction == bDown)	//если вызов вниз
			{
				//если текущая кабинка находиться между сравниваемой кабинкой
				//и вызовом и движется в нужном направлении
				if (currentFloor >= elev->currentFloor && (movingDirection != down)
					|| currentFloor - brakingFloor >= cb.floorNumber) {
					return true;
				}
			}
		}

		else //если сравниваемая кабинка НЕ может остановиться на этаже вызова или ниже
		{
			if (cb.direction == bDown)	//если вызов вниз
			{
				//если текущая кабинка находиться между сравниваемой кабинкой
				//и вызовом и движется в нужном направлении
				if (currentFloor >= elev->currentFloor && movingDirection != down
					|| currentFloor - brakingFloor >= cb.floorNumber) {
				}
				return true;
			}
			else if (cb.direction == bUp)	//если вызов вверх
			{
				//если текущая кабинка находиться между сравниваемой кабинкой
				//и вызовом и движется в нужном направлении
				if (!(movingDirection == up && currentFloor + brakingFloor > cb.floorNumber
					&& currentFloor < elev->currentFloor)) {
					return true;
				}
			}
		}
	}

	//если сравниваемая кабинка движется вниз
	else if (elev->movingDirection == down)
	{
		//если сравниваемая кабинка может остановиться на этаже вызова или выше
		if (elev->currentFloor-elev->GetBrakingFloor() > cb.floorNumber)
		{
			if (cb.direction == bDown)	//если вызов вниз
			{
				//если текущая кабинка находиться между сравниваемой кабинкой
				//и вызовом и движется в нужном направлении
				if (currentFloor <= elev->currentFloor && (movingDirection != up)
					&& currentFloor - brakingFloor >= cb.floorNumber) {
					return true;
				}
			}
			else if (cb.direction == bUp)	//если вызов вверх
			{
				//если текущая кабинка находиться между сравниваемой кабинкой
				//и вызовом и движется в нужном направлении
				if (currentFloor <= elev->currentFloor && (movingDirection != up)
					|| currentFloor + brakingFloor <= cb.floorNumber) {
					return true;
				}
			}
		}

		//если сравниваемая кабинка НЕ может остановиться на этаже вызова или выше
		else 
		{
			if (cb.direction == bUp)	//если вызов вверх
			{
				//если текущая кабинка находиться между сравниваемой кабинкой
				//и вызовом и движется в нужном направлении
				if (currentFloor <= elev->currentFloor && movingDirection == down
					|| currentFloor + brakingFloor <= cb.floorNumber) {
				}
				return true;
			}
			else if (cb.direction == bDown)	//если вызов вниз
			{
				//если текущая кабинка находиться между сравниваемой кабинкой
				//и вызовом и движется в нужном направлении
				if (!(movingDirection == up && currentFloor - brakingFloor < cb.floorNumber
					&& currentFloor > elev->currentFloor)) {
					return true;
				}
			}
		}
	}

	//если сравниваемая кабинка никуда не движется
	else if (elev->movingDirection == no)
	{
		//если сравниваемая кабинка выше вызова
		if (elev->currentFloor > cb.floorNumber)
		{
			if (cb.direction == bDown)	//если вызов вниз
			{
				//если текущая кабинка находиться между сравниваемой кабинкой
				//и вызовом и движется в нужном направлении
				if (currentFloor <= elev->currentFloor && (movingDirection != up)
					&& currentFloor - brakingFloor >= cb.floorNumber) {
					return true;
				}
			}
			else if (cb.direction == bUp)	//если вызов вверх
			{
				//если текущая кабинка находиться между сравниваемой кабинкой
				//и вызовом и движется в нужном направлении
				if (currentFloor <= elev->currentFloor && (movingDirection != up)
					|| currentFloor + brakingFloor <= cb.floorNumber) {
					return true;
				}
			}
		}

		//если сравниваемая кабинка ниже вызова
		else if (elev->currentFloor < cb.floorNumber)
		{
			if (cb.direction == bUp)	//если вызов вверх
			{
				//если текущая кабинка находиться между сравниваемой кабинкой
				//и вызовом и движется в нужном направлении
				if (currentFloor >= elev->currentFloor && (movingDirection != down)
					&& currentFloor + brakingFloor <= cb.floorNumber) {
					return true;
				}
			}
			else if (cb.direction == bDown)	//если вызов вниз
			{
				//если текущая кабинка находиться между сравниваемой кабинкой
				//и вызовом и движется в нужном направлении
				if (currentFloor >= elev->currentFloor && (movingDirection != down)
					|| currentFloor - brakingFloor >= cb.floorNumber) {
					return true;
				}
			}
		}
	}

	return false;
}

//целевой этаж находится раньше, чем вызов?
const bool ElevatorCar::TargetIsEarlierThanCall
(const CallButton cb)const
{
	parentElev->CheckButton(cb); //проверка вызова на исключение

	if (movingDirection == up)	//если кабинка движется вверх
	{
		//если вызов выше целевого этажа кабинки и у кабинки
		//нет цели-вызова с этажа вниз
		if (cb.floorNumber >= targetFloor &&
			(!targetCallAvailability || targetCall.direction == bUp)) {
			return true;
		}
		//если вызов ниже целевово этажа кабинки и вызов вниз
		if (cb.floorNumber <= targetFloor && cb.direction == bDown) {
			return true;
		}
	}
	else if (movingDirection == down)	//если кабинка движется вниз
	{
		//если вызов ниже целевого этажа кабинки и у кабинки
		//нет цели-вызова с этажа вверх
		if (cb.floorNumber <= targetFloor &&
			(!targetCallAvailability || targetCall.direction == bDown)) {
			return true;
		}
		//если вызов выше целевово этажа кабинки и вызов вверх
		if (cb.floorNumber >= targetFloor && cb.direction == bUp) {
			return true;
		}
	}
	return false;
}

//рассчитать время обработки всех заказов выше этажа pos,
//если вызов тоже выше этажа, то вернуть true
const bool ElevatorCar::CalculateHigher
(const CallButton cb, double& ariveTime, int& pos)const
{
	int upperLimit;	//самый верхний этаж, на который должена приехать кабинка

	//если вызов вверх выше целевого этажа
	if (cb.floorNumber > targetFloor && cb.direction == bUp)
		upperLimit = cb.floorNumber;
	else
		upperLimit = floorCount;

	//двигаемся вверх
	for (int j = targetFloor + 1; j < upperLimit; j++)
	{
		if (orderFloors[j]) //если есть заказ на этаж
		{
			//если с последний остановки до этого этажа всего один этаж
			if (j - pos == 1) {
				//прибавим время поездки кабинки на соседний этаж
				ariveTime += SLOW_MOVING_TIME;
			}
			else {	 //2 и больше этажей до этого
				//прибавим время ускорения и торможения а также быстрого движения
				ariveTime += ACCELERATION_TIME * 2 + (j - pos - 2)*MOVING_TIME;
			}
			//прибавим время открытия дверей, стоянки и закрытия дверей
			ariveTime += OPENING_DOOR_TIME + STANDING_TIME + CLOSING_DOOR_TIME;
			pos = j;
		}
	}

	//добавим время прибытия на вызов вверх если он выше целевого этажа
	if (cb.floorNumber > targetFloor && cb.direction == bUp)
	{
		if (cb.floorNumber - pos == 1)
			ariveTime += SLOW_MOVING_TIME;
		else
			ariveTime += ACCELERATION_TIME * 2 + (cb.floorNumber - pos - 2)*MOVING_TIME;

		ariveTime += OPENING_DOOR_TIME;
		return true;	//кабинка доехала до вызова
	}

	//если вызов вниз выше целевого этажа 
	if (cb.floorNumber >= targetFloor && cb.direction == bDown)
	{
		//если этаж вызова совпадает с целевым этажом, но имеет направление
		//обратное направлению движению кабинки и заказов сверху нет
		if (cb.floorNumber == targetFloor && pos == targetFloor)
			//возможны только состояния dOpening, stopped, dClosing;
		{
			if (state == stopped) {	//если кабинка стоит с открытыми дверьми
				//значит она стоит на целевом этаже 
				ariveTime = 0;	//и она уже может обработать вызов
			}
			if (state == doorClosing) {	//если двери закрываются
				//значит кабинка стоит на целевом этаже
				//и нужно только открыть двери
				ariveTime = OPENING_DOOR_TIME;
			}
		}
		else	//значит этаж вызова не совпадает с целевым или
				//были заказы сверху
		{
			//если уже рассчитано время прибытия на этаж вызова
			if (cb.floorNumber == pos) {	//возможно при заказе на верхний этаж
				//то не нужно было стоять и закрывать двери
				ariveTime -= STANDING_TIME - CLOSING_DOOR_TIME;
			}
			//если от последнего заказа до этажа вызова всего один этаж
			else if (abs(cb.floorNumber - pos) == 1) {
				ariveTime += SLOW_MOVING_TIME + OPENING_DOOR_TIME;
			}
			else {	//от последнего заказа до этажа вызова больше одного этажа
				ariveTime += ACCELERATION_TIME * 2 + (abs(cb.floorNumber - pos) - 2)
					*MOVING_TIME + OPENING_DOOR_TIME;
			}
		}
		return true;	//кабинка доехала до вызова
	}

	return false;	//кабинка еще НЕ доехала до вызова
}

//рассчитать время обработки всех заказов ниже этажа pos,
//если вызов тоже ниже этажа, то вернуть true
const bool ElevatorCar::CalculateLower
(const CallButton cb, double& ariveTime, int& pos)const
{
	int lowerLimit;	//самый нижний этаж, на который должена приехать кабинка

	//если вызов вниз ниже целевого этажа
	if (cb.floorNumber < targetFloor && cb.direction == bDown) {
		lowerLimit = cb.floorNumber + 1;
	}
	else {
		lowerLimit = 0;
	}

	//двигаемся вниз
	for (int j = targetFloor - 1; j >= lowerLimit; j--)
	{
		if (orderFloors[j])	//если есть заказ на этаж
		{
			//если с последний остановки до этого этажа всего один этаж
			if (pos - j == 1) {
				//прибавим время поездки кабинки на соседний этаж
				ariveTime += SLOW_MOVING_TIME;
			}
			else { //2 и больше этажей до этого
				//прибавим время ускорения и торможения а также быстрого движения
				ariveTime += ACCELERATION_TIME * 2 + (pos - j - 2)*MOVING_TIME;
			}
			//прибавим время открытия дверей, стоянки и закрытия дверей
			ariveTime += OPENING_DOOR_TIME + STANDING_TIME + CLOSING_DOOR_TIME;
			pos = j;
		}
	}

	//добавим время прибытия на вызов вниз если он ниже целевого этажа
	if (cb.floorNumber < targetFloor && cb.direction == bDown)
	{
		if (pos - cb.floorNumber == 1)
			ariveTime += SLOW_MOVING_TIME;
		else
			ariveTime += ACCELERATION_TIME * 2 + (pos - cb.floorNumber - 2)*MOVING_TIME;

		ariveTime += OPENING_DOOR_TIME;
		return true;	//кабинка доехала до вызова
	}


	//если вызов вверх ниже целевого этажа 
	if (cb.floorNumber <= targetFloor && cb.direction == bUp)
	{
		//если этаж вызова совпадает с целевым этажом, но имеет направление
		//обратное направлению движению кабинки и заказов снизу нет
		if (cb.floorNumber == targetFloor && pos == targetFloor)
			//возможны только состояния dOpening, stopped, dClosing;
		{
			if (state == stopped) {	//если кабинка стоит с открытыми дверьми
				//значит она стоит на целевом этаже 
				ariveTime = 0;	//и она уже может обработать вызов
			}
			if (state == doorClosing) {	//если двери закрываются
				//значит кабинка стоит на целевом этаже
				//и нужно только открыть двери
				ariveTime = OPENING_DOOR_TIME;
			}
		}
		else	//значит этаж вызова не совпадает с целевым или
				//были заказы снизу
		{
			//если уже рассчитано время прибытия на этаж вызова
			if (cb.floorNumber == pos) { //возможно при заказе на нижний этаж
				//то не нужно было стоять и закрывать двери
				ariveTime -= STANDING_TIME - CLOSING_DOOR_TIME;
			}
			//если от последнего заказа до этажа вызова всего один этаж
			else if (abs(cb.floorNumber - pos) == 1) {
				ariveTime += SLOW_MOVING_TIME + OPENING_DOOR_TIME;
			}
			else {	//от последнего заказа до этажа вызова больше одного этажа
				ariveTime += ACCELERATION_TIME * 2 + (abs(cb.floorNumber - pos) - 2)
					*MOVING_TIME + OPENING_DOOR_TIME;
			}
		}
		return true; //кабинка доехала до вызова
	}

	return false; //кабинка еще НЕ доехала до вызова
}

//расчитаь время прибытия кабинки на вызов с учетом, 
//что она выполнит все свои заказы и вызовы по пути
const double ElevatorCar::CalculateTheCallAriveTime
(const CallButton cb)const
{
	parentElev->CheckButton(cb); //проверка вызова на исключение

	double ariveTime = (timer < 0 ? 0 : timer);	//время прибытия на целевой этаж

	if (state == notUsed) 	//если кабинка свободна
	{
		//в зависимости от количества этажей до вызова
		//рассчитать время на дорогу до этажа и открытие дверей
		switch (abs(currentFloor - cb.floorNumber))
		{
		case 0: ariveTime = OPENING_DOOR_TIME; break;
		case 1: ariveTime = SLOW_MOVING_TIME + OPENING_DOOR_TIME; break;
		default:
			ariveTime = 2 * ACCELERATION_TIME + (abs(cb.floorNumber - currentFloor) - 2)
				* MOVING_TIME + OPENING_DOOR_TIME; break;
		}
		return ariveTime;	//время прибытия рассчитано
	}


	//если кабинка уже на целевом этаже то ее состяние может
	//быть только doorOpening, stopped или doorClosing

	//если целевой этаж и направление совпадают с вызовом
	if (cb.floorNumber == targetFloor && (cb.direction == bUp
		&& movingDirection == up || cb.direction == bDown && movingDirection == down))
	{
		if (state == stopped) {//значит целевой этаж совпадает с вызовом
			ariveTime = 0;
		}
		if (state == doorClosing) {//значит целевой этаж совпадает с вызовом
			ariveTime = OPENING_DOOR_TIME;
		}
		return ariveTime; //время прибытия рассчитано
	}


	//рассчитаем время поездки до целевого этажа и открытие дверей

	if (state == startsUp || state == movingUp
		|| state == startsDown || state == movingDown) {
		ariveTime += (abs(targetFloor - currentFloor) - 2)*MOVING_TIME
			+ ACCELERATION_TIME + OPENING_DOOR_TIME;
	}
	else if (state == stopsUp || state == stopsDown
		|| state == slowMovingUp || state == slowMovingDown) {
		ariveTime += OPENING_DOOR_TIME;
	}


	// теперь рассматриваются случаи, когда целевой этаж 
	//и направление не совпадают с вызовом
	if (state != stopped && state != doorClosing) {
		ariveTime += STANDING_TIME;	//не нужно, если этаж вызова совпадает с 
		//целевым этажом, но направления кабинки не совпадает с направлением 
		//вызова, а внутренних заказов нет
	}
	if (state != doorClosing) {
		ariveTime += CLOSING_DOOR_TIME; //также
	}

	int pos = targetFloor;//предидущий этаж остановки кабинки для расчетов

	//если кабинка движется вверх
	if (movingDirection == up)
	{
		//рассчитать время выполнение заказов выше кабинки
		//и если дошли до вызова и рассчитали его
		if (CalculateHigher(cb, ariveTime, pos)) {
			return ariveTime; //время прибытия рассчитано
		}
		//рассчитать время выполнение заказов и вызова ниже кабинки
		CalculateLower(cb, ariveTime, pos);

		return ariveTime; //время прибытия рассчитано
	}
	//если кабинка движется вниз
	else if (movingDirection == down)
	{
		//рассчитать время выполнение заказов ниже кабинки
		//и если дошли до вызова и рассчитали его
		if (CalculateLower(cb, ariveTime, pos)) {
			return ariveTime;	//время прибытия рассчитано
		}
		//рассчитать время выполнение заказов и вызова выше кабинки
		CalculateHigher(cb, ariveTime, pos);

		return ariveTime; //время прибытия рассчитано
	}
}

//расчитать время прибытия кабинки на вызов,
//игнориря все другие вызовы и заказы
const double ElevatorCar::MinCalculateTheCallAriveTime
(const CallButton cb)const
{
	parentElev->CheckButton(cb); //проверка вызова на исключение

	//если вызов находится между кабинкой и ее целевым этажом
	//или кабинка свободна
	if (CallIsBetweenCarAndTargetFloor(cb) || IsFree())
	{
		double arriveTime = (timer < 0 ? 0 : timer);	//время прибития на вызов

		//если вызов с текущего этажа кабинки
		if (currentFloor - cb.floorNumber == 0)
		{
			if (state == notUsed || state == doorClosing) {
				arriveTime = OPENING_DOOR_TIME;
			}
			else if (state == stopped) {
				arriveTime = 0;
			}
		}
		//если от этажа кабинки до этажа вызова один этаж
		else if (abs(currentFloor - cb.floorNumber) == 1)
		{
			if (state == notUsed || state == doorClosing) {
				arriveTime += SLOW_MOVING_TIME;
			}
			else if (state == stopped) {
				arriveTime += CLOSING_DOOR_TIME + SLOW_MOVING_TIME;
			}
			else if (state == doorOpening) {
				arriveTime += STANDING_TIME + CLOSING_DOOR_TIME + SLOW_MOVING_TIME;
			}

			arriveTime += OPENING_DOOR_TIME;
		}
		else	//если от этажа кабинки до этажа вызова бльше одного этажа
		{
			if (state == doorOpening) {
				arriveTime += STANDING_TIME;
			}

			if (state == stopped || state == doorOpening) {
				arriveTime += CLOSING_DOOR_TIME;
			}

			if (state == notUsed || state == doorClosing
				|| state == stopped || state == doorOpening){
				//добавить время преодаления этажа старта
				arriveTime += ACCELERATION_TIME;
			}

			//добавить время, необходимое чтобы доехать до этажа вызова
			arriveTime += (abs(cb.floorNumber - currentFloor) - 2)*MOVING_TIME
				+ ACCELERATION_TIME + OPENING_DOOR_TIME;
		}
		return arriveTime;
	}
	return -1;	//рассчитать время невозможно
}

//получить вызов increment по счету от первого этажа,
//на котором может остановиться кабинка.
//Если increment больше чем всего кнопок на этажах
//то будет возвращен пустой вызов
const CallButton ElevatorCar::NextFloor(int increment)const
{
	//если следующий вызов через increment этажей будет повторным
	if (increment < 0 || increment > 2 * floorCount -3) {
		return CallButton();	//вернуть пустой вызов
	}

	//если кабинка тормозит
	if (state == stopsUp || state == stopsDown 
		|| state == slowMovingUp || state == slowMovingDown) {
		//остановиться кабинка может только на следующем этаже
		increment++;
	}
	//если кабинка набирает или уже набрала скорость 
	else if (state == startsUp || state == startsDown 
		|| state == movingUp || state == movingDown){
		//остановиться кабинка сможет только через этаж 
		increment += 2;
	}

	if (movingDirection == up)	//если кабинка движется вверх
	{
		//если следующий вызов имеет то же направление и находится выше
		if (floorCount - currentFloor - 2 >= increment) {
			return CallButton(currentFloor + increment, bUp);
		}
		//если следующий вызов имеет противоположенное направление
		else if (2 * floorCount - currentFloor - 3 >= increment) {
			return CallButton(floorCount - (currentFloor + increment
				- (floorCount - 2)), bDown);
		}
		//если следующий вызов имеет то же направление, но находится ниже
		else	
		{
			return CallButton(currentFloor - (floorCount * 2 - 2 - increment), bUp);
		}
	}
	else //если кабинка стоит или движется вниз
	{
		//если следующий вызов имеет то же направление и находится ниже
		if (currentFloor > increment) {
			return CallButton(currentFloor - increment, bDown);
		}
		//если следующий вызов имеет противоположенное направление
		else if (increment - currentFloor < floorCount -1) {
			return CallButton(increment - currentFloor, bUp);
		}
		//если следующий вызов имеет то же направление, но находится выше
		else
		{
			return CallButton(currentFloor + (floorCount * 2 - 2 - increment), bDown);
		}
	}
}


//=============================ElevatorCar========================================
//================================================================================
//=============================ElevatorG==========================================


ElevatorG::ElevatorG(): Elevator(0)
{
	//создание массива для кабинок лифта
	cars = new ElevatorCar*[carCount];

	//создать массив ссылок на дочерний класс
	carsG = new ElevatorCarG*[carCount];

	//создание нужного числа кабинок лифта нового типа
	for (int j = 0; j < carCount; j++) 
	{
		cars[j] = carsG[j] = new ElevatorCarG(j + 1, this);
	}

	runningTime = 0;
	callGenerationLevel = 0;
	speedID = 2;

	callLogName = CALL_LOG_FILE_NAME;

	//подключить графику
	GraphicsON();

	//создать файл для записи вызовов и заказов
	callLogFile.open(callLogName);
	callLogFile.fill('0');

	stateFileLog = !callLogFile.is_open();	//задать обратную доступность файла
	UpdateFileLogState(!stateFileLog);	//изменить доступность файла журнала
	if (callLogFile.is_open()) {	//если журнал удалось открыть
		callLogFile.close();	//закрыть файл
	}
}

ElevatorG::~ElevatorG()
{
	callLogFile.open(callLogName, ios::app);
	if (callLogFile.is_open()) {
		callLogFile << ends;	//добавить 0 в конец файла журнала
	}

	delete carsG;
}

//включить графику
void ElevatorG::GraphicsON()
{
	int consolX = 53; //ширина консоли
	int consolY = floorCount + 13;	//высота консоли
	if (carCount > 4) {	//если больше 4 кабинок лифта
		consolX += 9 * (carCount - 4);	//расширить консоль
	}

	// инициализация графики и очистка экрана
	init_graphics(consolX, consolY);

	//рамка для названия программы
	set_cursor_pos(8, 1);
	set_color(cDARK_GRAY);
	cout << "\xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD";
	cout << "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCB\xCD\xCD\xCD\xCD";
	cout << "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB";
	set_cursor_pos(8, 2);
	cout << "\xBA                    \xBA                \xBA";
	set_cursor_pos(8, 3);
	cout << "\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD";
	cout << "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCA\xCD\xCD\xCD\xCD";
	cout << "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC";

	//время
	set_cursor_pos(30, 2);
	set_color(cDARK_RED);
	cout << "time: ";

	//название программы
	set_cursor_pos(9, 2);
	set_color(cDARK_GREEN);
	cout << "\x05 Elevator program \x05";

	set_cursor_pos(2, 4);
	set_color(cDARK_GRAY);
	cout << "FileLog connection:       Generation:0   Speed:1";

	set_color(cDARK_GREEN);
	set_cursor_pos(1, 5);
	//нарисовать верхний разделитель экрана
	for (int i = 0; i <= carCount; i++)
	{
		cout << "\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC2";
	}
	cout << "\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4";

	for (int j = 0; j < floorCount; j++) {
		set_cursor_pos(2, floorCount - j + 5);	//вывести номер этажа
		cout << setw(3) << j + 1;				//на экран слева
		set_cursor_pos(carCount * 9 + 14, floorCount - j + 5);
		cout << j + 1;							// и справа

		//нарисовать разделители между кабинками
		for (int i = 0; i <= carCount; i++)
		{
			set_cursor_pos(i * 9 + 9, floorCount - j + 5);
			cout << "\xB3";
		}
	}
	set_cursor_pos(3, floorCount + 6);
	cout << "\x1A\xFD\x1B";	//значок цели
	cout << "\nPress any key to stop time or +/- or \x11/\x10...";
}

// отобразить на экране состояние лифта в текущий момент времени
void ElevatorG::SystemDisplay()const
{
	set_cursor_pos(36, 2);	//место для времени

	//отобразить время
	if (runningTime / 1000 >= 60) {
		cout << runningTime / 60000 << "m ";
	}
	cout << setw(2) << runningTime % 60000 / 1000 << ".";
	cout << (runningTime % 1000) / 100 << "s";

	for (int j = 0; j < carCount; j++) {
		//отобразить кабинку
		carsG[j]->CarDisplay();
	}
}

// перевести систему в следующее состояние
// возвращает false в случае необходимости завершить программу
bool ElevatorG::SystemTick()
{
	if (_kbhit())	//если была нажата кнопка
	{
		//если получена команда на завершение
		if (!GetDirections()) {
			return false;
		}
	}

	Elevator::SystemTick(SPEED_VALUES[speedID]);

	runningTime += TIME_INTERVAL;
	return true;
}

//получить задачу для лифта через диалог с пользователем,
//если получено указание на завершение программы вернет false
const bool ElevatorG::GetDirections()
{
	//очистка нижней части экрана с диалогом
	for (int j = 0; j < 5; j++)
	{
		set_cursor_pos(1, floorCount + 8 + j);
		clear_line();
	}
	set_cursor_pos(1, floorCount + 8);

	char pressedKey = _getch();	//последнее нажатие клавиши

	//изменение интенсивности генерации пассажиров лифта
	if (pressedKey == 43 || pressedKey == 45)
	{
		pressedKey == 43 ? callGenerationLevel++ : callGenerationLevel--;
		callGenerationLevel = (callGenerationLevel + 10) % 10;
		DisplayGenerationLevel();
		return true;
	}

	//изменение скорости моделирования системы
	if (pressedKey == -32)
	{
		pressedKey = _getch();
		if (pressedKey == 77) {
			speedID++;	//выбрать следующий вариант скорости модуляции лифта
		}
		else if (pressedKey == 75) {
			speedID--;	//выбрать предидущий вариант скорости модуляции лифта
		}
		if (pressedKey == 75 || pressedKey == 77)
		{
			speedID = (speedID + NUMBER_OF_SPEEDS) % NUMBER_OF_SPEEDS;
			DisplaySpeed(); //отобразить скорость модуляции
			return true;
		}
	}

	//1 вопрос
	cout << "Are you inside the elevator?(yes/no/Enter/Esc): ";
	char answer;
	//цикл пока не нажмется допустимая кнопка
	do {
		answer = _getch();
	} while (answer != 'y' && answer != 'n'
		&& answer != '\x0D' && answer != '\x1B');

	if (answer == '\x0D') //нажат Enter
	{
		cout << "\bcancel";	//закончить диалог с пользователем
		return true;		// и продолжить выполнение программы
	}
	if (answer == '\x1B') {	//нажат ESC
		return false;		//закончить диалог и программу
	}
	cout << (answer == 'y' ? "yes" : "no");

	if (answer == 'y') //Если заказ изнутри лифта
	{
		char cElevNum;	//номер кабинки

		//получение номера кабинки
		cout << "\nEnter the elevator number:  ";
		do {
			cElevNum = _getch();
			//пока вводится несуществующий номер
		} while (cElevNum <'1' || cElevNum >('0' + carCount));
		cout << cElevNum;

		//получение этажа для высадки пассажиров или
		//команды на закрытие или открытие дверей
		cout << "\nEnter destination floor number or"
			<< "\n'c'/'o'-for closing/openning elevator door: ";
		char ch1 = '0', ch2;
		//получение первой цифры или буквы
		do {
			ch2 = _getch();
			//пока вводится несуществующий номер и не 'O', не 'C'
		} while (ch2 != 'c' && ch2 != 'o' && (ch2 <= '0' ||
			ch2 > ('0' + floorCount) || ch2 > '9'));
		cout << ch2;
		//если при любой второй цифре выйдет несуществующий
		//этаж, то не спрашивать вторую цифру

		//число десятков в числе этажей
		char nDec = '0' + floorCount / 10;
		if (ch2 <= nDec)
		{
			//получение второй цифры при необходимости
			ch1 = ch2;
			do {
				ch2 = _getch();
				if (ch2 == 13) //если нажат пробел
				{
					ch2 = ch1;	//завершить
					ch1 = '0';	//ввод
					break;		//этажа
				}
				//пока вводится несуществующий номер
			} while (!(ch1 == nDec && ch2 >= '0' && ch2 <= ('0' + floorCount % 10))
				&& !(ch1 != nDec && isdigit(ch2)));
			if (ch1 != '0') cout << ch2;
		}

		if (ch2 == 'c')
		{
			cout << "lose the door";
			GetCar(cElevNum - '0')->CloseDoor();
		}
		else if (ch2 == 'o')
		{
			cout << "pen the door";
			GetCar(cElevNum - '0')->OpenDoor();
		}
		else
		{
			int floor = (ch1 - '0') * 10 + (ch2 - '0') - 1;
			//соответствующая кабинка принимает заказ
			GetCar(cElevNum - '0')->SetOrderFloor(floor);

			set_cursor_pos(1, floorCount + 12);
			cout << "Order is accepted!";
		}
	}
	else // вызов с этажа
	{
		cout << "\nWhat floor are you on? ";
		char ch1 = '0', ch2;
		//получение первой цифры
		do {
			ch2 = _getch();
			//пока вводится несуществующий номер
		} while (ch2 <= '0' || ch2 > ('0' + floorCount) || ch2 > '9');
		cout << ch2;

		//число десятков в числе этажей
		char nDec = '0' + floorCount / 10;
		if (ch2 <= nDec)
		{
			//получение второй цифры при необходимости
			ch1 = ch2;
			do {
				ch2 = _getch();
				if (ch2 == 13) //если нажат пробел
				{
					ch2 = ch1;	//завершить
					ch1 = '0';	//ввод
					break;		//этажа
				}
				//пока вводится несуществующий номер
			} while (!(ch1 == nDec && ch2 >= '0' && ch2 <= ('0' + floorCount % 10))
				&& !(ch1 != nDec && isdigit(ch2)));
			if (ch1 != '0') cout << ch2;
		}

		CallButton cb; //кнопка вызова с этажа
		cb.floorNumber = (ch1 - '0') * 10 + (ch2 - '0') - 1;
		if (cb.floorNumber == 0) {	//если верхний этаж
			cb.direction = bUp;		//то только кнопка вниз
		}
		else if (cb.floorNumber == floorCount - 1) {	//если нижний этаж
			cb.direction = bDown;	//то только кнопка вверх
		}
		else //не верхний и не нижний этаж
		{
			char dir;
			cout << "\nWhich directon you need to move?(up/down) ";
			//получение разрешенного смвола для направления
			do {
				dir = _getch();
			} while (dir != 'u' && dir != 'd');
			cout << (dir == 'u' ? "up" : "down");

			cb.direction = (dir == 'u' ? bUp : bDown);
		}
		SetTheCall(cb); //обработать вызов с этажа

		//если направление неочевидно, то была еще одна строка с вопросом
		set_cursor_pos(1, floorCount +
			(cb.floorNumber == 0 || cb.floorNumber == floorCount - 1 ? 10 : 11));
		cout << "Call is accepted!";
	}
	return true;
}

//обработать вызов с этажа
//если вызов может быть обслужен в этот же момент, вернуть false
bool ElevatorG::SetTheCall(const CallButton cb)
{
	if (Elevator::SetTheCall(cb))
	{
		//отобразить вызов на экране
		set_cursor_pos(cb.direction == bUp ? 7 : 8,
			floorCount - cb.floorNumber + 5);
		cout << (cb.direction == bUp ? "\x1E" : "\x1F");

		set_cursor_pos(carCount * 9 + (cb.direction == bUp ? 12 : 11),
			floorCount - cb.floorNumber + 5);
		cout << (cb.direction == bUp ? "\x1E" : "\x1F");

		callLogFile.open(callLogName, ios::app);
		UpdateFileLogState(callLogFile.is_open());	//обновить доступность файла

		if (callLogFile.is_open())
		{
			//записать время и вызов
			callLogFile << setw(2) << runningTime / 3600000
				<< ":" << setw(2) << runningTime / 60000
				<< ":" << setw(2) << runningTime % 60000 / 1000
				<< "." << (runningTime % 1000) / 100
				<< "  " << cb.floorNumber + 1
				<< (cb.direction == bUp ? "\x1E" : "\x1F") << endl;

			callLogFile.close();
		}
		return true;
	}
	return false;
}

//выключить кнопку с этажа
void ElevatorG::CallButtonOff(const CallButton cb)
{
	Elevator::CallButtonOff(cb);

	//стереть отображение кнопки вызова
	set_cursor_pos(cb.direction == bUp ? 7 : 8,
		floorCount - cb.floorNumber + 5);
	cout << " ";

	set_cursor_pos(carCount * 9 + (cb.direction == bUp ? 12 : 11),
		floorCount - cb.floorNumber + 5);
	cout << " ";
}

//записать в журнал заказ из кабинки
void ElevatorG::WriteCarOrder(int idCar, int orderFloor)
{
	callLogFile.open(callLogName, ios::app);
	UpdateFileLogState(callLogFile.is_open()); //обновить доступность файла

	if (callLogFile.is_open())
	{
		//записать время и заказ
		callLogFile << setw(2) << runningTime / 3600000
			<< ":" << setw(2) << runningTime / 60000
			<< ":" << setw(2) << runningTime % 60000 / 1000
			<< "." << (runningTime % 1000) / 100
			<< "     " << orderFloor + 1
			<< "\x1B" << char('A' + idCar - 1) << endl;

		callLogFile.close();
	}
}

//вывести уровень генерации пассажиров на экран
void ElevatorG::DisplayGenerationLevel()const
{
	set_cursor_pos(39, 4);
	set_color(cYELLOW);
	cout << callGenerationLevel;
}

//получить уровень генерации пассажиров
int ElevatorG::GetGenerationLevel()const
{
	return callGenerationLevel;
}

//вывести скорость моделирования лифта
void ElevatorG::DisplaySpeed()const
{
	set_cursor_pos(49, 4);
	set_color(cBLUE);

	cout << left << setw(4) << SPEED_VALUES[speedID] << right;
}

//изменить доступность файла журнала вызовов
void ElevatorG::UpdateFileLogState(bool fstate)
{
	if (fstate != stateFileLog)	//если новое и старое состояния различны
	{
		set_cursor_pos(21, 4);
		if (fstate == true)	//если файл удалось открыть
		{
			stateFileLog = true; 

			set_color(cGREEN);
			cout << "true ";

			callLogFile.close();	//очистить файл журнала
			callLogFile.open(callLogName);

			//Запустить приложение для вывода журнала вызовов на экран
			int consolY = floorCount + 13;	//высота консоли
			string str = "start "; //команда для запуска приложения
			char buf[BUF_SIZE];	//массив для числа
			string str1 = itoa(consolY, buf, 10);	//аргумент-высота консоли
			//команда на запуск
			system((str + LOG_READER_NAME + " " + callLogName + " " + str1).c_str());
		}
		else {
			stateFileLog = false;
			set_color(cRED);
			cout << "false";
		}
	}
}

//===============================ElevatorG========================================
//================================================================================
//=============================ElevatorCarG=======================================


ElevatorCarG::ElevatorCarG(int nElevator, ElevatorG* ptrElevSys):
	ElevatorCar(nElevator, ptrElevSys),lastfloor(0), 
	parentElevG(ptrElevSys)
{}

//установить заказ на этаж
//если кабинка уже открыта на этом этаже, вернуть false
bool ElevatorCarG::SetOrderFloor(const int nFloor)
{
	if (ElevatorCar::SetOrderFloor(nFloor))
	{
		//отобразить заказанный этаж
		set_cursor_pos(2 + carNumber * 9, floorCount - nFloor + 5);
		cout << "\x1B";
		
		//записать заказ из лифта в файл
		parentElevG->WriteCarOrder(carNumber,nFloor);
		return true;
	}
	return false;
}

//выключить кнопку в кабинке или на этаже
void ElevatorCarG::TurnOffButton()
{
	ElevatorCar::TurnOffButton();

	//стереть отображение заказа на текущий этаж 
	set_cursor_pos(2 + carNumber * 9, floorCount - currentFloor + 5);
	cout << " ";
}

//первести кабинку в следующее состояние
void ElevatorCarG::Tick()
{
	lastfloor = currentFloor;
	ElevatorCar::Tick();
}

//отобразить кабинку на экране
void ElevatorCarG::CarDisplay()const
{
	//выбрать цвет для кабинки
	set_color(color(15 - carNumber));

	//стереть прошлое отображение состояния кабинки
	set_cursor_pos(3 + carNumber * 9, floorCount - lastfloor + 5);
	cout << "     ";

	//отобразить новое состояние кабинки
	set_cursor_pos(3 + carNumber * 9, floorCount - currentFloor + 5);
	switch (state)
	{
	case startsUp: cout << "\xDB\xB1\x18\xDB"; break;
	case startsDown: cout << "\xDB\xB1\x19\xDB"; break;
	case slowMovingUp: cout << "\xDB\xB1\x18\xDB"; break;
	case slowMovingDown: cout << "\xDB\xB1\x19\xDB"; break;
	case stopsUp: cout << "\xDB\xB1\x18\xDB"; break;
	case stopsDown: cout << "\xDB\xB1\x19\xDB"; break;
	case movingUp: cout << "\xDB\x18\x18\xDB"; break;
	case movingDown: cout << "\xDB\x19\x19\xDB"; break;
	case doorOpening: cout << "\xDB<>\xDB"; break;
	case stopped: cout << "\xDB  \xDB"; break;
	case doorClosing: cout << "\xDB><\xDB"; break;
	case notUsed: cout << "\xDB\xB1\xB1\xDB"; break;
	default:
		break;
	}

	//отобразить направление движения кабинки
	switch (movingDirection)
	{
	case up: cout << "\x1E"; break;
	case down: cout << "\x1F"; break;
	default: cout << " ";  break;
	}

	set_cursor_pos(4 + carNumber * 9, floorCount + 6);
	if (IsFree()) { //если лифт свободен
		cout << "   ";	//стереть цель
	}
	else
	{
		//отобразить целевой вызов или заказ кабинки
		cout << setw(2) << targetFloor + 1;
		//если кабинка едет на вызов
		if (targetCallAvailability) {
			cout << (targetCall.direction == bUp ? "\x1E" : "\x1F");
		}
		else {	//кабинка едет на заказ 
			cout << " ";
		}
	}

	//вернуть белый цвет шрифта
	set_color(cLIGHT_GRAY);
}


//=============================ElevatorCarG=======================================
//================================================================================
//============================CallGeneration======================================


namespace CallGeneration
{
	list<Man*> Man::people;
	ElevatorG* Man::elev = 0;

	Man::Man() : state(wait)
	{
		cb.floorNumber = rand() % NUM_FLOORS; //генерация номера этажа

		if (cb.floorNumber == NUM_FLOORS - 1) {	//если этаж верхний
			cb.direction = bDown;	//кнопка может быть только вниз
		}
		else if (cb.floorNumber == 0) {	//если этаж нижний
			cb.direction = bUp;	//кнопка может быть только вверх
		}
		else {
			cb.direction = (rand() % NUM_FLOORS > cb.floorNumber ?
				bUp : bDown);	//генерации направления (если этаж выше 
				//среднего, то вероятность кнопки вниз выше, чем вверх)
		}
		elev->SetTheCall(cb);	//вызвать лифт
	}

	//установить заказ
	void Man::SetOrder()
	{
		//выбрать такой этаж, который был бы ниже текущего этажа,
		//если был вызов вниз и наоборот
		if (cb.direction == bUp) {
			order = rand() % (NUM_FLOORS - cb.floorNumber - 1)
				+ cb.floorNumber + 1;
		}
		else {
			order = rand() % (cb.floorNumber);
		}
		//заказать этаж для соответствующей кабинки
		elev->GetCar(carNumber)->SetOrderFloor(order);
	}

	//перевести пассажира в следующее состояние(если он
	//уже сделал заказ, то вернуть false - можно удалять
	bool Man::Tick()
	{
		if (state == wait)	//если пассажир ожидает кабинку лифта
		{
			int car = elev->FindReadyCar(cb);	//проверить, есть ли готовая?
			if (car!=-1) //если есть какая-нибудь
			{
				state = entry;	//начать входить в нее
				carNumber = car;	//запомнить ее номер

				//сгенерировать время вхождения и нажатия на кнопку этажа
				carEntryTime = (rand() % (STANDING_TIME / TIME_INTERVAL))
					* TIME_INTERVAL + 1000;
				//от 0 до время стоянки лифта + 1 секунда
			}
		}
		else if (state == entry) //если пассажир входит в лифт
		{
			if (carEntryTime > 0) {	//пока время вхождения не истекло
				carEntryTime -= TIME_INTERVAL; //уменьшить его
			}
			else {	//если время истекло
				SetOrder();	//нажать кнопку заказа этажа
				return false;	//действия пассажира окончены
			}
		}
		return true;	//пассажир еще что-то сделает в будущем
	}

	//подготовка списка пассажиров
	void Man::Initialization(ElevatorG* el)
	{
		elev = el;	//запомнить лифт
		srand(time(NULL));	//обнулить ГСЧ
	}

	//перевести всех пассажиров в следующее состояние
	void Man::PeopleTick()
	{
		//перевести каждого пассажира в следующее состояние,
		//если он уже заказал этаж, то его можно удалить из списка
		list<Man*>::iterator iter = people.begin();
		while (iter != people.end())
		{
			if ((*iter)->Tick()) {
				iter++;
			}
			else {	//удалить пассажира
				list<Man*>::iterator temp = iter;
				temp++;
				delete (*iter);
				people.erase(iter);
				iter = temp;
			}
		}

		//получить уровень интенсивности генерации пассажиров
		int lvl = elev->GetGenerationLevel();
		if (lvl)	//если он не нулевой
		{
			//раситать диапозон значений (чем он больше - тем меньше вероятность
			//что сгенерируется 0 == создастся новый пассажир
			if (!(rand() % (((9 - lvl) * (9 - lvl) * 2 + 7))
				* 200 / TIME_INTERVAL)) {
				people.push_back(new Man);
			}
		}

	}

	//очистить список пассажиров
	void Man::RemovePeople()
	{
		//очистить память, занятую пассажирами
		for (int j = 0; j < people.size(); j++)
		{
			delete people.back();
			people.pop_back();
		}
	}
}

//end.