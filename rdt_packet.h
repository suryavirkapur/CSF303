#ifndef RDT_PACKET_H
#define RDT_PACKET_H

#define PAYLOAD_SIZE 500 // Max bytes for the payload (file line)
#define HEADER_SIZE (sizeof(int) * 3) // Approximate size for seq, size, last

typedef struct {
  int seq_no;       // Sequence number (0 or 1)
  int payload_size; // Actual size of the data in the payload array
  int is_last;      // Flag: 1 if this is the last packet, 0 otherwise
  char payload[PAYLOAD_SIZE]; // Data payload (a line from the file)
} DataPacket;

typedef struct {
  int seq_no; 
} AckPacket;

#endif 
