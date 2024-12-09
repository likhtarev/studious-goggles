int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Использование: " << argv[0] << " <IP-адрес>\n";
        return EXIT_FAILURE;
    }

    const char *ip_addr = argv[1];

    // Создание сырого сокета
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        perror("Ошибка создания сокета");
        return EXIT_FAILURE;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip_addr, &addr.sin_addr) <= 0) {
        perror("Неверный IP-адрес");
        close(sock);
        return EXIT_FAILURE;
    }

    char packet[64];
    memset(packet, 0, sizeof(packet));

    // Подготовка ICMP Echo Request
    struct icmp *icmp_hdr = (struct icmp *)packet;
    icmp_hdr->icmp_type = ICMP_ECHO;
    icmp_hdr->icmp_code = 0;
    icmp_hdr->icmp_id = getpid();
    icmp_hdr->icmp_seq = 1;
    icmp_hdr->icmp_cksum = 0;
    icmp_hdr->icmp_cksum = checksum(packet, sizeof(packet));

    struct timeval start_time, end_time;

    std::cout << "PING " << ip_addr << " (" << ip_addr << ") 64 bytes of data.\n";

    for (int i = 0; i < 4; ++i) {
        // Запоминаем время отправки
        gettimeofday(&start_time, nullptr);

        // Отправка ICMP Echo Request
        if (sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr)) <= 0) {
            perror("Ошибка отправки пакета");
            close(sock);
            return EXIT_FAILURE;
        }

        // Ожидание ответа
        char recv_buffer[1024];
        sockaddr_in recv_addr{};
        socklen_t recv_addr_len = sizeof(recv_addr);
        int bytes_received = recvfrom(sock, recv_buffer, sizeof(recv_buffer), 0,
                                      (struct sockaddr *)&recv_addr, &recv_addr_len);

        if (bytes_received < 0) {
            perror("Ошибка приема пакета");
            close(sock);
            return EXIT_FAILURE;
        }

        // Запоминаем время получения ответа
        gettimeofday(&end_time, nullptr);

        // Расчет RTT (Round Trip Time)
        double rtt = time_diff(&start_time, &end_time);

        std::cout << "64 bytes from " << ip_addr << ": icmp_seq=" << i + 1
                  << " ttl=" << (int)recv_buffer[8] << " time=" << rtt << " ms\n";

        sleep(1); // Пауза в 1 секунду между отправками
    }

    close(sock);
    std::cout << "--- " << ip_addr << " ping statistics ---\n";
    std::cout << "4 packets transmitted, 4 received, 0% packet loss\n";

    return EXIT_SUCCESS;
}
