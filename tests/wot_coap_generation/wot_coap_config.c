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

wot_td_object_required_t wot_td_echo_prop_aff_data_schema_data_schema_hello_required = {
    .value = "hello",
};

wot_td_data_enums_t wot_td_echo_prop_aff_data_schema_data_schema_hello_data_schema_enum_2 = {
    .value = "Error",
    .next = NULL,
};

wot_td_data_enums_t wot_td_echo_prop_aff_data_schema_data_schema_hello_data_schema_enum_1 = {
    .value = "Off",
    .next = &wot_td_echo_prop_aff_data_schema_data_schema_hello_data_schema_enum_2,
};

wot_td_data_enums_t wot_td_echo_prop_aff_data_schema_data_schema_hello_data_schema_enum_0 = {
    .value = "On",
    .next = &wot_td_echo_prop_aff_data_schema_data_schema_hello_data_schema_enum_1,
};

wot_td_data_schema_t wot_td_echo_prop_aff_data_schema_data_schema_hello_data_schema = {
    .constant = "world",
    .format = "email",
    .enumeration = &wot_td_echo_prop_aff_data_schema_data_schema_hello_data_schema_enum_0,
    .json_type = JSON_TYPE_STRING,
    .read_only = true,
    .write_only = false,
};

wot_td_data_schema_map_t wot_td_echo_prop_aff_data_schema_data_schema_hello_data_map = {
    .key = "hello",
    .value = &wot_td_echo_prop_aff_data_schema_data_schema_hello_data_schema,
    .next = NULL,
};

wot_td_object_schema_t wot_td_echo_prop_aff_data_schema_data_schema_object = {
    .properties = &wot_td_echo_prop_aff_data_schema_data_schema_hello_data_map,
    .required = &wot_td_echo_prop_aff_data_schema_data_schema_hello_required,
};

wot_td_type_t wot_td_echo_prop_aff_data_schema_data_schema_one_of_0_schema_type_0 = {
    .value = "test",
    .next = NULL,
};

wot_td_data_schema_t wot_td_echo_prop_aff_data_schema_data_schema_one_of_0_schema = {
    .type = &wot_td_echo_prop_aff_data_schema_data_schema_one_of_0_schema_type_0,
};

wot_td_data_schemas_t wot_td_echo_prop_aff_data_schema_data_schema_one_of_0 = {
    .value = &wot_td_echo_prop_aff_data_schema_data_schema_one_of_0_schema,
    .next = NULL,
};

wot_td_multi_lang_t wot_td_echo_prop_aff_data_schema_data_schema_title_1 = {
    .tag = "en",
    .value = "English Title",
    .next = NULL,
};

wot_td_multi_lang_t wot_td_echo_prop_aff_data_schema_data_schema_title_0 = {
    .tag = "de",
    .value = "Deutscher Titel",
    .next = &wot_td_echo_prop_aff_data_schema_data_schema_title_1,
};

wot_td_multi_lang_t wot_td_echo_prop_aff_data_schema_data_schema_description_1 = {
    .tag = "en",
    .value = "English description",
    .next = NULL,
};

wot_td_multi_lang_t wot_td_echo_prop_aff_data_schema_data_schema_description_0 = {
    .tag = "de",
    .value = "Deutsche Beschreibung",
    .next = &wot_td_echo_prop_aff_data_schema_data_schema_description_1,
};

wot_td_type_t wot_td_echo_prop_aff_data_schema_data_schema_type_0 = {
    .value = "Type",
    .next = NULL,
};

wot_td_data_schema_t wot_td_echo_prop_aff_data_schema_data_schema = {
    .type = &wot_td_echo_prop_aff_data_schema_data_schema_type_0,
    .descriptions = &wot_td_echo_prop_aff_data_schema_data_schema_description_0,
    .titles = &wot_td_echo_prop_aff_data_schema_data_schema_title_0,
    .one_of = &wot_td_echo_prop_aff_data_schema_data_schema_one_of_0,
    .json_type = JSON_TYPE_OBJECT,
    .schema = &wot_td_echo_prop_aff_data_schema_data_schema_object,
    .read_only = true,
    .write_only = false,
};

wot_td_extension_t wot_td_echo_prop_aff_int_form_0_extension = {0};

wot_td_media_type_parameter_t wot_td_echo_prop_aff_int_form_0_response_content_type_parameter_0 = {
    .key = "charset",
    .value = "iso-8859-1",
    .next = NULL,
};

wot_td_content_type_t wot_td_echo_prop_aff_int_form_0_response_content_type = {
    .media_type = CONTENT_TYPE_JSON,
    .media_type_parameter = &wot_td_echo_prop_aff_int_form_0_response_content_type_parameter_0,
};

wot_td_expected_res_t wot_td_echo_prop_aff_int_form_0_response = {
    .content_type = &wot_td_echo_prop_aff_int_form_0_response_content_type,
};

wot_td_auth_scopes_t wot_td_echo_prop_aff_int_form_0_scope_0 = {
    .value = "limited",
    .next = NULL,
};

wot_td_security_t wot_td_echo_prop_aff_int_form_0_security_basic = {
    .key = "basic",
    .value = &wot_td_security_schema_basic,
    .next = NULL,
};

wot_td_media_type_parameter_t wot_td_echo_prop_aff_int_form_0_content_type_parameter_0 = {
    .key = "charset",
    .value = "iso-8859-1",
    .next = NULL,
};

wot_td_content_type_t wot_td_echo_prop_aff_int_form_0_content_type = {
    .media_type = CONTENT_TYPE_JSON,
    .media_type_parameter = &wot_td_echo_prop_aff_int_form_0_content_type_parameter_0,
};

wot_td_uri_t wot_td_echo_prop_aff_int_form_0_href = {0};

wot_td_form_op_t wot_td_echo_prop_aff_int_form_0_op_0 = {
    .op_type = FORM_OP_READ_PROPERTY,
    .next = NULL,
};

wot_td_form_t wot_td_echo_prop_aff_int_form_0 = {
    .op = &wot_td_echo_prop_aff_int_form_0_op_0,
    .href = &wot_td_echo_prop_aff_int_form_0_href,
    .content_type = &wot_td_echo_prop_aff_int_form_0_content_type,
    .content_encoding = CONTENT_ENCODING_GZIP,
    .security = &wot_td_echo_prop_aff_int_form_0_security_basic,
    .scopes = &wot_td_echo_prop_aff_int_form_0_scope_0,
    .expected_response = &wot_td_echo_prop_aff_int_form_0_response,
    .extensions = &wot_td_echo_prop_aff_int_form_0_extension,
    .next = NULL,
};

wot_td_number_schema_t wot_td_echo_prop_aff_uri_variable_d_data_schema_number = {
    .maximum = 2.0,
};

wot_td_type_t wot_td_echo_prop_aff_uri_variable_d_data_schema_type_0 = {
    .value = "eg:Direction",
    .next = NULL,
};

wot_td_data_schema_t wot_td_echo_prop_aff_uri_variable_d_data_schema = {
    .type = &wot_td_echo_prop_aff_uri_variable_d_data_schema_type_0,
    .json_type = JSON_TYPE_NUMBER,
    .schema = &wot_td_echo_prop_aff_uri_variable_d_data_schema_number,
};

wot_td_data_schema_map_t wot_td_echo_prop_aff_uri_variable_d_data_map = {
    .key = "d",
    .value = &wot_td_echo_prop_aff_uri_variable_d_data_schema,
    .next = NULL,
};

wot_td_type_t wot_td_echo_prop_aff_uri_variable_p_data_schema_array_items_1_schema_type_0 = {
    .value = "test2",
    .next = NULL,
};

wot_td_data_schema_t wot_td_echo_prop_aff_uri_variable_p_data_schema_array_items_1_schema = {
    .type = &wot_td_echo_prop_aff_uri_variable_p_data_schema_array_items_1_schema_type_0,
};

wot_td_data_schemas_t wot_td_echo_prop_aff_uri_variable_p_data_schema_array_items_1 = {
    .value = &wot_td_echo_prop_aff_uri_variable_p_data_schema_array_items_1_schema,
    .next = NULL,
};

wot_td_type_t wot_td_echo_prop_aff_uri_variable_p_data_schema_array_items_0_schema_type_0 = {
    .value = "test",
    .next = NULL,
};

wot_td_data_schema_t wot_td_echo_prop_aff_uri_variable_p_data_schema_array_items_0_schema = {
    .type = &wot_td_echo_prop_aff_uri_variable_p_data_schema_array_items_0_schema_type_0,
};

wot_td_data_schemas_t wot_td_echo_prop_aff_uri_variable_p_data_schema_array_items_0 = {
    .value = &wot_td_echo_prop_aff_uri_variable_p_data_schema_array_items_0_schema,
    .next = &wot_td_echo_prop_aff_uri_variable_p_data_schema_array_items_1,
};

wot_td_array_schema_t wot_td_echo_prop_aff_uri_variable_p_data_schema_array = {
    .min_items = 0,
    .max_items = 16,
    .items = &wot_td_echo_prop_aff_uri_variable_p_data_schema_array_items_0,
};

wot_td_type_t wot_td_echo_prop_aff_uri_variable_p_data_schema_type_0 = {
    .value = "eg:SomeKindOfAngle",
    .next = NULL,
};

wot_td_data_schema_t wot_td_echo_prop_aff_uri_variable_p_data_schema = {
    .type = &wot_td_echo_prop_aff_uri_variable_p_data_schema_type_0,
    .json_type = JSON_TYPE_ARRAY,
    .schema = &wot_td_echo_prop_aff_uri_variable_p_data_schema_array,
};

wot_td_data_schema_map_t wot_td_echo_prop_aff_uri_variable_p_data_map = {
    .key = "p",
    .value = &wot_td_echo_prop_aff_uri_variable_p_data_schema,
    .next = &wot_td_echo_prop_aff_uri_variable_d_data_map,
};

wot_td_multi_lang_t wot_td_echo_prop_aff_int_title_1 = {
    .tag = "en",
    .value = "English Title",
    .next = NULL,
};

wot_td_multi_lang_t wot_td_echo_prop_aff_int_title_0 = {
    .tag = "de",
    .value = "Deutscher Titel",
    .next = &wot_td_echo_prop_aff_int_title_1,
};

wot_td_multi_lang_t wot_td_echo_prop_aff_int_description_1 = {
    .tag = "en",
    .value = "English description",
    .next = NULL,
};

wot_td_multi_lang_t wot_td_echo_prop_aff_int_description_0 = {
    .tag = "de",
    .value = "Deutsche Beschreibung",
    .next = &wot_td_echo_prop_aff_int_description_1,
};

wot_td_type_t wot_td_echo_prop_aff_int_type_0 = {
    .value = "Type",
    .next = NULL,
};

wot_td_int_affordance_t wot_td_echo_prop_aff_int = {
    .type = &wot_td_echo_prop_aff_int_type_0,
    .descriptions = &wot_td_echo_prop_aff_int_description_0,
    .titles = &wot_td_echo_prop_aff_int_title_0,
    .uri_variables = &wot_td_echo_prop_aff_uri_variable_p_data_map,
    .forms = &wot_td_echo_prop_aff_int_form_0,
};

wot_td_prop_affordance_t wot_td_echo_prop_aff = {
    .key = "echo",
    .int_affordance = &wot_td_echo_prop_aff_int,
    .data_schema = &wot_td_echo_prop_aff_data_schema_data_schema,
    .observable = false,
    .next = NULL,
};

wot_td_coap_prop_affordance_t wot_td_echo = {
    .coap_resource = &_wot_coap_resources[0],
    .affordance = &wot_td_echo_prop_aff,
    .form = &wot_td_echo_prop_aff_int_form_0,
};

wot_td_extension_t wot_td_status_prop_aff_int_form_0_extension = {0};

wot_td_uri_t wot_td_status_prop_aff_int_form_0_href = {0};

wot_td_form_op_t wot_td_status_prop_aff_int_form_0_op_0 = {
    .op_type = FORM_OP_READ_PROPERTY,
    .next = NULL,
};

wot_td_form_t wot_td_status_prop_aff_int_form_0 = {
    .op = &wot_td_status_prop_aff_int_form_0_op_0,
    .href = &wot_td_status_prop_aff_int_form_0_href,
    .extensions = &wot_td_status_prop_aff_int_form_0_extension,
    .next = NULL,
};

wot_td_int_affordance_t wot_td_status_prop_aff_int = {
    .forms = &wot_td_status_prop_aff_int_form_0,
};

wot_td_prop_affordance_t wot_td_status_prop_aff = {
    .key = "status",
    .int_affordance = &wot_td_status_prop_aff_int,
    .next = NULL,
};

wot_td_coap_prop_affordance_t wot_td_status = {
    .coap_resource = &_wot_coap_resources[1],
    .affordance = &wot_td_status_prop_aff,
    .form = &wot_td_status_prop_aff_int_form_0,
};

wot_td_multi_lang_t wot_td_toggle_action_aff_output_data_schema_title_1 = {
    .tag = "en",
    .value = "English title",
    .next = NULL,
};

wot_td_multi_lang_t wot_td_toggle_action_aff_output_data_schema_title_0 = {
    .tag = "de",
    .value = "Deutscher Titel",
    .next = &wot_td_toggle_action_aff_output_data_schema_title_1,
};

wot_td_multi_lang_t wot_td_toggle_action_aff_output_data_schema_description_1 = {
    .tag = "en",
    .value = "English description",
    .next = NULL,
};

wot_td_multi_lang_t wot_td_toggle_action_aff_output_data_schema_description_0 = {
    .tag = "de",
    .value = "Deutsche Beschreibung",
    .next = &wot_td_toggle_action_aff_output_data_schema_description_1,
};

wot_td_data_schema_t wot_td_toggle_action_aff_output_data_schema = {
    .descriptions = &wot_td_toggle_action_aff_output_data_schema_description_0,
    .titles = &wot_td_toggle_action_aff_output_data_schema_title_0,
};

wot_td_multi_lang_t wot_td_toggle_action_aff_input_data_schema_title_1 = {
    .tag = "en",
    .value = "English title",
    .next = NULL,
};

wot_td_multi_lang_t wot_td_toggle_action_aff_input_data_schema_title_0 = {
    .tag = "de",
    .value = "Deutscher Titel",
    .next = &wot_td_toggle_action_aff_input_data_schema_title_1,
};

wot_td_multi_lang_t wot_td_toggle_action_aff_input_data_schema_description_1 = {
    .tag = "en",
    .value = "English description",
    .next = NULL,
};

wot_td_multi_lang_t wot_td_toggle_action_aff_input_data_schema_description_0 = {
    .tag = "de",
    .value = "Deutsche Beschreibung",
    .next = &wot_td_toggle_action_aff_input_data_schema_description_1,
};

wot_td_data_schema_t wot_td_toggle_action_aff_input_data_schema = {
    .descriptions = &wot_td_toggle_action_aff_input_data_schema_description_0,
    .titles = &wot_td_toggle_action_aff_input_data_schema_title_0,
};

wot_td_extension_t wot_td_toggle_action_aff_int_form_0_extension = {0};

wot_td_uri_t wot_td_toggle_action_aff_int_form_0_href = {0};

wot_td_form_op_t wot_td_toggle_action_aff_int_form_0_op_0 = {
    .op_type = FORM_OP_INVOKE_ACTION,
    .next = NULL,
};

wot_td_form_t wot_td_toggle_action_aff_int_form_0 = {
    .op = &wot_td_toggle_action_aff_int_form_0_op_0,
    .href = &wot_td_toggle_action_aff_int_form_0_href,
    .sub_protocol = "longpoll",
    .extensions = &wot_td_toggle_action_aff_int_form_0_extension,
    .next = NULL,
};

wot_td_multi_lang_t wot_td_toggle_action_aff_int_title_1 = {
    .tag = "en",
    .value = "English title",
    .next = NULL,
};

wot_td_multi_lang_t wot_td_toggle_action_aff_int_title_0 = {
    .tag = "de",
    .value = "Deutscher Titel",
    .next = &wot_td_toggle_action_aff_int_title_1,
};

wot_td_multi_lang_t wot_td_toggle_action_aff_int_description_1 = {
    .tag = "en",
    .value = "English description",
    .next = NULL,
};

wot_td_multi_lang_t wot_td_toggle_action_aff_int_description_0 = {
    .tag = "de",
    .value = "Deutsche Beschreibung",
    .next = &wot_td_toggle_action_aff_int_description_1,
};

wot_td_int_affordance_t wot_td_toggle_action_aff_int = {
    .descriptions = &wot_td_toggle_action_aff_int_description_0,
    .titles = &wot_td_toggle_action_aff_int_title_0,
    .forms = &wot_td_toggle_action_aff_int_form_0,
};

wot_td_action_affordance_t wot_td_toggle_action_aff = {
    .key = "toggle",
    .int_affordance = &wot_td_toggle_action_aff_int,
    .safe = true,
    .idempotent = true,
    .input = &wot_td_toggle_action_aff_input_data_schema,
    .output = &wot_td_toggle_action_aff_output_data_schema,
    .next = NULL,
};

wot_td_coap_action_affordance_t wot_td_toggle = {
    .coap_resource = &_wot_coap_resources[2],
    .affordance = &wot_td_toggle_action_aff,
    .form = &wot_td_toggle_action_aff_int_form_0,
};

int wot_td_coap_config_init(wot_td_thing_t *thing)
{
    gcoap_register_listener(&_wot_coap_listener);
    wot_td_coap_prop_add(thing, &wot_td_echo);
    wot_td_coap_prop_add(thing, &wot_td_status);
    wot_td_coap_action_add(thing, &wot_td_toggle);
    return 0;
}
