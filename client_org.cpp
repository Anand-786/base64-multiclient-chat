#include <iostream>

#include <cstdio>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <arpa/inet.h>

#include <sys/socket.h>

#include <chrono>

using namespace std;

char * SERVER_IP;
int SERVER_PORT = 0;
FILE *f1,*f2;
// Function to calculate throughput
void calculateThroughput(int k, size_t totalBytes, double durationInSeconds) {
    double throughput = totalBytes / durationInSeconds; // Bytes per second
    if (k == 0)
        cout << "\nFor TCP : \n";
    else
        cout << "\nFor UDP : \n";
    cout << "\nTotal size sent " << totalBytes / 2 << "B\nThroughput: " << throughput << " Bytes/sec (" <<
        throughput * 8 << " bits/sec)\n" << endl;

	if(k==0){
		fprintf(f1,"%d,%lf\n", (int)totalBytes/2, throughput);
	} else {
		fprintf(f2,"%d,%lf\n", (int)totalBytes/2, throughput);
	}
	
}

int exchange_udp_ports(int test_payload_size = 4) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset( & server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, & server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }

    if (connect(sock, (struct sockaddr * ) & server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    if (test_payload_size == 0) {
        int udp_port = 0;
        for (int o = 1; o <= 32; o++) {
            test_payload_size = o;

            auto startTime = std::chrono::high_resolution_clock::now(); // Start timer

            char test_payload[test_payload_size * 1024 + 4]; // dummy data
            memset(test_payload, '$', sizeof(test_payload));
            // printf("Payload: %s\n", test_payload);
            char tcp_msg[sizeof(test_payload) + 10]; // +4 for metadata: msg type and msg length
            // = "1$0$";
            snprintf(tcp_msg, sizeof(tcp_msg), "1$%d$%s", (int) sizeof(test_payload), test_payload);
            // printf("Sending data: %s\n", tcp_msg);

            udp_port = 0;
            int total_sent_data = 0;
            while (total_sent_data < strlen(tcp_msg)) {
                int bytes_sent = send(sock, tcp_msg + total_sent_data, strlen(tcp_msg) - total_sent_data, 0);
                printf("TCP Communication: bytes sent: %d\n", bytes_sent);
                if (bytes_sent < 0) {
                    fprintf(stderr, "Error occured in tcp transmission\n");
                    exit(0);
                }
                total_sent_data += bytes_sent;
                char buffer[1024] = {
                    0
                };
                read(sock, buffer, 1024);
                // response format: 2$4$PORT_NO
                if (udp_port == 0) {
                    for (int i = 4; i < 8; i++) {
                        udp_port *= 10;
                        udp_port += (buffer[i] - '0');
                    }
                }
                int type = buffer[0] - '0';
                int length = 0;
                int i = 2;
                while (buffer[i] != '$') {
                    length = (length * 10) + (buffer[i] - '0');
                    i++;
                }
                i++;
                printf("TCP Server message: \nType\t\tLength\t\tMessage\n----\t\t------\t\t-------\n%d(Res)\t\t  %d\t\t %s\n\n", type, length, buffer + i);
            }
            auto endTime = std::chrono::high_resolution_clock::now(); // End timer
            // Calculate throughput
            int totalBytes = total_sent_data; // Total bytes sent and received
            double durationInSeconds = std::chrono::duration < double > (endTime - startTime).count();
            cout << "\nTCP Size : " << o << "KB\n";
            calculateThroughput(0, totalBytes * 2, durationInSeconds); // Multiply by 2 for send + receive
        }

        close(sock);

        return udp_port;
    }

    if (test_payload_size >= 1024  && test_payload_size <= 10240) {
        int udp_port = 0;
        int total_data = 0;
        auto startTime = std::chrono::high_resolution_clock::now(); // Start timer
        for (int o = 1; o <= test_payload_size; o++) {
            int t1 = 1;

            auto startTime = std::chrono::high_resolution_clock::now(); // Start timer

            char test_payload[t1 * 1024 + 4]; // dummy data
            memset(test_payload, '$', sizeof(test_payload));
            // printf("Payload: %s\n", test_payload);
            char tcp_msg[sizeof(test_payload) + 10]; // +4 for metadata: msg type and msg length
            // = "1$0$";
            snprintf(tcp_msg, sizeof(tcp_msg), "1$%d$%s", (int) sizeof(test_payload), test_payload);
            // printf("Sending data: %s\n", tcp_msg);

            udp_port = 0;
            int total_sent_data = 0;
            while (total_sent_data < strlen(tcp_msg)) {
                int bytes_sent = send(sock, tcp_msg + total_sent_data, strlen(tcp_msg) - total_sent_data, 0);
                printf("TCP Communication: bytes sent: %d\n", bytes_sent);
                if (bytes_sent < 0) {
                    fprintf(stderr, "Error occured in tcp transmission\n");
                    exit(0);
                }
                total_sent_data += bytes_sent;
                char buffer[1024] = {
                    0
                };
                read(sock, buffer, 1024);
                // response format: 2$4$PORT_NO
                if (udp_port == 0) {
                    for (int i = 4; i < 8; i++) {
                        udp_port *= 10;
                        udp_port += (buffer[i] - '0');
                    }
                }
                int type = buffer[0] - '0';
                int length = 0;
                int i = 2;
                while (buffer[i] != '$') {
                    length = (length * 10) + (buffer[i] - '0');
                    i++;
                }
                i++;
                printf("TCP Server message: \nType\t\tLength\t\tMessage\n----\t\t------\t\t-------\n%d(Res)\t\t  %d\t\t %s\n\n", type, length, buffer + i);
            }

        }
        auto endTime = std::chrono::high_resolution_clock::now(); // End timer
        // Calculate throughput
        int totalBytes = 1024 * test_payload_size; // Total bytes sent and received
        double durationInSeconds = std::chrono::duration < double > (endTime - startTime).count();
        cout << "\nTCP Size : " << test_payload_size << "KB\n";
        calculateThroughput(0, totalBytes * 2, durationInSeconds); // Multiply by 2 for send + receive

        close(sock);

        return udp_port;

    }

    char test_payload[test_payload_size * 1024 + 4]; // dummy data
    memset(test_payload, '$', sizeof(test_payload));
    // printf("Payload: %s\n", test_payload);
    char tcp_msg[sizeof(test_payload) + 10]; // +4 for metadata: msg type and msg length
    // = "1$0$";
    snprintf(tcp_msg, sizeof(tcp_msg), "1$%d$%s", (int) sizeof(test_payload), test_payload);
    // printf("Sending data: %s\n", tcp_msg);

    auto startTime = std::chrono::high_resolution_clock::now(); // Start timer
    int udp_port = 0;
    int total_sent_data = 0;
    while (total_sent_data < strlen(tcp_msg)) {
        int bytes_sent = send(sock, tcp_msg + total_sent_data, strlen(tcp_msg) - total_sent_data, 0);
        printf("TCP Communication: bytes sent: %d\n", bytes_sent);
        if (bytes_sent < 0) {
            fprintf(stderr, "Error occured in tcp transmission\n");
            exit(0);
        }
        total_sent_data += bytes_sent;
        char buffer[1024] = {
            0
        };
        read(sock, buffer, 1024);
        // response format: 2$4$PORT_NO
        if (udp_port == 0) {
            for (int i = 4; i < 8; i++) {
                udp_port *= 10;
                udp_port += (buffer[i] - '0');
            }
        }
        int type = buffer[0] - '0';
        int length = 0;
        int i = 2;
        while (buffer[i] != '$') {
            length = (length * 10) + (buffer[i] - '0');
            i++;
        }
        i++;
        printf("TCP Server message: \nType\t\tLength\t\tMessage\n----\t\t------\t\t-------\n%d(Res)\t\t  %d\t\t %s\n\n", type, length, buffer + i);
    }

    auto endTime = std::chrono::high_resolution_clock::now(); // End timer
    // Calculate throughput
    int totalBytes = total_sent_data; // Total bytes sent and received
    double durationInSeconds = std::chrono::duration < double > (endTime - startTime).count();
    cout << "\nTCP Size : " << totalBytes << "KB\n";
    calculateThroughput(0, totalBytes * 2, durationInSeconds); // Multiply by 2 for send + receive

    close(sock);

    return udp_port;
}

// type_length_message
// type1: tcp/client: udp port request
// type2: tcp/server: udp port response
// type3: udp/client: data message
// type4: udp/server: data response
int main(int argc, char * argv[]) {
	f1=fopen("tcp.txt","w+");
	f2=fopen("udp.txt","w+");
    SERVER_IP = argv[1];
    for (int i = 0; i < 4; i++) {
        SERVER_PORT = (SERVER_PORT * 10) + (argv[2][i] - '0');
    }
    int dummy_payload_size = 4; // for tcp 
    do {
        printf("Enter the dummy payload size to size in KB (0 for throughput 1kb-32kb)	(1024-10240 for throughput 1mb-10mb): ");
        scanf("%d", & dummy_payload_size);
        if (dummy_payload_size > 32 && dummy_payload_size != 1024 && dummy_payload_size != 10240) {
            fprintf(stderr, "Dummy payload size of greater than 32 KB not allowed!\n");
        } else if (dummy_payload_size < 0) {
            fprintf(stderr, "Negative payload size not allowed!\n");
        }
    } while (dummy_payload_size < 0 || (dummy_payload_size > 34 &&( dummy_payload_size < 1024 || dummy_payload_size > 10240)));

    int UDP_PORT = exchange_udp_ports(dummy_payload_size);

    if (UDP_PORT == -1) {
        // error
        return 1;
    }
    // UDP Communication
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        fprintf(stderr, "UDP socket creation failed");
        return 1;
    }

    struct sockaddr_in udp_server_addr;
    udp_server_addr.sin_family = AF_INET;
    udp_server_addr.sin_port = htons(UDP_PORT); // Use the specified UDP port
    inet_pton(AF_INET, SERVER_IP, & udp_server_addr.sin_addr);

    for (int size = 1; size <= 32; size++) {
        auto startTime = std::chrono::high_resolution_clock::now(); // Start timer

        printf("Sending udp message with payload size: %d\n", size);
        char test_payload[size * 1024]; // 1KB, 2KB,....., 32KB
        memset(test_payload, '$', sizeof(test_payload) / sizeof(char));

        char udp_message[sizeof(test_payload) + 4];
        snprintf(udp_message, sizeof(udp_message), "3$%d$%s", (int) sizeof(test_payload), test_payload);
        // = "3$5$Hello";
        if (sendto(udp_sock, udp_message, strlen(udp_message), 0,
                (struct sockaddr * ) & udp_server_addr, sizeof(udp_server_addr)) < 0) {
            fprintf(stderr, "Failed to send UDP message\n");
            close(udp_sock);
            return 1;
        }

        auto endTime = std::chrono::high_resolution_clock::now(); // End timer
        // Calculate throughput
        int totalBytes = size * 1024; // Total bytes sent and received
        double durationInSeconds = std::chrono::duration < double > (endTime - startTime).count();
        cout << "\nUDP Size : " << size << "KB\n";
        calculateThroughput(1, totalBytes * 2, durationInSeconds); // Multiply by 2 for send + receive

        char udp_buffer[1024] = {
            0
        };
        socklen_t addr_len = sizeof(udp_server_addr);
        if (recvfrom(udp_sock, udp_buffer, sizeof(udp_buffer), 0,
                (struct sockaddr * ) & udp_server_addr, & addr_len) < 0) {
            fprintf(stderr, "Failed to receive UDP message\n");
            close(udp_sock);
            return 1;
        }
        int type = udp_buffer[0] - '0';
        int length = 0;
        int i = 2;
        while (udp_buffer[i] != '$') {
            length = (length * 10) + (udp_buffer[i] - '0');
            i++;
        }
        i++;

        printf("UDP Server message: \nType\t\tLength\t\tMessage\n----\t\t------\t\t-------\n%d(Data)\t\t %d\t%s\n\n", type, length, udp_buffer + i);
    }

    close(udp_sock); // Close UDP socket
	fclose(f1);
	fclose(f2);
    return 0;
}