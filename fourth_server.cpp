#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    const int port = 8080;

    // Создание сокета
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        perror("Ошибка создания сокета");
        return EXIT_FAILURE;
    }

    // Настройка адреса сервера
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Привязка сокета
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Ошибка привязки сокета");
        close(server_socket);
        return EXIT_FAILURE;
    }

    std::cout << "UDP сервер запущен на порту " << port << ". Ожидание сообщений...\n";

    char buffer[1024];
    sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);

    while (true) {
        std::memset(buffer, 0, sizeof(buffer));

        // Получение данных от клиента
        int bytes_received = recvfrom(server_socket, buffer, sizeof(buffer) - 1, 0,
                                      (struct sockaddr *)&client_addr, &client_addr_len);
        if (bytes_received < 0) {
            perror("Ошибка при получении данных");
            continue;
        }

        std::cout << "Получено сообщение: " << buffer << " от "
                  << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << "\n";

        // Отправка данных обратно клиенту
        int bytes_sent = sendto(server_socket, buffer, bytes_received, 0,
                                (struct sockaddr *)&client_addr, client_addr_len);
        if (bytes_sent < 0) {
            perror("Ошибка при отправке данных");
        }
    }

    close(server_socket);
    return 0;
}
