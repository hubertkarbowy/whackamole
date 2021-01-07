SHELL := bash
SRC_DIR := src
SRCTEST_DIR := test
OBJ_DIR := obj
BIN_DIR := bin
DUINO_TMP_DIR := ./duino_tmp
DUINO_TMP_CACHE := ./duino_tmp_cache

EXE := ${BIN_DIR}/whack
TEST_EXE := ${BIN_DIR}/test_whack

SRC := $(wildcard $(SRC_DIR)/*.cpp) 
SRC := $(filter-out $(SRC_DIR)/mainwhack.cpp, $(SRC))
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
MAIN_SRC := $(SRC_DIR)/mainwhack.cpp
MAIN_OBJ := $(OBJ_DIR)/mainwhack.o
TEST_SRC := $(wildcard $(SRCTEST_DIR)/*.cpp) 
TEST_OBJ := $(TEST_SRC:$(SRCTEST_DIR)/%.cpp=$(OBJ_DIR)/%.o)

############ PC compiler (applies only to x86 target - for Arduino we use its own build system)  #########
CPPFLAGS := -Iinclude -I. -MMD -MP -DCOMPILE_FOR_PC
CFLAGS   := -Wall
LDFLAGS  := -Llib
LDLIBS   := -lm -lpthread
LDLIBS_CPPUNIT := -lcppunit

# The -MMD -MP flags are used to generate the header dependencies automatically.
# We will use this later on to trigger a compilation when only a header changes.

.PHONY: all duino clean test

all: ${EXE}

test: ${TEST_EXE}

duino: $(DUINO_TMP_DIR) $(DUINO_TMP_CACHE)
	./duino_check.sh
	${ARDUINO_HOME}/arduino-builder -compile -logger=machine \
	-hardware ${ARDUINO_HOME}/hardware -hardware ${HOME}/.arduino15/packages \
	-tools ${ARDUINO_HOME}/tools-builder \
	-tools ${ARDUINO_HOME}/hardware/tools/avr \
	-tools ${HOME}/.arduino15/packages \
	-built-in-libraries ${ARDUINO_HOME}/libraries \
	-libraries ${HOME}/Arduino/libraries \
	-fqbn=arduino:mbed:nano33ble \
	-vid-pid=2341_805A -ide-version=10813 \
	-build-path ./duino_tmp/ -warnings=none \
	-build-cache ./duino_tmp_cache/ \
	-prefs=build.warn_data_percentage=75 \
	-prefs=runtime.tools.openocd.path=${HOME}/.arduino15/packages/arduino/tools/openocd/0.10.0-arduino13 \
	-prefs=runtime.tools.openocd-0.10.0-arduino13.path=${HOME}/.arduino15/packages/arduino/tools/openocd/0.10.0-arduino13 \
	-prefs=runtime.tools.dfu-util.path=${HOME}/.arduino15/packages/arduino/tools/dfu-util/0.9.0-arduino2 \
	-prefs=runtime.tools.dfu-util-0.9.0-arduino2.path=${HOME}/.arduino15/packages/arduino/tools/dfu-util/0.9.0-arduino2 \
	-prefs=runtime.tools.arm-none-eabi-gcc.path=${HOME}/.arduino15/packages/arduino/tools/arm-none-eabi-gcc/7-2017q4 \
	-prefs=runtime.tools.arm-none-eabi-gcc-7-2017q4.path=${HOME}/.arduino15/packages/arduino/tools/arm-none-eabi-gcc/7-2017q4 \
	-prefs=runtime.tools.bossac.path=${HOME}/.arduino15/packages/arduino/tools/bossac/1.9.1-arduino2 \
	-prefs=runtime.tools.bossac-1.9.1-arduino2.path=${HOME}/.arduino15/packages/arduino/tools/bossac/1.9.1-arduino2 \
	-verbose ./arduino_main.ino
	@echo ""
	@echo ""
	@echo "*********************************************************************************"
	@echo "* Completed compilation with arduino-build.                                     *"
	@echo "*********************************************************************************"
	@echo ""
	@echo "Now you can flash the binary to your board using this command:"
	@echo "sudo ${HOME}/.arduino15/packages/arduino/tools/bossac/1.9.1-arduino2/bossac -d --port=ttyACM0 -U -i -a -w ./duino_tmp/arduino_main.ino.bin -R"

${EXE}: ${OBJ} ${MAIN_OBJ} | $(BIN_DIR) # Note that $BIN_DIR might not exist yet - let's tell make that you want it to check for that first
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRCTEST_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR) $(DUINO_TMP_DIR) $(DUINO_TMP_CACHE):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR) $(DUINO_TMP_DIR) $(DUINO_TMP_CACHE)

${TEST_EXE}: ${OBJ} $(TEST_OBJ) | $(OBJ_DIR) $(BIN_DIR)
	$(info Running tests now...)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(CPPFLAGS) $^ $(LDLIBS) $(LDLIBS_CPPUNIT) -o $@
	${BIN_DIR}/test_whack
