MODNAME		?= kopycat

obj-m		+= $(MODNAME).o
$(MODNAME)-y	+= main.o

################################################################################

PAYLOAD_CFLAGS := $(KBUILD_CPPFLAGS) $(KBUILD_CFLAGS)
PAYLOAD_CFLAGS := $(filter-out -pg, $(PAYLOAD_CFLAGS))
PAYLOAD_CFLAGS := $(filter-out -mfentry, $(PAYLOAD_CFLAGS))
PAYLOAD_CFLAGS := $(filter-out -mcmodel=%, $(PAYLOAD_CFLAGS))
PAYLOAD_CFLAGS := $(filter-out -mindirect-branch=%, $(PAYLOAD_CFLAGS))
PAYLOAD_CFLAGS := $(PAYLOAD_CFLAGS) $(LINUXINCLUDE) -std=gnu99 -fno-stack-protector # add more flags

$(src)/main.o: payload
payload: FORCE
	$(CC) $(PAYLOAD_CFLAGS) -fpie -nostdlib -nostartfiles -nodefaultlibs -Wl,--script=$(src)/payload.lds -Wl,--no-dynamic-linker $(src)/payload.c -o $(src)/payload
	$(SHELL) $(src)/tools/dumper.sh $(src)/payload >$(src)/payload.inc
clean-files += payload payload.inc
FORCE:
