#EXECUTABLE MAKE FILE

PROG_NAME := a

SRC_DIR := ./src
BUILD_DIR := ./build
INCLUDE_DIR := ./include

EXT_INCLUDES := -I/usr/local/lib/wx/include/gtk3-unicode-3.2 -I/usr/local/include/wx-3.2 -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__ -pthread
EXT_LIBS := -lwx_baseu-3.2 -lwx_gtk3u_core-3.2

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

$(PROG_NAME): $(OBJS) $(BUILD_DIR)/chessboard.o
	g++ -o $@ $^ $(EXT_LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	g++ -c -o $@ $< $(EXT_INCLUDES)

$(BUILD_DIR)/chessboard.o: ../chessboard/chessboard.cpp
	g++ -c -o ./build/chessboard.o ../chessboard/chessboard.cpp

clean: 
	rm *.o $(PROG_NAME) $(BUILD_DIR)/*.o
