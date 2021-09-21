RIOTBOOT_SERIAL := $(RIOTTOOLS)/riotboot_serial/riotboot_serial

FLASHER   ?= $(RIOTBOOT_SERIAL)
FLASHFILE ?= $(HEXFILE)
PROG_BAUD ?= 115200
FFLAGS    ?= $(FLASHFILE) $(PORT) $(PROG_BAUD)

ROM_OFFSET ?= $(RIOTBOOT_LEN)

FLASHDEPS += $(RIOTBOOT_SERIAL)

$(RIOTBOOT_SERIAL):
	$(Q)env -u CC -u CFLAGS $(MAKE) -C $(RIOTTOOLS)/riotboot_serial
