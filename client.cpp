#include <stdio.h>
#include <conio.h>
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string>
#include <iostream>

const int BUFFER = 512;
char str[BUFFER];
char answer;

void menu()
{
	printf("menu:\n1\t-\twrite\n2\t-\texit\n");
}

// Приём сообщения
void getMessage(SOCKET s)
{
	char srti[BUFFER];
	if (recv(s, srti, BUFFER, 0) != 0)
	{
		printf("%s", srti);
	}
}

// Отправка сообщения
void sendMessage(SOCKET s)
{
	bool working = true;
	while (working)
	{
		str[0] = getchar();
		if (str[0] == '\n')
		{
			working = false;
		}
		send(s, str, 1, 0);
	}
}

int main()
{
#pragma region CONNECT
	WSADATA wsaData;
	WSAStartup(0x202, &wsaData);
	sockaddr_in serv;
	serv.sin_family = AF_INET;
	serv.sin_port = htons(1111);
	serv.sin_addr.s_addr = inet_addr("127.0.0.1");
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
	{
		printf("socket error %d\n", WSAGetLastError());
		system("pause");
		closesocket(s);
		WSACleanup();
		return -1;
	}

	if (connect(s, (sockaddr *)&serv, sizeof(serv)))
	{
		printf("connect error %d\n", WSAGetLastError());
		system("pause");
		closesocket(s);
		WSACleanup();
		return -1;
	}

#pragma endregion CONNECT

	getMessage(s);
	sendMessage(s);		// Ввод имени

	getMessage(s);
	sendMessage(s);		// Ввод типа подключения (торговец, покупатель)

	while (true)
	{
		system("cls");
		getMessage(s);
		sendMessage(s);
	}

	closesocket(s);
	WSACleanup();
	system("pause");
	return 0;
}