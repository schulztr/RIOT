MODULE = wot_coap

WOT_TD_CONFIG = $(BINDIR)/coap_config.c
GENSRC += $(WOT_TD_CONFIG)

all: generate-wot-config

generate-wot-config:
	$(WOT_TD_GEN) --appdir $(APPDIR) --board $(BOARD) --used_modules $(USEMODULE) --thing_models $(WOT_THING_MODELS) --thing_instance_info $(WOT_INSTANCE_INFO) --output_path $(WOT_TD_CONFIG)

include $(RIOTMAKE)/tools/wot_td_coap.inc.mk
include $(RIOTBASE)/Makefile.base