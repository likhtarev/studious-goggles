#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>

void set_non_blocking(int socket) {
    int flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    const char *server_ip = "127.0.0.1"; // Локальный сервер
    const int port = 8080;

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Ошибка создания сокета");
        return EXIT_FAILURE;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Неверный адрес сервера");
        return EXIT_FAILURE;
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Ошибка подключения к серверу");
        return EXIT_FAILURE;
    }

    set_non_blocking(client_socket);
    std::cout << "Подключено к серверу. Вы можете отправлять сообщения.\n";

    fd_set read_fds;
    char buffer[1024];

    while (true) {
        FD_ZERO(&read_fds);
        FD_SET(client_socket, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        int max_fd = client_socket > STDIN_FILENO ? client_socket : STDIN_FILENO;

        timeval timeout = {1, 0};
        int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, &timeout);

        if (activity < 0) {
            perror("Ошибка select");
            break;
        }

        // Проверка ввода от пользователя
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            std::string message;
            std::getline(std::cin, message);

            if (message == "exit") {
                break;
            }

            send(client_socket, message.c_str(), message.size(), 0);
        }

        // Проверка входящих сообщений от сервера
        if (FD_ISSET(client_socket, &read_fds)) {
            std::memset(buffer, 0, sizeof(buffer));
            int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

            if (bytes_received <= 0) {
                std::cout << "Соединение с сервером разорвано.\n";
                break;
            }

            std::cout << "Сообщение: " << buffer << "\n";
        }
    }

    close(client_socket);
    return 0;
}
