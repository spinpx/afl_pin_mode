#include <cstdio>
extern "C" int mytest() {
  printf("123\n");
}

int main() {
  int sum = 0;

  //   for ( int j = 0; j < 10000; j++) {
  //     sum += (i%12300)*(j%5555);
  //     sum = sum%1025;
  //   }

  // }

  // for (int i = 0; i < 1000; i++) {
  //   printf("sum %d\n", sum + i);
  // }
  return sum%256;
}
