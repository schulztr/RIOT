#include <stddef.h>
#include <string.h>

#include "thread.h"
#include "net/ipv6/addr.h"
#include "net/wot.h"
#include "net/wot/serialization.h"
#include "net/wot/config.h"
#include "net/wot/coap/config.h"
#include "msg.h"

#define WOT_TD_COAP_AFF_ADD(ptr, func_name)                             \
    func_name(thing, ptr->affordance);                                  \
    _add_endpoint_to_int_affordance(                                    \
        ptr->affordance->int_affordance, ptr->coap_resource);           \
    _add_method_to_int_affordance(                                      \
        ptr->affordance->int_affordance, ptr->coap_resource);           \
    gcoap_register_listener(ptr->coap_resource);                        \
    return 0;


#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

sock_udp_ep_t wot_coap_sock = { .port=COAP_PORT, .family=AF_INET6 };

wot_td_thing_t wot_thing;

static char wot_thing_addr[IPV6_ADDR_MAX_STR_LEN + 1];

const char wot_td_coap_schema[] = "coap://";

wot_td_uri_t _wot_thing_id = {
        .schema = wot_td_coap_schema,
        .value = wot_thing_addr,
};

//Todo: Implement CoAP RDF bindings.
//See: https://github.com/w3c/wot-binding-templates/issues/97
json_ld_context_t wot_td_coap_binding_context = {
        .key = "cov",
        .value = "http://www.example.org/coap-binding#",
};

//static kernel_pid_t wot_td_coap_pid;
//static char wot_td_coap_stack[THREAD_STACKSIZE_DEFAULT + THREAD_EXTRA_STACKSIZE_PRINTF];

const char coap_get_method_name[] = "GET";
const char coap_post_method_name[] = "POST";
const char coap_put_method_name[] = "PUT";

const char wot_td_coap_method_name[] = "methodName";

coap_block_slicer_t _wot_td_coap_slicer;
ssize_t _wot_td_coap_plen;
uint8_t *_wot_td_coap_buf;

void _wot_td_coap_write_string(
        wot_td_serialize_receiver_t receiver, const char * string, size_t length){
    for(size_t i = 0; i < length; i++){
        receiver(&string[i]);
    }
}

void _wot_td_coap_write_cov_method_json_key(wot_td_serialize_receiver_t receiver){
    receiver("\"");
    _wot_td_coap_write_string(
            receiver, wot_td_coap_binding_context.key,
            sizeof(wot_td_coap_binding_context.key));
    receiver(":");
    _wot_td_coap_write_string(
            receiver, wot_td_coap_method_name, sizeof(wot_td_coap_method_name));
    receiver("\"");
    receiver(":");
}

void _wot_td_coap_method_ser(
        wot_td_serialize_receiver_t receiver, const char * name, void * data){
    if(name == wot_td_coap_binding_context.key){
        coap_method_flags_t *methods = (coap_method_flags_t*)data;

        _wot_td_coap_write_cov_method_json_key(receiver);

        receiver("\"");
        if(*methods & COAP_GET){
            _wot_td_coap_write_string(
                    receiver, coap_get_method_name,
                    sizeof(coap_get_method_name));
        }else if(*methods & COAP_POST){
            _wot_td_coap_write_string(
                    receiver, coap_post_method_name,
                    sizeof(coap_post_method_name));
        }else if(*methods & COAP_PUT){
            _wot_td_coap_write_string(
                    receiver, coap_put_method_name,
                    sizeof(coap_put_method_name));
        }
        receiver("\"");
    }
}

void _wot_td_coap_ser_receiver(const char *c){
    _wot_td_coap_plen += coap_blockwise_put_char(&_wot_td_coap_slicer, _wot_td_coap_buf+_wot_td_coap_plen, (char) *c);
}

static ssize_t _wot_td_coap_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx){
    (void)ctx;

    _wot_td_coap_buf = buf;
    memset(&_wot_td_coap_slicer, 0, sizeof(coap_block_slicer_t));
    coap_block2_init(pdu, &_wot_td_coap_slicer);

    gcoap_resp_init(pdu, _wot_td_coap_buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
    coap_opt_add_block2(pdu, &_wot_td_coap_slicer, 1);
    _wot_td_coap_plen = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

    wot_td_ser_slicer_t _wot_td_slicer = {
            .start = _wot_td_coap_slicer.start,
            .end = _wot_td_coap_slicer.end,
    };

    wot_td_serialize_thing((wot_td_serialize_receiver_t) &_wot_td_coap_ser_receiver, &wot_thing, &_wot_td_slicer);

    coap_block2_finish(&_wot_td_coap_slicer);

    return _wot_td_coap_plen;
}

//Todo: Implement WoT-discovery.
//https://w3c.github.io/wot-discovery/
static const coap_resource_t _wot_td_coap_resources[] = {
        { "/.well-known/wot-thing-description", COAP_GET, _wot_td_coap_handler, NULL },
};

gcoap_listener_t _wot_td_gcoap_listener = {
        &_wot_td_coap_resources[0],
        sizeof(_wot_td_coap_resources) / sizeof(_wot_td_coap_resources[0]),
        NULL,
        NULL
};

gcoap_listener_t * _find_last_gcoap_listener(void){
    gcoap_listener_t *listener = &_wot_td_gcoap_listener;
    while(listener->next != NULL){
        listener = listener->next;
    }
    return listener;
}

//Todo: Add COAP_MATCH_SUBTREE to href. 
void _add_method_to_int_affordance(wot_td_int_affordance_t * affordance, gcoap_listener_t * listener){
    wot_td_form_t *form = affordance->forms;
    wot_td_extension_t *extension;
    while(form != NULL){
        extension = form->extension;
        extension->name = wot_td_coap_binding_context.key;
        extension->data = &(listener->resources->methods);
        extension->parser = (wot_td_ser_parser_t) &_wot_td_coap_method_ser;
        form = form->next;
    }
}

void _add_endpoint_to_int_affordance(wot_td_int_affordance_t * affordance, gcoap_listener_t * listener){
    wot_td_form_t *form = affordance->forms;
    while(form != NULL){
        wot_td_uri_t *href = form->href;
        href->value = listener->resources->path;
        form = form->next;
    }
}

int wot_td_coap_prop_add(wot_td_thing_t *thing, wot_td_coap_prop_affordance_t *property){
    WOT_TD_COAP_AFF_ADD(property, wot_td_thing_prop_add);
}

int wot_td_coap_action_add(wot_td_thing_t *thing, wot_td_coap_action_affordance_t *action){
    WOT_TD_COAP_AFF_ADD(action, wot_td_thing_action_add);
}

int wot_td_coap_event_add(wot_td_thing_t *thing, wot_td_coap_event_affordance_t *event){
    WOT_TD_COAP_AFF_ADD(event, wot_td_thing_event_add);
}

void wot_td_coap_server_init(void)
{
    wot_td_thing_context_add(&wot_thing, &wot_td_coap_binding_context);
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    //Todo: Implement proper IP address resolve solution and add it to the TD

    wot_td_config_init(&wot_thing);
    wot_td_coap_config_init(&wot_thing);

    gcoap_init();
    gcoap_register_listener(&_wot_td_gcoap_listener);
}
