#include <cstdio>

#include <iostream>

#include <cstring>

#include <unistd.h>

#include <arpa/inet.h>

#include <sys/select.h>

#include <chrono>

#define MAX_CLIENTS 3

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int running_thread = 0;

// Function to calculate throughput
void calculateThroughput(int k, size_t totalBytes, double durationInSeconds) {
    double throughput = totalBytes / durationInSeconds; // Bytes per second
    if (k == 0)
        std::cout << "\nFor TCP : \n";
    else
        std::cout << "\nFor UDP : \n";
    std::cout << "\nTotal size sent " << totalBytes / 2 << "B\nThroughput: " << throughput << " Bytes/sec (" <<
        throughput * 8 << " bits/sec)\n" << std::endl;
}

void * handle_client_tcp_rr(void * arg) {
    int client_socket = ((int * ) arg)[0];
    int UDP_PORT = ((int * ) arg)[1];
    int threadid = ((int * ) arg)[2];
    // free(arg);  // Free the memory allocated for the client socket descriptor
    char buffer[33 * 1024] = {
        0
    };

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        // Read message from client
        pthread_mutex_lock( & mutex);
        while (running_thread != threadid) {
            pthread_cond_wait( & cond, & mutex); // releases locks and waits for notify
            // mutex lock is implicitly re-acquired when cond_wait returns
        }

        //Critical Section
        if (read(client_socket, buffer, sizeof(buffer)) <= 0) {
            printf("Client disconnected\n\n");
            return NULL;
        }

        printf("Connection accepted from thread %d client : %d\n\n", threadid, client_socket);
        int type = buffer[0] - '0';
        int length = 0;
        // fprintf(stdout, "Message reciedved from client: %s\n", buffer);
        for (int i = 2;
            (buffer[i] != '$'); i++) length = 10 * length + (buffer[i] - '0');

        // printf("\nTCP Client message: \n\nType\t\tLength\t\tMessage\n----\t\t------\t\t-------\n%d(Req)\t\t%d\t\t  -\n\n",type,length);
        memset(buffer, 0, sizeof(buffer));
        snprintf(buffer, sizeof(buffer), "2$4$%d", UDP_PORT);

        running_thread = (running_thread + 1) % MAX_CLIENTS;
        pthread_cond_broadcast( & cond);
        pthread_mutex_unlock( & mutex);

        pthread_mutex_lock( & mutex);
        while (running_thread != threadid) {
            pthread_cond_wait( & cond, & mutex);
        }

        // Send response to client(Critical Section)
        printf("UDP PORT NO. sent to thread %d client: %d\n\n", threadid, UDP_PORT);
        send(client_socket, buffer, strlen(buffer), 0);

        running_thread = (running_thread + 1) % MAX_CLIENTS;
        pthread_cond_broadcast( & cond);
        pthread_mutex_unlock( & mutex);
    }

    // Close the client socket
    close(client_socket);
    printf("exit: handle_client_tcp()\n");
    return NULL;
}

// Function to handle communication with a client
void * handle_client_tcp_fcfs(void * arg) {
    int client_socket = ((int * ) arg)[0];
    int UDP_PORT = ((int * ) arg)[1];
    printf("Connection accepted from: %d, UDP PORT NO. passed: %d\n\n", client_socket, UDP_PORT);
    // free(arg);  // Free the memory allocated for the client socket descriptor

    char buffer[33 * 1024] = {
        0
    };
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        // Read message from client
        if (read(client_socket, buffer, sizeof(buffer)) <= 0) {
            printf("Client disconnected\n\n");
            break;
        }
        int type = buffer[0] - '0';
        int length = 0;

        for (int i = 2;
            (buffer[i] != '$'); i++) length = 10 * length + (buffer[i] - '0');

        printf("\nTCP Client message: \nType\t\tLength\t\tMessage\n----\t\t------\t\t-------\n%d(Req)\t\t  %d\t\t  %s\n\n", type, length, buffer);

        memset(buffer, 0, sizeof(buffer));
        snprintf(buffer, sizeof(buffer), "2$4$%d", UDP_PORT);
        // Send response to client
        send(client_socket, buffer, strlen(buffer), 0);
        printf("Response sent to client.\n");
    }

    // Close the client socket
    close(client_socket);
    printf("exit: handle_client_tcp()\n");
    return NULL;
}

void * listen_tcp(void * arg) {
    printf("entered: listen_tcp()\n");
    const int TCP_PORT = ((int * ) arg)[0]; // interpret the argument as int* and dereference it 
    const int UDP_PORT = ((int * ) arg)[1];
    const int policy = ((int * ) arg)[2];
    // Create TCP socket
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0) {
        perror("TCP socket creation failed");
        exit(1);
    }

    // Configure TCP server address
    struct sockaddr_in tcp_server_addr;
    tcp_server_addr.sin_family = AF_INET;
    tcp_server_addr.sin_addr.s_addr = INADDR_ANY;
    tcp_server_addr.sin_port = htons(TCP_PORT);

    // Bind and listen on TCP socket
    if (bind(tcp_socket, (struct sockaddr * ) & tcp_server_addr, sizeof(tcp_server_addr)) < 0) {
        perror("TCP bind failed");
        close(tcp_socket);
        exit(1);
    }

    if (listen(tcp_socket, MAX_CLIENTS) < 0) {
        perror("TCP listen failed");
        close(tcp_socket);
        exit(1);
    }

    fprintf(stdout, "Server listening for TCP connections on port %d\n", TCP_PORT);
    // int conns[MAX_CLIENTS];
    // int new_conn_idx =0;
    int id = 0;
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        int client_socket = accept(tcp_socket, (struct sockaddr * ) & client_addr, & addr_len);
        if (client_socket < 0) {
            perror("TCP accept failed");
            continue;
        }
        int params[3] = {
            client_socket,
            UDP_PORT,
            id
        };
        if (policy == 'f') {
            handle_client_tcp_fcfs((void * ) params);
        } else {
            id = (id + 1) % MAX_CLIENTS;
            pthread_t rr_tcp_id;
            if (pthread_create( & rr_tcp_id, NULL, handle_client_tcp_rr, (void * ) params) != 0) {
                perror("Thread creation failed for TCP socket\n");
                continue;
            }
        }
    }

    close(tcp_socket);
    printf("exit: listen_tcp()\n");
    return NULL;
}

void * listen_udp(void * arg) {

    printf("entered: listen_udp()\n");
    const int UDP_PORT = * (int * ) arg;
    // Create UDP socket
    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        perror("UDP socket creation failed");
        close(udp_socket);
        exit(1);
    }

    // Configure UDP server address
    struct sockaddr_in udp_server_addr;
    udp_server_addr.sin_family = AF_INET;
    udp_server_addr.sin_addr.s_addr = INADDR_ANY;
    udp_server_addr.sin_port = htons(UDP_PORT);

    // Bind UDP socket
    if (bind(udp_socket, (struct sockaddr * ) & udp_server_addr, sizeof(udp_server_addr)) < 0) {
        perror("UDP bind failed");
        close(udp_socket);
        exit(1);
    }

    fprintf(stdout, "Server listening for UDP connections on port %d\n", UDP_PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        char buffer[1024] = {
            0
        };

        int bytes_received = recvfrom(udp_socket, buffer, sizeof(buffer), 0,
            (struct sockaddr * ) & client_addr, & addr_len);
        if (bytes_received < 0) {
            perror("UDP receive failed");
            continue;
        }
        int type = buffer[0] - '0';
        int length = 0;
        int i = 2;
        while (buffer[i] != '$') {
            length = (length * 10) + (buffer[i] - '0');
            i++;
        }
        i++;
        printf("UDP Client message: \nType\t\tLength\t\tMessage\n----\t\t------\t\t-------\n%d(Data)\t\t %d\t\t%s\n\n", type, length, buffer + i);

        const char * response = "4$20$Successful Reception";
        sendto(udp_socket, response, strlen(response), 0,
            (struct sockaddr * ) & client_addr, addr_len);
        printf("Acknowledgement sent to client.\n\n");
    }

    close(udp_socket);
    printf("exit: listen_udp()\n");
    return NULL;
}

int main(int argc, char * argv[]) {
    char scheduling_policy = * argv[2];
    const int TCP_PORT = 8080;
    int temp = 0;
    for (int i = 0; i < 4; i++)
        temp = (temp * 10) + (argv[1][i] - '0');
    const int UDP_PORT = temp;

    int params[3] = {
        TCP_PORT,
        UDP_PORT,
        scheduling_policy
    };
    if (scheduling_policy == 'f')
        printf("\nScheduling policy selected : FCFS\n\n");
    else if (scheduling_policy == 'r')
        printf("\nScheduling policy selected : Round-Robin\n\n");
    pthread_t tcp_thread_id, udp_thread_id;
    if (pthread_create( & tcp_thread_id, NULL, listen_tcp, (void * ) params) != 0) {
        perror("Thread creation failed for TCP socket\n");
        return 1;
    }

    if (pthread_create( & udp_thread_id, NULL, listen_udp, (void * ) & UDP_PORT) != 0) {
        perror("Thread creation failed for TCP socket\n");
        return 1;
    }

    pthread_join(tcp_thread_id, NULL);
    pthread_join(udp_thread_id, NULL);

    return 0;
}