# Multithreaded TCP/UDP Secure Client-Server

This project is a C++ implementation of a multithreaded client-server application that communicates over both TCP and UDP sockets. It is designed to measure network throughput while securing all data in transit using a custom encryption layer. The server supports multiple concurrent clients and can operate under different scheduling policies.

## Application Flow

The interaction between the client and server follows a specific sequence designed to set up a secure channel and then measure performance.

1.  **TCP Handshake:** The client initiates a connection to the server on a fixed TCP port (8080). The server's main thread accepts this connection and, based on the chosen scheduling policy, assigns a worker thread to handle the client.

2.  **UDP Port Exchange:** The client sends its first message over the newly established TCP connection. This message, which is encrypted and Base64 encoded, requests a dedicated UDP port for the subsequent data transfer phase.

3.  **Server Response:** The server's worker thread decrypts the incoming request. It then allocates a unique UDP port for the client and sends this port number back in an encrypted TCP response. Once this is done, the TCP connection is closed.

4.  **UDP Data Transfer:** The client decrypts the server's response to get the UDP port number. It then enters a loop, sending a series of data packets of varying sizes (1KB to 32KB) to the server on the assigned UDP port. Each of these packets is also encrypted.

5.  **Throughput Measurement:** For each packet sent and received over both TCP and UDP, the client and server calculate the time taken and the amount of data transferred to measure the network throughput. The results are printed to the console and saved to `tcp.txt` and `udp.txt`.

## Core Technical Concepts

### 1. Multithreaded Socket Server

The server is built to handle multiple clients at the same time using the POSIX Threads (`pthread`) library. This concurrency is managed by one of two scheduling policies that can be chosen at runtime:

* **First-Come, First-Served (FCFS):** The server spawns a new, dedicated thread for each client connection as it arrives.
* **Round-Robin (RR):** The server uses a fixed pool of threads and manages execution turns using a mutex and condition variables. This demonstrates a method for controlled resource sharing among clients.

### 2. TCP/UDP Communication Protocol

The application uses a hybrid protocol approach for setup and data transfer.

* **TCP for Initial Handshake:** A reliable, connection-oriented TCP socket is used for the initial communication. This guarantees the delivery of setup information, where the server sends a dedicated UDP port number to the client.
* **UDP for Data Transfer:** After the handshake, communication switches to the connectionless UDP protocol. This is used for the bulk data transfer to measure network throughput with minimal protocol overhead.

### 3. Secure Message Transmission

A two-step process is applied to every message to ensure security during network transit.

* **XOR Encryption:** A symmetric XOR cipher encrypts the plaintext message. This simple and fast algorithm provides a basic layer of confidentiality, requiring the same secret key on both the client and server.
* **Base64 Encoding:** The binary output from the encryption step is then encoded into a Base64 string. This ensures the encrypted data consists of standard, printable characters that can be safely transmitted over any network without data corruption.

## How to Build and Run

You will need a C++ compiler (like g++) and the `pthread` library.

### 1. Compilation

Open a terminal in the project directory and run the following commands:

**Compile the server:**
```sh
g++ server.cpp util.cpp -o server -std=c++11 -pthread
```

**Compile the client:**
```sh
g++ client.cpp util.cpp -o client -std=c++11 -pthread
```

### 2. Execution

**Start the server:**
```sh
# Example: Run server on UDP port 9001 with Round-Robin scheduling
./server 9001 r
```
**Run the client:**
```sh
# Example: Connect to a server on the same machine
./client 127.0.0.1 8080
```
