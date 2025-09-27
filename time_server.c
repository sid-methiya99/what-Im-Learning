#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

int main() {
    printf("Configuring local address...\n");
    // This is used to create a struct named hints which has all addrinfo content
    struct addrinfo hints;
    struct addrinfo* bind_addresses;
    // This function sets members inside hints to 0
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    //  it generates an address that's suitable for bind().
    getaddrinfo(0, "8080", &hints, &bind_addresses);

    SOCKET socket_listen;
    socket_listen = socket(bind_addresses->ai_family, bind_addresses->ai_socktype, bind_addresses->ai_protocol);

    if (!ISVALIDSOCKET(socket_listen)) {
        fprintf(stderr, "socket failed. (%d) \n", GETSOCKETERRNO());
        return 1;
    }

    printf("Binding socket to local address...\n");
    if (bind(socket_listen, bind_addresses->ai_addr, bind_addresses->ai_addrlen)) {
        fprintf(stderr, "bind failed. (%d) \n", GETSOCKETERRNO());
        return 1;
    }

    freeaddrinfo(bind_addresses);

    // Here 10 means if more than 10 connection are pending in queue then OS can reject it
    if (listen(socket_listen, 10) < 0) {
        fprintf(stderr, "listen failed. (%d) \n", GETSOCKETERRNO());
        return 1;
    }

    printf("Waiting for connection...\n");

    // This stores address info of the connecting client
    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);

    /*
        So accept function waits for new connection and when it receives new connection it creates a new socket for that client and keeps listening for new connection on existing connection
    */
    SOCKET socket_client = accept(socket_listen, (struct sockaddr*)&client_address, &client_len);
    if (!ISVALIDSOCKET(socket_client)) {
        fprintf(stderr, "accept failed. (%d) \n", GETSOCKETERRNO());
        return 1;
    }

    printf("Client is connected... ");
    char address_buffer[100];
    getnameinfo((struct sockaddr*)&client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
    printf("%s\n", address_buffer);

    printf("Reading requests...\n");
    char request[1024];

    int bytes_received = recv(socket_client, request, 1024, 0);
    printf("bytes received: %.*s\n", bytes_received, request);

    printf("Sending response...\n");
    const char* response =
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-Type: text/plain\r\n\r\n"
        "Local time is: ";
    int bytes_sent = send(socket_client, response, strlen(response), 0);
    printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(response));

    time_t timer;
    time(&timer);
    char* time_msg = ctime(&timer);
    bytes_sent = send(socket_client, time_msg, strlen(time_msg), 0);
    printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(time_msg));

    printf("Closing connection...\n");
    CLOSESOCKET(socket_client);
    return 0;
}
