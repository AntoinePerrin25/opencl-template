CC := cc

ifeq ($(shell uname -s),Darwin)
OPENCL_LDLIBS := -framework OpenCL
else
OPENCL_LDLIBS := -lOpenCL
endif

.PHONY: all run clean

all: build/main

build/main: main.c kernel.cl
	@mkdir -p build
	$(CC) -std=c11 -Wall -Wextra -Wpedantic main.c $(OPENCL_LDLIBS) -o $@

run: all
	./build/main

clean:
	rm -rf build
