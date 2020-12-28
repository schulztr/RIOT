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

static ssize_t _led_toggle_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    unsigned method_flag = coap_method2flag(coap_get_code_detail(pdu));

    if (method_flag == COAP_POST)
    {
        puts("Hi.");
        return gcoap_response(pdu, buf, len, COAP_CODE_CHANGED);
    }

    return 0;
}

const coap_resource_t _wot_coap_resources[] = {
    {"/led/toggle", COAP_POST, _led_toggle_handler, NULL},
};

static const char *_wot_link_params[] = {
    NULL
}

static gcoap_listener_t _wot_coap_listener = {
    &_wot_coap_resources[0],
    ARRAY_SIZE(_wot_coap_resources),
    _wot_encode_link,
    NULL,
    NULL
};

static ssize_t _wot_encode_link(const coap_resource_t *resource, char *buf,
                            size_t maxlen, coap_link_encoder_ctx_t *context) {
    ssize_t res = gcoap_encode_link(resource, buf, maxlen, context);
    if (res > 0) {
        if (_wot_link_params[context->link_pos]
                && (strlen(_wot_link_params[context->link_pos]) < (maxlen - res))) {
            if (buf) {
                memcpy(buf+res, _wot_link_params[context->link_pos],
                       strlen(_wot_link_params[context->link_pos]));
            }
            return res + strlen(_wot_link_params[context->link_pos]);
        }
    }

    return res;
}

void wot_td_coap_init(void)
{
    gcoap_register_listener(&_wot_coap_listener);
}
