#pragma comment(lib,"Ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS 1

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h> // Needed for _wtoi
#include <iostream>
//#include <mutex>
#include <list>
#include <stdexcept>
#include <memory>

int readn(SOCKET s, char * buf, size_t len);
int charToInt(char ch);
const char * intToStr(int i);
int strToInt(const char *str);
void sendMessage(SOCKET client_socket, const char *str);
void getMessage(SOCKET client_socket, char *str);
//void currentListOfLot(std::list<Lot> lst, char *str);
const int BUFFER = 512;
enum ClientType {MANAGER, BIDDER};

class ClientThread
{
public: 
SOCKET client_socket;
void *HandleThread;
unsigned long IdThread;

ClientThread(SOCKET client)
{
	client_socket = client; 
	std::cout << "I've connected!";
	HandleThread = CreateThread(NULL, 0, ThreadF, this, 0, &IdThread); 
}
~ClientThread()
{
TerminateThread(HandleThread, NULL);
CloseHandle(HandleThread);
}

private: 
static DWORD WINAPI ThreadF(void *arg)
{
return ((ClientThread*)arg)->operation_order();
}

protected:
	unsigned int operation_order()
	{
		printf("You entered!!!\n");
		return 0;
	}
};
class Lot
{
private:
	char *name;
	int cost;
public:
	Lot()
	{
		this->name = new char[1];
		this->cost = 0;
	}

	Lot(const char *name, int cost)
	{
		int len = strlen(name);
		this->name = new char[len + 1];
		strcpy(this->name, name);

		this->cost = cost;
	}

	Lot(const Lot &cpy)
	{
		int len = strlen(cpy.name);
		this->name = new char[len + 1];
		strcpy(this->name, cpy.name);

		this->cost = cpy.cost;
	}

	Lot & operator= (const Lot &cpy)
	{
		if (this == &cpy)
			return *this;

		delete[]name;
		int len = strlen(cpy.name);
		this->name = new char[len + 1];
		strcpy(this->name, cpy.name);

		this->cost = cpy.cost;
	}

	const char *getName()
	{
		return this->name;
	}
	void setName(const char *name)
	{
		delete[](this->name);
		this->name = new char[strlen(name) + 1];
		strcpy(this->name, name);
	}

	int getCost()
	{
		return this->cost;
	}
	void setCost(int cost)
	{
		this->cost = cost;
	}

	void increase(int i)
	{
		this->cost += i;
	}

	~Lot()
	{
		delete[]name;
	}
};

void currentListOfLot(std::list<Lot> lst, char *str);
std::list<Lot> listOfLot;
void ThreadProc(SOCKET client_socket, SOCKET sock, sockaddr_in client_addr)
{
#pragma region CONNECTED
	if (client_socket < 0)
	{
		// Ошибка
		printf("Listen Error %d\n", WSAGetLastError());
		//system("pause");
		closesocket(sock);
		WSACleanup();
		//return -5;
	}

	char* client_IP = inet_ntoa(client_addr.sin_addr); //адрес того кто к тебе подключается
													   //printf("%s connected \n", client_IP);
#pragma endregion CONNECTED

	char str[BUFFER];
	char *clientName;
	ClientType clientType;

#pragma region HELLO

	printf("%i connected \n", client_socket);

	// Запрос имени
	strcpy(str, "Hello!\n");
	strcat(str, "Your name: ");
	sendMessage(client_socket, str);
	getMessage(client_socket, str);							
	clientName = new char[strlen(str) + 1];
	strcpy(clientName, str);

	// Запрос типа подключения
	strcpy(str, "\nType client:\n");
	strcat(str, "1 - Manager\n");
	strcat(str, "2 - Bidder\n");
	sendMessage(client_socket, str);
	getMessage(client_socket, str);
	if (str[0] == '1')
		clientType = MANAGER;
	else if (str[0] == '2')
		clientType = BIDDER;
	else
		printf("error");

#pragma endregion

	bool working = true;
	strcpy(str, "");
	while (working)
	{
		switch (clientType)
		{
#pragma region MANAGER
		case MANAGER:
			strcat(str, "Menu:\n");
			strcat(str, "1 - add a Lot\n");
			strcat(str, "2 - closing of a Lot\n");
			strcat(str, "0 - exit\n");
			strcat(str, "answer: ");
			sendMessage(client_socket, str);
			getMessage(client_socket, str);
			// add a Lot
			if (str[0] == '1')
			{
				Lot temp;
				strcpy(str, "add a Lot:\n");
				strcat(str, "get name of a Lot: ");
				sendMessage(client_socket, str);
				getMessage(client_socket, str);
				temp.setName(str);

				sendMessage(client_socket, "get initial cost: ");
				getMessage(client_socket, str);
				temp.setCost(strToInt(str));

				printf("new Lot: \"%s\" - %i\n", temp.getName(), temp.getCost());
				listOfLot.push_back(temp);
				strcpy(str, "");
			}
			// closing of a Lot
			else if (str[0] == '2')
			{
				strcpy(str, "current list of a Lots:\n");
				int i = 1;
				for (std::list<Lot>::iterator it = listOfLot.begin(); it != listOfLot.end(); it++)
				{
					strcat(str, intToStr(i++));
					strcat(str, " - ");
					strcat(str, (*it).getName());
					strcat(str, "( current Value: ");
					strcat(str, intToStr((*it).getCost()));
					strcat(str, " )\n");
				}
				strcat(str, "\nWhat lot to close?: \n");
				sendMessage(client_socket, str);
				getMessage(client_socket, str);

				int temp = strToInt(str);
				if (temp > listOfLot.size() || temp <= 0)
				{
					strcpy(str, "The lot doesn't exist\n\n");
					continue;
				}

				std::list<Lot>::iterator it = listOfLot.begin();
				for (int i = 1; i < temp; i++)
					it++;

				printf("close lot %s with the price %i\n", (*it).getName(), (*it).getCost());
				strcpy(str, "close lot ");
				strcat(str, (*it).getName());
				strcat(str, " with the price ");
				strcat(str, intToStr((*it).getCost()));
				strcat(str, "\n\n");

				listOfLot.erase(it);
			}
			// Exit
			else if (str[0] == '0')
			{
				working = false;
			}
			// Error
			else
			{
				printf("error");
				working = false;
			}
			break;
#pragma endregion MANAGER MENU

#pragma region BIDDER
		case BIDDER:
			strcat(str, "Current list of a lots:\n");
			currentListOfLot(listOfLot, str);
			strcat(str, "1 - update\n");
			strcat(str, "2 - value increase\n");
			strcat(str, "0 - exit\n");
			strcat(str, "answer: ");
			sendMessage(client_socket, str);
			getMessage(client_socket, str);
			// update
			if (str[0] == '1')
			{
				continue;
			}
			// value increase
			else if (str[0] == '2')
			{
				strcpy(str, "current list of a Lots:\n");
				currentListOfLot(listOfLot, str);
				strcat(str, "value increment:\n");
				strcat(str, "number of the lot: ");
				sendMessage(client_socket, str);
				getMessage(client_socket, str);

				int temp = strToInt(str);
				try
				{
					std::list<Lot>::iterator it = listOfLot.begin();
					for (int i = 1; i < temp; i++)
						it++;

					strcpy(str, intToStr(temp));
					strcat(str, " - ");
					strcat(str, (*it).getName());
					strcat(str, "  (current Value: ");
					strcat(str, intToStr((*it).getCost()));
					strcat(str, " )\n");
					strcat(str, "your rate: ");
					sendMessage(client_socket, str);
					getMessage(client_socket, str);

					(*it).increase(strToInt(str));
				}
				catch (std::exception &ex)
				{
					strcpy(str, ex.what());
					continue;
				}
			}
			// Exit
			else if (str[0] == '0')
			{
				working = false;
			}
			// Error
			else
			{
				printf("error");
				working = false;
			}
			break;
#pragma endregion BIDDER MENU
		}
	}

	/*
	for (int i = 0; i < 3; i++)
	{
		int len = 0;
		len = readn(client_socket, srti, 100);

		WaitForSingleObject(ghMutex, INFINITE);

		printf("%i write: ", client_socket);
		for (int i = 0; i<len; i++)
		{
			printf("%c", srti[i]);
			send(client_socket, &srti[i], sizeof("d"), 0);
		}

		ReleaseMutex(ghMutex);
	}
	*/

	delete[]clientName;
	printf("CLOSE\n");
}



int strToInt(const char *str)
{
	int temp = 0;
	for (size_t i = 0; i < strlen(str); i++)
	{
		temp *= 10;
		switch (str[i])
		{
		case '1':	temp += 1;	break;
		case '2':	temp += 2;	break;
		case '3':	temp += 3;	break;
		case '4':	temp += 4;	break;
		case '5':	temp += 5;	break;
		case '6':	temp += 6;	break;
		case '7':	temp += 7;	break;
		case '8':	temp += 8;	break;
		case '9':	temp += 9;	break;
		case '0':	temp += 0;	break;
		default:	return 0;
		}
	}

	return temp;
}
int charToInt(char ch)
{
	switch (ch)
	{
	case '1': return 1;
	case '2': return 2;
	case '3': return 3;
	case '4': return 4;
	case '5': return 5;
	case '6': return 6;
	case '7': return 7;
	case '8': return 8;
	case '9': return 9;
	case '0': return 0;
	default:	return -1;
	}
}
const char * intToStr(int i)
{
	char str[BUFFER];
	int pointer = 0;
	if (i < 0)
		str[pointer++] = '-';

	while (i != 0)
	{
		switch (i % 10)
		{
		case 1:	str[pointer++] = '1';	break;
		case 2:	str[pointer++] = '2';	break;
		case 3:	str[pointer++] = '3';	break;
		case 4:	str[pointer++] = '4';	break;
		case 5:	str[pointer++] = '5';	break;
		case 6:	str[pointer++] = '6';	break;
		case 7:	str[pointer++] = '7';	break;
		case 8:	str[pointer++] = '8';	break;
		case 9:	str[pointer++] = '9';	break;
		case 0:	str[pointer++] = '0';	break;
		}

		i /= 10;
	}

	for (int i = 0; i < pointer / 2; i++)
	{
		char temp = str[i];
		str[i] = str[pointer - i - 1];
		str[pointer - i - 1] = temp;
	}

	str[pointer] = '\0';
	return str;
}

//int readn(SOCKET s, char * buf, size_t len);
void sendMessage(SOCKET client_socket, const char *str)
{
	send(client_socket, str, strlen(str) + 1, 0);
}
void getMessage(SOCKET client_socket, char *str)
{
	int len = readn(client_socket, str, BUFFER);
	str[len] = '\0';
	printf("%i - %s\n", client_socket, str);
}
void currentListOfLot(std::list<Lot> lst, char *str)
{
	int i = 1;
	for (std::list<Lot>::iterator it = lst.begin(); it != lst.end(); it++)
	{
		strcat(str, intToStr(i++));
		strcat(str, " - ");
		strcat(str, (*it).getName());
		strcat(str, "  (current Value: ");
		strcat(str, intToStr((*it).getCost()));
		strcat(str, " )\n");
	}
}

HANDLE ghMutex;

int main()
{
	SOCKET client_socket;
	SOCKET sock = INVALID_SOCKET;
	sockaddr_in client_addr;

	WSADATA wsaData = { 0 };
	int iResult = 0;

	int iFamily = AF_INET;
	int iType = SOCK_STREAM; // c установлением соединения 
	int iProtocol = 0;

	iResult = WSAStartup(0x0202, &wsaData); // версия библиотеки 0x0202
	if (iResult != 0) {
		wprintf(L"WSAStartup failed: %d\n", iResult);
		system("pause");
		return -1;
	}

	sock = socket(iFamily, iType, iProtocol);
	if (sock == INVALID_SOCKET)
	{
		std::cout << "Creation Socket failed";
		system("pause");
		return -2;
	}

	sockaddr_in service;

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr("127.0.0.1");
	service.sin_port = htons(1111);

	if (bind(sock, (sockaddr *)& service, sizeof(service)))
	{
		// Ошибка
		printf("Bind Error %d\n", WSAGetLastError());
		//system("pause");
		closesocket(sock);
		WSACleanup(); //очистка ресурсов библиотеки винсок

		return -3;
	}

	if (listen(sock, 5))
	{
		// Ошибка
		printf("Listen Error %d\n", WSAGetLastError());
		//system("pause");
		closesocket(sock);
		WSACleanup();
		system("pause");
		return -4;
	}

	//SOCKET client_socket; //клиентский сокет
	int client_addr_size = sizeof(client_addr);
	
	while(1)
	{

		client_socket = accept(sock, (sockaddr *)&client_addr, &client_addr_size);

		if (client_socket < 0)
		{
			// Ошибка
			printf("Listen Error %d\n", WSAGetLastError());
			//system("pause");
			closesocket(sock);
			WSACleanup();
			//return -5;
		}
			
		ClientThread * newClient = new ClientThread(client_socket);

		char* client_IP = inet_ntoa(client_addr.sin_addr); //адрес того кто к тебе подключается
															   //printf("%s connected \n", client_IP);
		printf("%i connected \n", client_socket);

		send(client_socket, "Hello \n", sizeof("Hello \n"), 0);
		int n = 0;
		char srti[100];
		// Ввод трёх слов
		//for (int i = 0; i < 3; i++)
		//{
			int len = 0;
			len = readn(client_socket, srti, 100);
			//WaitForSingleObject(ghMutex, INFINITE);
			//mutexObj.lock();
			//printf("lock()\n");
			printf("%i write: ", client_socket);
			for (int i = 0; i<len; i++)
			{
				printf("%c", srti[i]);
				send(client_socket, &srti[i], sizeof("d"), 0);
			}
			//printf("\nunlock()\n\n");
			//ReleaseMutex(ghMutex);
			//mutexObj.unlock();
		//}
	}

	closesocket(client_socket);
	closesocket(sock);
	WSACleanup();

	system("pause");
	return 0;
}

int readn(SOCKET fd, char *bp, size_t len)
{
	int cnt;
	int rc;
	cnt = 0;
	while (cnt != len)
	{
		rc = recv(fd, bp, 1, 0);
		if (*bp == '\n') return cnt;

		if (rc < 0) /* Ошибка чтения? */
		{
			if (errno == -1) /* Вызов прерван? */
				continue; /* Повторить чтение. */
			return -1; /* Вернуть код ошибки. */
		}
		if (rc == 0) /* Конец файла? */
			return len - cnt; /* Вернуть неполный счетчик. */
		bp += rc;
		cnt += rc;
	}
	return cnt;
}
