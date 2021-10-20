#ifndef WOT_SERIALIZATION_H
#define WOT_SERIALIZATION_H

typedef struct {
    uint16_t start;
    uint16_t end;
    uint16_t cur;
} wot_td_ser_slicer_t;

typedef void (*wot_td_serialize_receiver_t)(const char *c);

int wot_td_serialize_thing(
        wot_td_serialize_receiver_t receiver, wot_td_thing_t *thing,
        wot_td_ser_slicer_t *slicer);

#endif //WOT_SERIALIZATION_H
