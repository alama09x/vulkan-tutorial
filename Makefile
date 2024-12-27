.PHONY: test clean

CFLAGS = -O2
LDFLAGS = -Iinclude -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
BINARY = main

test_macos: build_macos
	./bin/$(BINARY)

test_linux: build_linux
	./bin/$(BINARY)

build_macos: shaders
	clang -rpath ~/VulkanSDK/1.3.296.0/macOS/lib \
		$(CFLAGS) -o ./bin/$(BINARY) $(wildcard ./src/*.c) \
		$(wildcard ./src/init/*.c) $(LDFLAGS)

build_linux: shaders
	clang $(CFLAGS) -o ./bin/$(BINARY) $(wildcard ./src/*.c) \
		$(wildcard ./src/init/*.c) $(LDFLAGS)

shaders:
	glslc ./shaders/shader.vert -o ./bin/vert.spv
	glslc ./shaders/shader.frag -o ./bin/frag.spv

clean:
	rm -f ./bin/$(BINARY)
