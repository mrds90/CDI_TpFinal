PROGRAM_PATH := $(CURDIR)/src
PROGRAM_PATH += $(CURDIR)/test/support
DOC_PATH := $(CURDIR)/Documentation
TESTDIR := $(CURDIR)/test
NAME := test
BUILDDIR := $(CURDIR)/build
CFLAGS := -Wall -g  # Add the -g option for debugging information
CC := gcc
DEFINES := -DTEST 

# Buscar todos los archivos .c y .h de forma recursiva en src y test
ALL_FILES := $(shell find $(PROGRAM_PATH) $(TESTDIR) -type f \( -name "*.c" -o -name "*.h" \))

# Buscar todos los archivos .c de forma recursiva en PROGRAM_PATH
# APPLST := $(shell find $(PROGRAM_PATH) -type f -name "*.c")
APPLST := $(shell find $(PROGRAM_PATH) -type d -name TEST_OSAL -prune -o -type f -name "*.c" -print)


# Buscar todos los archivos .h de forma recursiva en PROGRAM_PATH
INCDIR := $(shell find $(PROGRAM_PATH) -type d -name TEST_OSAL -prune -o -type f -name "*.h" -print)
INCDIR := $(sort $(dir $(INCDIR)))

all: $(BUILDDIR)
	$(CC) $(CFLAGS) $(DEFINES) $(addprefix -I,$(INCDIR)) $(APPLST) -o $(BUILDDIR)/$(NAME).o


uncrustify:
	uncrustify -c uncrustifyC.cfg --replace --no-backup -q --if-changed $(ALL_FILES)


clean:
	rm -r $(BUILDDIR)

clean_docs:
	rm -r $(DOC_PATH)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)
