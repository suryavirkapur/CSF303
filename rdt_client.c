#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h> // For srand() and time()
#include <unistd.h>

#include "rdt_packet.h" // Include the shared packet structure

#define SERVER_PORT 8888
#define PACKET_DROP_RATE 10 // Percentage (0-100)
#define OUTPUT_FILENAME "output.txt"

// Helper function for error reporting
void die(char *s) {
  perror(s);
  exit(EXIT_FAILURE);
}

int main() {
  int sockfd;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_len = sizeof(client_addr);
  DataPacket recv_pkt;
  AckPacket send_ack;
  FILE *fp;
  int expected_seq_no = 0; // Server starts expecting sequence number 0
  int n;

  // Seed the random number generator for packet dropping
  srand(time(NULL));

  // --- Create UDP socket ---
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    die("socket creation failed");
  }
  printf("Server socket created.\n");

  // --- Configure Server Address ---
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Listen on any interface
  server_addr.sin_port = htons(SERVER_PORT);

  // --- Bind socket to port ---
  if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    close(sockfd);
    die("bind failed");
  }
  printf("Server bound to port %d.\n", SERVER_PORT);

  // --- Open the output file ---
  fp = fopen(OUTPUT_FILENAME, "w"); // Open in write mode, overwriting previous
  if (fp == NULL) {
    close(sockfd);
    die("fopen failed for output file");
  }
  printf("Opened output file: %s\n", OUTPUT_FILENAME);

  // --- Main loop: Receive packets ---
  printf("Server waiting for packets...\n");
  while (1) {
    // Receive packet (blocking call)
    n = recvfrom(sockfd, &recv_pkt, sizeof(DataPacket), 0,
                 (struct sockaddr *)&client_addr, &client_len);

    if (n < 0) {
      perror("recvfrom failed");
      continue; // Wait for the next packet
    }

    printf("RCVD DATA:Seq.No%d---of size%d---Bytes from %s:%d\n",
           recv_pkt.seq_no, recv_pkt.payload_size,
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // --- Simulate Packet Loss ---
    if ((rand() % 100) < PACKET_DROP_RATE) {
      printf("DROP DATA:Seq.No%d--- of size%d---Bytes\n", recv_pkt.seq_no,
             recv_pkt.payload_size);
      // Do not send ACK, just continue waiting for the next packet (or retransmission)
      continue;
    }

    // --- Process Received Packet ---
    if (recv_pkt.seq_no == expected_seq_no) {
      // Correct sequence number received
      printf("Packet %d is expected.\n", expected_seq_no);

      // Write payload to file if it's not the final signal packet
      if (!recv_pkt.is_last && recv_pkt.payload_size > 0) {
        // Ensure null termination just in case, though fwrite uses size
        recv_pkt.payload[recv_pkt.payload_size] = '\0';
        if (fwrite(recv_pkt.payload, 1, recv_pkt.payload_size, fp) <
            recv_pkt.payload_size) {
          perror("fwrite failed");
          // Decide how to handle: maybe send NACK? For now, continue.
        } else {
          // Write a newline after each line for readability in output.txt
          fputc('\n', fp);
          fflush(fp); // Ensure data is written to disk
        }
      }

      // Prepare and send ACK for the received packet
      send_ack.seq_no = expected_seq_no;
      printf("SENT ACK: for PKT with Seq.No.%d\n", send_ack.seq_no);
      if (sendto(sockfd, &send_ack, sizeof(AckPacket), 0,
                 (struct sockaddr *)&client_addr, client_len) < 0) {
        perror("sendto ACK failed");
      }

      // If this was the last packet signal, exit the loop
      if (recv_pkt.is_last) {
        printf("Last packet signal received and ACKed. Shutting down.\n");
        break;
      }

      // Flip expected sequence number for the next packet
      expected_seq_no = 1 - expected_seq_no;

    } else {
      // Duplicate packet received (wrong sequence number)
      printf("Packet %d is a DUPLICATE (expected %d).\n", recv_pkt.seq_no,
             expected_seq_no);

      // Resend ACK for the *last correctly received* packet
      // This helps the client if its previous ACK was lost
      send_ack.seq_no = recv_pkt.seq_no; // ACK the duplicate's seq number
      printf("SENT DUPLICATE ACK: for PKT with Seq.No.%d\n",
             send_ack.seq_no);
      if (sendto(sockfd, &send_ack, sizeof(AckPacket), 0,
                 (struct sockaddr *)&client_addr, client_len) < 0) {
        perror("sendto duplicate ACK failed");
      }
    }
  } // End of main receiving loop

  // --- Cleanup ---
  printf("Server shutting down.\n");
  fclose(fp);
  close(sockfd);
  return 0;
}
