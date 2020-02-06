#include "../src/common/bitstream.h"
#include "../src/common/fixed_array.hpp"
#include <vector>
#include <unistd.h>
#include <random>
//AFL_HARDEN=1 afl-clang++ ../src/common/fixed_array.hpp ../src/common/bitstream.cpp test_bitstream.cpp -std=c++14
#define STREAM_SIZE 70
int choose_op(bitstream& bs);
int main()
{
  srand(NULL);
  char input[STREAM_SIZE];
  size_t length = read(STDIN_FILENO, input, STREAM_SIZE);
  bitstream bs(input, length);

  uint8_t u8; 
  bs>>u8;
  uint16_t u16;
  bs>>u16;
  uint32_t u32;
  bs>>u32;
  bs.reset();
  std::vector<bool> bit_arr (STREAM_SIZE, 0);
  bs>>bit_arr;

  for (size_t i = 0; i < 42; ++i)
  {
    choose_op(bs);
  }
  return 0;
}

int choose_op(bitstream& bs)
{
  static int num_ops = 9;
  std::vector<bool> bit_arr(rand(), 0);
  std::vector<bool> bit_arr_1(rand(), 0);
  uint8_t u8 = rand();
  uint8_t u8_1 = rand();
  uint8_t new_val [rand()%STREAM_SIZE];
  char bs_other_arr[rand()];
  bitstream bs_other(bs_other_arr, sizeof(bs_other_arr));
  if (rand() % 2 == 0)
    bs.reset();
  switch(rand() % num_ops )
  {
    case 0:
      return bs.has_next();
    case 1:
      return bs.nextN(rand()%STREAM_SIZE, &u8);
    case 2:
      return bs.nextN(rand()%STREAM_SIZE, bit_arr);
    case 3:
      return bs.peekN(rand()%STREAM_SIZE, &u8_1);
    case 4:
      return bs.peekN(rand(), bit_arr_1);
    case 5:
      return bs.seekG(rand()%STREAM_SIZE, rand()%2);
    case 6:
      return bs.editN(rand()%STREAM_SIZE, new_val);
    case 7:
      return bs.editN(rand()%sizeof(bs_other_arr), bs_other);
    case 8:
      bs.reset();
  }
  return 0;
}