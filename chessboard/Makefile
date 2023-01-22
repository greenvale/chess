#EXECUTABLE MAKE FILE

PROG_NAME := a

SRC_DIR := .
BUILD_DIR := .
INCLUDE_DIR := .

EXT_INCLUDES := -I../. -I../dev-tools/.
EXT_LIBS :=

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

$(PROG_NAME): $(OBJS)
	g++ -o $@ $^ $(EXT_LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	g++ -c -o $@ $< $(EXT_INCLUDES)

clean: 
	rm *.o $(PROG_NAME) $(BUILD_DIR)/*.o
