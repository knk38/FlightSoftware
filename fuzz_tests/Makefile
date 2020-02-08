CXX = afl-g++
TARGET_EXEC ?= a.out

BUILD_DIR ?= ./build
SRC_DIRS ?= ./fuzz_bitstream

BITSTREAM_SRCS = ../src/common/bitstream.cpp
UPLINK_SRCS =  ../src/common/bitstream.cpp ../../src/common/StateField*.cpp ../../src/fsw/FCCode/UplinkCo*.cpp ../../src/fsw/FCCode/TimedControlTask.cpp ../../src/common/debug_console.cpp

SRCS := $(shell find $(SRC_DIRS) -name *.cpp) $(BITSTREAM_SRCS)

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -DDESKTOP -std=c++14

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