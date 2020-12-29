#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "net/gcoap.h"
#include "od.h"
#include "fmt.h"
#include "net/wot.h"
#include "net/wot/coap.h"
#include "net/wot/coap/config.h"
#include "led_controller.h"

extern ssize_t _led_toggle_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx);

static ssize_t wot_led_brightness_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    return _led_brightness_handler(&pdu, &buf, len, &ctx);
}

static ssize_t wot_led_toggle_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    return _led_toggle_handler(&pdu, &buf, len, &ctx);
}

const coap_resource_t _wot_coap_resources[] = {
    {"/led/brightness", COAP_GET | COAP_POST, wot_led_brightness_handler, NULL},
    {"/led/toggle", COAP_POST, wot_led_toggle_handler, NULL},
};

static const char *_wot_link_params[] = {
    NULL,
    NULL,
}

static gcoap_listener_t _wot_coap_listener = {
    &_wot_coap_resources[0],
    ARRAY_SIZE(_wot_coap_resources),
    _wot_encode_link,
    NULL,
    NULL,
};

static ssize_t _wot_encode_link(const coap_resource_t *resource, char *buf,
                                size_t maxlen, coap_link_encoder_ctx_t *context)
{
    ssize_t res = gcoap_encode_link(resource, buf, maxlen, context);
    if (res > 0)
    {
        if (_wot_link_params[context->link_pos] && (strlen(_wot_link_params[context->link_pos]) < (maxlen - res)))
        {
            if (buf)
            {
                memcpy(buf + res, _wot_link_params[context->link_pos],
                       strlen(_wot_link_params[context->link_pos]));
            }
            return res + strlen(_wot_link_params[context->link_pos]);
        }
    }

    return res;
}

wot_td_form_op_t wot_td_brightness_form_1_op_0 = {
    .op_type = FORM_OP_WRITE_PROPERTY,
    .next = NULL,
};

wot_td_form_t wot_td_brightness_aff_form_1 = {
    .op = &wot_td_brightness_form_1_op_0,
    .content_type = &wot_td_brightness_content_type_1,
    .href = &wot_td_brightness_aff_form_href_1,
    .extensions = &wot_td_brightness_form_coap_1,
    .next = NULL,
};

wot_td_form_op_t wot_td_brightness_form_0_op_0 = {
    .op_type = FORM_OP_READ_PROPERTY,
    .next = NULL,
};

wot_td_form_t wot_td_brightness_aff_form_0 = {
    .op = &wot_td_brightness_form_0_op_0,
    .content_type = &wot_td_brightness_content_type_0,
    .href = &wot_td_brightness_aff_form_href_0,
    .extensions = &wot_td_brightness_form_coap_0,
    .next = &wot_td_brightness_aff_form_1,
};

wot_td_int_affordance_t wot_brightness_int_affordance = {
    .forms = &wot_td_brightness_aff_form_0,
};

wot_td_prop_affordance_t wot_brightness_affordance = {
    .key = "brightness",
    .int_affordance = &wot_brightness_int_affordance,
    .next = NULL,
};

wot_td_coap_prop_affordance_t wot_coap_brightness_affordance = {
    .affordance = &wot_brightness_affordance,
};

wot_td_form_t wot_td_toggle_aff_form_0 = {
    .content_type = &wot_td_toggle_content_type_0,
    .href = &wot_td_toggle_aff_form_href_0,
    .extensions = &wot_td_toggle_form_coap_0,
    .next = NULL,
};

wot_td_int_affordance_t wot_toggle_int_affordance = {
    .forms = &wot_td_toggle_aff_form_0,
};

wot_td_action_affordance_t wot_toggle_affordance = {
    .key = "toggle",
    .int_affordance = &wot_toggle_int_affordance,
    .next = NULL,
};

wot_td_coap_action_affordance_t wot_coap_toggle_affordance = {
    .affordance = &wot_toggle_affordance,
};

int wot_td_coap_config_init(wot_td_thing_t *thing)
{
    gcoap_register_listener(&_wot_coap_listener);
    wot_td_coap_prop_add(thing, &wot_coap_brightness_affordance);
    wot_td_coap_action_add(thing, &wot_coap_toggle_affordance);
    return 0;
}
