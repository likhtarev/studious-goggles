#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

void error(const std::string &msg) {
    perror(msg.c_str());
    exit(EXIT_FAILURE);
}

int main() {
    const char *hostname = "pmk.tversu.ru";
    const int port = 80;
    const std::string request =
        "GET / HTTP/1.1\r\n"
        "Host: pmk.tversu.ru\r\n"
        "Connection: close\r\n\r\n";

    // Получение IP-адреса сервера
    struct hostent *server = gethostbyname(hostname);
    if (server == nullptr) {
        error("Ошибка: не удалось получить IP-адрес сервера");
    }

    // Создание сокета
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        error("Ошибка: не удалось создать сокет");
    }

    // Заполнение структуры адреса сервера
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    std::memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    // Подключение к серверу
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error("Ошибка: не удалось подключиться к серверу");
    }

    // Отправка HTTP-запроса
    if (send(sock, request.c_str(), request.length(), 0) < 0) {
        error("Ошибка: не удалось отправить запрос");
    }

    // Получение и вывод ответа
    char buffer[4096];
    int bytes_received;
    while ((bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';  // Добавление нуля для завершения строки
        std::cout << buffer;
    }

    if (bytes_received < 0) {
        error("Ошибка: не удалось получить данные");
    }

    // Закрытие сокета
    close(sock);

    return 0;
}
