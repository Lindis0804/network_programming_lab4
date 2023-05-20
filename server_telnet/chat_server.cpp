#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
bool check_account(char *username, char *password, char *filename) {
  FILE *fptr;
  fptr = fopen(filename, "r");
  char str[300];
  char account[300];
  snprintf(account, sizeof(account), "%s %s", username, password);
  printf("account: %s\n", account);
  while (fgets(str, 100, fptr)) {
    printf("%s\n", str);
    str[strlen(str) - 1] = 0;
    if (!strcmp(str, account))
      return true;
  }
  return false;
}
int main() {
  char buf[256];
  char answer[256];
  char filename[100] = "account.txt";
  bool authorized_clients[256];
  char *temp = (char *)malloc(256 * sizeof(char));
  char *username = (char *)malloc(256 * sizeof(char));
  char *password = (char *)malloc(256 * sizeof(char));
  for (int i = 0; i < 256; i++) {
    authorized_clients[i] = false;
  }
  int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (listener == -1) {
    perror("socket() failed.");
    return 1;
  }
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(9000);
  if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
    perror("bind() failed.");
    return 1;
  }
  if (listen(listener, 5)) {
    perror("listen() failed.");
    return 1;
  }
  // create an pollfd set
  struct pollfd fds[64];
  int nfds = 1;
  fds[0].fd = listener;
  fds[0].events = POLLIN;
  while (1) {
    int ret = poll(fds, nfds, -1);
    if (ret < 0) {
      perror("poll() failed.");
      break;
    }
    if (fds[0].revents & POLLIN) {
      int client = accept(listener, NULL, NULL);
      if (nfds == 64) {
        close(client);
      } else {
        fds[nfds].fd = client;
        fds[nfds].events = POLLIN;
        nfds++;
        printf("New client connected: %d\n", client);
      }
    }
    for (int i = 1; i < nfds; i++)
      if (fds[i].revents & POLLIN) {
        ret = recv(fds[i].fd, buf, sizeof(buf), 0);
        if (ret <= 0) {
          close(fds[i].fd);
          // delete from array
          if (i < nfds - 1) {
            fds[i] = fds[nfds - 1];
          }
          nfds--;
          i--;
        } else {
          buf[ret] = 0;
          printf("Received from %d: %s\n", fds[i].fd, buf);
          if (!authorized_clients[fds[i].fd]) {
            temp = strtok(buf, " ");
            strcpy(username, temp);
            temp = strtok(NULL, " ");
            strcpy(password, temp);
            printf("username: %s password: %s\n", username, password);
            if (check_account(username, password, filename)) {
              authorized_clients[fds[i].fd] = true;
              strcpy(buf, "Log in successfully.");
              printf("Client %d log in successfully.\n", fds[i].fd);
            } else
              strcpy(buf, "Username or password is wrong.");
            send(fds[i].fd, buf, strlen(buf), 0);
          } else {
            system(buf);
            snprintf(answer, sizeof(answer), "Command %s is executed.", buf);
            send(fds[i].fd, answer, strlen(answer), 0);
          }
        }
      }
  }
  close(listener);
  return 0;
}