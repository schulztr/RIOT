MODULE = wot_core

all:
	$(WOT_TD_GEN) --appdir $(APPDIR) --board $(BOARD) --used_modules $(USEMODULE) --thing_models $(WOT_THING_MODELS) --thing_instance_info $(WOT_INSTANCE_INFO)

include $(RIOTMAKE)/tools/wot_td.inc.mk
include $(RIOTBASE)/Makefile.base