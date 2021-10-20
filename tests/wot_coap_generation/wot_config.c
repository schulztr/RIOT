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

extern ssize_t _led_status_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx);
extern ssize_t _led_toggle_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx);

static ssize_t wot_led_status_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    return _led_status_handler(pdu, buf, len, ctx);
}

static ssize_t wot_led_toggle_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    return _led_toggle_handler(pdu, buf, len, ctx);
}

const coap_resource_t _wot_coap_resources[] = {
    {"/led/status", COAP_GET, wot_led_status_handler, NULL},
    {"/led/toggle", COAP_POST, wot_led_toggle_handler, NULL},
};

static const char *_wot_link_params[] = {
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

wot_td_digest_sec_scheme_t wot_td_security_schema_digest_test_sec_scheme_definitions = {
    .qop = SECURITY_SCHEME_IN_DEFAULT,
    .name = "querykey",
    .in = SECURITY_SCHEME_IN_QUERY,
};

wot_td_multi_lang_t wot_td_security_schema_digest_test_sec_scheme_description_1 = {
    .tag = "de",
    .value = "Digest-Sicherheitsschema",
    .next = NULL,
};

wot_td_multi_lang_t wot_td_security_schema_digest_test_sec_scheme_description_0 = {
    .tag = "en",
    .value = "Digest sec schema",
    .next = &wot_td_security_schema_digest_test_sec_scheme_description_1,
};

wot_td_uri_t wot_td_security_schema_digest_test_sec_scheme_proxy = {
    .value = "https://example.org",
};

wot_td_sec_scheme_t wot_td_security_schema_digest_test_sec_scheme = {
    .scheme_type = SECURITY_SCHEME_DIGEST,
    .proxy = &wot_td_security_schema_digest_test_sec_scheme_proxy,
    .descriptions = &wot_td_security_schema_digest_test_sec_scheme_description_0,
    .scheme = &wot_td_security_schema_digest_test_sec_scheme_definitions,
};

wot_td_security_t wot_td_security_schema_digest_test = {
    .key = "digest_test",
    .value = &wot_td_security_schema_digest_test_sec_scheme,
    .next = NULL,
};

wot_td_basic_sec_scheme_t wot_td_security_schema_basic_sec_scheme_definitions = {
    .name = "querykey",
    .in = SECURITY_SCHEME_IN_QUERY,
};

wot_td_multi_lang_t wot_td_security_schema_basic_sec_scheme_description_1 = {
    .tag = "de",
    .value = "Einfaches Sicherheitsschema",
    .next = NULL,
};

wot_td_multi_lang_t wot_td_security_schema_basic_sec_scheme_description_0 = {
    .tag = "en",
    .value = "Basic sec schema",
    .next = &wot_td_security_schema_basic_sec_scheme_description_1,
};

wot_td_uri_t wot_td_security_schema_basic_sec_scheme_proxy = {
    .value = "https://example.org",
};

wot_td_sec_scheme_t wot_td_security_schema_basic_sec_scheme = {
    .scheme_type = SECURITY_SCHEME_BASIC,
    .proxy = &wot_td_security_schema_basic_sec_scheme_proxy,
    .descriptions = &wot_td_security_schema_basic_sec_scheme_description_0,
    .scheme = &wot_td_security_schema_basic_sec_scheme_definitions,
};

wot_td_security_t wot_td_security_schema_basic = {
    .key = "basic",
    .value = &wot_td_security_schema_basic_sec_scheme,
    .next = &wot_td_security_schema_digest_test,
};

wot_td_extension_t wot_td_toggle_action_aff_int_form_0_extension = {0};

wot_td_uri_t wot_td_toggle_action_aff_int_form_0_href = {0};

wot_td_form_t wot_td_toggle_action_aff_int_form_0 = {
    .href = &wot_td_toggle_action_aff_int_form_0_href,
    .extensions = &wot_td_toggle_action_aff_int_form_0_extension,
    .next = NULL,
};

wot_td_int_affordance_t wot_td_toggle_action_aff_int = {
    .forms = &wot_td_toggle_action_aff_int_form_0,
};

wot_td_action_affordance_t wot_td_toggle_action_aff = {
    .key = "toggle",
    .int_affordance = &wot_td_toggle_action_aff_int,
    .next = NULL,
};

wot_td_coap_action_affordance_t wot_td_toggle = {
    .coap_resource = &_wot_coap_resources[1],
    .affordance = &wot_td_toggle_action_aff,
    .form = &wot_td_toggle_action_aff_int_form_0,
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
    .coap_resource = &_wot_coap_resources[0],
    .affordance = &wot_td_status_prop_aff,
    .form = &wot_td_status_prop_aff_int_form_0,
};

wot_td_extension_t wot_td_thing_form_1_extension = {0};

wot_td_content_type_t wot_td_thing_form_1_content_type = {
    .media_type = CONTENT_TYPE_JSON,
};

wot_td_uri_t wot_td_thing_form_1_href = {0};

wot_td_form_op_t wot_td_thing_form_1_op_0 = {
    .op_type = FORM_OP_WRITE_ALL_PROPERTIES,
    .next = NULL,
};

wot_td_form_t wot_td_thing_form_1 = {
    .op = &wot_td_thing_form_1_op_0,
    .href = &wot_td_thing_form_1_href,
    .content_type = &wot_td_thing_form_1_content_type,
    .extensions = &wot_td_thing_form_1_extension,
    .next = NULL,
};

wot_td_extension_t wot_td_thing_form_0_extension = {0};

wot_td_content_type_t wot_td_thing_form_0_content_type = {
    .media_type = CONTENT_TYPE_JSON,
};

wot_td_uri_t wot_td_thing_form_0_href = {0};

wot_td_form_op_t wot_td_thing_form_0_op_0 = {
    .op_type = FORM_OP_READ_ALL_PROPERTIES,
    .next = NULL,
};

wot_td_form_t wot_td_thing_form_0 = {
    .op = &wot_td_thing_form_0_op_0,
    .href = &wot_td_thing_form_0_href,
    .content_type = &wot_td_thing_form_0_content_type,
    .extensions = &wot_td_thing_form_0_extension,
    .next = &wot_td_thing_form_1,
};

json_ld_context_t wot_td_thing_context_1 = {
    .key = "riot_os",
    .value = "http://www.example.org/riot-os-definitions#",
    .next = NULL,
};

json_ld_context_t wot_td_thing_context_0 = {
    .value = "http://www.w3.org/ns/td",
    .next = &wot_td_thing_context_1,
};

wot_td_type_t wot_td_thing_type_0 = {
    .value = "ThingModel",
    .next = NULL,
};

int wot_td_config_init(wot_td_thing_t *wot_td_thing){
    wot_td_thing->type = &wot_td_thing_type_0;
    wot_td_thing->context = &wot_td_thing_context_0;
    wot_td_thing->forms = &wot_td_thing_form_0;
    wot_td_thing->security = &wot_td_security_schema_basic;

    return 0;
}

int wot_td_coap_config_init(wot_td_thing_t *thing)
{
    gcoap_register_listener(&_wot_coap_listener);
    wot_td_coap_prop_add(thing, &wot_td_status);
    wot_td_coap_action_add(thing, &wot_td_toggle);
    return 0;
}
