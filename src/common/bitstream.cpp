#include "bitstream.h"
#include <cstring>
#include "fixed_array.hpp"

bitstream::bitstream(char* input, uint32_t stream_size) :
  bit_offset(0),
  stream(reinterpret_cast<uint8_t*>(input)),
  byte_offset(0),
  max_len(stream_size)
{
}

bitstream::bitstream(const std::vector<bool>& bit_array, char* res) :
  bit_offset(0),
  byte_offset(0)
{
  unsigned int stream_size = (bit_array.size() + 7)/8;
  for (unsigned int i = 0; i < stream_size; ++i)
  {
    res[i] = 0;
    for (int j = 0; j < 8; ++j)
    {
      res[i] |= ( (bit_array[i*8 + j]) <<  j );
    }
  }
  max_len = stream_size;
  stream = reinterpret_cast<uint8_t*>(res);
}

bool bitstream::has_next()
{
  return byte_offset < max_len;
}

unsigned int bitstream::next(unsigned int num_bits, uint8_t* u8)
{
  if (num_bits > 8) return 0;
  uint8_t bits_read = 0;
  uint8_t val = *reinterpret_cast<uint8_t*>(stream + byte_offset);
  val >>= bit_offset;
  
  // Read until the next byte starts
  unsigned int amt = 8 - bit_offset;
  for (; bits_read < amt && bits_read < num_bits; val >>= 1, ++bits_read)
    *u8 = bit_array::modify_bit(*u8, bits_read, val&1);

  // See if there is a next byte to write
  if (byte_offset + 1 < max_len && bits_read < num_bits)
  {
    val = *reinterpret_cast<uint8_t*>(stream + byte_offset + 1);
    // Read to the beginning of the next byte
    for (unsigned int i = 0; i < bit_offset && bits_read < num_bits; ++i, val >>= 1, ++bits_read)
      *u8 = bit_array::modify_bit(*u8, bits_read, val&1);
  }
  seekG(bits_read, bs_end);
  return bits_read;
}

unsigned int bitstream::nextN(unsigned int num_bits, uint8_t* res)
{
  memset(res, 0, (num_bits + 7)/8);
  unsigned int bits_written = 0;

  unsigned int num_iters = num_bits/8;
  for (unsigned int i = 0; i < num_iters && has_next(); ++i)
  {
    bits_written  += next(8, res + i);
  }
  unsigned int modulo = num_bits%8;
  if (modulo != 0 && has_next())
  {
    bits_written += next(modulo, res + num_iters);
  }
  return bits_written;
}

unsigned int bitstream::nextN(unsigned int num_bits, std::vector<bool>& bit_arr)
{
  unsigned int arr_size = bit_arr.size();
  if (arr_size < num_bits)
    return 0;
  for (unsigned int i = 0; i < arr_size; ++i) bit_arr[i] = 0;
  unsigned int bits_written = 0;
  uint8_t u8 = 0;
  for (; bits_written < num_bits && has_next(); ++bits_written)
  {
    if ( next(1, &u8) )
    {
      bit_arr[bits_written] = u8;
    }
    else
    {
      break; // leave if there are no more bits available
    }
  }
  return bits_written;
}

unsigned int bitstream::peekN(unsigned int num_bits, uint8_t* res)
{
  unsigned int bits_peeked = 0;

  bits_peeked = nextN(num_bits, res);
  seekG(bits_peeked, bs_beg);
  return bits_peeked;
}

unsigned int bitstream::peekN(unsigned int num_bits, std::vector<bool>& bit_arr)
{
  unsigned int bits_peeked = 0;

  bits_peeked = nextN(num_bits, bit_arr);
  seekG(bits_peeked, bs_beg);
  return bits_peeked;
}

unsigned int bitstream::seekG(unsigned int amt, int dir)
{
  if (dir != -1 && dir != 1)
    return 0;

  // get the current absolute bit offset and adjust it accordingly
  int desired_off = 8*byte_offset + bit_offset;
  desired_off += (dir*amt);

  if (desired_off > 8*(int)max_len || desired_off < 0) 
    return 0;

  // calculate new offset from the current offset
  unsigned int new_byte_off = desired_off/8;
  unsigned int new_bit_off = desired_off%8;

  byte_offset = new_byte_off;
  bit_offset = new_bit_off;

  return amt;
}

unsigned int bitstream::edit(unsigned int num_bits, uint8_t* val)
{
  uint8_t old = *reinterpret_cast<uint8_t*>(stream + byte_offset);
  uint8_t u8 = *val;
  uint8_t bits_written = 0;
  // Write until the next byte starts
  unsigned int amt = 8 - bit_offset;
  for (unsigned int i = 0; i < amt && bits_written < num_bits; ++i, u8 >>= 1, ++bits_written)
    old = bit_array::modify_bit(old, bit_offset + i, u8&1);
  
  // Write the lower bits
  *reinterpret_cast<uint8_t*>(stream + byte_offset) = old;
  
  // See if there is a next byte to write
  if (byte_offset + 1 < max_len && bits_written < num_bits)
  {
    old = *reinterpret_cast<uint8_t*>(stream + byte_offset + 1);
    // Write to the beginning of the next byte
    for (unsigned int i = 0; i < bit_offset && bits_written < num_bits; ++i, u8 >>= 1, ++bits_written)
    {
      old = bit_array::modify_bit(old, i, u8&1);
    }
    // Write the rest of the upper bits
    *reinterpret_cast<uint8_t*>(stream + byte_offset + 1) = old;
  }
  seekG(bits_written, bs_end);
  return bits_written;
}

unsigned int bitstream::editN(unsigned int num_bits, uint8_t* new_val)
{
  unsigned int num_iters = num_bits/8;
  if (num_iters > byte_offset + max_len)
    num_iters = max_len - byte_offset;
  unsigned int modulo = num_bits%8;
  unsigned int bits_written = 0;
  for (unsigned int i = 0; i < num_iters; ++i)
  {
    bits_written += edit(8, new_val + i);
  }
  if (modulo != 0)
  {
    bits_written += edit(modulo, new_val + num_iters);
  }
  return bits_written;
}

unsigned int bitstream::editN(unsigned int num_bits, bitstream& bs_other)
{
  unsigned int arr_size = (num_bits + 7)/8;
  uint8_t tmp[arr_size];
  memset(tmp, 0, arr_size);

  unsigned int bits_written = 0;

  // Attempt to read [num_bits] from bs_other
  bits_written = bs_other.nextN(num_bits, tmp);
  // Write those bits to this bitstream
  bits_written = editN(bits_written, tmp);
  return bits_written;
}

void bitstream::reset()
{
  byte_offset = 0;
  bit_offset = 0;
}

bitstream& operator >>(bitstream& bs, uint32_t& res)
{
  bs.nextN(32, reinterpret_cast<uint8_t*>(&res));
  return bs;
}

bitstream& operator >>(bitstream& bs, uint16_t& res)
{
  bs.nextN(16, reinterpret_cast<uint8_t*>(&res));
  return bs;
}

bitstream& operator >>(bitstream& bs, uint8_t& res)
{
  bs.nextN(8, &res);
  return bs;
}

bitstream& operator >>(bitstream& bs, std::vector<bool>& bit_arr)
{
  bs.nextN(bit_arr.size(), bit_arr);
  return bs;
}

bitstream& operator <<(uint32_t& u32, bitstream& bs)
{ 
  bs.editN(32, reinterpret_cast<uint8_t*>(&u32));
  return bs;
}

bitstream& operator <<(uint16_t& u16, bitstream& bs)
{
  bs.editN(16, reinterpret_cast<uint8_t*>(&u16));
  return bs;
}

bitstream& operator <<(uint8_t& u8, bitstream& bs)
{
  bs.editN(8, &u8);
  return bs;
}

bitstream& operator <<(std::vector<bool>& ba, bitstream& bs)
{
  char backed[ba.size()];
  bitstream bs_other(ba, backed);
  bs_other << bs;
  return bs;
}

bitstream& operator <<(bitstream& bs_other, bitstream& bs)
{
  unsigned int num_bits = (bs_other.max_len - bs_other.byte_offset)*8 - bs_other.bit_offset;
  bs.editN(num_bits, bs_other);
  return bs;
}