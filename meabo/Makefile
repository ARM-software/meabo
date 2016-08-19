# To enable/disable PAPI, please set the #define ENABLE_PAPI appropriately in meabo.h
PAPI_INC_PATH =  				# Please set this appropriately 
PAPI_LIB_PATH = 
PAPI_LIB = # -lpapi

OMP_INC = -fopenmp
OMP_LIB = -lgomp

HOST=$(shell hostname)

CC = gcc

CFLAGS = $(PAPI_INC_PATH) $(OMP_INC) -std=c99  -O3 -ffast-math -funroll-loops -ftree-vectorize -fopenmp -Wall -fmessage-length=0 -frecord-gcc-switches -MMD  
LDFLAGS = $(PAPI_LIB_PATH) $(PFM_LIB_PATH)
LIBS = $(OMP_LIB) $(PAPI_LIB) $(PFM_LIB) -lm -lrt

SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)

MEABO_EXE=meabo.default

ARCH?=

ifneq ($(ARCH),)
	include Makefile.$(ARCH)
endif

all: $(MEABO_EXE)
	@echo $(HOST)
	@echo $(SOURCES)

-include *.d

%.o: %.c
	@echo 'Now buildling: $<'
	@echo $(HOST)
	$(CC) $(CFLAGS) -MF .$*.d -c $< -o $@
	@echo 'Done building: $<'
	@echo ' '

$(MEABO_EXE): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(MEABO_EXE) $(OBJECTS) $(LIBS)

clean:
	rm -f *.o $(MEABO_EXE)

