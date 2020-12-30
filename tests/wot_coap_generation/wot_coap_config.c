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

extern ssize_t _echo_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx);
extern ssize_t _led_status_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx);
extern ssize_t _led_toggle_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx);

static ssize_t wot_echo_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    return _echo_handler(pdu, buf, len, ctx);
}

static ssize_t wot_led_status_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    return _led_status_handler(pdu, buf, len, ctx);
}

static ssize_t wot_led_toggle_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    return _led_toggle_handler(pdu, buf, len, ctx);
}

const coap_resource_t _wot_coap_resources[] = {
    {"/echo", COAP_GET, wot_echo_handler, NULL},
    {"/led/status", COAP_GET, wot_led_status_handler, NULL},
    {"/led/toggle", COAP_POST, wot_led_toggle_handler, NULL},
};

static const char *_wot_link_params[] = {
    NULL,
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

static gcoap_listener_t _wot_coap_listener = {
    &_wot_coap_resources[0],
    ARRAY_SIZE(_wot_coap_resources),
    _wot_encode_link,
    NULL,
    NULL,
};

wot_td_extension_t wot_td_echo_form_coap_0 = {0};

wot_td_uri_t wot_td_echo_aff_form_href_0 = {0};

wot_td_form_op_t wot_td_echo_form_0_op_0 = {
    .op_type = FORM_OP_READ_PROPERTY,
    .next = NULL,
};

wot_td_form_t wot_td_echo_aff_form_0 = {
    .op = &wot_td_echo_form_0_op_0,
    .href = &wot_td_echo_aff_form_href_0,
    .extensions = &wot_td_echo_form_coap_0,
    .next = NULL,
};

wot_td_int_affordance_t wot_echo_int_affordance = {
    .forms = &wot_td_echo_aff_form_0,
};

wot_td_object_required_t wot_echo_hello_required = {
    .value = "hello",
};

wot_td_data_schema_t wot_echo_hello_data_schema = {
    .json_type = JSON_TYPE_STRING,
    .read_only = true,
    .write_only = false,
};

wot_td_data_schema_map_t wot_echo_hello_data_map = {
    .key = "hello",
    .value = &wot_echo_hello_data_schema,
};

wot_td_object_schema_t wot_echo_data_schema_obj = {
    .properties = &wot_echo_hello_data_map,
    .required = &wot_echo_hello_required,
};

wot_td_data_schema_t wot_echo_data_schema = {
    .json_type = JSON_TYPE_OBJECT,
    .read_only = true,
    .write_only = false,
    .schema = &wot_echo_data_schema_obj,
};

wot_td_prop_affordance_t wot_echo_affordance = {
    .key = "echo",
    .int_affordance = &wot_echo_int_affordance,
    .data_schema = &wot_echo_data_schema,
    .next = NULL,
};

wot_td_coap_prop_affordance_t wot_coap_echo_affordance = {
    .coap_resource_t = &_wot_coap_resources[0],
    .affordance = &wot_echo_affordance,
};

wot_td_extension_t wot_td_status_form_coap_0 = {0};

wot_td_uri_t wot_td_status_aff_form_href_0 = {0};

wot_td_form_op_t wot_td_status_form_0_op_0 = {
    .op_type = FORM_OP_READ_PROPERTY,
    .next = NULL,
};

wot_td_form_t wot_td_status_aff_form_0 = {
    .op = &wot_td_status_form_0_op_0,
    .href = &wot_td_status_aff_form_href_0,
    .extensions = &wot_td_status_form_coap_0,
    .next = NULL,
};

wot_td_int_affordance_t wot_status_int_affordance = {
    .forms = &wot_td_status_aff_form_0,
};

wot_td_prop_affordance_t wot_status_affordance = {
    .key = "status",
    .int_affordance = &wot_status_int_affordance,
    .next = NULL,
};

wot_td_coap_prop_affordance_t wot_coap_status_affordance = {
    .coap_resource_t = &_wot_coap_resources[1],
    .affordance = &wot_status_affordance,
};

wot_td_extension_t wot_td_toggle_form_coap_0 = {0};

wot_td_uri_t wot_td_toggle_aff_form_href_0 = {0};

wot_td_form_op_t wot_td_toggle_form_0_op_0 = {
    .op_type = FORM_OP_INVOKE_ACTION,
    .next = NULL,
};

wot_td_form_t wot_td_toggle_aff_form_0 = {
    .op = &wot_td_toggle_form_0_op_0,
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
    .coap_resource_t = &_wot_coap_resources[2],
    .affordance = &wot_toggle_affordance,
};

int wot_td_coap_config_init(wot_td_thing_t *thing)
{
    gcoap_register_listener(&_wot_coap_listener);
    wot_td_coap_prop_add(thing, &wot_coap_echo_affordance);
    wot_td_coap_prop_add(thing, &wot_coap_status_affordance);
    wot_td_coap_action_add(thing, &wot_coap_toggle_affordance);
    return 0;
}
