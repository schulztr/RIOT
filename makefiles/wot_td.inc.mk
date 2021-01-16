.PHONY: wot-config
wot-config: $(wildcard *.json)
	python3 $(RIOTBASE)/sys/net/wot/core/generate.py --appdir $(APPDIR) --board $(BOARD) --used_modules $(USEMODULE) --thing_models $(WOT_THING_MODELS) --thing_instance_info $(WOT_INSTANCE_INFO) 
