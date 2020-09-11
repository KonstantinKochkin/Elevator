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

//��� ���� ������ �� ������: ����� � ����
enum buttonDirectionType { bUp, bDown };
//����������� �������� ��� ������� �����
enum movingDirectionType { up, down, no };
//��� ��������� ��������� ������� �����
enum carStateType {
	startsUp, movingUp, stopsUp, doorOpening, stopped,
	doorClosing, startsDown, movingDown, stopsDown,
	notUsed, slowMovingUp, slowMovingDown
};

//��������� ��� ������ �� ������`
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
	bool* callsDown;	// ������ ���� �� ������
	bool* callsUp;		//������ ����� �� ������
protected:
	ElevatorCar** cars;	//������ � �������� �� ��� ������� ������
public:
	const int floorCount;	//����� ������
	const int carCount;		//����� ������� �����

private:
	//�������� �� ������ ����� ������ �������? 
	bool theCallIsTargetOfAnotherElevators(const CallButton cb)const; //new

public:
	Elevator(bool key = 1);  //key=0  ��� ������������
	virtual ~Elevator();

	//���������� ����� � �����
	//���� ����� ����� ���� �������� � ���� �� ������, ������� false
	virtual bool SetTheCall(const CallButton);

	//�������� �� ������ �� �����?
	bool theCallButtonIsOn(const CallButton)const;

	//�������� ������� ����� �� ��� ������
	ElevatorCar*const GetCar(const int carNum)const; //new

	//����� ����� ��������� ������ ��� ������� �����,
	//� ������ ���� ��������� �������
	CallButton FindTask(const ElevatorCar*const theCar)const;

	//��������� ������ � �����
	virtual void CallButtonOff(const CallButton);//new

	// ��������� ������� � ��������� ���������
	//factor - ����������� ������� ��������, ������������ TIME_INTERVAL 
	void SystemTick(double factor = 1.0);

	//����� �������, ������� ��� �������� �� ����� � ������� �� �����
	int FindReadyCar(const CallButton cb)const;

	void CheckButton(const CallButton cb)const;
};

class ElevatorCar //������� �����
{
private:
	bool* orderFloors;			//������ � �������� ������ ������ 
	int timer;			//����� ���������� � ������� ���������

protected:
	const int carNumber;				//����� �������
	Elevator*const parentElev;			//������������ ����
	const int floorCount;				//����� ����� ������
	int currentFloor;					//������� ����
	movingDirectionType movingDirection; //����������� ��������
	carStateType state;					//���������
	int targetFloor;					// ������� ����
	bool targetCallAvailability;		//������� �������� ������
	CallButton targetCall;				//������� �����

private:

	//���������� ����� ��������� ���� ������� ���� ����� pos,
	//���� ����� ���� ���� �����, �� ������� true
	const bool CalculateHigher
	(const CallButton cb, double& ariveTime, int& pos)const;

	//���������� ����� ��������� ���� ������� ���� ����� pos,
	//���� ����� ���� ���� �����, �� ������� true
	const bool CalculateLower
	(const CallButton cb, double& ariveTime, int& pos)const;

protected:
	//���������� ������� ���� ��� �������,
	//���� �������� ����� ���, ������� false
	const bool DeterminateTargetFloor(); //new

	//��������� ����������� ����� ������, �������
	//��������� ������� ��� ����������
	const int GetBrakingFloor()const;

public:
	ElevatorCar(int nElevator, Elevator* ptrElevSys);
	ElevatorCar(ElevatorCar& elev);
	virtual ~ElevatorCar();

	//�������� ����� �������
	const int GetCarNumber()const;
	//�������� ������� ���� �������
	const int GetCurrentFloor()const;
	//�������� ����������� �������� �������
	const movingDirectionType GetMovingDirection()const;
	//�������� ��������� �������
	const carStateType GetState()const;
	//�������� �� ���� ����������?
	const bool isOrderFloor(const int floor)const;
	//�������� ������� ����
	const int GetTargetFloor()const;
	//���� �� �����,���������� ����� �������
	const bool GetTargetCallAvailability()const;
	//������� ����� ��� �������
	const CallButton GetTargetCall()const;
	//�������� �� �������?
	const bool IsFree()const;

	//������� �����
	void OpenDoor();

	//������� �����
	void CloseDoor();

	//���������� ����� �� ����
	//���� ������� ��� ������� �� ���� �����, ������� false
	virtual bool SetOrderFloor(const int nFloor);

	//���������� ������� ����
	void SetTargetFloor(CallButton cb);

	//��������� ������ � ������� ��� �� �����
	virtual void TurnOffButton();

	//�������� ������� � ��������� ���������
	virtual void Tick(); //new

	//��������� ������� � ������� ���������, � timer
	//�������� ����� ����������� ��� �������� �� ����� 
	void MoveElevator(const CallButton cb, const double time);

	//����� ��������� ����� �������� � ������� ������?
	const bool CallIsBetweenCarAndTargetFloor(const CallButton cb)const; //new

	//��� ������� ����� ������ �������� � �������? 
	const bool IsBetweenCarAndCall
	(const ElevatorCar*const elev, const CallButton cb)const ;

	//������� ���� ��������� ������, ��� �����?
	const bool TargetIsEarlierThanCall(const CallButton cb)const;

	//�������� ����� �������� ������� �� ����� � ������, 
	//��� ��� �������� ��� ���� ������ � ������ �� ����
	const double CalculateTheCallAriveTime(const CallButton cb)const;

	//��������� ����� �������� ������� �� �����,
	//�������� ��� ������ ������ � ������
	const double MinCalculateTheCallAriveTime(const CallButton cb)const ;

	//�������� ����� increment �� ����� �� ������� �����,
	//�� ������� ����� ������������ �������.
	//���� increment ������ ��� ����� ������ �� ������
	//�� ����� ��������� ������ �����
	const CallButton NextFloor(int increment)const;

	void CheckFloor(const int floor)const;
};

class ElevatorCarG;

//���� � ����������� �����������
class ElevatorG : public Elevator
{
private:
	ElevatorCarG** carsG;	//������ � �������� �� ��� ������� ������
	long long runningTime;	//����� ������ �����
	string callLogName;		//��� ������� �������
	ofstream callLogFile;	//����� ����� ������� �������
	bool stateFileLog;		//�������� ��� �� ��������
	int callGenerationLevel;//������������� ��������� ���������� [0..9]
	int speedID;			//������� �������� ��������� �����
public:
	ElevatorG();
	~ElevatorG();

	//�������� �������
	void GraphicsON();

	//���������� �� ������ ��������� ����� � ������� ������ �������
	void SystemDisplay()const;

	// ��������� ������� � ��������� ���������
	// ���������� false � ������ ������������� ��������� ���������
	bool SystemTick();

	//�������� ������ ��� ����� ����� ������ � �������������,
	//���� �������� �������� �� ���������� ��������� ������ false
	const bool GetDirections();

	//���������� ����� � �����
	//���� ����� ����� ���� �������� � ���� �� ������, ������� false
	bool SetTheCall(const CallButton) override;

	//��������� ������ � �����
	void CallButtonOff(const CallButton) override;//new

	//�������� � ������ ����� �� �������
	void WriteCarOrder(int idCar, int orderFloor);

	//������� ������� �������� ���������� �� �����
	void DisplayGenerationLevel()const;

	//�������� ������� ��������� ����������
	int GetGenerationLevel()const;

	//������� �������� ������������� �����
	void DisplaySpeed()const;

	//�������� ����������� ����� ������� �������
	void UpdateFileLogState(bool fstate);
};

//������� � ����������� �����������
class ElevatorCarG : public ElevatorCar
{
private:
	ElevatorG*const parentElevG;
	int lastfloor;	//����, �� ������� ���� ���������
					//� ������� ������ �������
public:
	ElevatorCarG(int nElevator, ElevatorG* ptrElevSys);

	//���������� ����� �� ����
	//���� ������� ��� ������� �� ���� �����, ������� false
	bool SetOrderFloor(const int nFloor) override;

	//��������� ������ � ������� ��� �� �����
	void TurnOffButton() override;

	//�������� ������� � ��������� ���������
	void Tick() override; 

	//���������� ������� �� ������
	void CarDisplay()const;
};


namespace CallGeneration
{
	class Man	//�������� �����
	{
	private:
		//��������� ����������
		enum manState { wait, entry, going };
		//wait - ������ ���� � ���� ��������
		//entry - ������� � ������� � �������� ������
		//going - ���� �� ���� ����

		manState state;	//��������� ���������
		CallButton cb;	//�����, ������� ������ ��������
		int carEntryTime;	//����� ������� �� ������ ������
		int carNumber;	//����� �������, ���������� �� �����
		int order;	//����� ����� � �����

		//����, � ������� ��������������� ��������
		static ElevatorG* elev;
		static list<Man*> people; //������ ���� ����������
	public:
		Man();	//���������� �����
		void SetOrder(); //���������� �����

		//��������� ��������� � ��������� ���������(���� ��
		//��� ������ �����, �� �������� false - ����� �������
		bool Tick(); 

		//���������� ������ ����������
		static void Initialization(ElevatorG* el);

		//��������� ���� ���������� � ��������� ���������
		static void PeopleTick();

		//�������� ������ ����������
		static void RemovePeople();
	};
}