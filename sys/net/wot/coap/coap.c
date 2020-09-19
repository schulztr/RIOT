#include <stddef.h>
#include <string.h>

#include "thread.h"
#include "net/ipv6/addr.h"
#include "net/wot.h"
#include "net/wot/serialization.h"
#include "net/wot/config.h"
#include "net/wot/coap/config.h"
#include "net/nanocoap.h"
#include "net/sock/udp.h"
#include "msg.h"

#define WOT_TD_COAP_AFF_ADD(ptr, func_name) ({                          \
    if(wot_coap_current_index <= WOT_COAP_NUM){                         \
        coap_resources[wot_coap_current_index] = ptr->coap_resource;    \
        wot_coap_current_index++;                                       \
        func_name(thing, ptr->affordance);                              \
    }else{                                                              \
        println("ERROR!");                                              \
    }                                                                   \
})

extern kernel_pid_t wot_coap_pid;

#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

/**
 * @brief   Stack for the wot coap thread
 */
#define WOT_COAP_STACKSIZE THREAD_STACKSIZE_DEFAULT
static char _wot_coap_stack[WOT_COAP_STACKSIZE];

sock_udp_ep_t wot_coap_sock = { .port=COAP_PORT, .family=AF_INET6 };

wot_td_thing_t wot_thing;

static char wot_thing_addr[IPV6_ADDR_MAX_STR_LEN + 1];

wot_td_uri_t wot_thing_id = {
        .schema = "coap://",
        .value = wot_thing_addr,
};

//Todo: Implement CoAP RDF bindings when done.
//See: https://github.com/w3c/wot-binding-templates/issues/97
json_ld_context_t wot_td_some_context = {
        .key = "cov",
        .value = "http://www.example.org/coap-binding#",
};

coap_block_slicer_t wot_td_coap_slicer;
uint8_t *wot_td_coap_payload;
uint8_t *wot_td_coap_bufpos;

void _wot_td_coap_ser_receiver(const char * string, uint32_t length){
    for(int i = 0; i < (int)length; i++){
        printf("%c", string[i]);
    }

    wot_td_coap_bufpos += coap_blockwise_put_bytes(
                    &wot_td_coap_slicer, wot_td_coap_bufpos,
                    (const uint8_t *) string, length);
}

static ssize_t _wot_td_coap_handler(coap_pkt_t *pkt, uint8_t *buf, size_t len, void *context)
{
    (void)context;
    coap_block2_init(pkt, &wot_td_coap_slicer);
    wot_td_coap_payload = buf + coap_get_total_hdr_len(pkt);
    wot_td_coap_bufpos = wot_td_coap_payload;

    wot_td_coap_bufpos += coap_put_option_ct(wot_td_coap_bufpos, 0, COAP_FORMAT_TEXT);
    wot_td_coap_bufpos += coap_opt_put_block2(wot_td_coap_bufpos, COAP_OPT_CONTENT_FORMAT, &wot_td_coap_slicer, 1);
    *wot_td_coap_bufpos++ = 0xff;

    wot_td_serialize_thing((wot_td_serialize_receiver_t) &_wot_td_coap_ser_receiver, &wot_thing);

    unsigned payload_len = wot_td_coap_bufpos - wot_td_coap_payload;
    return coap_block2_build_reply(pkt, COAP_CODE_205,
                                   buf, len, payload_len, &wot_td_coap_slicer);
}

#define WOT_COAP_NUM 1
const coap_resource_t coap_resources[WOT_COAP_NUM + 2] = {
        COAP_WELL_KNOWN_CORE_DEFAULT_HANDLER,
        { "/", COAP_GET, _wot_td_coap_handler, NULL },
};

const unsigned coap_resources_numof = ARRAY_SIZE(coap_resources);

int wot_coap_current_index = 2;

int wot_td_coap_prop_add(wot_td_thing_t *thing, wot_td_coap_prop_affordance_t *property){
    WOT_TD_COAP_AFF_ADD(property, wot_td_thing_prop_add);
}

int wot_td_coap_action_add(wot_td_thing_t *thing, wot_td_coap_action_affordance_t *action){
    WOT_TD_COAP_AFF_ADD(action, wot_td_thing_action_add);
}

int wot_td_coap_event_add(wot_td_thing_t *thing, wot_td_coap_event_affordance_t *event){
    WOT_TD_COAP_AFF_ADD(event, wot_td_thing_event_add);
}

#define WOT_COAP_BUF_SIZE COAP_INBUF_SIZE
static void *_wot_coap_server(void *arg)
{
    uint8_t buf[WOT_COAP_BUF_SIZE];
    nanocoap_server(&wot_coap_sock, buf, sizeof(buf));
}

kernel_pid_t wot_td_coap_server_init(void)
{
    wot_td_config_init(&wot_thing);
    wot_td_coap_config_init(&wot_thing);
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    ipv6_addr_to_str(wot_thing_addr, wot_coap_sock.addr);
    if (wot_coap_pid == KERNEL_PID_UNDEF) {
        wot_coap_pid = thread_create(_wot_coap_stack, sizeof(_stack), WOT_COAP_PRIO,
                                     THREAD_CREATE_STACKTEST,
                                     _wot_coap_server, NULL, "wot_coap");
    }
    return wot_coap_pid;
}
