# Makefile for Go wrapper

# Variables
CC = g++
CFLAGS = -fPIC
SWIG = swig
SWIG_FLAGS = -c++ -intgosize 64 -go
GO = go

# Default target
all: libmetaf.so explain metaf

# Generate wrapper with SWIG
metaf_wrap.cxx metaf_wrap.o: metaf.i
	$(SWIG) $(SWIG_FLAGS) $<
	$(CC) $(CFLAGS) -c metaf_wrap.cxx -o metaf_wrap.o

# Compile C++ wrapper
metaf_wrapper.o: metaf_wrapper.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Create shared library
libmetaf.so: metaf_wrapper.o metaf_wrap.o
	$(CC) -shared $^ -o $@

# Build the explain command
explain: libmetaf.so
	$(GO) build -o explain ./cmd/explain/main.go

# Build the metaf command
explain: libmetaf.so
	$(GO) build -o metaf ./cmd/metaf/main.go

# Clean target
clean:
	rm -f *.o *.so metaf_wrap.cxx explain metaf

# Phony targets
.PHONY: all clean
