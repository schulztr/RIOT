ifneq (,$(filter wot_core,$(USEMODULE)))
  DIRS += core
endif
ifneq (,$(filter wot_coap,$(USEMODULE)))
  DIRS += coap
endif

${APPDIR}/wot_config.c: $(wildcard *.json)
	python3 $(RIOTBASE)/sys/net/wot/core/generate.py --appdir $(APPDIR) --board $(BOARD) --security=no

include $(RIOTBASE)/Makefile.base
