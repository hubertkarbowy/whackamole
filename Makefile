SRC_DIR := src
SRCTEST_DIR := test
OBJ_DIR := obj
BIN_DIR := bin

EXE := ${BIN_DIR}/whack
TEST_EXE := ${BIN_DIR}/test_whack

SRC := $(wildcard $(SRC_DIR)/*.cpp) 
SRC := $(filter-out $(SRC_DIR)/mainwhack.cpp, $(SRC))
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
MAIN_SRC := $(SRC_DIR)/mainwhack.cpp
MAIN_OBJ := $(OBJ_DIR)/mainwhack.o
TEST_SRC := $(wildcard $(SRCTEST_DIR)/*.cpp) 
TEST_OBJ := $(TEST_SRC:$(SRCTEST_DIR)/%.cpp=$(OBJ_DIR)/%.o)

############ compiler #########
CPPFLAGS := -Iinclude -I. -MMD -MP
CFLAGS   := -Wall
LDFLAGS  := -Llib
LDLIBS   := -lm
LDLIBS_CPPUNIT := -lcppunit

# The -MMD -MP flags are used to generate the header dependencies automatically.
# We will use this later on to trigger a compilation when only a header changes.

.PHONY: all clean test

all: ${EXE}

test: ${TEST_EXE}

${EXE}: ${OBJ} ${MAIN_OBJ} | $(BIN_DIR) # Note that $BIN_DIR might not exist yet - let's tell make that you want it to check for that first
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRCTEST_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

${TEST_EXE}: ${OBJ} $(TEST_OBJ) | $(OBJ_DIR) $(BIN_DIR)
	$(info Running tests now...)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(CPPFLAGS) $^ $(LDLIBS) $(LDLIBS_CPPUNIT) -o $@
	${BIN_DIR}/test_whack
