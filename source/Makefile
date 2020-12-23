DEBUG ?= 1
CXXFLAGS_BASE = -c -g -pthread -fPIC -std=c++20 -Wall -Wno-unknown-pragmas -Wno-ignored-attributes -Wno-int-in-bool-context
BIN_DIR_BASE:=../../bin/
OUT_DIR_BASE := .obj/
ifeq ($(DEBUG), 1)
	OUT_DIR=$(OUT_DIR_BASE)debug
	CXXFLAGS= $(CXXFLAGS_BASE) -O0 -I$(OUT_DIR) -fsanitize=address -fno-omit-frame-pointer
	BIN_DIR=$(BIN_DIR_BASE)asan
else
	OUT_DIR=$(OUT_DIR_BASE)release
	CXXFLAGS= $(CXXFLAGS_BASE) -march=native -DNDEBUG -O3 -I$(OUT_DIR)
	BIN_DIR=$(BIN_DIR_BASE)release
endif

OUTPUT_FILE = libJde.Blockly.Custom.so
OUTPUT = $(BIN_DIR)/$(OUTPUT_FILE)
LIBS =

all: $(OUTPUT)
	$(NOECHO) $(NOOP)

OBJECTS = $(OUT_DIR)/TestClass

LDFLAGS = -pthread -shared -Wl,-z,origin -Wl,-rpath='$$ORIGIN'

$(OUTPUT): $(OUT_DIR)/stdafx.h.gch $(OBJECTS)
	$(CXX) -L$(BIN_DIR) $(LDFLAGS) $(OBJECTS) -o$(OUTPUT) $(LIBS)

$(OUT_DIR)/%.o: ./%.cpp ./%.h pc.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) ./$< -o$@ -c

clean:
	rm -rf -d $(OUT_DIR)/*.*
	rm -rf -d $(OUTPUT)
