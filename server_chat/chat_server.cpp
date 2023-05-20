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
bool check_id_exist(char **client_ids, int *clients, int start, int end,
                    char *id) {
  for (int i = start; i <= end; i++) {
    if (!strcmp(client_ids[clients[i]], id))
      return true;
  }
  return false;
}
void process_client_id(char **client_ids, int *clients, int start, int end,
                       int i, char *buf, int buf_size) {
  char *temp = (char *)malloc(256 * sizeof(char));
  char *token = (char *)malloc(256 * sizeof(char));
  strcpy(temp, buf);
  token = strtok(temp, ": ");
  printf("token:%s buf:%s ss:%d\n", token, buf, strcmp(buf, token));
  if (!strcmp(buf, token)) {
    strcpy(buf, "Client_name is invalid.");
  } else {
    if (!strcmp(token, "")) {
      strcpy(buf, "Client_id is not defined.");
    } else {
      if (check_id_exist(client_ids, clients, start, end, token))
        strcpy(buf, "Client_id exist.");
      else {
        printf("save client id\n");
        strcpy(client_ids[clients[i]], token);
        snprintf(buf, buf_size, "client_id: %s", client_ids[clients[i]]);
      }
    }
  }
}
int main() {
  int *clients = (int *)malloc(64 * sizeof(int));
  char **client_ids = (char **)malloc(256 * sizeof(char *));
  for (int i = 0; i < 256; i++) {
    client_ids[i] = (char *)malloc(256 * sizeof(char));
  }
  for (int i = 0; i < 256; i++) {
    strcpy(client_ids[i], "");
  }
  char buf[256];
  char answer[256];
  struct tm tm;
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
        clients[nfds] = client;
        nfds++;
        printf("New client connected: %d\n", client);
      }
    }
    for (int i = 1; i < nfds; i++)
      if (fds[i].revents & POLLIN) {
        ret = recv(fds[i].fd, buf, sizeof(buf), 0);
        time_t t = time(NULL);
        tm = *localtime(&t);
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
          printf("Received from %d: %s\n", clients[i], buf);
          int cmp = strcmp(client_ids[clients[i]], "");
          if (cmp == 0) {
            process_client_id(client_ids, clients, 1, nfds - 1, i, buf,
                              sizeof(buf));
            printf("send: %s\n", buf);
            send(clients[i], buf, strlen(buf), 0);
          } else {
            snprintf(answer, sizeof(answer), "%d/%d/%d %d:%d:%d %s: %s",
                     tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
                     tm.tm_min, tm.tm_sec, client_ids[clients[i]], buf);
            printf("Send message: %s", answer);
            for (int i = 1; i < nfds - 1; i++) {
              send(clients[i], answer, sizeof(answer), 0);
            }
            if (strcmp(client_ids[clients[nfds - 1]], ""))
              send(clients[nfds - 1], answer, sizeof(answer), 0);
          }
        }
      }
  }
  close(listener);
  return 0;
}