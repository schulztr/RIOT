WOT_THING_MODELS = thing_model_1.json
WOT_THING_MODELS += thing_model_2.json

.PHONY: wot-config
wot-config: $(wildcard *.json)
	python3 $(RIOTBASE)/sys/net/wot/core/generate.py --appdir $(APPDIR) --board $(BOARD) --used_modules $(USEMODULE) --thing_models $(WOT_THING_MODELS)

