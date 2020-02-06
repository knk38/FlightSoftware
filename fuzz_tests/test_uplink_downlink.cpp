#define DESKTOP
#include <vector>
#include <unistd.h>
#include <random>
#include "bitstream.h"
#include "UplinkCommon.h"
#include "UplinkProducer.h"
#include "UplinkConsumer.h"

//AFL_HARDEN=4 afl-clang++ ../src/common/bitstream.cpp test_uplink-downlink.cpp -std=c++14
#define STREAM_SIZE 70


int main()
{
  srand(NULL);
  char input[STREAM_SIZE] = {0};
  size_t length = read(STDIN_FILENO, input, STREAM_SIZE);

  return 0;
}

void encode()
{

}

void decode()
{

}