#include <signal.h>
#include <poll.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#define MAX_CLIENTS 32
#define PORT 3000

struct pollfd client[MAX_CLIENTS];

int main() {
  // Create the server socket
  int server = socket(AF_INET, SOCK_STREAM, 0);

  // Bind to the TCP port
  struct sockaddr_in address;
  memset(&address, 0, sizeof(struct sockaddr_in));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(PORT);
  bind(server, (struct sockaddr*)&address, sizeof(struct sockaddr_in));

  // Listen on socket
  listen(server, 16);

  client[0].fd = server;
  client[0].events = POLLIN;

  for (int i = 1; i < MAX_CLIENTS; i++) {
    client[i].fd = -1;
  }

  printf("Starting server");
  for (;;) {
    int nready = poll(client, 0, 500);
    printf("neady %d", nready);


  }

  printf("Hello\n");
  poll(client, 0, 1000);
  printf("World\n");
  return 0;
}
