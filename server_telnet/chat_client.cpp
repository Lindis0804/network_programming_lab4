#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
bool check_server_access_allowance(char *str1) {
  if (!strcmp(str1, "Username or password is wrong."))
    return false;
  return true;
}
int main() {
  int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_port = htons(9000);

  if (connect(client, (struct sockaddr *)&addr, sizeof(addr))) {
    perror("connect() failed.");
    return 1;
  }
  struct pollfd fds[2];
  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;

  fds[1].fd = client;
  fds[1].events = POLLIN;

  char buf[256];
  char account[256];
  while (1) {
    printf("Input your username, password, follow this format: "
           "username: password \n");
    int ret = poll(fds, 2, -1);
    if (ret < 0) {
      perror("poll() failed.");
      break;
    }
    if (ret == 0) {
      printf("Timed out.\n");
      // continue;
    }
    // printf("ret = %d\n", ret);
    if (fds[0].revents & POLLIN) {
      fgets(account, sizeof(account), stdin);
      account[strlen(account) - 1] = 0;
      printf("Send: %s\n", account);
      send(client, account, strlen(account), 0);
      ret = recv(client, buf, sizeof(buf), 0);
      if (ret <= 0) {
        break;
      }
      buf[ret] = 0;
      printf("Received: %s\n", buf);
      if (check_server_access_allowance(buf))
        break;
    }
    if (fds[1].revents & POLLIN) {
      fgets(account, sizeof(account), stdin);
      account[strlen(account) - 1] = 0;
      printf("Send: %s\n", account);
      send(client, account, strlen(account), 0);
      ret = recv(client, buf, sizeof(buf), 0);
      if (ret <= 0) {
        break;
      }
      buf[ret] = 0;
      printf("Received: %s\n", buf);
      if (check_server_access_allowance(buf))
        break;
    }
  }
  while (1) {
    printf("Send command:\n");
    int ret = poll(fds, 2, -1);
    if (ret < 0) {
      perror("poll() failed.");
      break;
    }
    // tại sao hai đoạn này bị lặp vô hạn???
    if (ret == 0) {
      printf("Timed out.\n");
      continue;
    }
    // printf("ret = %d\n", ret);
    if (fds[0].revents & POLLIN) {
      fgets(buf, sizeof(buf), stdin);
      buf[strlen(buf) - 1] = 0;
      send(client, buf, strlen(buf), 0);
    }
    if (fds[1].revents & POLLIN) {
      ret = recv(client, buf, sizeof(buf), 0);
      if (ret <= 0) {
        break;
      }
      buf[ret] = 0;
      printf("Received: %s\n", buf);
    }
  }
  close(client);
  return 0;
}