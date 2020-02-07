#define DESKTOP
#include <vector>
#include <unistd.h>
#include <random>
#include <common/bitstream.h>
#include <fsw/FCCode/UplinkCommon.h>
#include <fsw/FCCode/UplinkConsumer.h>
#include "../test/StateFieldRegistryMock.hpp"
#include "test_fixture.hpp"


// TIL how to use the -I flag
// AFL_HARDEN=4 afl-clang++ -DPAN_LEADER -I ../src/common -I ../lib/common/libsbp/include -I../src -I ../lib/common/psim/include -I ../src/fsw/FCCode -I ../lib/common/concurrentqueue test_uplink_downlink.cpp ../src/common/StateField*.cpp ../src/common/bitstream.cpp ../src/fsw/FCCode/UplinkCo*.cpp ../src/fsw/FCCode/TimedControlTask.cpp ../src/common/debug_console.cpp -I ../lib/common/ArduinoJson/src -std=c++14 -DFUNCTIONAL_TEST -DDESKTOP
// convert n to bits
#define STREAM_SIZE 70
class TestFixture {
  public:
    StateFieldRegistryMock registry;

    std::unique_ptr<UplinkConsumer> uplink_consumer;

    std::shared_ptr<InternalStateField<size_t>> radio_mt_packet_len_fp;
    std::shared_ptr<InternalStateField<char*>> radio_mt_packet_fp;

    std::shared_ptr<WritableStateField<unsigned int>> cycle_no_fp;
    std::shared_ptr<WritableStateField<unsigned char>> adcs_state_fp;
    std::shared_ptr<WritableStateField<f_quat_t>> adcs_cmd_attitude_fp;
    std::shared_ptr<ReadableStateField<float>> adcs_ang_rate_fp;
    std::shared_ptr<WritableStateField<float>> adcs_min_stable_ang_rate_fp;
    std::shared_ptr<WritableStateField<unsigned char>> mission_mode_fp;
    std::shared_ptr<WritableStateField<unsigned char>> sat_designation_fp;

    // Test Helper function will map field names to indices
    std::map<std::string, size_t> field_map;
   
   // Make external fields
    char mt_buffer[350];
    
    // Create a TestFixture instance of QuakeManager with the following parameters
    TestFixture() : registry() {
        srand(1);
        // Create dummy fields
        cycle_no_fp = registry.create_writable_field<unsigned int>("pan.cycle_no");
        adcs_state_fp = registry.create_writable_field<unsigned char>("adcs.state", 10);
        adcs_cmd_attitude_fp = registry.create_writable_field<f_quat_t>("adcs.cmd_attitude");
        adcs_ang_rate_fp = registry.create_readable_field<float>("adcs.ang_rate", 0, 10, 4);
        adcs_min_stable_ang_rate_fp = registry.create_writable_field<float>("adcs.min_stable_ang_rate", 0, 10, 4);
        mission_mode_fp = registry.create_writable_field<unsigned char>("pan.state");
        sat_designation_fp = registry.create_writable_field<unsigned char>("pan.sat_designation"); // should be 6 writable fields --> 3 bits 

        radio_mt_packet_len_fp = registry.create_internal_field<size_t>("uplink.len");
        radio_mt_packet_fp = registry.create_internal_field<char*>("uplink.ptr");

        // Initialize internal fields
        uplink_consumer = std::make_unique<UplinkConsumer>(registry, 0);
        // uplink_producer = std::make_unique<UplinkProducer>(registry);

        radio_mt_packet_fp->set(mt_buffer);
        field_map = std::map<std::string, size_t>();

        // Setup field_map and assign some values
        for (size_t i = 0; i < registry.writable_fields.size(); ++i)
        {
            auto w = registry.writable_fields[i];
            field_map[w->name().c_str()] = i;
            w->name();
            // from_ull(w, rand()); // no seed so should be the same each time
        }
        uplink_consumer->init_uplink();

    }
};

int main()
{
  srand(0);
  char input[STREAM_SIZE] = {0};
  size_t length = read(STDIN_FILENO, input, STREAM_SIZE);

  // make an uplink packet from this random thing
  char up_packet[STREAM_SIZE] = {0};
  bitstream bs(up_packet, length);

  TestFixture tf;
  size_t max_index = tf.uplink_consumer->registry.writable_fields.size();

  // set mt buffer
  tf.radio_mt_packet_fp->set(input);
  // set mt length
  tf.radio_mt_packet_len_fp->set(length);
  // process mt packet
  tf.uplink_consumer->execute();
  // check that we got what we asked for

  return 0;
}