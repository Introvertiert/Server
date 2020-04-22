#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma warning(disable: 4996)

SOCKET Connections[128]; //массив сокетов
int Counter = 0;//счетчик соединений

enum Packet
{
	P_ChatMessage,
	P_Test
};

bool ProcessPacket(int index, Packet packettype)
{
	switch (packettype)
	{
	case P_ChatMessage:
	{
		int msg_size = 0;
		recv(Connections[index], (char*)& msg_size, sizeof(int), NULL);
		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		recv(Connections[index], msg, msg_size, NULL);
		for (int i = 0; i < Counter; ++i)
		{
			if (i == index) continue;
			Packet msgtype = P_ChatMessage;
			send(Connections[i], (char*)& msgtype, sizeof(Packet), NULL);
			send(Connections[i], (char*)& msg_size, sizeof(int), NULL);
			send(Connections[i], msg, msg_size, NULL);
		}
		delete[] msg;
		break;
	}
	case P_Test:
		std::cout << "Test packege\n";
		break;
	

	default:
		std::cout << "Unrecognized packet: " << packettype << std::endl;
		break;
	}
	return true;
}


void ClientHandler(int index)
{
	Packet packettype;
	
	while (true)
	{
		recv(Connections[index], (char*)& packettype, sizeof(Packet), NULL);
		if (!ProcessPacket(index, packettype))
		{
			break;
		}
	}
	closesocket(Connections[index]);
}


int main(int argc, char** argv)
{
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1); //запрашиваемая версия библиотеки winsock
	if (WSAStartup(DLLVersion, &wsaData) != 0) //проверка на загрузку библиотеки
	{
		std::cout << "Error loading" << std::endl;
		exit(1);
	}
	SOCKADDR_IN addr; //структура хранения адреса (для интернет протоколов)
	int sizeOfaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");//local host adders
	addr.sin_port = htons(1111);//портировка программы с поступающими данными
	addr.sin_family = AF_INET;//семейстро протоколов  (для интернета AF_INET)

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL); //создание сокета с присвоением результата выполнения функции парам:(семейство протоколов, протокол устонавливающий соединение 

	bind(sListen, (SOCKADDR*)& addr, sizeof(addr)); //привязка адреса сокету парамм:(сокет, указатель на структуру адреса, размер структуры)
	listen(sListen, SOMAXCONN); //прослушивание порта в ожидании связи с клиентом парамм: сокет, максимальное число запросов

	SOCKET newConnection; //сокет для удержания соединения с клиентом
	 
	//accept - возвращает указатель на новый сокет,который можно использовать для общения с клиентом

	for (size_t i = 0; i < 128; ++i) {

		newConnection = accept(sListen, (SOCKADDR*)& addr, &sizeOfaddr);//парам: сокет на прослушивание, указатель на структуру, ссылка на размер объекта структуры

		if (newConnection == 0) // проверко на наличие подключения клиента к серверу
		{
			std::cout << "Error connection" << std::endl;
			Sleep(100000);
		}
		else
		{
			std::cout << "Client Connected!\n";
			/*std::string mes;
			std::cout << "SEND mess: ";
			std::getline(std::cin, mes);
			send(newConnection, mes.c_str(), sizeof(mes), NULL);*/
			std::string msg = "Hello";
			//std::getline(std::cin, msg);
			int msg_size = msg.size();
			Packet msgtype = P_ChatMessage;
			send(newConnection, (char*)& msgtype, sizeof(Packet), NULL);
			send(newConnection, (char*)& msg_size, sizeof(int), NULL);
			send(newConnection, msg.c_str(), msg_size, NULL);

			Connections[i] = newConnection;
			++Counter;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL); // создание нового потока, парам: ..., третий параметр - указатель на процедуру, 4 параметр - аргумент процедуры, 5- флаг, 6 - куда записывать thread id
			
			Packet testPak = P_Test;
			send(newConnection, (char*)& testPak, sizeof(Packet), NULL);
		
		}
	}
	system("pause >> NULL");
	return 0;
}