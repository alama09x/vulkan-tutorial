UNAME_S = $(shell uname -s)

CC = clang
GLSLC = glslc

CFLAGS = -std=c2x -O3 -g -Wall -Wextra -Isrc
LDFLAGS = -lglfw -lvulkan -lX11 -lXxf86vm -lXrandr -lXi

SRC = $(wildcard src/*.c) $(wildcard src/**/*.c) \
	  $(wildcard src/**/**/*.c) $(wildcard src/**/**/**/*.c)
BIN = bin
BINARY = main

ifeq ($(UNAME_S), Darwin)
	LDFLAGS += -rpath ${VULKAN_SDK}/lib
endif

ifeq ($(UNAME_S), Linux)
	LDFLAGS += -ldl -lpthread
endif

.PHONY: all clean

all: dirs shaders build run

dirs:
	mkdir -p ./$(BIN)

build:
	$(CC) $(CFLAGS) -o ./$(BIN)/$(BINARY) $(SRC) $(LDFLAGS)

shaders:
	$(GLSLC) ./shaders/shader.vert -o ./bin/vert.spv
	$(GLSLC) ./shaders/shader.frag -o ./bin/frag.spv

run:
	./$(BIN)/$(BINARY)

clean:
	rm -f ./$(BIN)
