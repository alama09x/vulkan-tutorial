.PHONY: test clean

CFLAGS = -O2
LDFLAGS = -Iinclude -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
BINARY = main

test: build
	./bin/$(BINARY)

build:
	clang $(CFLAGS) -o ./bin/$(BINARY) $(wildcard ./src/*.c) $(wildcard ./src/init/*.c) $(LDFLAGS)

clean:
	rm -f ./bin/$(BINARY)
