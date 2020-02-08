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
  $ make
6. Run as sudo
  $ sudo su
6. Create your input and output folders
  $ mkdir -p findings test_cases
7. Add a test case to the test_cases folder
  $ echo "a" > ./test_cases/a
8. To start fuzzing
  $ afl-fuzz -i test_cases -o findings -- ./build/a.out
  The directory ./in should contain test cases and ./out is the afl-fuzz output
9. Vagrant will save the state of your virtual machine. To destroy it
  $ vagrant destroy

Right now, only fuzz_bitstream compiles. One could try to get fuzz_uplink to compile by changing

See the QuickStart guide on afl-fuzz 
https://github.com/google/AFL/blob/master/docs/QuickStartGuide.txt
