#include "net/wot/wot_config.h"

json_ld_context_t wot_td_some_context = {
        .key = "some_key1",
        .value = "some_value1",
};

wot_td_type_t wot_td_saref_type = {
        .value = "saref:LightSwitch"
};

int wot_td_config_init(wot_td_thing_t *thing){
    wot_td_thing_context_add(thing, &wot_td_some_context);
    wot_td_thing_type_add(thing, &wot_td_saref_type);
}
