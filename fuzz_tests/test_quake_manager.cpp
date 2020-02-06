#include "../src/common/Serializer.hpp"
#include "../src/common/GPSTime.hpp"
#include "../.pio/libdeps/teensy35_hitl/CommonSoftware/include/libsbp/navigation.h"
#include "../src/common/StateField.hpp"
#include "../src/common/debug_console.hpp"
#include "../src/fsw/FCCode/QuakeManager.h"
#include "../src/fsw/FCCode/TimedControlTask.hpp"
#include "../src/fsw/FCCode/radio_state_t.enum"
#include "../test/StateFieldRegistryMock.hpp"
#include "../src/common/bitstream.h"
#include "../src/fsw/FCCode/ControlTask.hpp"
#include <unistd.h>
#include <random>

#define SNAP_SIZE 350
#define MT_LEN 345

//AFL_HARDEN=4 afl-clang++ ../src/common/bitstream.cpp test_bitstream.cpp -std=c++14
int choose_op(QuakeManager& qm);
int main()
{
  srand(NULL);
  char input[SNAP_SIZE] = {0};
  size_t length = read(STDIN_FILENO, input, SNAP_SIZE);
  StateFieldRegistryMock registry;
  std::shared_ptr<InternalStateField<char*>> radio_mo_packet_fp;
  std::shared_ptr<InternalStateField<size_t>> snapshot_size_fp;
  snapshot_size_fp = registry.create_internal_field<size_t>("downlink.snap_size");
  radio_mo_packet_fp = registry.create_internal_field<char*>("downlink.ptr");
  snapshot_size_fp->set(static_cast<int>(SNAP_SIZE));
  radio_mo_packet_fp->set(input);
  QuakeManager qm(registry, 0);

  for (size_t i = 0; i < 426969; ++i)
  {
    if (qm.radio_state_f.get() == static_cast<unsigned char>(radio_state_t::disabled))
      choose_op(qm);
    if (qm.radio_state_f.get() == static_cast<unsigned char>(radio_state_t::wait))
      choose_op(qm);
    if (qm.execute() == false)
      assert(0);
  }
  return 0;
}

int choose_op(QuakeManager& qm)
{
  static int num_ops = 5;
  switch(rand() % num_ops)
  {
    case 0:
    return qm.dispatch_config();
    case 1:
    return qm.dispatch_transceive();
    case 2:
    return qm.dispatch_read();
    case 3:
    return qm.dispatch_write();
    case 4:
    return qm.dispatch_wait();
  }
  return 0;
}