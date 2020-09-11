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

//������� ����
Elevator::Elevator(bool key) :
	floorCount(NUM_FLOORS), carCount(NUM_CARS)
{
	if (key)	//��� ������������ ����� ����������� ������ �������
	{
		//�������� ������� ��� ������� �����
		cars = new ElevatorCar*[carCount];
		//�������� ������� ����� ������� �����
		for (int j = 0; j < carCount; j++) {
			cars[j] = new ElevatorCar(j + 1, this);
		}
	}

	// ������� ����� ������ �� ������
	callsDown = new bool[floorCount];
	callsUp = new bool[floorCount];

	for (int j = 0; j < floorCount; j++) {
		//��������� ������ �� �����
		callsDown[j] = false;
		callsUp[j] = false;
	}
}

//������� ����
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

//���������� ����� � �����
//���� ����� ����� ���� �������� � ���� �� ������, ������� false
bool Elevator::SetTheCall(const CallButton cb)
{
	CheckButton(cb); //��������� ������ �� ����������

	//���� ����� ������ ��� ������
	if (cb.direction == bUp && callsUp[cb.floorNumber]
		|| cb.direction == bDown && callsDown[cb.floorNumber]) {
		return false;
	}

	//���� ���� ������� �� ����� ������, ������� ����� 
	//����� ������� �����, �� ������� �����
	for (int j = 0; j < carCount; j++)
	{
		if (cars[j]->GetCurrentFloor() == cb.floorNumber)
		{
			carStateType eState = cars[j]->GetState();
			movingDirectionType mDirection = cars[j]->GetMovingDirection();

			//���� ����������� �������� ������� ��������
			if ((mDirection == up || mDirection == no) && cb.direction == bUp
				|| (mDirection == down || mDirection == no) && cb.direction == bDown)
			{
				//���� ����� ����������� ��� ��� �������
				if (eState == doorOpening || eState == stopped) {
					return false; //������ �� ������
				}
				//���� ����� ����������� ��� �������
				else if (eState == doorClosing || eState == notUsed)
				{
					cars[j]->OpenDoor(); //������� �����
					return false;
				}
			}
		}
	}

	//���������� ����� � ������
	if (cb.direction == bUp) {
		callsUp[cb.floorNumber] = true;
	}
	else if (cb.direction == bDown) {
		callsDown[cb.floorNumber] = true;
	}

	//���� ����� �������� ����� �����-�� �������, �� �� ������������ ���
	for (int j = 0; j < carCount; j++)
	{
		//���� � ������� ���� ������� �����
		if (cars[j]->GetTargetCallAvailability())
		{
			//���� ���� ����� ��������� � �������
			if (cars[j]->GetTargetCall() == cb) {
				return true;
			}
		}
	}


	//�� ���� ��������� ������� � �������, ������� �� ���� � �������� 
	//����� ����� ��������� �����, ������� ��, ������� �������� ����� ������
	double minArivTime = -1;
	ElevatorCar* theMostQuicklyCar;

	for (int j = 0; j < carCount; j++)
	{
		//���� ������� �������� ��� ����� ��������� ����� �������� � 
		//�� ������� ������
		if (cars[j]->IsFree() || cars[j]->CallIsBetweenCarAndTargetFloor(cb))
		{
			//���������� ����� �������� �� ����� ��� �������,
			//���� ��� ������������� ���� ������� ����
			double arivTime = cars[j]->MinCalculateTheCallAriveTime(cb);
			//���� ��� ������ ���������� �������
			if (minArivTime == -1)
			{
				minArivTime = arivTime;
				theMostQuicklyCar = cars[j];
			}
			else //2 ��� 3 ��� ... ���������� �������
				if (arivTime < minArivTime)
				{
					minArivTime = arivTime;
					theMostQuicklyCar = cars[j];
				}
		}
	}

	//���� ���� �������, ������� ����� � ������ ��� theMostQuicklyCar,
	//�� �� ������� ���� ������ ������, �� ����� ����� ������� �� ���
	if (minArivTime > -1) {	//���� ����� ���� ������� ������ ����
							//�������, ������� ����� ���������� �����
		double minArivTimeBusyElev = -1;
		//�������, ������� ����� ������������ ���� ����� ������� 
		//������, ������ ��������� ����� ������� ����
		ElevatorCar* theMostQuicklyBusyElevator;

		for (int j = 0; j < carCount; j++)
		{
			//���� ������� � �� �������� ���� ��������� �����
			//theMostQuicklyCar � �������
			if (cars[j]->IsBetweenCarAndCall(theMostQuicklyCar, cb))
			{
				//��������� ����� �������� �� �����,
				//��������� ���� ����
				double arivTime = cars[j]->CalculateTheCallAriveTime(cb);
				//���� ��� ������ ���������� �������
				if (minArivTimeBusyElev == -1)
				{
					minArivTimeBusyElev = arivTime;
					theMostQuicklyBusyElevator = cars[j];
				}
				else
					//2 ��� 3 ��� ... ���������� �������
					if (arivTime < minArivTimeBusyElev)
					{
						minArivTimeBusyElev = arivTime;
						theMostQuicklyBusyElevator = cars[j];
					}
			}
		}

		//���� �� ���� ������� �� ����� ���������� ������� ��� ��� ����
		//������� � ����� �� �������� >= ��� � theMostQuicklyCar 
		if (minArivTimeBusyElev == -1 || minArivTimeBusyElev >= minArivTime) {
			//���������� ������� ���� ��� theMostQuicklyCar
			theMostQuicklyCar->SetTargetFloor(cb);
		}
		//����� �� ��������� ����� ������, � ������� �����
	}
	return true;
}

//�������� �� ������ �� �����?
bool Elevator::theCallButtonIsOn(const CallButton cb)const
{
	CheckButton(cb); ////��������� ������ �� ����������

	//���� ����� �����, �� ��������� � ������� ������ �����
	//���� ����� ����, - � ������� ������ ����
	if (cb.direction == bUp && callsUp[cb.floorNumber]
		|| cb.direction == bDown && callsDown[cb.floorNumber]) {
		return true;
	}
	return false;
}

//�������� �� ������ ����� ������ �������? 
bool Elevator::theCallIsTargetOfAnotherElevators(const CallButton cb)const
{
	CheckButton(cb); //��������� ������ �� ����������

	for (int j = 0; j < carCount; j++)
	{
		//���� � ������� ���� ������� �����
		if (cars[j]->GetTargetCallAvailability())
		{
			//� ���� �� ��������� � �������
			if (cars[j]->GetTargetCall() == cb)
				return true;
		}
	}
	return false;
}

//�������� ������� ����� �� ��� ������
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

//����� ����� ��������� ������ ��� ������� �����,
//� ������ ���� ��������� �������
CallButton Elevator::FindTask(const ElevatorCar*const theCar)const
{
	if (theCar == 0) {
		throw ElevatorError(3,"The link points to a non-existent car");
	}

	//�������� ����� ���� ������� �����
	ElevatorCar** newCars = new ElevatorCar*[carCount];
	for (int j = 0; j < carCount; j++)
	{
		newCars[j] = new ElevatorCar(*cars[j]);
	}
	//������� ����� ������ ���� ��������� ��� ������� ���������
	//����� ����� � �������, ��� ����� � ����� ����� �������

	
	int j = 0;
	//������ ���� � ������������, �� ������� ������� ����� ������������
	CallButton cb = theCar->NextFloor(j++);

	//���� �� ����� ���������� ��� ����� � ������������ �� ������ ����
	while (cb.floorNumber != -1)
	{
		//���� ���� �������� �������
		if (theCar->isOrderFloor(cb.floorNumber))
		{
			//�������� ����� �������
			for (int i = 0; i < carCount; i++) {
				delete newCars[i];
			}
			delete newCars;

			return cb; //������� ���� �����
		}

		//���� ������, �������������� ����� � ����������� ��������
		if (theCallButtonIsOn(cb))
		{
			bool isServiced = false; //����� ��� �������������?
			for (int elev = 0; elev < carCount; elev++)
			{
				//���� � ������� ���� ������� ����� � �� ����� ���������
				if (newCars[elev]->GetTargetCallAvailability() &&
					newCars[elev]->GetTargetCall() == cb)
				{
					isServiced = true; //������ �� �������������
					break;
				}
			}
			if (!isServiced) //���� ����� �� �������������
			{
				// ������ ����� ������� �������, ������� ������
				//������� �����

				double minArivTime = -1;
				ElevatorCar* theMostQuicklyCar;

				for (int j = 0; j < carCount; j++)
				{
					//���� ������� �������� ��� ��������� ����� ��������,
					//��� ������� ������������ ������ � ������� � 
					//������� ���� ������� ������, ��� �����
					if (newCars[j]->IsFree() ||
						(newCars[j]->IsBetweenCarAndCall(theCar, cb)
							&& newCars[j]->TargetIsEarlierThanCall(cb)))
					{
						//��������� ����� �������� ������� �� �����
						double arivTime = newCars[j]->CalculateTheCallAriveTime(cb);
						//���� ��� ������ ���������� �������
						if (minArivTime == -1)
						{
							minArivTime = arivTime;
							theMostQuicklyCar = newCars[j];
						}
						else //2 ��� 3 ��� ... ���������� �������
							if (arivTime < minArivTime)
							{
								minArivTime = arivTime;
								theMostQuicklyCar = newCars[j];
							}
					}
				}
				// ���� ����� �������� ��� ������� ������� <= ���
				//��� ����� ������� ��� ���������� ������� �� �������
				if (theCar->CalculateTheCallAriveTime(cb) <= minArivTime || minArivTime == -1)
				{
					//������� ����� �������
					for (int i = 0; i < carCount; i++) {
						delete newCars[i];
					}
					delete newCars;

					return cb; //������ ����� (���� � ������������ ��������)
				}
				else { //���� ������� ������� �� ������ ���� ������ �� �����
					//�� ��������� ��������� ����� ������� ������� � 
					//���������, ����� ��� �������� �� �����, ������� � 
					//timer ����������� �� ��� �����
					theMostQuicklyCar->MoveElevator(cb, minArivTime);
				}
			}
		}

		//��������� ���� � ������������� ������� � ������ ��������,
		//�� ������� ����� ������������ ���� (��� ���� j=0)
		cb = theCar->NextFloor(j++);
	}

	//��� ����� ���� ����� ���������� ������, ���� �� �������
	//�� ����� ������ ��� ������� �������

	//������� ����� �������
	for (int j = 0; j < carCount; j++) {
		delete newCars[j];
	}
	delete newCars;

	return CallButton(); //������ ������ �����
}

//��������� ������ � �����
void Elevator::CallButtonOff(const CallButton cb)
{
	CheckButton(cb); //��������� ������ �� ����������

	//���� ������ �����, �� ��������� �� � ������� ������ �����
	if (cb.direction == bUp) {
		callsUp[cb.floorNumber] = false;
	}
	//���� ������ ����, �� ��������� �� � ������� ������ ����
	else if (cb.direction == bDown) {
		callsDown[cb.floorNumber] = false;
	}
}

// ��������� ������� � ��������� ���������
//factor - ����������� ������� ��������, ������������ TIME_INTERVAL
void Elevator::SystemTick(double factor)
{
	Sleep(1.0*TIME_INTERVAL/factor);	//�������� 

	for (int j = 0; j < carCount; j++)
	{
		//��������� ������� � ��������� ���������
		cars[j]->Tick();
	}
}

//����� �������, ������� ��� �������� �� ����� � ������� �� �����
int Elevator::FindReadyCar(CallButton cb)const
{
	CheckButton(cb); //��������� ������ �� ����������

	for (int j = 0; j < carCount; j++)
	{
		//���� ������� �� ������ ����� � ������ ������������ � �������
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


//������������� ������� �����
ElevatorCar::ElevatorCar(int nCar, Elevator* ptrElev) :
	carNumber(nCar), parentElev(ptrElev),floorCount(ptrElev->floorCount)
{
	targetFloor = currentFloor = rand()%floorCount; //��������� ��������� ����
	movingDirection = no;
	state = notUsed;

	//�������� ������� ����� ����������� ������ ������ �����
	orderFloors = new bool[floorCount];
	for (int j = 0; j < floorCount; j++)
	{
		orderFloors[j] = false;
	}
	targetCallAvailability = false;
	timer = 0;
}

//����������� ������� �����
ElevatorCar::ElevatorCar(ElevatorCar& elev):
	carNumber(elev.carNumber), parentElev(elev.parentElev),
	floorCount(elev.floorCount)
{
	currentFloor = elev.currentFloor;
	movingDirection = elev.movingDirection;
	state = elev.state;

	//�������� ����� ������� ������
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

//�������� ������� �����
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

//�������� ����� �������
const int ElevatorCar::GetCarNumber()const
{
	return carNumber;
}

//�������� ������� ���� �������
const int ElevatorCar::GetCurrentFloor()const
{
	return currentFloor;
}

//�������� ����������� �������� �������
const movingDirectionType ElevatorCar::GetMovingDirection()const
{
	return movingDirection;
}

//�������� ��������� �������
const carStateType ElevatorCar::GetState()const
{
	return state;
}

//�������� �� ���� ����������?
const bool ElevatorCar::isOrderFloor(const int floor)const
{
	return orderFloors[floor];
}

//�������� ������� ����
const int ElevatorCar::GetTargetFloor()const
{
	return targetFloor;
}

//���� �� �����,���������� ����� �������
const bool ElevatorCar::GetTargetCallAvailability()const
{
	return targetCallAvailability;
}

//������� ����� ��� �������
const CallButton ElevatorCar::GetTargetCall()const
{
	return targetCall;
}

//�������� �� �������?
const bool ElevatorCar::IsFree()const
{
	return state == notUsed;
}

//������� �����
void ElevatorCar::OpenDoor()
{
	if (state == doorClosing || state == notUsed) {
		state = doorOpening;
		timer = OPENING_DOOR_TIME;
	}
}

//������� �����
void ElevatorCar::CloseDoor()
{
	if (state == stopped)
	{
		state = doorClosing;
		timer = CLOSING_DOOR_TIME;
	}
}

//���������� ����� �� ����
//���� ������� ��� ������� �� ���� �����, ������� false
bool ElevatorCar::SetOrderFloor(const int nFloor)
{
	CheckFloor(nFloor); //��������� ���� �� ����������

	//���� ������� ����, �� ������� ������� ������� 
	//��� ������� ��� ���������� ����
	if (nFloor == currentFloor && state == stopped || orderFloors[nFloor]) {
		return false;  //������ �� ������
	}

	//���� ������� ����, �� ������� ������� ��������� ����� ��� �� ������������
	if (nFloor == currentFloor && (state == doorClosing || state == notUsed)) {
		OpenDoor(); //������� �����
		return false; //� ������ ������ �� ������
	}

	orderFloors[nFloor] = true; //���������� ���������� ����

	DeterminateTargetFloor(); //���������� ������� ����

	return true;
}

//���������� ������� ����
void ElevatorCar::SetTargetFloor(const CallButton cb)
{
	parentElev->CheckButton(cb); //������� ����� �� ����������

	targetCallAvailability = true; //���� ������� - ����� � �����
	targetCall = cb; //��� ������� �����
	targetFloor = cb.floorNumber; //������� ����

	//���� ���� ������ ���� �������� ����� �������
	if (cb.floorNumber > currentFloor) {
		movingDirection = up; //����������� �������� - �����
	}
	//���� ���� ������ ���� �������� ����� �������
	else if (cb.floorNumber < currentFloor) {
		movingDirection = down; //����������� �������� - ����
	}
}

//��������� ������ � ������� ��� �� �����
void ElevatorCar::TurnOffButton()
{
	//��������� ������ �������� ����� � �������
	orderFloors[currentFloor] = false;

	//���� � ������� ��� ����� ����� � �����
	if (targetCallAvailability)
	{
		//��������� ��������������� ������ ������ �� �����
		parentElev->CallButtonOff(targetCall);

		//�������� ���� �������, ����� � �����
		targetCallAvailability = false;
	}
}

//���������� ������� ���� ��� �������,
//���� �������� ����� ���, ������� false
const bool ElevatorCar::DeterminateTargetFloor()
{
	//���� ���� ��������, ������ �� ��������� �������� �� ������� ����
	//� ����������� ��������� ���� ���������� �� ���
	if (state == stopsUp || state == stopsDown || state == slowMovingUp
		|| state == slowMovingDown) {
		return true; //������� ���� �������� �������
	}

	//�������� ������ ��������� �����
	targetCallAvailability = false;

	//�������� ������ (����� ��� �����)
	CallButton taskCB = parentElev->FindTask(this);

	if (taskCB.floorNumber == -1) //���� ������ ���
	{
		movingDirection = no; //������� ��������
		return false; //���� �� ���������
	}
	else //���� ���� ������� ������
	{
		CheckFloor(taskCB.floorNumber); //�������� ����� �� ����������

		targetFloor = taskCB.floorNumber; //���������� ������� ����

		//���� ������(���� ������ ��� ������) ���� �������
		if (taskCB.floorNumber > currentFloor) {
			movingDirection = up; //����������� �������� �����
		}
		//���� ������(���� ������ ��� ������) ���� �������
		else if (taskCB.floorNumber < currentFloor) {
			movingDirection = down; //����������� �������� ����
		}

		//���� ���� ����� � �����, ��������������� ������
		if (parentElev->theCallButtonIsOn(taskCB))
		{
			parentElev->CheckButton(taskCB); //�������� ������ �� �����������

			targetCallAvailability = true; //���� - ������� �����
			targetCall = taskCB; //������� ����� ����� ������
		}

		return true; //������� ���� ���������
	}
}

//�������� ������� � ��������� ���������
void ElevatorCar::Tick()
{
	CheckFloor(targetFloor); //�������� ����� �� ����������
	if (targetCallAvailability) {
		parentElev->CheckButton(targetCall); //�������� ������
	}


	if (timer > 0) //���� ����� �������� ��������� �� �����������
	{
		timer -= TIME_INTERVAL;	//��������� ���������� �����
		return;
	}

	//���� ������� ��� �������� ����� � ������� ��������� 
	if (state == movingUp || state == startsUp)
	{
		currentFloor++;
		//���� ����� ������������ �� ��������� �����
		if (targetFloor - currentFloor == 1)
		{
			state = stopsUp; //��������, �������� �����
			timer = ACCELERATION_TIME - TIME_INTERVAL;
		}
		//���� ����� ���������� ������ �� ���� �������� �������
		else if (targetFloor - currentFloor > 1) 
		{
			state = movingUp; //��������� ����� ������
			timer = MOVING_TIME - TIME_INTERVAL;
		}
		else    //���� ����� ������������ �� ������� ����� 
				//��� ����� ����
		{
			state = stopsUp; //���� ��������, ��������
			timer = ACCELERATION_TIME - TIME_INTERVAL;
		}
		return;
	}
	
	//���� ������� ��� �������� ���� � ������� ���������
	if (state == movingDown || state == startsDown)
	{
		currentFloor--;
		//���� ����� ������������ �� ��������� �����
		if (currentFloor - targetFloor == 1)
		{
			state = stopsDown; //��������, �������� ����
			timer = ACCELERATION_TIME - TIME_INTERVAL;
		}
		//���� ����� ���������� ������ �� ���� �������� �������
		else if (currentFloor - targetFloor > 1) 
		{
			state = movingDown; //��������� ���� ������
			timer = MOVING_TIME - TIME_INTERVAL;
		}
		else	//���� ����� ������������ �� ������� �����
				//��� ����� ����
		{
			state = stopsDown; //���� ��������, ��������
			timer = ACCELERATION_TIME - TIME_INTERVAL;
		}
		return;
	}

	//���� ������� ������������
	//(���������� ���������� ���� ����������)
	if (state == stopsUp || state == slowMovingUp
		|| state == stopsDown || state == slowMovingDown)
	{
		//������� ��������� �������� �����
		if (state == stopsUp || state == slowMovingUp) {
			currentFloor++;
		}
		else { //������� ��������� �������� ����
			currentFloor--;
		}

		state = doorOpening;

		//���� � ������� ��� ����� ����� � �����
		if (targetCallAvailability)
		{
			//� ����������� �� ����������� �������� ������
			//���������� ����������� �������� �������
			movingDirection = (targetCall.direction == bUp ? up : down);
		}
		//��������� ������ ������ �����/�� �����
		TurnOffButton();

		timer = OPENING_DOOR_TIME - TIME_INTERVAL;
		return;
	}

	//���� ������� ��������� �����
	if (state == doorOpening)
	{
		//������ �������/������� ����������
		state = stopped;
		timer = STANDING_TIME - TIME_INTERVAL;
		return;
	}

	//���� �������/������� ���������
	if (state == stopped)
	{
		//������� �����
		state = doorClosing;
		timer = CLOSING_DOOR_TIME - TIME_INTERVAL;
		return;
	}

	//���� ����� ��������� ��� ���� �� �������������
	if (state == doorClosing || state == notUsed)
	{
		//���� �� ���������� ����� ����� � �����
		if (!targetCallAvailability)
		{
			//���� ������� ���� ��� ������� �� ��� ���������
			if (!DeterminateTargetFloor())
			{
				//�������� ������� � �����
				state = notUsed;
				return;
			}
		}

		//������ ����� ������� �������� ����� � �����
		//��� ��������� ������� ����

		//���� ������� ��������� �� ������ �����
		if (targetFloor == currentFloor) 
		{
			state = doorOpening;
			//��������� ������ �������� ����� ������ �������
			orderFloors[currentFloor] = false;

			//���� ����� ������� �������� ����� � �����
			if (targetCallAvailability) 
			{
				//���������� ����������� �������� �������
				movingDirection = (targetCall.direction == bUp ? up : down);
				//�������� ���� �������, ����� � �����
				targetCallAvailability = false;
				//��������� ��������������� ������ ������ �� �����
				parentElev->CallButtonOff(targetCall);
			}
			timer = OPENING_DOOR_TIME - TIME_INTERVAL;
		}
		else 
		{
			//���� ���� ���������� �� ����� ����
			if (targetFloor - currentFloor == 1)
			{
				//������ �� ���� ����
				state = slowMovingUp;
				timer = SLOW_MOVING_TIME - TIME_INTERVAL;
			}
			//���� ���� ���������� �� ����� ����
			else if (targetFloor - currentFloor == -1)
			{
				//����� �� ���� ����
				state = slowMovingDown;
				timer = SLOW_MOVING_TIME - TIME_INTERVAL;
			}
			else
			{
				//���� ���� ���������� ��������� ������, �������
				//��������� �����, ����� ���������� ����
				state = (targetFloor > currentFloor) ? startsUp : startsDown;
				timer = ACCELERATION_TIME - TIME_INTERVAL;
			}
		}
		return;
	}
}

//��������� ����������� ����� ������, �������
//��������� ������� ��� ����������
const int ElevatorCar::GetBrakingFloor()const
{
	//���� ������� ����� ��� �����
	if (state == doorOpening || state == stopped || state == doorClosing || state == notUsed) {
		return 0;
	}
	//���� ������� ���������������
	else if (state == stopsUp || state == stopsDown || state == slowMovingUp || state == slowMovingDown) {
		return 1;
	}
	else {	//�������� ������ ��� ����������
		return 2;
	}
}

//��������� ������� � ������� ���������, � timer
//�������� ����� ����������� ��� �������� �� ����� 
void ElevatorCar::MoveElevator(CallButton cb, double time)
{
	parentElev->CheckButton(cb); //�������� ������ �� ����������

	//����������� ����� ������ ��� ����������
	int brakingFloor = GetBrakingFloor();

	//���� ������� �������� �����
	if (movingDirection == up)
	{
		//���� ����� ����� ���� �������
		if (cb.floorNumber >= currentFloor + brakingFloor && cb.direction == bUp)
		{
			for (int j = currentFloor + brakingFloor; j <= cb.floorNumber; j++)
			{
				//��������� ��� ������ ������ ������� ��������������� ���������� ������
				orderFloors[j] = false; 
			}
		}
		//���� ����� ����
		else if (cb.direction == bDown)
		{
			//�������� �� ������ �������� �����
			for (int j = currentFloor + brakingFloor; j < floorCount; j++)
			{
				orderFloors[j] = false; //��������� ������ � �������
			}
			//��������� �� ������
			for (int j = currentFloor + brakingFloor; j >= cb.floorNumber; j--)
			{
				orderFloors[j] = false; //��������� ������ � �������
			}
		}
		else //����� �����, �� ������� ��� �������� ��� ��� �� ����� �����������
		{
			//������� ������� ��� �����
			for (int j = 0; j < floorCount; j++)
			{
				orderFloors[j] = false; //��������� ������ � �������
			}
		}
	}
	//���� ������� �������� ����
	else if (movingDirection == down)
	{
		//���� ����� ���� ���� �������
		if (cb.floorNumber <= currentFloor - brakingFloor && cb.direction == bDown)
		{
			for (int j = currentFloor - brakingFloor; j >= cb.floorNumber; j--)
			{
				//��������� ��� ������ ������ ������� ��������������� ���������� ������
				orderFloors[j] = false;
			}
		}
		//���� ����� �����
		else if (cb.direction == bUp)
		{
			//��������� �� ������ ������� �����
			for (int j = currentFloor - brakingFloor; j >= 0; j--)
			{
				orderFloors[j] = false; //��������� ������ � �������
			}
			//�������� �� ������
			for (int j = currentFloor - brakingFloor; j <= cb.floorNumber; j++)
			{
				orderFloors[j] = false; //��������� ������ � �������
			}
		}
		else //����� ����, �� ������� ��� �������� ��� ��� �� ����� �����������
		{
			//������� ������� ��� �����
			for (int j = 0; j < floorCount; j++)
			{
				orderFloors[j] = false; //��������� ������ � �������
			}
		}
	}
	//���� ������� �� ������������
	else if (movingDirection == no)
	{
		//���� ����� ���� �������� �����
		if (cb.floorNumber >= currentFloor)
		{
			//�������� �� ������
			for (int j = currentFloor; j <= cb.floorNumber; j++)
			{
				orderFloors[j] = false; //��������� ������ � �������
			}
		}
		else {	//����� ���� �������� ����� ��� ��������� � ������
			//��������� �� ������
			for (int j = currentFloor; j >= cb.floorNumber; j--)
			{
				orderFloors[j] = false; //��������� ������ � �������
			}
		}
	}

	//���������� ������� ���� ��� ������� � ����������� �� ����
	currentFloor = targetFloor = cb.floorNumber;
	//������� ������� ����������� ��������
	movingDirection = (cb.direction == bUp ? up : down);
	state = doorOpening;
	//�������� ����������� �� ����������� ����� � ������ 
	timer = time;
}

//����� ��������� ����� �������� � ������� ������?
const bool ElevatorCar::CallIsBetweenCarAndTargetFloor
(const CallButton cb)const
{
	parentElev->CheckButton(cb); //�������� ������ �� ����������

	//����������� ����� ������ ��� ����������
	int brakingFloor = GetBrakingFloor();

	//���� ������� �������� �����
	if (movingDirection == up)
	{
		//���� ���� ������� �� ����� ��� ����� �����
		if (!(targetCallAvailability && targetCall.direction == down))
		{
			//���� ����� ����� �������� � �� ������� ������
			if (cb.direction == bUp && (cb.floorNumber >= currentFloor + brakingFloor)
				&& cb.floorNumber <= targetFloor) {
				return true;
			}
		}
		else	//���� ������� - ����� ����
		{
			//���� ����� ����� �������� � �� ������� ������
			if (cb.direction == bUp && (cb.floorNumber >= currentFloor + brakingFloor)
				|| cb.direction == bDown && cb.floorNumber >= targetFloor) {
				return true;
			}
		}
	}
	//���� ������� �������� ����
	else if (movingDirection == down)
	{
		//���� ���� ������� �� ����� ��� ����� ����
		if (!(targetCallAvailability && targetCall.direction == up))
		{
			//���� ����� ����� �������� � �� ������� ������
			if (cb.direction == bDown && (cb.floorNumber <= currentFloor - brakingFloor)
				&& cb.floorNumber >= targetFloor) {
				return true;
			}
		}
		else	//���� ������� - ����� �����
		{
			//���� ����� ����� �������� � �� ������� ������
			if (cb.direction == bDown && (cb.floorNumber <= currentFloor - brakingFloor)
				|| cb.direction == bUp && cb.floorNumber <= targetFloor) {
				return true;
			}
		}
	}

	return false;
}

//��� ������� ����� ������ �������� � �������? 
const bool ElevatorCar::IsBetweenCarAndCall
(const ElevatorCar*const elev, const CallButton cb)const
{
	parentElev->CheckButton(cb); //�������� ������ �� ����������
	if (elev == 0) {	//�������� ������� ����� �� ����������
		throw ElevatorError(3, "The link points to a non-existent car");
	}

	//����������� ����� ������ ��� ����������
	int brakingFloor = GetBrakingFloor();

	//���� ������������ ������� �������� �����
	if (elev->movingDirection == up)
	{
		//���� ������������ ������� ����� ������������ �� ����� ������ ��� ����
		if (elev->currentFloor+elev->GetBrakingFloor() <= cb.floorNumber)
		{
			if (cb.direction == bUp)	//���� ����� �����
			{
				//���� ������� ������� ���������� ����� ������������ ��������
				//� ������� � �������� � ������ �����������
				if (currentFloor >= elev->currentFloor && (movingDirection != down)
					&& currentFloor + brakingFloor <= cb.floorNumber) {
					return true;
				}
			}
			else if (cb.direction == bDown)	//���� ����� ����
			{
				//���� ������� ������� ���������� ����� ������������ ��������
				//� ������� � �������� � ������ �����������
				if (currentFloor >= elev->currentFloor && (movingDirection != down)
					|| currentFloor - brakingFloor >= cb.floorNumber) {
					return true;
				}
			}
		}

		else //���� ������������ ������� �� ����� ������������ �� ����� ������ ��� ����
		{
			if (cb.direction == bDown)	//���� ����� ����
			{
				//���� ������� ������� ���������� ����� ������������ ��������
				//� ������� � �������� � ������ �����������
				if (currentFloor >= elev->currentFloor && movingDirection != down
					|| currentFloor - brakingFloor >= cb.floorNumber) {
				}
				return true;
			}
			else if (cb.direction == bUp)	//���� ����� �����
			{
				//���� ������� ������� ���������� ����� ������������ ��������
				//� ������� � �������� � ������ �����������
				if (!(movingDirection == up && currentFloor + brakingFloor > cb.floorNumber
					&& currentFloor < elev->currentFloor)) {
					return true;
				}
			}
		}
	}

	//���� ������������ ������� �������� ����
	else if (elev->movingDirection == down)
	{
		//���� ������������ ������� ����� ������������ �� ����� ������ ��� ����
		if (elev->currentFloor-elev->GetBrakingFloor() > cb.floorNumber)
		{
			if (cb.direction == bDown)	//���� ����� ����
			{
				//���� ������� ������� ���������� ����� ������������ ��������
				//� ������� � �������� � ������ �����������
				if (currentFloor <= elev->currentFloor && (movingDirection != up)
					&& currentFloor - brakingFloor >= cb.floorNumber) {
					return true;
				}
			}
			else if (cb.direction == bUp)	//���� ����� �����
			{
				//���� ������� ������� ���������� ����� ������������ ��������
				//� ������� � �������� � ������ �����������
				if (currentFloor <= elev->currentFloor && (movingDirection != up)
					|| currentFloor + brakingFloor <= cb.floorNumber) {
					return true;
				}
			}
		}

		//���� ������������ ������� �� ����� ������������ �� ����� ������ ��� ����
		else 
		{
			if (cb.direction == bUp)	//���� ����� �����
			{
				//���� ������� ������� ���������� ����� ������������ ��������
				//� ������� � �������� � ������ �����������
				if (currentFloor <= elev->currentFloor && movingDirection == down
					|| currentFloor + brakingFloor <= cb.floorNumber) {
				}
				return true;
			}
			else if (cb.direction == bDown)	//���� ����� ����
			{
				//���� ������� ������� ���������� ����� ������������ ��������
				//� ������� � �������� � ������ �����������
				if (!(movingDirection == up && currentFloor - brakingFloor < cb.floorNumber
					&& currentFloor > elev->currentFloor)) {
					return true;
				}
			}
		}
	}

	//���� ������������ ������� ������ �� ��������
	else if (elev->movingDirection == no)
	{
		//���� ������������ ������� ���� ������
		if (elev->currentFloor > cb.floorNumber)
		{
			if (cb.direction == bDown)	//���� ����� ����
			{
				//���� ������� ������� ���������� ����� ������������ ��������
				//� ������� � �������� � ������ �����������
				if (currentFloor <= elev->currentFloor && (movingDirection != up)
					&& currentFloor - brakingFloor >= cb.floorNumber) {
					return true;
				}
			}
			else if (cb.direction == bUp)	//���� ����� �����
			{
				//���� ������� ������� ���������� ����� ������������ ��������
				//� ������� � �������� � ������ �����������
				if (currentFloor <= elev->currentFloor && (movingDirection != up)
					|| currentFloor + brakingFloor <= cb.floorNumber) {
					return true;
				}
			}
		}

		//���� ������������ ������� ���� ������
		else if (elev->currentFloor < cb.floorNumber)
		{
			if (cb.direction == bUp)	//���� ����� �����
			{
				//���� ������� ������� ���������� ����� ������������ ��������
				//� ������� � �������� � ������ �����������
				if (currentFloor >= elev->currentFloor && (movingDirection != down)
					&& currentFloor + brakingFloor <= cb.floorNumber) {
					return true;
				}
			}
			else if (cb.direction == bDown)	//���� ����� ����
			{
				//���� ������� ������� ���������� ����� ������������ ��������
				//� ������� � �������� � ������ �����������
				if (currentFloor >= elev->currentFloor && (movingDirection != down)
					|| currentFloor - brakingFloor >= cb.floorNumber) {
					return true;
				}
			}
		}
	}

	return false;
}

//������� ���� ��������� ������, ��� �����?
const bool ElevatorCar::TargetIsEarlierThanCall
(const CallButton cb)const
{
	parentElev->CheckButton(cb); //�������� ������ �� ����������

	if (movingDirection == up)	//���� ������� �������� �����
	{
		//���� ����� ���� �������� ����� ������� � � �������
		//��� ����-������ � ����� ����
		if (cb.floorNumber >= targetFloor &&
			(!targetCallAvailability || targetCall.direction == bUp)) {
			return true;
		}
		//���� ����� ���� �������� ����� ������� � ����� ����
		if (cb.floorNumber <= targetFloor && cb.direction == bDown) {
			return true;
		}
	}
	else if (movingDirection == down)	//���� ������� �������� ����
	{
		//���� ����� ���� �������� ����� ������� � � �������
		//��� ����-������ � ����� �����
		if (cb.floorNumber <= targetFloor &&
			(!targetCallAvailability || targetCall.direction == bDown)) {
			return true;
		}
		//���� ����� ���� �������� ����� ������� � ����� �����
		if (cb.floorNumber >= targetFloor && cb.direction == bUp) {
			return true;
		}
	}
	return false;
}

//���������� ����� ��������� ���� ������� ���� ����� pos,
//���� ����� ���� ���� �����, �� ������� true
const bool ElevatorCar::CalculateHigher
(const CallButton cb, double& ariveTime, int& pos)const
{
	int upperLimit;	//����� ������� ����, �� ������� ������� �������� �������

	//���� ����� ����� ���� �������� �����
	if (cb.floorNumber > targetFloor && cb.direction == bUp)
		upperLimit = cb.floorNumber;
	else
		upperLimit = floorCount;

	//��������� �����
	for (int j = targetFloor + 1; j < upperLimit; j++)
	{
		if (orderFloors[j]) //���� ���� ����� �� ����
		{
			//���� � ��������� ��������� �� ����� ����� ����� ���� ����
			if (j - pos == 1) {
				//�������� ����� ������� ������� �� �������� ����
				ariveTime += SLOW_MOVING_TIME;
			}
			else {	 //2 � ������ ������ �� �����
				//�������� ����� ��������� � ���������� � ����� �������� ��������
				ariveTime += ACCELERATION_TIME * 2 + (j - pos - 2)*MOVING_TIME;
			}
			//�������� ����� �������� ������, ������� � �������� ������
			ariveTime += OPENING_DOOR_TIME + STANDING_TIME + CLOSING_DOOR_TIME;
			pos = j;
		}
	}

	//������� ����� �������� �� ����� ����� ���� �� ���� �������� �����
	if (cb.floorNumber > targetFloor && cb.direction == bUp)
	{
		if (cb.floorNumber - pos == 1)
			ariveTime += SLOW_MOVING_TIME;
		else
			ariveTime += ACCELERATION_TIME * 2 + (cb.floorNumber - pos - 2)*MOVING_TIME;

		ariveTime += OPENING_DOOR_TIME;
		return true;	//������� ������� �� ������
	}

	//���� ����� ���� ���� �������� ����� 
	if (cb.floorNumber >= targetFloor && cb.direction == bDown)
	{
		//���� ���� ������ ��������� � ������� ������, �� ����� �����������
		//�������� ����������� �������� ������� � ������� ������ ���
		if (cb.floorNumber == targetFloor && pos == targetFloor)
			//�������� ������ ��������� dOpening, stopped, dClosing;
		{
			if (state == stopped) {	//���� ������� ����� � ��������� �������
				//������ ��� ����� �� ������� ����� 
				ariveTime = 0;	//� ��� ��� ����� ���������� �����
			}
			if (state == doorClosing) {	//���� ����� �����������
				//������ ������� ����� �� ������� �����
				//� ����� ������ ������� �����
				ariveTime = OPENING_DOOR_TIME;
			}
		}
		else	//������ ���� ������ �� ��������� � ������� ���
				//���� ������ ������
		{
			//���� ��� ���������� ����� �������� �� ���� ������
			if (cb.floorNumber == pos) {	//�������� ��� ������ �� ������� ����
				//�� �� ����� ���� ������ � ��������� �����
				ariveTime -= STANDING_TIME - CLOSING_DOOR_TIME;
			}
			//���� �� ���������� ������ �� ����� ������ ����� ���� ����
			else if (abs(cb.floorNumber - pos) == 1) {
				ariveTime += SLOW_MOVING_TIME + OPENING_DOOR_TIME;
			}
			else {	//�� ���������� ������ �� ����� ������ ������ ������ �����
				ariveTime += ACCELERATION_TIME * 2 + (abs(cb.floorNumber - pos) - 2)
					*MOVING_TIME + OPENING_DOOR_TIME;
			}
		}
		return true;	//������� ������� �� ������
	}

	return false;	//������� ��� �� ������� �� ������
}

//���������� ����� ��������� ���� ������� ���� ����� pos,
//���� ����� ���� ���� �����, �� ������� true
const bool ElevatorCar::CalculateLower
(const CallButton cb, double& ariveTime, int& pos)const
{
	int lowerLimit;	//����� ������ ����, �� ������� ������� �������� �������

	//���� ����� ���� ���� �������� �����
	if (cb.floorNumber < targetFloor && cb.direction == bDown) {
		lowerLimit = cb.floorNumber + 1;
	}
	else {
		lowerLimit = 0;
	}

	//��������� ����
	for (int j = targetFloor - 1; j >= lowerLimit; j--)
	{
		if (orderFloors[j])	//���� ���� ����� �� ����
		{
			//���� � ��������� ��������� �� ����� ����� ����� ���� ����
			if (pos - j == 1) {
				//�������� ����� ������� ������� �� �������� ����
				ariveTime += SLOW_MOVING_TIME;
			}
			else { //2 � ������ ������ �� �����
				//�������� ����� ��������� � ���������� � ����� �������� ��������
				ariveTime += ACCELERATION_TIME * 2 + (pos - j - 2)*MOVING_TIME;
			}
			//�������� ����� �������� ������, ������� � �������� ������
			ariveTime += OPENING_DOOR_TIME + STANDING_TIME + CLOSING_DOOR_TIME;
			pos = j;
		}
	}

	//������� ����� �������� �� ����� ���� ���� �� ���� �������� �����
	if (cb.floorNumber < targetFloor && cb.direction == bDown)
	{
		if (pos - cb.floorNumber == 1)
			ariveTime += SLOW_MOVING_TIME;
		else
			ariveTime += ACCELERATION_TIME * 2 + (pos - cb.floorNumber - 2)*MOVING_TIME;

		ariveTime += OPENING_DOOR_TIME;
		return true;	//������� ������� �� ������
	}


	//���� ����� ����� ���� �������� ����� 
	if (cb.floorNumber <= targetFloor && cb.direction == bUp)
	{
		//���� ���� ������ ��������� � ������� ������, �� ����� �����������
		//�������� ����������� �������� ������� � ������� ����� ���
		if (cb.floorNumber == targetFloor && pos == targetFloor)
			//�������� ������ ��������� dOpening, stopped, dClosing;
		{
			if (state == stopped) {	//���� ������� ����� � ��������� �������
				//������ ��� ����� �� ������� ����� 
				ariveTime = 0;	//� ��� ��� ����� ���������� �����
			}
			if (state == doorClosing) {	//���� ����� �����������
				//������ ������� ����� �� ������� �����
				//� ����� ������ ������� �����
				ariveTime = OPENING_DOOR_TIME;
			}
		}
		else	//������ ���� ������ �� ��������� � ������� ���
				//���� ������ �����
		{
			//���� ��� ���������� ����� �������� �� ���� ������
			if (cb.floorNumber == pos) { //�������� ��� ������ �� ������ ����
				//�� �� ����� ���� ������ � ��������� �����
				ariveTime -= STANDING_TIME - CLOSING_DOOR_TIME;
			}
			//���� �� ���������� ������ �� ����� ������ ����� ���� ����
			else if (abs(cb.floorNumber - pos) == 1) {
				ariveTime += SLOW_MOVING_TIME + OPENING_DOOR_TIME;
			}
			else {	//�� ���������� ������ �� ����� ������ ������ ������ �����
				ariveTime += ACCELERATION_TIME * 2 + (abs(cb.floorNumber - pos) - 2)
					*MOVING_TIME + OPENING_DOOR_TIME;
			}
		}
		return true; //������� ������� �� ������
	}

	return false; //������� ��� �� ������� �� ������
}

//�������� ����� �������� ������� �� ����� � ������, 
//��� ��� �������� ��� ���� ������ � ������ �� ����
const double ElevatorCar::CalculateTheCallAriveTime
(const CallButton cb)const
{
	parentElev->CheckButton(cb); //�������� ������ �� ����������

	double ariveTime = (timer < 0 ? 0 : timer);	//����� �������� �� ������� ����

	if (state == notUsed) 	//���� ������� ��������
	{
		//� ����������� �� ���������� ������ �� ������
		//���������� ����� �� ������ �� ����� � �������� ������
		switch (abs(currentFloor - cb.floorNumber))
		{
		case 0: ariveTime = OPENING_DOOR_TIME; break;
		case 1: ariveTime = SLOW_MOVING_TIME + OPENING_DOOR_TIME; break;
		default:
			ariveTime = 2 * ACCELERATION_TIME + (abs(cb.floorNumber - currentFloor) - 2)
				* MOVING_TIME + OPENING_DOOR_TIME; break;
		}
		return ariveTime;	//����� �������� ����������
	}


	//���� ������� ��� �� ������� ����� �� �� �������� �����
	//���� ������ doorOpening, stopped ��� doorClosing

	//���� ������� ���� � ����������� ��������� � �������
	if (cb.floorNumber == targetFloor && (cb.direction == bUp
		&& movingDirection == up || cb.direction == bDown && movingDirection == down))
	{
		if (state == stopped) {//������ ������� ���� ��������� � �������
			ariveTime = 0;
		}
		if (state == doorClosing) {//������ ������� ���� ��������� � �������
			ariveTime = OPENING_DOOR_TIME;
		}
		return ariveTime; //����� �������� ����������
	}


	//���������� ����� ������� �� �������� ����� � �������� ������

	if (state == startsUp || state == movingUp
		|| state == startsDown || state == movingDown) {
		ariveTime += (abs(targetFloor - currentFloor) - 2)*MOVING_TIME
			+ ACCELERATION_TIME + OPENING_DOOR_TIME;
	}
	else if (state == stopsUp || state == stopsDown
		|| state == slowMovingUp || state == slowMovingDown) {
		ariveTime += OPENING_DOOR_TIME;
	}


	// ������ ��������������� ������, ����� ������� ���� 
	//� ����������� �� ��������� � �������
	if (state != stopped && state != doorClosing) {
		ariveTime += STANDING_TIME;	//�� �����, ���� ���� ������ ��������� � 
		//������� ������, �� ����������� ������� �� ��������� � ������������ 
		//������, � ���������� ������� ���
	}
	if (state != doorClosing) {
		ariveTime += CLOSING_DOOR_TIME; //�����
	}

	int pos = targetFloor;//���������� ���� ��������� ������� ��� ��������

	//���� ������� �������� �����
	if (movingDirection == up)
	{
		//���������� ����� ���������� ������� ���� �������
		//� ���� ����� �� ������ � ���������� ���
		if (CalculateHigher(cb, ariveTime, pos)) {
			return ariveTime; //����� �������� ����������
		}
		//���������� ����� ���������� ������� � ������ ���� �������
		CalculateLower(cb, ariveTime, pos);

		return ariveTime; //����� �������� ����������
	}
	//���� ������� �������� ����
	else if (movingDirection == down)
	{
		//���������� ����� ���������� ������� ���� �������
		//� ���� ����� �� ������ � ���������� ���
		if (CalculateLower(cb, ariveTime, pos)) {
			return ariveTime;	//����� �������� ����������
		}
		//���������� ����� ���������� ������� � ������ ���� �������
		CalculateHigher(cb, ariveTime, pos);

		return ariveTime; //����� �������� ����������
	}
}

//��������� ����� �������� ������� �� �����,
//�������� ��� ������ ������ � ������
const double ElevatorCar::MinCalculateTheCallAriveTime
(const CallButton cb)const
{
	parentElev->CheckButton(cb); //�������� ������ �� ����������

	//���� ����� ��������� ����� �������� � �� ������� ������
	//��� ������� ��������
	if (CallIsBetweenCarAndTargetFloor(cb) || IsFree())
	{
		double arriveTime = (timer < 0 ? 0 : timer);	//����� �������� �� �����

		//���� ����� � �������� ����� �������
		if (currentFloor - cb.floorNumber == 0)
		{
			if (state == notUsed || state == doorClosing) {
				arriveTime = OPENING_DOOR_TIME;
			}
			else if (state == stopped) {
				arriveTime = 0;
			}
		}
		//���� �� ����� ������� �� ����� ������ ���� ����
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
		else	//���� �� ����� ������� �� ����� ������ ����� ������ �����
		{
			if (state == doorOpening) {
				arriveTime += STANDING_TIME;
			}

			if (state == stopped || state == doorOpening) {
				arriveTime += CLOSING_DOOR_TIME;
			}

			if (state == notUsed || state == doorClosing
				|| state == stopped || state == doorOpening){
				//�������� ����� ����������� ����� ������
				arriveTime += ACCELERATION_TIME;
			}

			//�������� �����, ����������� ����� ������� �� ����� ������
			arriveTime += (abs(cb.floorNumber - currentFloor) - 2)*MOVING_TIME
				+ ACCELERATION_TIME + OPENING_DOOR_TIME;
		}
		return arriveTime;
	}
	return -1;	//���������� ����� ����������
}

//�������� ����� increment �� ����� �� ������� �����,
//�� ������� ����� ������������ �������.
//���� increment ������ ��� ����� ������ �� ������
//�� ����� ��������� ������ �����
const CallButton ElevatorCar::NextFloor(int increment)const
{
	//���� ��������� ����� ����� increment ������ ����� ���������
	if (increment < 0 || increment > 2 * floorCount -3) {
		return CallButton();	//������� ������ �����
	}

	//���� ������� ��������
	if (state == stopsUp || state == stopsDown 
		|| state == slowMovingUp || state == slowMovingDown) {
		//������������ ������� ����� ������ �� ��������� �����
		increment++;
	}
	//���� ������� �������� ��� ��� ������� �������� 
	else if (state == startsUp || state == startsDown 
		|| state == movingUp || state == movingDown){
		//������������ ������� ������ ������ ����� ���� 
		increment += 2;
	}

	if (movingDirection == up)	//���� ������� �������� �����
	{
		//���� ��������� ����� ����� �� �� ����������� � ��������� ����
		if (floorCount - currentFloor - 2 >= increment) {
			return CallButton(currentFloor + increment, bUp);
		}
		//���� ��������� ����� ����� ����������������� �����������
		else if (2 * floorCount - currentFloor - 3 >= increment) {
			return CallButton(floorCount - (currentFloor + increment
				- (floorCount - 2)), bDown);
		}
		//���� ��������� ����� ����� �� �� �����������, �� ��������� ����
		else	
		{
			return CallButton(currentFloor - (floorCount * 2 - 2 - increment), bUp);
		}
	}
	else //���� ������� ����� ��� �������� ����
	{
		//���� ��������� ����� ����� �� �� ����������� � ��������� ����
		if (currentFloor > increment) {
			return CallButton(currentFloor - increment, bDown);
		}
		//���� ��������� ����� ����� ����������������� �����������
		else if (increment - currentFloor < floorCount -1) {
			return CallButton(increment - currentFloor, bUp);
		}
		//���� ��������� ����� ����� �� �� �����������, �� ��������� ����
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
	//�������� ������� ��� ������� �����
	cars = new ElevatorCar*[carCount];

	//������� ������ ������ �� �������� �����
	carsG = new ElevatorCarG*[carCount];

	//�������� ������� ����� ������� ����� ������ ����
	for (int j = 0; j < carCount; j++) 
	{
		cars[j] = carsG[j] = new ElevatorCarG(j + 1, this);
	}

	runningTime = 0;
	callGenerationLevel = 0;
	speedID = 2;

	callLogName = CALL_LOG_FILE_NAME;

	//���������� �������
	GraphicsON();

	//������� ���� ��� ������ ������� � �������
	callLogFile.open(callLogName);
	callLogFile.fill('0');

	stateFileLog = !callLogFile.is_open();	//������ �������� ����������� �����
	UpdateFileLogState(!stateFileLog);	//�������� ����������� ����� �������
	if (callLogFile.is_open()) {	//���� ������ ������� �������
		callLogFile.close();	//������� ����
	}
}

ElevatorG::~ElevatorG()
{
	callLogFile.open(callLogName, ios::app);
	if (callLogFile.is_open()) {
		callLogFile << ends;	//�������� 0 � ����� ����� �������
	}

	delete carsG;
}

//�������� �������
void ElevatorG::GraphicsON()
{
	int consolX = 53; //������ �������
	int consolY = floorCount + 13;	//������ �������
	if (carCount > 4) {	//���� ������ 4 ������� �����
		consolX += 9 * (carCount - 4);	//��������� �������
	}

	// ������������� ������� � ������� ������
	init_graphics(consolX, consolY);

	//����� ��� �������� ���������
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

	//�����
	set_cursor_pos(30, 2);
	set_color(cDARK_RED);
	cout << "time: ";

	//�������� ���������
	set_cursor_pos(9, 2);
	set_color(cDARK_GREEN);
	cout << "\x05 Elevator program \x05";

	set_cursor_pos(2, 4);
	set_color(cDARK_GRAY);
	cout << "FileLog connection:       Generation:0   Speed:1";

	set_color(cDARK_GREEN);
	set_cursor_pos(1, 5);
	//���������� ������� ����������� ������
	for (int i = 0; i <= carCount; i++)
	{
		cout << "\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC2";
	}
	cout << "\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4";

	for (int j = 0; j < floorCount; j++) {
		set_cursor_pos(2, floorCount - j + 5);	//������� ����� �����
		cout << setw(3) << j + 1;				//�� ����� �����
		set_cursor_pos(carCount * 9 + 14, floorCount - j + 5);
		cout << j + 1;							// � ������

		//���������� ����������� ����� ���������
		for (int i = 0; i <= carCount; i++)
		{
			set_cursor_pos(i * 9 + 9, floorCount - j + 5);
			cout << "\xB3";
		}
	}
	set_cursor_pos(3, floorCount + 6);
	cout << "\x1A\xFD\x1B";	//������ ����
	cout << "\nPress any key to stop time or +/- or \x11/\x10...";
}

// ���������� �� ������ ��������� ����� � ������� ������ �������
void ElevatorG::SystemDisplay()const
{
	set_cursor_pos(36, 2);	//����� ��� �������

	//���������� �����
	if (runningTime / 1000 >= 60) {
		cout << runningTime / 60000 << "m ";
	}
	cout << setw(2) << runningTime % 60000 / 1000 << ".";
	cout << (runningTime % 1000) / 100 << "s";

	for (int j = 0; j < carCount; j++) {
		//���������� �������
		carsG[j]->CarDisplay();
	}
}

// ��������� ������� � ��������� ���������
// ���������� false � ������ ������������� ��������� ���������
bool ElevatorG::SystemTick()
{
	if (_kbhit())	//���� ���� ������ ������
	{
		//���� �������� ������� �� ����������
		if (!GetDirections()) {
			return false;
		}
	}

	Elevator::SystemTick(SPEED_VALUES[speedID]);

	runningTime += TIME_INTERVAL;
	return true;
}

//�������� ������ ��� ����� ����� ������ � �������������,
//���� �������� �������� �� ���������� ��������� ������ false
const bool ElevatorG::GetDirections()
{
	//������� ������ ����� ������ � ��������
	for (int j = 0; j < 5; j++)
	{
		set_cursor_pos(1, floorCount + 8 + j);
		clear_line();
	}
	set_cursor_pos(1, floorCount + 8);

	char pressedKey = _getch();	//��������� ������� �������

	//��������� ������������� ��������� ���������� �����
	if (pressedKey == 43 || pressedKey == 45)
	{
		pressedKey == 43 ? callGenerationLevel++ : callGenerationLevel--;
		callGenerationLevel = (callGenerationLevel + 10) % 10;
		DisplayGenerationLevel();
		return true;
	}

	//��������� �������� ������������� �������
	if (pressedKey == -32)
	{
		pressedKey = _getch();
		if (pressedKey == 77) {
			speedID++;	//������� ��������� ������� �������� ��������� �����
		}
		else if (pressedKey == 75) {
			speedID--;	//������� ���������� ������� �������� ��������� �����
		}
		if (pressedKey == 75 || pressedKey == 77)
		{
			speedID = (speedID + NUMBER_OF_SPEEDS) % NUMBER_OF_SPEEDS;
			DisplaySpeed(); //���������� �������� ���������
			return true;
		}
	}

	//1 ������
	cout << "Are you inside the elevator?(yes/no/Enter/Esc): ";
	char answer;
	//���� ���� �� �������� ���������� ������
	do {
		answer = _getch();
	} while (answer != 'y' && answer != 'n'
		&& answer != '\x0D' && answer != '\x1B');

	if (answer == '\x0D') //����� Enter
	{
		cout << "\bcancel";	//��������� ������ � �������������
		return true;		// � ���������� ���������� ���������
	}
	if (answer == '\x1B') {	//����� ESC
		return false;		//��������� ������ � ���������
	}
	cout << (answer == 'y' ? "yes" : "no");

	if (answer == 'y') //���� ����� ������� �����
	{
		char cElevNum;	//����� �������

		//��������� ������ �������
		cout << "\nEnter the elevator number:  ";
		do {
			cElevNum = _getch();
			//���� �������� �������������� �����
		} while (cElevNum <'1' || cElevNum >('0' + carCount));
		cout << cElevNum;

		//��������� ����� ��� ������� ���������� ���
		//������� �� �������� ��� �������� ������
		cout << "\nEnter destination floor number or"
			<< "\n'c'/'o'-for closing/openning elevator door: ";
		char ch1 = '0', ch2;
		//��������� ������ ����� ��� �����
		do {
			ch2 = _getch();
			//���� �������� �������������� ����� � �� 'O', �� 'C'
		} while (ch2 != 'c' && ch2 != 'o' && (ch2 <= '0' ||
			ch2 > ('0' + floorCount) || ch2 > '9'));
		cout << ch2;
		//���� ��� ����� ������ ����� ������ ��������������
		//����, �� �� ���������� ������ �����

		//����� �������� � ����� ������
		char nDec = '0' + floorCount / 10;
		if (ch2 <= nDec)
		{
			//��������� ������ ����� ��� �������������
			ch1 = ch2;
			do {
				ch2 = _getch();
				if (ch2 == 13) //���� ����� ������
				{
					ch2 = ch1;	//���������
					ch1 = '0';	//����
					break;		//�����
				}
				//���� �������� �������������� �����
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
			//��������������� ������� ��������� �����
			GetCar(cElevNum - '0')->SetOrderFloor(floor);

			set_cursor_pos(1, floorCount + 12);
			cout << "Order is accepted!";
		}
	}
	else // ����� � �����
	{
		cout << "\nWhat floor are you on? ";
		char ch1 = '0', ch2;
		//��������� ������ �����
		do {
			ch2 = _getch();
			//���� �������� �������������� �����
		} while (ch2 <= '0' || ch2 > ('0' + floorCount) || ch2 > '9');
		cout << ch2;

		//����� �������� � ����� ������
		char nDec = '0' + floorCount / 10;
		if (ch2 <= nDec)
		{
			//��������� ������ ����� ��� �������������
			ch1 = ch2;
			do {
				ch2 = _getch();
				if (ch2 == 13) //���� ����� ������
				{
					ch2 = ch1;	//���������
					ch1 = '0';	//����
					break;		//�����
				}
				//���� �������� �������������� �����
			} while (!(ch1 == nDec && ch2 >= '0' && ch2 <= ('0' + floorCount % 10))
				&& !(ch1 != nDec && isdigit(ch2)));
			if (ch1 != '0') cout << ch2;
		}

		CallButton cb; //������ ������ � �����
		cb.floorNumber = (ch1 - '0') * 10 + (ch2 - '0') - 1;
		if (cb.floorNumber == 0) {	//���� ������� ����
			cb.direction = bUp;		//�� ������ ������ ����
		}
		else if (cb.floorNumber == floorCount - 1) {	//���� ������ ����
			cb.direction = bDown;	//�� ������ ������ �����
		}
		else //�� ������� � �� ������ ����
		{
			char dir;
			cout << "\nWhich directon you need to move?(up/down) ";
			//��������� ������������ ������ ��� �����������
			do {
				dir = _getch();
			} while (dir != 'u' && dir != 'd');
			cout << (dir == 'u' ? "up" : "down");

			cb.direction = (dir == 'u' ? bUp : bDown);
		}
		SetTheCall(cb); //���������� ����� � �����

		//���� ����������� ����������, �� ���� ��� ���� ������ � ��������
		set_cursor_pos(1, floorCount +
			(cb.floorNumber == 0 || cb.floorNumber == floorCount - 1 ? 10 : 11));
		cout << "Call is accepted!";
	}
	return true;
}

//���������� ����� � �����
//���� ����� ����� ���� �������� � ���� �� ������, ������� false
bool ElevatorG::SetTheCall(const CallButton cb)
{
	if (Elevator::SetTheCall(cb))
	{
		//���������� ����� �� ������
		set_cursor_pos(cb.direction == bUp ? 7 : 8,
			floorCount - cb.floorNumber + 5);
		cout << (cb.direction == bUp ? "\x1E" : "\x1F");

		set_cursor_pos(carCount * 9 + (cb.direction == bUp ? 12 : 11),
			floorCount - cb.floorNumber + 5);
		cout << (cb.direction == bUp ? "\x1E" : "\x1F");

		callLogFile.open(callLogName, ios::app);
		UpdateFileLogState(callLogFile.is_open());	//�������� ����������� �����

		if (callLogFile.is_open())
		{
			//�������� ����� � �����
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

//��������� ������ � �����
void ElevatorG::CallButtonOff(const CallButton cb)
{
	Elevator::CallButtonOff(cb);

	//������� ����������� ������ ������
	set_cursor_pos(cb.direction == bUp ? 7 : 8,
		floorCount - cb.floorNumber + 5);
	cout << " ";

	set_cursor_pos(carCount * 9 + (cb.direction == bUp ? 12 : 11),
		floorCount - cb.floorNumber + 5);
	cout << " ";
}

//�������� � ������ ����� �� �������
void ElevatorG::WriteCarOrder(int idCar, int orderFloor)
{
	callLogFile.open(callLogName, ios::app);
	UpdateFileLogState(callLogFile.is_open()); //�������� ����������� �����

	if (callLogFile.is_open())
	{
		//�������� ����� � �����
		callLogFile << setw(2) << runningTime / 3600000
			<< ":" << setw(2) << runningTime / 60000
			<< ":" << setw(2) << runningTime % 60000 / 1000
			<< "." << (runningTime % 1000) / 100
			<< "     " << orderFloor + 1
			<< "\x1B" << char('A' + idCar - 1) << endl;

		callLogFile.close();
	}
}

//������� ������� ��������� ���������� �� �����
void ElevatorG::DisplayGenerationLevel()const
{
	set_cursor_pos(39, 4);
	set_color(cYELLOW);
	cout << callGenerationLevel;
}

//�������� ������� ��������� ����������
int ElevatorG::GetGenerationLevel()const
{
	return callGenerationLevel;
}

//������� �������� ������������� �����
void ElevatorG::DisplaySpeed()const
{
	set_cursor_pos(49, 4);
	set_color(cBLUE);

	cout << left << setw(4) << SPEED_VALUES[speedID] << right;
}

//�������� ����������� ����� ������� �������
void ElevatorG::UpdateFileLogState(bool fstate)
{
	if (fstate != stateFileLog)	//���� ����� � ������ ��������� ��������
	{
		set_cursor_pos(21, 4);
		if (fstate == true)	//���� ���� ������� �������
		{
			stateFileLog = true; 

			set_color(cGREEN);
			cout << "true ";

			callLogFile.close();	//�������� ���� �������
			callLogFile.open(callLogName);

			//��������� ���������� ��� ������ ������� ������� �� �����
			int consolY = floorCount + 13;	//������ �������
			string str = "start "; //������� ��� ������� ����������
			char buf[BUF_SIZE];	//������ ��� �����
			string str1 = itoa(consolY, buf, 10);	//��������-������ �������
			//������� �� ������
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

//���������� ����� �� ����
//���� ������� ��� ������� �� ���� �����, ������� false
bool ElevatorCarG::SetOrderFloor(const int nFloor)
{
	if (ElevatorCar::SetOrderFloor(nFloor))
	{
		//���������� ���������� ����
		set_cursor_pos(2 + carNumber * 9, floorCount - nFloor + 5);
		cout << "\x1B";
		
		//�������� ����� �� ����� � ����
		parentElevG->WriteCarOrder(carNumber,nFloor);
		return true;
	}
	return false;
}

//��������� ������ � ������� ��� �� �����
void ElevatorCarG::TurnOffButton()
{
	ElevatorCar::TurnOffButton();

	//������� ����������� ������ �� ������� ���� 
	set_cursor_pos(2 + carNumber * 9, floorCount - currentFloor + 5);
	cout << " ";
}

//�������� ������� � ��������� ���������
void ElevatorCarG::Tick()
{
	lastfloor = currentFloor;
	ElevatorCar::Tick();
}

//���������� ������� �� ������
void ElevatorCarG::CarDisplay()const
{
	//������� ���� ��� �������
	set_color(color(15 - carNumber));

	//������� ������� ����������� ��������� �������
	set_cursor_pos(3 + carNumber * 9, floorCount - lastfloor + 5);
	cout << "     ";

	//���������� ����� ��������� �������
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

	//���������� ����������� �������� �������
	switch (movingDirection)
	{
	case up: cout << "\x1E"; break;
	case down: cout << "\x1F"; break;
	default: cout << " ";  break;
	}

	set_cursor_pos(4 + carNumber * 9, floorCount + 6);
	if (IsFree()) { //���� ���� ��������
		cout << "   ";	//������� ����
	}
	else
	{
		//���������� ������� ����� ��� ����� �������
		cout << setw(2) << targetFloor + 1;
		//���� ������� ���� �� �����
		if (targetCallAvailability) {
			cout << (targetCall.direction == bUp ? "\x1E" : "\x1F");
		}
		else {	//������� ���� �� ����� 
			cout << " ";
		}
	}

	//������� ����� ���� ������
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
		cb.floorNumber = rand() % NUM_FLOORS; //��������� ������ �����

		if (cb.floorNumber == NUM_FLOORS - 1) {	//���� ���� �������
			cb.direction = bDown;	//������ ����� ���� ������ ����
		}
		else if (cb.floorNumber == 0) {	//���� ���� ������
			cb.direction = bUp;	//������ ����� ���� ������ �����
		}
		else {
			cb.direction = (rand() % NUM_FLOORS > cb.floorNumber ?
				bUp : bDown);	//��������� ����������� (���� ���� ���� 
				//��������, �� ����������� ������ ���� ����, ��� �����)
		}
		elev->SetTheCall(cb);	//������� ����
	}

	//���������� �����
	void Man::SetOrder()
	{
		//������� ����� ����, ������� ��� �� ���� �������� �����,
		//���� ��� ����� ���� � ��������
		if (cb.direction == bUp) {
			order = rand() % (NUM_FLOORS - cb.floorNumber - 1)
				+ cb.floorNumber + 1;
		}
		else {
			order = rand() % (cb.floorNumber);
		}
		//�������� ���� ��� ��������������� �������
		elev->GetCar(carNumber)->SetOrderFloor(order);
	}

	//��������� ��������� � ��������� ���������(���� ��
	//��� ������ �����, �� ������� false - ����� �������
	bool Man::Tick()
	{
		if (state == wait)	//���� �������� ������� ������� �����
		{
			int car = elev->FindReadyCar(cb);	//���������, ���� �� �������?
			if (car!=-1) //���� ���� �����-������
			{
				state = entry;	//������ ������� � ���
				carNumber = car;	//��������� �� �����

				//������������� ����� ��������� � ������� �� ������ �����
				carEntryTime = (rand() % (STANDING_TIME / TIME_INTERVAL))
					* TIME_INTERVAL + 1000;
				//�� 0 �� ����� ������� ����� + 1 �������
			}
		}
		else if (state == entry) //���� �������� ������ � ����
		{
			if (carEntryTime > 0) {	//���� ����� ��������� �� �������
				carEntryTime -= TIME_INTERVAL; //��������� ���
			}
			else {	//���� ����� �������
				SetOrder();	//������ ������ ������ �����
				return false;	//�������� ��������� ��������
			}
		}
		return true;	//�������� ��� ���-�� ������� � �������
	}

	//���������� ������ ����������
	void Man::Initialization(ElevatorG* el)
	{
		elev = el;	//��������� ����
		srand(time(NULL));	//�������� ���
	}

	//��������� ���� ���������� � ��������� ���������
	void Man::PeopleTick()
	{
		//��������� ������� ��������� � ��������� ���������,
		//���� �� ��� ������� ����, �� ��� ����� ������� �� ������
		list<Man*>::iterator iter = people.begin();
		while (iter != people.end())
		{
			if ((*iter)->Tick()) {
				iter++;
			}
			else {	//������� ���������
				list<Man*>::iterator temp = iter;
				temp++;
				delete (*iter);
				people.erase(iter);
				iter = temp;
			}
		}

		//�������� ������� ������������� ��������� ����������
		int lvl = elev->GetGenerationLevel();
		if (lvl)	//���� �� �� �������
		{
			//�������� �������� �������� (��� �� ������ - ��� ������ �����������
			//��� ������������� 0 == ��������� ����� ��������
			if (!(rand() % (((9 - lvl) * (9 - lvl) * 2 + 7))
				* 200 / TIME_INTERVAL)) {
				people.push_back(new Man);
			}
		}

	}

	//�������� ������ ����������
	void Man::RemovePeople()
	{
		//�������� ������, ������� �����������
		for (int j = 0; j < people.size(); j++)
		{
			delete people.back();
			people.pop_back();
		}
	}
}

//end.