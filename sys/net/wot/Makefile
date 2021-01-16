ifneq (,$(filter wot_core,$(USEMODULE)))
  DIRS += core
endif
ifneq (,$(filter wot_coap,$(USEMODULE)))
  DIRS += coap
endif

WOT_THING_MODELS = thing_model_1.json
WOT_THING_MODELS += thing_model_2.json

${APPDIR}/wot_config.c: $(wildcard *.json)
	python3 $(RIOTBASE)/sys/net/wot/core/generate.py --appdir $(APPDIR) --board $(BOARD) --used_modules $(USEMODULE) --thing_models $(WOT_THING_MODELS)

include $(RIOTBASE)/Makefile.base
