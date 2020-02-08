CXX = afl-clang++
TARGET_EXEC ?= a.out

BUILD_DIR ?= ./build

BITSTREAM_SRCS = ../src/common/bitstream.cpp 
UPLINK_SRCS =  ../src/common/bitstream.cpp ../src/common/StateField*.cpp ../src/fsw/FCCode/UplinkCo*.cpp ../src/fsw/FCCode/TimedControlTask.cpp ../src/common/debug_console.cpp

# fuzzing bitstream
SRC_DIRS ?= ./fuzz_bitstream ../src/common ../src/fsw/FCCode ../src/gsw ../lib/common/psim/lin/include ../src ../lib/common/libsbp/src ../lib/common/libsbp
SRCS := $(shell find $(SRC_DIRS) -name *.cpp) $(BITSTREAM_SRCS) $(UPLINK_SRCS)
EXCLUDES := ../src/fsw/targets ../src/adcs ../lib/common/psim/test ../lib/common/psim/geograv ../lib/common/psim/MATLAB ../lib/common/ArduinoJson
SRCS := $(filter-out $(shell find $(EXCLUDES) -name *.cpp ), $(SRCS))
#fuzzing uplink
# SRC_DIRS ?= ./fuzz_uplink
# SRCS := $(shell find $(SRC_DIRS) -name *.cpp) $(UPLINK_SRCS)

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d) ../src/gsw ../lib/fsw/json ../lib/common/concurrentqueue ../lib/common/libsbp/include ../lib/common/psim/include ../src/fsw/FCCode ../lib/fsw/memuse
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -DDESKTOP -DFUNCTIONAL_TEST -DPAN_LEADER -DGSW -DUNIT_TEST -std=c++14 

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	AFL_HARDEN=4 $(CXX) $(OBJS) -o $@ $(LDFLAGS)

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	AFL_HARDEN=4 $(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR) ./src

-include $(DEPS)

MKDIR_P ?= mkdir -p
