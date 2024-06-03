PROGRAM_PATH := $(CURDIR)/identificacion_simulada
DOC_PATH := $(CURDIR)/Documentation
NAME := test
BUILDDIR := $(CURDIR)/build
CFLAGS := -Wall -g  # Add the -g option for debugging information
CC := gcc

# Buscar todos los archivos .c y .h de forma recursiva en src y test
ALL_FILES := $(shell find $(PROGRAM_PATH) -type f \( -name "*.c" -o -name "*.h" \))

# Buscar todos los archivos .c de forma recursiva en PROGRAM_PATH
APPLST := $(shell find $(PROGRAM_PATH) -type f -name "*.c")

# Buscar todos los archivos .h de forma recursiva en PROGRAM_PATH
INCDIR := $(shell find $(PROGRAM_PATH) -type f -name "*.h")
INCDIR := $(sort $(dir $(INCDIR)))

all: $(BUILDDIR)
	$(CC) $(CFLAGS) $(addprefix -I,$(INCDIR)) $(APPLST) -o $(BUILDDIR)/$(NAME).o


uncrustify:
	uncrustify -c uncrustifyC.cfg --replace --no-backup -q --if-changed $(ALL_FILES)


clean:
	rm -r $(BUILDDIR)

clean_docs:
	rm -r $(DOC_PATH)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)
