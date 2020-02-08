Fuzz testing for telemetry

To protect SSD and CPU resources, fuzz inside Virtual machine!

1. install vagrant and virtualbox
  ( OSX: https://medium.com/@JohnFoderaro/macos-sierra-vagrant-quick-start-guide-2b8b78913be3)
2. while in THIS directory, 
  $ vagrant up
3. login to vagrant,
  $ vagrant ssh
4. Vagrantfile should have cloned FlightSoftware. You should be in ~/afl/FlightSoftware/fuzz_tests
5. Run make to make the targets
6. To start fuzzing
  $ afl-fuzz -i test_cases -o findings -- ./build/a.out
  The directory ./in should contain test cases and ./out is the afl-fuzz output
7. Vagrant will save the state of your virtual machine. To destroy it
  $ vagrant destroy

Right now, only fuzz_bitstream compiles. One could try to get fuzz_uplink to compile by changing
SRCS := $(shell find $(SRC_DIRS) -name *.cpp) $(BITSTREAM_SRCS) 
to
SRCS := $(shell find $(SRC_DIRS) -name *.cpp) $(UPLINK_SRCS)

See the QuickStart guide on afl-fuzz 
https://github.com/google/AFL/blob/master/docs/QuickStartGuide.txt
