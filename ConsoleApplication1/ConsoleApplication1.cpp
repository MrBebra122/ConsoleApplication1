#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

// Функция сервера
void server() {
    WSADATA wsaData;
    ADDRINFO hints;
    ADDRINFO* addrResult;
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ConnectSocket = INVALID_SOCKET;
    char recvBuffer[512];

    const char* sendBuffer = "Hello from server";

    // Инициализация Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "Server: WSAStartup failed with result: " << result << endl;
        return;
    }

    // Настройка структуры hints
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Разрешение локального адреса и порта для прослушивания
    result = getaddrinfo(NULL, "6666", &hints, &addrResult);
    if (result != 0) {
        cout << "Server: getaddrinfo failed with error: " << result << endl;
        WSACleanup();
        return;
    }

    // Создание сокета для прослушивания
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Server: Socket creation failed" << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return;
    }

    // Привязка сокета
    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Server: Bind failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return;
    }

    // Начало прослушивания
    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        cout << "Server: Listen failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return;
    }

    // Принятие подключения
    ConnectSocket = accept(ListenSocket, NULL, NULL);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Server: Accept failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return;
    }

    // Закрытие сокета прослушивания, так как подключение установлено
    closesocket(ListenSocket);

    // Обработка данных от клиента
    do {
        ZeroMemory(recvBuffer, sizeof(recvBuffer));
        result = recv(ConnectSocket, recvBuffer, sizeof(recvBuffer), 0);
        if (result > 0) {
            cout << "Server: Received " << result << " bytes" << endl;
            cout << "Server: Received data: " << recvBuffer << endl;

            // Отправка ответа клиенту
            result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
            if (result == SOCKET_ERROR) {
                cout << "Server: Send failed, error: " << WSAGetLastError() << endl;
                closesocket(ConnectSocket);
                freeaddrinfo(addrResult);
                WSACleanup();
                return;
            }
        }
        else if (result == 0) {
            cout << "Server: Connection closing" << endl;
        }
        else {
            cout << "Server: Recv failed, error: " << WSAGetLastError() << endl;
            closesocket(ConnectSocket);
            freeaddrinfo(addrResult);
            WSACleanup();
            return;
        }
    } while (result > 0);

    // Отключение отправки для сокета
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "Server: Shutdown failed, error: " << WSAGetLastError() << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return;
    }

    // Очистка ресурсов
    closesocket(ConnectSocket);
    freeaddrinfo(addrResult);
    WSACleanup();
}

// Функция клиента
void client() {
    WSADATA wsaData;
    ADDRINFO hints;
    ADDRINFO* addrResult;
    SOCKET ConnectSocket = INVALID_SOCKET;
    char recvBuffer[512];

    const char* sendBuffer1 = "Hello from client 1";
    const char* sendBuffer2 = "Hello from client 2";

    // Инициализация Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "Client: WSAStartup failed with result: " << result << endl;
        return;
    }

    // Настройка структуры hints
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Получение информации об адресе сервера
    result = getaddrinfo("localhost", "6666", &hints, &addrResult);
    if (result != 0) {
        cout << "Client: getaddrinfo failed with error: " << result << endl;
        WSACleanup();
        return;
    }

    // Создание сокета для подключения к серверу
    ConnectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Client: Socket creation failed" << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return;
    }

    // Установка соединения с сервером
    result = connect(ConnectSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Client: Unable to connect to server" << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return;
    }

    // Отправка первого сообщения серверу
    result = send(ConnectSocket, sendBuffer1, (int)strlen(sendBuffer1), 0);
    if (result == SOCKET_ERROR) {
        cout << "Client: Send failed, error: " << WSAGetLastError() << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return;
    }
    cout << "Client: Sent: " << result << " bytes" << endl;

    // Отправка второго сообщения серверу
    result = send(ConnectSocket, sendBuffer2, (int)strlen(sendBuffer2), 0);
    if (result == SOCKET_ERROR) {
        cout << "Client: Send failed, error: " << WSAGetLastError() << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return;
    }
    cout << "Client: Sent: " << result << " bytes" << endl;

    // Отключение отправки для сокета
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "Client: Shutdown failed, error: " << WSAGetLastError() << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return;
    }

    // Получение ответа от сервера
    do {
        ZeroMemory(recvBuffer, sizeof(recvBuffer));
        result = recv(ConnectSocket, recvBuffer, sizeof(recvBuffer), 0);
        if (result > 0) {
            cout << "Client: Received " << result << " bytes" << endl;
            cout << "Client: Received data: " << recvBuffer << endl;
        }
        else if (result == 0) {
            cout << "Client: Connection closed" << endl;
        }
        else {
            cout << "Client: Recv failed, error: " << WSAGetLastError() << endl;
        }
    } while (result > 0);

    // Очистка ресурсов
    closesocket(ConnectSocket);
    freeaddrinfo(addrResult);
    WSACleanup();
}

int main() {
    // Запуск сервера в отдельном потоке
    thread serverThread(server);

    // Задержка для ожидания запуска сервера
    Sleep(1000);

    // Запуск клиента в главном потоке
    client();

    // Ожидание завершения работы сервера
    serverThread.join();

    return 0;
}
