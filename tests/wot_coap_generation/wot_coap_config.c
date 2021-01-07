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

wot_td_type_t wot_td_echo_uri_variable_d_data_map_data_schema_type_0 = {
    .value = "eg:Direction",
    .next = NULL,
};

wot_td_data_schema_t wot_td_echo_uri_variable_d_data_map_data_schema = {
    .type = &wot_td_echo_uri_variable_d_data_map_data_schema_type_0,
    .json_type = JSON_TYPE_INTEGER,
};

wot_td_data_schema_map_t wot_td_echo_uri_variable_d_data_map = {
    .key = "d",
    .value = &wot_td_echo_uri_variable_d_data_map_data_schema,
    .next = NULL,
};

wot_td_type_t wot_td_echo_uri_variable_p_data_map_data_schema_type_0 = {
    .value = "eg:SomeKindOfAngle",
    .next = NULL,
};

wot_td_data_schema_t wot_td_echo_uri_variable_p_data_map_data_schema = {
    .type = &wot_td_echo_uri_variable_p_data_map_data_schema_type_0,
    .json_type = JSON_TYPE_INTEGER,
};

wot_td_data_schema_map_t wot_td_echo_uri_variable_p_data_map = {
    .key = "p",
    .value = &wot_td_echo_uri_variable_p_data_map_data_schema,
    .next = &wot_td_echo_uri_variable_d_data_map,
};

wot_td_multi_lang_t wot_td_echo_int_affordance_title_1 = {
    .tag = "en",
    .value = "English Title",
    .next = NULL,
};

wot_td_multi_lang_t wot_td_echo_int_affordance_title_0 = {
    .tag = "de",
    .value = "Deutscher Titel",
    .next = &wot_td_echo_int_affordance_title_1,
};

wot_td_multi_lang_t wot_td_echo_int_affordance_description_1 = {
    .tag = "en",
    .value = "English description",
    .next = NULL,
};

wot_td_multi_lang_t wot_td_echo_int_affordance_description_0 = {
    .tag = "de",
    .value = "Deutsche Beschreibung",
    .next = &wot_td_echo_int_affordance_description_1,
};

wot_td_type_t wot_td_echo_int_affordance_type_0 = {
    .value = "Type",
    .next = NULL,
};

wot_td_int_affordance_t wot_td_echo_int_affordance = {
    .type = &wot_td_echo_int_affordance_type_0,
    .descriptions = &wot_td_echo_int_affordance_description_0,
    .titles = &wot_td_echo_int_affordance_title_0,
    .uri_variables = &wot_td_echo_uri_variable_p_data_map,
    .forms = &wot_td_echo_aff_form_0,
};

wot_td_object_required_t wot_td_echo_affordance_data_schema_hello_required = {
    .value = "hello",
};

wot_td_data_schema_t wot_td_echo_affordance_data_schema_hello_data_map_data_schema = {
    .json_type = JSON_TYPE_STRING,
    .read_only = true,
    .write_only = false,
};

wot_td_data_schema_map_t wot_td_echo_affordance_data_schema_hello_data_map = {
    .key = "hello",
    .value = &wot_td_echo_affordance_data_schema_hello_data_map_data_schema,
    .next = NULL,
};

wot_td_object_schema_t wot_td_echo_affordance_data_schema_object = {
    .properties = &wot_td_echo_affordance_data_schema_hello_data_map,
    .required = &wot_td_echo_affordance_data_schema_hello_required,
};

wot_td_type_t wot_td_echo_affordance_data_schema_type_0 = {
    .value = "Type",
    .next = NULL,
};

wot_td_data_schema_t wot_td_echo_affordance_data_schema = {
    .type = &wot_td_echo_affordance_data_schema_type_0,
    .json_type = JSON_TYPE_OBJECT,
    .read_only = true,
    .write_only = false,
    .schema = &wot_td_echo_affordance_data_schema_object,
};

wot_td_prop_affordance_t wot_td_echo_affordance = {
    .key = "echo",
    .int_affordance = &wot_td_echo_int_affordance,
    .data_schema = &wot_td_echo_affordance_data_schema,
    .next = NULL,
};

wot_td_coap_prop_affordance_t wot_coap_echo_affordance = {
    .coap_resource = &_wot_coap_resources[0],
    .affordance = &wot_td_echo_affordance,
    .form = &wot_td_echo_aff_form_0,
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

wot_td_int_affordance_t wot_td_status_int_affordance = {
    .forms = &wot_td_status_aff_form_0,
};

wot_td_prop_affordance_t wot_td_status_affordance = {
    .key = "status",
    .int_affordance = &wot_td_status_int_affordance,
    .next = NULL,
};

wot_td_coap_prop_affordance_t wot_coap_status_affordance = {
    .coap_resource = &_wot_coap_resources[1],
    .affordance = &wot_td_status_affordance,
    .form = &wot_td_status_aff_form_0,
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

wot_td_multi_lang_t wot_td_toggle_int_affordance_title_1 = {
    .tag = "en",
    .value = "English title",
    .next = NULL,
};

wot_td_multi_lang_t wot_td_toggle_int_affordance_title_0 = {
    .tag = "de",
    .value = "Deutscher Titel",
    .next = &wot_td_toggle_int_affordance_title_1,
};

wot_td_multi_lang_t wot_td_toggle_int_affordance_description_1 = {
    .tag = "en",
    .value = "English description",
    .next = NULL,
};

wot_td_multi_lang_t wot_td_toggle_int_affordance_description_0 = {
    .tag = "de",
    .value = "Deutsche Beschreibung",
    .next = &wot_td_toggle_int_affordance_description_1,
};

wot_td_int_affordance_t wot_td_toggle_int_affordance = {
    .descriptions = &wot_td_toggle_int_affordance_description_0,
    .titles = &wot_td_toggle_int_affordance_title_0,
    .forms = &wot_td_toggle_aff_form_0,
};

wot_td_action_affordance_t wot_td_toggle_affordance = {
    .key = "toggle",
    .int_affordance = &wot_td_toggle_int_affordance,
    .next = NULL,
};

wot_td_coap_action_affordance_t wot_coap_toggle_affordance = {
    .coap_resource = &_wot_coap_resources[2],
    .affordance = &wot_td_toggle_affordance,
    .form = &wot_td_toggle_aff_form_0,
};

int wot_td_coap_config_init(wot_td_thing_t *thing)
{
    gcoap_register_listener(&_wot_coap_listener);
    wot_td_coap_prop_add(thing, &wot_coap_echo_affordance);
    wot_td_coap_prop_add(thing, &wot_coap_status_affordance);
    wot_td_coap_action_add(thing, &wot_coap_toggle_affordance);
    return 0;
}
