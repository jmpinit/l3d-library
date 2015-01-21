#include "stub.h"
#include <unistd.h>

void delay(long millis)
{
  usleep(millis * 1000);
}

void setup(void);
void loop(void);

int main()
{
  setup();

  while(true)
    loop();

  return 0;
}
