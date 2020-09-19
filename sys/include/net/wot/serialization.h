#ifndef WOT_SERIALIZATION_H
#define WOT_SERIALIZATION_H

typedef void (*wot_td_serialize_receiver_t)(const char *, uint32_t);

int wot_td_serialize_thing(wot_td_serialize_receiver_t receiver, wot_td_thing_t *thing);

#endif //WOT_SERIALIZATION_H
