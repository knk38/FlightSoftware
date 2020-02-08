
#include "../../src/common/bitstream.h"
#include <vector>
#include <unistd.h>
#include <random>
#include <assert.h>
//AFL_HARDEN=4 afl-clang++ ../src/common/fixed_array.hpp ../src/common/bitstream.cpp test_bitstream.cpp -std=c++14
#define STREAM_SIZE 70
int choose_op(bitstream& bs);
void test10(char* myarr, size_t arr_size);
int main()
{
  srand(0);
  char input[STREAM_SIZE] = {0};
  size_t length = read(STDIN_FILENO, input, STREAM_SIZE);
  // bitstream bs(input, length);
  test10(input, length);

  // uint8_t u8;
  // bs>>u8;
  // uint16_t u16;
  // bs>>u16;
  // uint32_t u32;
  // bs>>u32;
  // bs.reset();
  // std::vector<bool> bit_arr (STREAM_SIZE, 0);
  // bs>>bit_arr;

  // for (size_t i = 0; i < 69; ++i)
  // {
  //   choose_op(bs);
  // }
  // return 0;
}


void test10(char* myarr, size_t arr_size)
{
  // char* myarr = (char*)"\xde\xad\xbe\xef\xab\xcd\xef";
  // '0b 1101 1110 1010 1101 1011 1110 1110 1111 1010 1011 1100 1101 1110 1111'
  // 0xdeadbeefabcdef
  bitstream bs(myarr, arr_size);
  std::vector<bool> my_ba = std::vector<bool>(arr_size*8, 0); 
  size_t subset_size = 4269%(arr_size*8);
  size_t bits_read = bs.nextN(subset_size, my_ba);
  assert(subset_size == bits_read);
  uint8_t expect;
  for (int i = 0; i < arr_size; ++i ) 
  {
    expect = 0;
    for (int j = 0; j < 8; ++j)
    {
      if (i*8 + j >= subset_size) // since i only asked for subset_size, there should only be at most subset_size
        break;
      expect = (myarr[i] >> j) & 1;
      assert( expect == my_ba[i*8 + j] );
    }
  }
  for (int i = subset_size; i < 8*arr_size; ++i) // everyone else should be 0
  {
    assert( 0 == my_ba[i] );
  }
}

int choose_op(bitstream& bs)
{
  static int num_ops = 9;
  std::vector<bool> bit_arr(rand()%STREAM_SIZE, 0);
  std::vector<bool> bit_arr_1(rand()%STREAM_SIZE, 0);
  uint8_t u8 = rand();
  uint8_t u8_1 = rand();
  size_t size_new_val = rand()%STREAM_SIZE;
  uint8_t new_val [size_new_val];
  size_t size_bs_other = rand()%STREAM_SIZE;
  char bs_other_arr[size_bs_other];
  bitstream bs_other(bs_other_arr, size_bs_other);
  int dir = (rand()%2 == 0) ? -1 : 1;
 if (rand() % 7 == 0)
    bs.reset();
  switch(rand() % num_ops )
  {
    case 0:
      return bs.has_next();
    case 1:
      return bs.nextN(rand()%8, &u8);
    case 2:
      return bs.nextN(rand(), bit_arr);
    case 3:
      return bs.peekN(rand()%8, &u8_1);
    case 4:
      return bs.peekN(rand(), bit_arr_1);
    case 5:
      return bs.seekG(rand(), dir);
    case 6:
      return bs.editN(rand()%size_new_val, new_val);
    case 7:
      return bs.editN(rand()%size_bs_other, bs_other);
    case 8:
      bs.reset();
  }
  return 0;
}
