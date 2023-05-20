#include <stdio.h>
void increase(int &x) { x++; }
void increase1(int *x) { x[0] = 2; }
int main() {
  int x = 2;
  int arr[3] = {0, 1, 2};
  increase(x);
  increase1(arr);
  printf("x: %d\n", x);
  printf("%d %d %d", arr[0], arr[1], arr[2]);
  return 0;
}