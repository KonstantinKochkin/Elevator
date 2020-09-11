#include"elevator.h"
using namespace CallGeneration;

int main()
{
	const int ERROR_LIMIT = 10;

	ElevatorG theElevatorSystem;

	//�������� ������ � ���� ������ �� ����
	Man::Initialization(&theElevatorSystem);

	int ErrorCounter = 0;
	bool result = true;
	do {
		try {
			//���������� ����
			theElevatorSystem.SystemDisplay();

			//��������� ���������� � ��������� ������ �������
			Man::PeopleTick();

			//��������� ���� � ��������� ���������
			//� ������ ������� ��������� ���������
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






//�� ������� �� �����???
//����� ������� ��������� ����������
// �������� ���� ����� ���� �������������� ����

//�������� �������� �� ����������� ����� � 
//����������� � ������� �������� ������� � �������


//������� ����������� ���������� ������

//����������� (���������)0

//���� � ����������� ��������� � ����

//��������� �������