#include "net/wot/wot.h"
#include "net/wot/coap/coap.h"

static ssize_t _echo_handler(coap_pkt_t *pkt, uint8_t *buf, size_t len, void *context)
{
    (void)context;
    char uri[CONFIG_NANOCOAP_URI_MAX];

    if (coap_get_uri_path(pkt, (uint8_t *)uri) <= 0) {
        return coap_reply_simple(pkt, COAP_CODE_INTERNAL_SERVER_ERROR, buf,
                                 len, COAP_FORMAT_TEXT, NULL, 0);
    }
    char *sub_uri = uri + strlen("/echo/");
    size_t sub_uri_len = strlen(sub_uri);
    return coap_reply_simple(pkt, COAP_CODE_CONTENT, buf, len, COAP_FORMAT_TEXT,
                             (uint8_t *)sub_uri, sub_uri_len);
}

wot_td_coap_prop_affordance_t wot_coap_echo_affordance = {
        .coap_resource = {
                .method = COAP_GET,
                .handler = _echo_handler,
                .path = "/echo",
        },
        .affordance = {
                .observable = false,
                .key = "echo",
                .int_affordance = {
                        .forms = {

                        }
                }
        }
};

int wot_td_coap_config_init(wot_td_thing_t *thing){
    wot_td_coap_prop_add(thing, &wot_coap_echo_affordance);
}

