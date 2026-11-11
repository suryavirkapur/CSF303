#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
   
    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    printf("udp_fd %d\n", udp_fd);
    printf("tcp_fd %d\n", tcp_fd);
    if (udp_fd == -1 || tcp_fd == -1) {
        perror("socket could not be created");
        exit(1);
    }
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(9999);

    int server = bind(udp_fd, (struct sockaddr_in *)&address, sizeof(address));

    if (server < 0) {
        perror("couldn't bind");
        exit(1);
    }

    printf("[SERVER] Listening on PORT %d", 9999);

    close(tcp_fd);
    close(udp_fd);
    return 0;
}