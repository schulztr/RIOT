#include "net/wot.h"
#include "net/wot/serialization.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

const char wot_td_ser_obj_context_key[] = "@context";
const char wot_td_ser_w3c_context_value[] = "https://www.w3.org/2019/wot/td/v1";
const char wot_td_ser_true[] = "true";
const char wot_td_ser_false[] = "false";
const char wot_td_type_obj_key[] = "@type";
const char wot_td_titles_obj_key[] = "titles";
const char wot_td_description_obj_key[] = "descriptions";
const char wot_td_in_obj_key[] = "in";
const char wot_td_name_obj_key[] = "name";
const char wot_td_qop_obj_key[] = "qop";
const char wot_td_schema_obj_key[] = "scheme";
const char wot_td_ser_id_obj_key[] = "id";
const char wot_td_security_def_obj_key[] = "securityDefinitions";
const char wot_td_op_obj_key[] = "op";
const char wot_td_href_obj_key[] = "href";
const char wot_td_content_type_obj_key[] = "contentType";
const char wot_td_content_encoding_obj_key[] = "contentCoding";
const char wot_td_sub_protocol_obj_key[] = "subprotocol";
const char wot_td_scopes_obj_key[] = "scopes";
const char wot_td_observable_obj_key[] = "observable";
const char wot_td_required_obj_key[] = "required";
const char wot_td_item_obj_key[] = "items";
const char wot_td_min_items_obj_key[] = "minItems";
const char wot_td_max_items_obj_key[] = "maxItems";
const char wot_td_min_obj_key[] = "minimum";
const char wot_td_max_obj_key[] = "maximum";
const char wot_td_constant_obj_key[] = "const";
const char wot_td_unit_obj_key[] = "unit";
const char wot_td_one_of_obj_key[] = "oneOf";
const char wot_td_enum_obj_key[] = "enum";
const char wot_td_read_only_obj_key[] = "readOnly";
const char wot_td_write_only_obj_key[] = "writeOnly";
const char wot_td_format_obj_key[] = "format";
const char wot_td_action_aff_obj_key[] = "actions";
const char wot_td_input_obj_key[] = "input";
const char wot_td_output_obj_key[] = "output";
const char wot_td_safe_obj_key[] = "safe";
const char wot_td_idempotent_obj_key[] = "idempotent";
const char wot_td_events_obj_key[] = "events";
const char wot_td_subscription_obj_key[] = "subscription";
const char wot_td_data_obj_key[] = "data";
const char wot_td_cancellation_obj_key[] = "cancellation";
const char wot_td_ser_links_obj_key[] = "links";
const char wot_td_ser_base_obj_key[] = "base";
const char wot_td_rel_obj_key[] = "rel";
const char wot_td_anchor_obj_key[] = "anchor";
const char wot_td_ser_support_obj_key[] = "support";
const char wot_td_ser_version_obj_key[] = "version";
const char wot_td_ser_instance_obj_key[] = "instance";
const char wot_td_ser_form_obj_key[] = "forms";
const char wot_td_created_obj_key[] = "created";
const char wot_td_modified_obj_key[] = "modified";
const char wot_td_ser_prop_aff_obj_key[] = "properties";

void _reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void _itoa(int n, char s[])
{
    int i, sign;
    if ((sign = n) < 0)
        n = -n;
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    _reverse(s);
}

//Todo: Validate Thing struct. Separate functions?

//Fixme: circuit breaker pattern? IPC message passing? Make it more elegant.
//Fixme: Not working with slicer->start.
//https://riot-os.org/api/group__core__msg.html
int _wot_td_fill_json_receiver(wot_td_serialize_receiver_t receiver, const char *string, uint32_t length, wot_td_ser_slicer_t *slicer){
    for(uint32_t i = 0; i < length; i++){
        if(slicer->cur <= slicer->end){
            receiver(&string[i]);
        }

        slicer->cur += 1;
    }
    return 0;
}

//Todo: Clearer naming convention?
void _wot_td_fill_json_string(wot_td_serialize_receiver_t receiver, const char *string, uint32_t length, wot_td_ser_slicer_t *slicer){
    _wot_td_fill_json_receiver(receiver, "\"", 1, slicer);
    _wot_td_fill_json_receiver(receiver, string, length, slicer);
    _wot_td_fill_json_receiver(receiver, "\"", 1, slicer);
}

void _wot_td_fill_json_uri(wot_td_serialize_receiver_t receiver, wot_td_uri_t *uri, wot_td_ser_slicer_t *slicer){
    _wot_td_fill_json_receiver(receiver, "\"", 1, slicer);
    //Fixme: Not very clean to have it here. Find better solution.
    if(uri->schema != NULL){
        _wot_td_fill_json_receiver(receiver, uri->schema, strlen(uri->schema), slicer);
    }
    _wot_td_fill_json_receiver(receiver, uri->value, strlen(uri->value), slicer);
    _wot_td_fill_json_receiver(receiver, "\"", 1, slicer);
}

void _wot_td_fill_json_date(wot_td_serialize_receiver_t receiver, wot_td_date_time_t *date, wot_td_ser_slicer_t *slicer){
    _wot_td_fill_json_receiver(receiver, "\"", 1, slicer);
    char s[11];
    _itoa(date->year, s);
    _wot_td_fill_json_receiver(receiver, s, strlen(s), slicer);
    _wot_td_fill_json_receiver(receiver, "-", 1, slicer);
    _itoa(date->month, s);
    _wot_td_fill_json_receiver(receiver, s, strlen(s), slicer);
    _wot_td_fill_json_receiver(receiver, "-", 1, slicer);
    _itoa(date->day, s);
    _wot_td_fill_json_receiver(receiver, s, strlen(s), slicer);
    _wot_td_fill_json_receiver(receiver, "T", 1, slicer);
    _itoa(date->hour, s);
    _wot_td_fill_json_receiver(receiver, s, strlen(s), slicer);
    _wot_td_fill_json_receiver(receiver, ":", 1, slicer);
    _itoa(date->minute, s);
    _wot_td_fill_json_receiver(receiver, s, strlen(s), slicer);
    _wot_td_fill_json_receiver(receiver, ":", 1, slicer);
    _itoa(date->second, s);
    _wot_td_fill_json_receiver(receiver, s, strlen(s), slicer);
    _itoa(date->timezone_offset, s);
    _wot_td_fill_json_receiver(receiver, s, strlen(s), slicer);

    _wot_td_fill_json_receiver(receiver, "\"", 1, slicer);
}

void _wot_td_fill_json_obj_key(wot_td_serialize_receiver_t receiver, const char *string, uint32_t length, wot_td_ser_slicer_t *slicer){
    _wot_td_fill_json_string(receiver, string, length, slicer);
    _wot_td_fill_json_receiver(receiver, ":", 1, slicer);
}

void _wot_td_fill_json_bool(wot_td_serialize_receiver_t receiver, bool value, wot_td_ser_slicer_t *slicer){
    if(value){
        _wot_td_fill_json_receiver(receiver, wot_td_ser_true, sizeof(wot_td_ser_true)-1, slicer);
    }else{
        _wot_td_fill_json_receiver(receiver, wot_td_ser_false, sizeof(wot_td_ser_false)-1, slicer);
    }
}

void _previous_prop_check(wot_td_serialize_receiver_t receiver, bool has_previous_prop, wot_td_ser_slicer_t *slicer){
    if(has_previous_prop){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
    }
}

void _serialize_context(wot_td_serialize_receiver_t receiver, json_ld_context_t *context, wot_td_ser_slicer_t *slicer){
    if(context->key != NULL){
        _wot_td_fill_json_receiver(receiver, "{", 1, slicer);

        _wot_td_fill_json_obj_key(receiver, context->key, strlen(context->key), slicer);
    }

    _wot_td_fill_json_string(receiver, context->value, strlen(context->value), slicer);

    if(context->key != NULL) {
        _wot_td_fill_json_receiver(receiver, "}", 1, slicer);
    }
}

void _serialize_context_array(wot_td_serialize_receiver_t receiver, json_ld_context_t *context, wot_td_ser_slicer_t *slicer){
    json_ld_context_t *tmp_ctx = context;

    while(tmp_ctx != NULL){
        _serialize_context(receiver, tmp_ctx, slicer);
        if(tmp_ctx != NULL && tmp_ctx->next != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        }
        tmp_ctx = tmp_ctx->next;
    }
}

void _serialize_type_array(wot_td_serialize_receiver_t receiver, wot_td_type_t *type, wot_td_ser_slicer_t *slicer){
    _wot_td_fill_json_obj_key(receiver, wot_td_type_obj_key, sizeof(wot_td_type_obj_key)-1, slicer);
    _wot_td_fill_json_receiver(receiver, "[", 1, slicer);
    wot_td_type_t *tmp_type = type;
    while(tmp_type != NULL){
        _wot_td_fill_json_string(receiver, tmp_type->value, strlen(tmp_type->value), slicer);
        if(tmp_type != NULL && tmp_type->next != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        }
        tmp_type = tmp_type->next;
    }
    _wot_td_fill_json_receiver(receiver, "]", 1, slicer);
}

void _serialize_lang(wot_td_serialize_receiver_t receiver, wot_td_multi_lang_t *lang, wot_td_ser_slicer_t *slicer){
    _wot_td_fill_json_obj_key(receiver, lang->tag, strlen(lang->tag), slicer);
    _wot_td_fill_json_string(receiver, lang->value, strlen(lang->value), slicer);
}

void _serialize_title_array(wot_td_serialize_receiver_t receiver, wot_td_multi_lang_t *titles, char *lang, wot_td_ser_slicer_t *slicer){
    bool has_previous_prop = false;
    wot_td_multi_lang_t *tmp = titles;
    wot_td_multi_lang_t *default_title = NULL;
    _wot_td_fill_json_obj_key(receiver, wot_td_titles_obj_key, sizeof(wot_td_titles_obj_key)-1, slicer);

    _wot_td_fill_json_receiver(receiver, "{", 1, slicer);
    while(tmp != NULL){
        has_previous_prop = true;
        _serialize_lang(receiver, tmp, slicer);
        
        if(lang != NULL && strcmp(tmp->tag, lang) == 0){
            default_title = tmp;
        }

        if(tmp->next != NULL){
            _previous_prop_check(receiver, has_previous_prop, slicer);
        }
        tmp = tmp->next;
    }
    _wot_td_fill_json_receiver(receiver, "}", 1, slicer);

    if(default_title != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, wot_td_titles_obj_key, sizeof(wot_td_titles_obj_key)-2, slicer);
        _wot_td_fill_json_string(receiver, default_title->value, strlen(default_title->value), slicer);
    }
}

void _serialize_description_array(wot_td_serialize_receiver_t receiver, wot_td_multi_lang_t *desc, char *lang, wot_td_ser_slicer_t *slicer){
    bool has_previous_prop = false;
    wot_td_multi_lang_t *tmp = desc;
    wot_td_multi_lang_t *default_description = NULL;
    _wot_td_fill_json_obj_key(receiver, wot_td_description_obj_key, sizeof(wot_td_description_obj_key)-1, slicer);
    _wot_td_fill_json_receiver(receiver, "{", 1, slicer);
    while(tmp != NULL){

        has_previous_prop = true;
        _serialize_lang(receiver, tmp, slicer);

        if(lang != NULL && strcmp(tmp->tag, lang) == 0){
            default_description = tmp;
        }

        if(tmp->next != NULL){
            _previous_prop_check(receiver, has_previous_prop, slicer);
        }
        tmp = tmp->next;
    }
    _wot_td_fill_json_receiver(receiver, "}", 1, slicer);

    if(default_description != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, wot_td_description_obj_key, sizeof(wot_td_description_obj_key)-2, slicer);
        _wot_td_fill_json_string(receiver, default_description->value, strlen(default_description->value), slicer);
    }
}

//Fixme: Use const vars
void _security_scheme_string(wot_td_serialize_receiver_t receiver, wot_td_sec_scheme_type_t scheme_type, wot_td_ser_slicer_t *slicer){
    switch (scheme_type) {
        case SECURITY_SCHEME_NONE:
            _wot_td_fill_json_string(receiver, "nosec", sizeof("nosec")-1, slicer);
            break;
        case SECURITY_SCHEME_BASIC:
            _wot_td_fill_json_string(receiver, "basic", sizeof("basic")-1, slicer);
            break;
        case SECURITY_SCHEME_DIGEST:
            _wot_td_fill_json_string(receiver, "digest", sizeof("digest")-1, slicer);
            break;
        case SECURITY_SCHEME_API_KEY:
            _wot_td_fill_json_string(receiver, "apikey", sizeof("apikey")-1, slicer);
            break;
        case SECURITY_SCHEME_BEARER:
            _wot_td_fill_json_string(receiver, "bearer", sizeof("bearer")-1, slicer);
            break;
        case SECURITY_SCHEME_PSK:
            _wot_td_fill_json_string(receiver, "psk", sizeof("psk")-1, slicer);
            break;
        case SECURITY_SCHEME_OAUTH2:
            _wot_td_fill_json_string(receiver, "oauth2", sizeof("oauth2")-1, slicer);
            break;
        default:
            _wot_td_fill_json_string(receiver, "nosec", sizeof("nosec")-1, slicer);
            break;
    }
}

void _security_schema_in_string(wot_td_serialize_receiver_t receiver, wot_td_sec_scheme_in_t in, wot_td_ser_slicer_t *slicer){
    switch(in){
        case SECURITY_SCHEME_IN_HEADER:
            _wot_td_fill_json_string(receiver, "header", sizeof("header")-1, slicer);
            break;
        case SECURITY_SCHEME_IN_QUERY:
            _wot_td_fill_json_string(receiver, "query", sizeof("query")-1, slicer);
            break;
        case SECURITY_SCHEME_IN_BODY:
            _wot_td_fill_json_string(receiver, "body", sizeof("body")-1, slicer);
            break;
        case SECURITY_SCHEME_IN_COOKIE:
            _wot_td_fill_json_string(receiver, "cookie", sizeof("cookie")-1, slicer);
            break;
        default:
            _wot_td_fill_json_string(receiver, "header", sizeof("header")-1, slicer);
    }
}

void _serialize_sec_scheme_basic(wot_td_serialize_receiver_t receiver, wot_td_basic_sec_scheme_t *scheme, wot_td_ser_slicer_t *slicer){
    _wot_td_fill_json_obj_key(receiver, wot_td_in_obj_key, sizeof(wot_td_in_obj_key)-1, slicer);
    _security_schema_in_string(receiver, scheme->in, slicer);

    _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
    _wot_td_fill_json_obj_key(receiver, wot_td_name_obj_key, sizeof(wot_td_name_obj_key)-1, slicer);
    _wot_td_fill_json_string(receiver, scheme->name, strlen(scheme->name), slicer);
}

void _security_digest_qop_string(wot_td_serialize_receiver_t receiver, wot_td_digest_qop_t qop, wot_td_ser_slicer_t *slicer){
    if(qop == SECURITY_DIGEST_QOP_AUTH_INT){
        _wot_td_fill_json_string(receiver, "auth-int", sizeof("auth-int")-1, slicer);
    }else{
        _wot_td_fill_json_string(receiver, "auth", sizeof("auth")-1, slicer);
    }
}

void _serialize_sec_scheme_digest(wot_td_serialize_receiver_t receiver, wot_td_digest_sec_scheme_t *scheme, wot_td_ser_slicer_t *slicer){
    _wot_td_fill_json_obj_key(receiver, wot_td_qop_obj_key, sizeof(wot_td_qop_obj_key)-1, slicer);
    _security_digest_qop_string(receiver, scheme->qop, slicer);

    _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
    _wot_td_fill_json_obj_key(receiver, wot_td_in_obj_key, sizeof(wot_td_in_obj_key)-1, slicer);
    _security_schema_in_string(receiver, scheme->in, slicer);
    _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
    _wot_td_fill_json_obj_key(receiver, wot_td_name_obj_key, sizeof(wot_td_name_obj_key)-1, slicer);
    _wot_td_fill_json_string(receiver, scheme->name, strlen(scheme->name), slicer);
}

void _serialize_sec_scheme_api_key(wot_td_serialize_receiver_t receiver, wot_td_api_key_sec_scheme_t *scheme, wot_td_ser_slicer_t *slicer){
    _wot_td_fill_json_obj_key(receiver, wot_td_in_obj_key, sizeof(wot_td_in_obj_key)-1, slicer);
    _security_schema_in_string(receiver, scheme->in, slicer);
    if(scheme->name != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, wot_td_name_obj_key, sizeof(wot_td_name_obj_key)-1, slicer);
        _wot_td_fill_json_string(receiver, scheme->name, strlen(scheme->name), slicer);
    }
}

void _serialize_sec_scheme_bearer(wot_td_serialize_receiver_t receiver, wot_td_bearer_sec_scheme_t *scheme, wot_td_ser_slicer_t *slicer){
    bool has_prev_prop = false;
    if(scheme->authorization != NULL){
        has_prev_prop = true;
        _wot_td_fill_json_obj_key(receiver, "authorization", sizeof("authorization")-1, slicer);
        _wot_td_fill_json_uri(receiver, scheme->authorization, slicer);
    }

    //Todo: Use enum instead
    if(scheme->alg != NULL){
        _previous_prop_check(receiver, has_prev_prop, slicer);
        has_prev_prop = true;
        _wot_td_fill_json_obj_key(receiver, "alg", sizeof("alg")-1, slicer);
        _wot_td_fill_json_string(receiver, scheme->alg, strlen(scheme->alg), slicer);
    }

    if(scheme->format != NULL){
        _previous_prop_check(receiver, has_prev_prop, slicer);
        has_prev_prop = true;
        _wot_td_fill_json_obj_key(receiver, "format", sizeof("format")-1, slicer);
        _wot_td_fill_json_string(receiver, scheme->format, strlen(scheme->format), slicer);
    }

    if(scheme->in != SECURITY_SCHEME_IN_DEFAULT){
        _previous_prop_check(receiver, has_prev_prop, slicer);
        has_prev_prop = true;
        _wot_td_fill_json_obj_key(receiver, wot_td_in_obj_key, sizeof(wot_td_in_obj_key)-1, slicer);
        _security_schema_in_string(receiver, scheme->in, slicer);
    }

    if(scheme->name != NULL){
        _previous_prop_check(receiver, has_prev_prop, slicer);
        has_prev_prop = true;
        _wot_td_fill_json_obj_key(receiver, wot_td_name_obj_key, sizeof(wot_td_name_obj_key)-1, slicer);
        _wot_td_fill_json_string(receiver, scheme->name, strlen(scheme->name), slicer);
    }
}

void _serialize_sec_scheme_psk(wot_td_serialize_receiver_t receiver, wot_td_psk_sec_scheme_t *scheme, wot_td_ser_slicer_t *slicer){
    if(scheme->identity != NULL){
        _wot_td_fill_json_obj_key(receiver, "identity", sizeof("identity")-1, slicer);
        _wot_td_fill_json_string(receiver, scheme->identity, strlen(scheme->identity), slicer);
    }
}

void _serialize_sec_scheme_oauth2(wot_td_serialize_receiver_t receiver, wot_td_oauth2_sec_scheme_t *scheme, wot_td_ser_slicer_t *slicer){
    _wot_td_fill_json_obj_key(receiver, "flow", sizeof("flow")-1, slicer);
    _wot_td_fill_json_string(receiver, scheme->flow, strlen(scheme->flow), slicer);

    if(scheme->authorization != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, "authorization", sizeof("authorization")-1, slicer);
        _wot_td_fill_json_uri(receiver, scheme->authorization, slicer);
    }

    //Todo: Use macro to generate it.
    if(scheme->token != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, "token", sizeof("token")-1, slicer);
        _wot_td_fill_json_uri(receiver, scheme->token, slicer);
    }

    if(scheme->refresh != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, "refresh", sizeof("refresh")-1, slicer);
        _wot_td_fill_json_uri(receiver, scheme->refresh, slicer);
    }

    if(scheme->scopes != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, "scopes", sizeof("scopes")-1, slicer);
        _wot_td_fill_json_uri(receiver, scheme->refresh, slicer);
    }
}

void _serialize_security_schema(wot_td_serialize_receiver_t receiver, wot_td_sec_scheme_t *security, wot_td_ser_slicer_t *slicer){
    switch (security->scheme_type) {
        default:
            return;
        case SECURITY_SCHEME_BASIC:
            _serialize_sec_scheme_basic(receiver, (wot_td_basic_sec_scheme_t *) security->scheme, slicer);
            break;
        case SECURITY_SCHEME_DIGEST:
            _serialize_sec_scheme_digest(receiver, (wot_td_digest_sec_scheme_t *) security->scheme, slicer);
            break;
        case SECURITY_SCHEME_API_KEY:
            _serialize_sec_scheme_api_key(receiver, (wot_td_api_key_sec_scheme_t *) security->scheme, slicer);
            break;
        case SECURITY_SCHEME_BEARER:
            _serialize_sec_scheme_bearer(receiver, (wot_td_bearer_sec_scheme_t *) security->scheme, slicer);
            break;
        case SECURITY_SCHEME_PSK:
            _serialize_sec_scheme_psk(receiver, (wot_td_psk_sec_scheme_t *) security->scheme, slicer);
            break;
        case SECURITY_SCHEME_OAUTH2:
            _serialize_sec_scheme_oauth2(receiver, (wot_td_oauth2_sec_scheme_t *) security->scheme, slicer);
            break;
    }
}

void _serialize_security(wot_td_serialize_receiver_t receiver, wot_td_sec_scheme_t *scheme, char *lang, wot_td_ser_slicer_t *slicer){
    _wot_td_fill_json_receiver(receiver, "{", 1, slicer);

    _wot_td_fill_json_obj_key(receiver, wot_td_schema_obj_key, sizeof(wot_td_schema_obj_key)-1, slicer);
    _security_scheme_string(receiver, scheme->scheme_type, slicer);
    _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
    _serialize_description_array(receiver, scheme->descriptions, lang, slicer);
    if(scheme->proxy != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, "proxy", sizeof("proxy"), slicer);
        _wot_td_fill_json_uri(receiver, scheme->proxy, slicer);
    }

    if(scheme->scheme_type != SECURITY_SCHEME_NONE){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
    }
    _serialize_security_schema(receiver, scheme, slicer);

    _wot_td_fill_json_receiver(receiver, "}", 1, slicer);
}

void _serialize_security_array(wot_td_serialize_receiver_t receiver, wot_td_security_t *security, char *lang, wot_td_ser_slicer_t *slicer){
    wot_td_security_t *tmp_sec = security;
    wot_td_sec_scheme_t *scheme = NULL;
    _wot_td_fill_json_obj_key(receiver, wot_td_security_def_obj_key, sizeof(wot_td_security_def_obj_key)-1, slicer);
    _wot_td_fill_json_receiver(receiver, "{", 1, slicer);
    while (tmp_sec != NULL){
        scheme = tmp_sec->value;
        _wot_td_fill_json_obj_key(receiver, tmp_sec->key, strlen(tmp_sec->key), slicer);
        _serialize_security(receiver, scheme, lang, slicer);
        tmp_sec = tmp_sec->next;
    }
    _wot_td_fill_json_receiver(receiver, "}", 1, slicer);

    _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
    _wot_td_fill_json_obj_key(receiver, wot_td_security_def_obj_key, 8, slicer);
    _wot_td_fill_json_receiver(receiver, "[", 1, slicer);
    tmp_sec = security;
    while (tmp_sec != NULL){
        _wot_td_fill_json_string(receiver, tmp_sec->key, strlen(tmp_sec->key), slicer);
        tmp_sec = tmp_sec->next;
    }
    _wot_td_fill_json_receiver(receiver, "]", 1, slicer);
}

//Fixme: Use const vars
void _form_op_type_string(wot_td_serialize_receiver_t receiver, wot_td_form_op_type_t op_type, wot_td_ser_slicer_t *slicer){
    switch (op_type) {
        case FORM_OP_READ_PROPERTY:
            _wot_td_fill_json_string(receiver, "readproperty", sizeof("readproperty")-1, slicer);
            break;
        case FORM_OP_WRITE_PROPERTY:
            _wot_td_fill_json_string(receiver, "writeproperty", sizeof("writeproperty")-1, slicer);
            break;
        case FORM_OP_OBSERVE_PROPERTY:
            _wot_td_fill_json_string(receiver, "writeproperty", sizeof("writeproperty")-1, slicer);
            break;
        case FORM_OP_UNOBSERVE_PROPERTY:
            _wot_td_fill_json_string(receiver, "unobserveproperty", sizeof("unobserveproperty")-1, slicer);
            break;
        case FORM_OP_INVOKE_ACTION:
            _wot_td_fill_json_string(receiver, "invokeaction", sizeof("invokeaction")-1, slicer);
            break;
        case FORM_OP_SUBSCRIBE_EVENT:
            _wot_td_fill_json_string(receiver, "subscribeevent", sizeof("subscribeevent")-1, slicer);
            break;
        case FORM_OP_UNSUBSCRIBE_EVENT:
            _wot_td_fill_json_string(receiver, "unsubscribeevent", sizeof("unsubscribeevent")-1, slicer);
            break;
        case FORM_OP_READ_ALL_PROPERTIES:
            _wot_td_fill_json_string(receiver, "readallproperties", sizeof("readallproperties")-1, slicer);
            break;
        case FORM_OP_WRITE_ALL_PROPERTIES:
            _wot_td_fill_json_string(receiver, "writeallproperties", sizeof("writeallproperties")-1, slicer);
            break;
        case FORM_OP_READ_MULTIPLE_PROPERTIES:
            _wot_td_fill_json_string(receiver, "readmultipleproperties", sizeof("readmultipleproperties")-1, slicer);
            break;
        case FORM_OP_WRITE_MULTIPLE_PROPERTIES:
            _wot_td_fill_json_string(receiver, "writemultipleproperties", sizeof("writemultipleproperties")-1, slicer);
            break;
        default:
            _wot_td_fill_json_string(receiver, " ", sizeof(" "), slicer);
            break;
    }
}

void _serialize_op_array(wot_td_serialize_receiver_t receiver, wot_td_form_op_t *op, wot_td_ser_slicer_t *slicer){
    wot_td_form_op_t *tmp = op;
    _wot_td_fill_json_receiver(receiver, "[", 1, slicer);
    while(tmp != NULL){
        _form_op_type_string(receiver, tmp->op_type, slicer);
        tmp = tmp->next;
    }
    _wot_td_fill_json_receiver(receiver, "]", 1, slicer);
}

void _content_type_string(wot_td_serialize_receiver_t receiver, wot_td_content_type_t *content_type, wot_td_ser_slicer_t *slicer){
    _wot_td_fill_json_receiver(receiver, "\"", 1, slicer);
    switch (content_type->media_type) {
        case CONTENT_TYPE_JSON:
            _wot_td_fill_json_receiver(receiver, "application/json", sizeof("application/json")-1, slicer);
            break;
        case CONTENT_TYPE_TEXT_PLAIN:
            _wot_td_fill_json_receiver(receiver, "text/plain", sizeof("text/plain")-1, slicer);
            break;
        case CONTENT_TYPE_JSON_LD:
            _wot_td_fill_json_receiver(receiver, "application/ld+json", sizeof("application/ld+json")-1, slicer);
            break;
        case CONTENT_TYPE_CSV:
            _wot_td_fill_json_receiver(receiver, "text/csv", sizeof("text/csv")-1, slicer);
            break; 
        default:
            _wot_td_fill_json_string(receiver, "", sizeof("")-1, slicer);
            break;
    }
    if(content_type->media_type_paramter != NULL){
        _wot_td_fill_json_receiver(receiver, ";", 1, slicer);
        wot_td_media_type_parameter_t *tmp = content_type->media_type_paramter;
        while(tmp != NULL){
            _wot_td_fill_json_receiver(receiver, tmp->key, strlen(tmp->key), slicer);
            _wot_td_fill_json_receiver(receiver, "=", sizeof("="), slicer);
            _wot_td_fill_json_receiver(receiver, tmp->value, strlen(tmp->value), slicer);
            tmp = tmp->next;
        }
    }
    _wot_td_fill_json_receiver(receiver, "\"", 1, slicer);
}

void _content_encoding_string(wot_td_serialize_receiver_t receiver, wot_td_content_encoding_type_t encoding, wot_td_ser_slicer_t *slicer){
    switch (encoding) {
        case CONTENT_ENCODING_GZIP:
            _wot_td_fill_json_string(receiver, "gzip", sizeof("gzip")-1, slicer);
            break;
        case CONTENT_ENCODING_COMPRESS:
            _wot_td_fill_json_string(receiver, "compress", sizeof("compress")-1, slicer);
            break;
        case CONTENT_ENCODING_DEFLATE:
            _wot_td_fill_json_string(receiver, "deflate", sizeof("deflate")-1, slicer);
            break;
        case CONTENT_ENCODING_IDENTITY:
            _wot_td_fill_json_string(receiver, "identity", sizeof("identity")-1, slicer);
            break;
        case CONTENT_ENCODING_BROTLI:
            _wot_td_fill_json_string(receiver, "br", sizeof("br")-1, slicer);
            break;
        default:
            _wot_td_fill_json_string(receiver, "", sizeof("")-1, slicer);
            break;
    }
}

void _serialize_expected_response(wot_td_serialize_receiver_t receiver, wot_td_expected_res_t *res, wot_td_ser_slicer_t *slicer){
    _wot_td_fill_json_receiver(receiver, "{", 1, slicer);
    _wot_td_fill_json_obj_key(receiver, "contentType", sizeof("contentType")-1, slicer);
    _content_type_string(receiver, res->content_type, slicer);
    _wot_td_fill_json_receiver(receiver, "}", 1, slicer);
}

void _serialize_form_array(wot_td_serialize_receiver_t receiver, wot_td_form_t *form, wot_td_ser_slicer_t *slicer){
    wot_td_form_t *tmp = form;

    _wot_td_fill_json_receiver(receiver, "[", 1, slicer);
    while (tmp != NULL){
        _wot_td_fill_json_receiver(receiver, "{", 1, slicer);

        if(tmp->op != NULL){
            _wot_td_fill_json_obj_key(receiver, wot_td_op_obj_key, sizeof(wot_td_op_obj_key)-1, slicer);
            _serialize_op_array(receiver, tmp->op, slicer);
        }

        if(tmp->href != NULL && tmp->href->value != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
            _wot_td_fill_json_obj_key(receiver, wot_td_href_obj_key, sizeof(wot_td_href_obj_key)-1, slicer);
            _wot_td_fill_json_uri(receiver, tmp->href, slicer);
        }

        if(tmp->content_type != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
            _wot_td_fill_json_obj_key(receiver, wot_td_content_type_obj_key, sizeof(wot_td_content_type_obj_key)-1, slicer);
            _content_type_string(receiver, tmp->content_type, slicer);
        }

        if(tmp->content_encoding != CONTENT_ENCODING_NONE){
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
            _wot_td_fill_json_obj_key(receiver, wot_td_content_encoding_obj_key, sizeof(wot_td_content_encoding_obj_key)-1, slicer);
            _content_encoding_string(receiver, tmp->content_encoding, slicer);
        }

        if(tmp->sub_protocol != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
            _wot_td_fill_json_obj_key(receiver, wot_td_sub_protocol_obj_key, sizeof(wot_td_sub_protocol_obj_key)-1, slicer);
            _wot_td_fill_json_string(receiver, tmp->sub_protocol, strlen(tmp->sub_protocol), slicer);
        }

        if(tmp->security != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
            _wot_td_fill_json_obj_key(receiver, wot_td_security_def_obj_key, sizeof(wot_td_security_def_obj_key)-9, slicer);
            wot_td_security_t *sec = tmp->security;
            _wot_td_fill_json_receiver(receiver, "[", 1, slicer);
            while (sec != NULL){
                _wot_td_fill_json_string(receiver, sec->key, strlen(sec->key), slicer);
                sec = sec->next;
            }
            _wot_td_fill_json_receiver(receiver, "]", 1, slicer);
        }

        if(tmp->scopes != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
            _wot_td_fill_json_obj_key(receiver, wot_td_scopes_obj_key, sizeof(wot_td_scopes_obj_key)-1, slicer);
            wot_td_auth_scopes_t *scope = tmp->scopes;
            _wot_td_fill_json_receiver(receiver, "[", 1, slicer);
            while(scope != NULL){
                _wot_td_fill_json_string(receiver, scope->value, strlen(scope->value), slicer);
                scope = scope->next;
            }
            _wot_td_fill_json_receiver(receiver, "]", 1, slicer);
        }

        if(tmp->expected_response != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
            _wot_td_fill_json_obj_key(receiver, "response", sizeof("response")-1, slicer);

        }

        if(tmp->extensions != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
            wot_td_extension_t *extension = tmp->extensions;
            while(extension != NULL){
                wot_td_ser_parser_t parser = extension->parser;
                parser(receiver, extension->name, extension->data);
                extension = extension->next;
            }
        }

        _wot_td_fill_json_receiver(receiver, "}", 1, slicer);
        tmp = tmp->next;
    }
    _wot_td_fill_json_receiver(receiver, "]", 1, slicer);
}

void _serialize_data_schema(wot_td_serialize_receiver_t receiver, wot_td_data_schema_t *data_schema, char *lang, bool as_obj, wot_td_ser_slicer_t *slicer);

void _serialize_int_aff(wot_td_serialize_receiver_t receiver, wot_td_int_affordance_t *int_aff, char *lang, wot_td_ser_slicer_t *slicer){

    _wot_td_fill_json_obj_key(receiver, wot_td_ser_form_obj_key, sizeof(wot_td_ser_form_obj_key)-1, slicer);
    _serialize_form_array(receiver, int_aff->forms, slicer);
    if(int_aff->titles != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _serialize_title_array(receiver, int_aff->titles, lang, slicer);
    }

    if(int_aff->descriptions != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _serialize_description_array(receiver, int_aff->descriptions, lang, slicer);
    }

    if(int_aff->uri_variables != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, "uriVariables", sizeof("uriVariables")-1, slicer);
        wot_td_data_schema_map_t *data_map = int_aff->uri_variables;
        _wot_td_fill_json_receiver(receiver, "{", 1, slicer);
        while (data_map != NULL){
            _wot_td_fill_json_obj_key(receiver, data_map->key, strlen(data_map->key), slicer);
            _serialize_data_schema(receiver, data_map->value, lang, true, slicer);
            data_map = data_map->next;
        }
        _wot_td_fill_json_receiver(receiver, "}", 1, slicer);
    }

    if(int_aff->type != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, wot_td_type_obj_key, sizeof(wot_td_type_obj_key)-1, slicer);
        _wot_td_fill_json_string(receiver, int_aff->type, strlen(int_aff->type), slicer);
    }

}

void _serialize_prop_aff(wot_td_serialize_receiver_t receiver, wot_td_prop_affordance_t *prop_aff, char *lang, wot_td_ser_slicer_t *slicer){
    _wot_td_fill_json_receiver(receiver, "{", 1, slicer);
    _wot_td_fill_json_obj_key(receiver, wot_td_observable_obj_key, sizeof(wot_td_observable_obj_key)-1, slicer);
    _wot_td_fill_json_bool(receiver, prop_aff->observable, slicer);
    _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
    if(prop_aff->data_schema != NULL){
        _serialize_data_schema(receiver, prop_aff->data_schema, lang, false, slicer);
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
    }
    _serialize_int_aff(receiver, prop_aff->int_affordance, lang, slicer);
    _wot_td_fill_json_receiver(receiver, "}", 1, slicer);
}

void _serialize_prop_aff_array(wot_td_serialize_receiver_t receiver, wot_td_prop_affordance_t *prop_aff, char *lang, wot_td_ser_slicer_t *slicer){
    wot_td_prop_affordance_t *tmp = prop_aff;

    _wot_td_fill_json_obj_key(receiver, wot_td_ser_prop_aff_obj_key, sizeof(wot_td_ser_prop_aff_obj_key)-1, slicer);
    _wot_td_fill_json_receiver(receiver, "{", 1, slicer);
    while(tmp != NULL){
        _wot_td_fill_json_obj_key(receiver, tmp->key, strlen(tmp->key), slicer);
        _serialize_prop_aff(receiver, tmp, lang, slicer);
        tmp = tmp->next;
    }

    _wot_td_fill_json_receiver(receiver, "}", 1, slicer);
}


void _serialize_data_schema_object(wot_td_serialize_receiver_t receiver, wot_td_object_schema_t *schema, char *lang, wot_td_ser_slicer_t *slicer){
    _wot_td_fill_json_obj_key(receiver, wot_td_ser_prop_aff_obj_key, sizeof(wot_td_ser_prop_aff_obj_key)-1, slicer);
    wot_td_data_schema_map_t *property = schema->properties;
    _wot_td_fill_json_receiver(receiver, "{", 1, slicer);
    while (property != NULL){
        _wot_td_fill_json_obj_key(receiver, property->key, strlen(property->key), slicer);
        _serialize_data_schema(receiver, property->value, lang, true, slicer);
        if(property->next != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        }
        property = property->next;
    }
    _wot_td_fill_json_receiver(receiver, "}", 1, slicer);
    _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
    _wot_td_fill_json_obj_key(receiver, wot_td_required_obj_key, sizeof(wot_td_required_obj_key)-1, slicer);
    wot_td_object_required_t *required = schema->required;
    _wot_td_fill_json_receiver(receiver, "[", 1, slicer);
    while (required != NULL){
        _wot_td_fill_json_string(receiver, required->value, strlen(required->value), slicer);
        required = required->next;
    }
    _wot_td_fill_json_receiver(receiver, "]", 1, slicer);
}

void _serialize_data_schema_array(wot_td_serialize_receiver_t receiver, wot_td_array_schema_t *schema, char *lang, wot_td_ser_slicer_t *slicer){
    if(schema->items != NULL){
        _wot_td_fill_json_obj_key(receiver, wot_td_item_obj_key, sizeof(wot_td_item_obj_key)-1, slicer);
        wot_td_data_schemas_t *item = schema->items;
        _wot_td_fill_json_receiver(receiver, "[", 1, slicer);
        while (item != NULL){
            _serialize_data_schema(receiver, item->value, lang, true, slicer);
            item = item->next;
        }
        _wot_td_fill_json_receiver(receiver, "]", 1, slicer);
    }

    if(schema->min_items != NULL){
        if(schema->items != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        }
        _wot_td_fill_json_obj_key(receiver, wot_td_min_items_obj_key, sizeof(wot_td_min_items_obj_key) - 1, slicer);
        char min_items_output[32];
        _itoa(*schema->min_items, min_items_output);
        _wot_td_fill_json_string(receiver, min_items_output, strlen(min_items_output), slicer);
    }

    if(schema->max_items != NULL){
        if(schema->min_items != NULL || schema->items != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        }
        _wot_td_fill_json_obj_key(receiver, wot_td_max_items_obj_key, sizeof(wot_td_max_items_obj_key)-1, slicer);
        char max_items_output[32];
        _itoa(*schema->max_items, max_items_output);
        _wot_td_fill_json_string(receiver, max_items_output, strlen(max_items_output), slicer);
    }
}

void _serialize_data_schema_number(wot_td_serialize_receiver_t receiver, wot_td_number_schema_t *schema, wot_td_ser_slicer_t *slicer){
    char buf[16];
    bool has_minimum = false;
    if(schema->minimum != NULL){
        _wot_td_fill_json_obj_key(receiver, wot_td_min_obj_key, sizeof(wot_td_min_obj_key)-1, slicer);
        sprintf(buf, "%f", *schema->minimum); 

    }
    if(schema->maximum != NULL){
        if(has_minimum){
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        }
        
        _wot_td_fill_json_obj_key(receiver, wot_td_max_obj_key, sizeof(wot_td_max_obj_key)-1, slicer);
        memset(buf, 0, 16);
        sprintf(buf, "%f", *schema->maximum);
    }
}

//Todo: Use const vars
void _serialize_data_schema_int(wot_td_serialize_receiver_t receiver, wot_td_integer_schema_t *schema, wot_td_ser_slicer_t *slicer){
    if(schema->minimum != NULL){
        _wot_td_fill_json_obj_key(receiver, wot_td_min_obj_key, sizeof(wot_td_min_obj_key)-1, slicer);
        char min_output[23];
        _itoa(*schema->minimum, min_output);
        _wot_td_fill_json_string(receiver, min_output, strlen(min_output), slicer);
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);

        _wot_td_fill_json_obj_key(receiver, wot_td_max_obj_key, sizeof(wot_td_max_obj_key)-1, slicer);
        char max_output[23];
        _itoa(*schema->minimum, max_output);
        _wot_td_fill_json_string(receiver, max_output, strlen(max_output), slicer);
    }
}

void _serialize_data_schema_subclass(wot_td_serialize_receiver_t receiver, wot_td_data_schema_t *data_schema, char *lang, wot_td_ser_slicer_t *slicer){
    switch (data_schema->json_type) {
        case JSON_TYPE_OBJECT:
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
            _serialize_data_schema_object(receiver, (wot_td_object_schema_t *) data_schema->schema, lang, slicer);
            break;
        case JSON_TYPE_ARRAY:
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
            _serialize_data_schema_array(receiver, (wot_td_array_schema_t *) data_schema->schema, lang, slicer);
            break;
        case JSON_TYPE_NUMBER:
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
            _serialize_data_schema_number(receiver, (wot_td_number_schema_t *) data_schema->schema, slicer);
            break;
        case JSON_TYPE_INTEGER:
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
            _serialize_data_schema_int(receiver, (wot_td_integer_schema_t *) data_schema->schema, slicer);
            break;
        default:
            return;
    }
}

void _serialize_json_type(wot_td_serialize_receiver_t receiver, wot_td_json_type_t json_type, wot_td_ser_slicer_t *slicer) {
    _wot_td_fill_json_obj_key(receiver, wot_td_type_obj_key+1, sizeof(wot_td_type_obj_key)-1, slicer);
    switch(json_type){
        case JSON_TYPE_ARRAY:
            _wot_td_fill_json_string(receiver, "array", sizeof("array")-1, slicer);
            break;
        case JSON_TYPE_OBJECT:
            _wot_td_fill_json_string(receiver, "object", sizeof("object")-1, slicer);
            break;
        case JSON_TYPE_NUMBER:
            _wot_td_fill_json_string(receiver, "number", sizeof("number")-1, slicer);
            break;  
        case JSON_TYPE_INTEGER:
            _wot_td_fill_json_string(receiver, "integer", sizeof("integer")-1, slicer);
            break;
        case JSON_TYPE_NULL:
            _wot_td_fill_json_string(receiver, "null", sizeof("null")-1, slicer);
            break;
        case JSON_TYPE_BOOLEAN:
            _wot_td_fill_json_string(receiver, "boolean", sizeof("boolean")-1, slicer);
            break;
        case JSON_TYPE_STRING:
            _wot_td_fill_json_string(receiver, "string", sizeof("string")-1, slicer);
            break;
        default:
            return;
    }
}

void _serialize_data_schema(wot_td_serialize_receiver_t receiver, wot_td_data_schema_t *data_schema, char *lang, bool as_obj, wot_td_ser_slicer_t *slicer){
    bool has_previous_prop = false;
    if(as_obj){
        _wot_td_fill_json_receiver(receiver, "{", 1, slicer);
    }

    if(data_schema->type != NULL){
        has_previous_prop = true;
        wot_td_type_t *type = data_schema->type;
        _wot_td_fill_json_obj_key(receiver, wot_td_type_obj_key, sizeof(wot_td_type_obj_key)-1, slicer);

        while(type != NULL){
            _wot_td_fill_json_string(receiver, type->value, strlen(type->value), slicer);
            if(type->next != NULL){
                _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
            }
            type = type->next;
        }
    }

    if(data_schema->titles != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _serialize_title_array(receiver, data_schema->titles, lang, slicer);
    }

    if(data_schema->descriptions != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _serialize_description_array(receiver, data_schema->descriptions, lang, slicer);
    }

    if(data_schema->json_type != JSON_TYPE_NONE){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _serialize_json_type(receiver, data_schema->json_type, slicer);
        _serialize_data_schema_subclass(receiver, data_schema, lang, slicer);
    }

    if(data_schema->constant != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _wot_td_fill_json_obj_key(receiver, wot_td_constant_obj_key, sizeof(wot_td_constant_obj_key)-1, slicer);
        _wot_td_fill_json_string(receiver, data_schema->constant, strlen(data_schema->constant), slicer);
    }

    if(data_schema->unit != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _wot_td_fill_json_obj_key(receiver, wot_td_unit_obj_key, sizeof(wot_td_unit_obj_key)-1, slicer);
        _wot_td_fill_json_string(receiver, data_schema->unit, strlen(data_schema->unit), slicer);
    }

    if(data_schema->one_of != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _wot_td_fill_json_obj_key(receiver, wot_td_one_of_obj_key, sizeof(wot_td_one_of_obj_key)-1, slicer);
        wot_td_data_schemas_t *tmp = data_schema->one_of;
        _wot_td_fill_json_string(receiver, "[", 1, slicer);
        while (tmp != NULL){
            _serialize_data_schema(receiver, tmp->value, lang, true, slicer);
            tmp = tmp->next;
        }
        _wot_td_fill_json_string(receiver, "]", 1, slicer);
    }

    if(data_schema->enumeration != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _wot_td_fill_json_obj_key(receiver, wot_td_enum_obj_key, sizeof(wot_td_enum_obj_key)-1, slicer);
        wot_td_data_enums_t *tmp = data_schema->enumeration;
        _wot_td_fill_json_string(receiver, "[", 1, slicer);
        while (tmp != NULL){
            _wot_td_fill_json_string(receiver, tmp->value, strlen(tmp->value), slicer);
            tmp = tmp->next;
        }
        _wot_td_fill_json_string(receiver, "]", 1, slicer);
    }

    _previous_prop_check(receiver, has_previous_prop, slicer);
    _wot_td_fill_json_obj_key(receiver, wot_td_read_only_obj_key, sizeof(wot_td_read_only_obj_key)-1, slicer);
    _wot_td_fill_json_bool(receiver, data_schema->read_only, slicer);

    _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
    _wot_td_fill_json_obj_key(receiver, wot_td_write_only_obj_key, sizeof(wot_td_write_only_obj_key)-1, slicer);
    _wot_td_fill_json_bool(receiver, data_schema->write_only, slicer);

    if(data_schema->format != NULL){
        _wot_td_fill_json_string(receiver, ",", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, wot_td_format_obj_key, sizeof(wot_td_format_obj_key)-1, slicer);
        _wot_td_fill_json_string(receiver, data_schema->format, strlen(data_schema->format), slicer);
    }

    if(as_obj){
        _wot_td_fill_json_receiver(receiver, "}", 1, slicer);
    }
}

void _serialize_action_aff_array(wot_td_serialize_receiver_t receiver, wot_td_action_affordance_t *action_aff, char *lang, wot_td_ser_slicer_t *slicer){
    wot_td_action_affordance_t *tmp = action_aff;

    _wot_td_fill_json_obj_key(receiver, wot_td_action_aff_obj_key, sizeof(wot_td_action_aff_obj_key)-1, slicer);
    _wot_td_fill_json_receiver(receiver, "{", 1, slicer);
    while (tmp != NULL){
        _wot_td_fill_json_obj_key(receiver, tmp->key, strlen(tmp->key), slicer);
        _wot_td_fill_json_receiver(receiver, "{", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, wot_td_input_obj_key, sizeof(wot_td_input_obj_key)-1, slicer);
        _serialize_data_schema(receiver, tmp->input, lang, true, slicer);
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, wot_td_output_obj_key, sizeof(wot_td_output_obj_key)-1, slicer);
        _serialize_data_schema(receiver,tmp->output, lang, true, slicer);
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, wot_td_safe_obj_key, sizeof(wot_td_safe_obj_key)-1, slicer);
        _wot_td_fill_json_bool(receiver, tmp->safe, slicer);
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, wot_td_idempotent_obj_key, sizeof(wot_td_idempotent_obj_key)-1, slicer);
        _wot_td_fill_json_bool(receiver, tmp->idempotent, slicer);
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _serialize_int_aff(receiver, tmp->int_affordance, lang, slicer);
        _wot_td_fill_json_receiver(receiver, "}", 1, slicer);
        tmp = tmp->next;
    }

    _wot_td_fill_json_receiver(receiver, "}", 1, slicer);
}

void _serialize_event_aff_array(wot_td_serialize_receiver_t receiver, wot_td_event_affordance_t *event_aff, char *lang, wot_td_ser_slicer_t *slicer){
    wot_td_event_affordance_t *tmp = event_aff;
    bool has_previous_prop = false;

    _wot_td_fill_json_obj_key(receiver, wot_td_events_obj_key, sizeof(wot_td_events_obj_key)-1, slicer);
    _wot_td_fill_json_receiver(receiver, "{", 1, slicer);
    while (tmp != NULL){
        _wot_td_fill_json_obj_key(receiver, tmp->key, strlen(tmp->key), slicer);
        _wot_td_fill_json_receiver(receiver, "{", 1, slicer);
        if(tmp->subscription != NULL){
            has_previous_prop = true;
            _wot_td_fill_json_obj_key(receiver, wot_td_subscription_obj_key, sizeof(wot_td_subscription_obj_key)-1, slicer);
            _serialize_data_schema(receiver, tmp->subscription, lang, true, slicer);
        }
        if(tmp->data != NULL){
            _previous_prop_check(receiver, has_previous_prop, slicer);
            has_previous_prop = true;
            _wot_td_fill_json_obj_key(receiver, wot_td_data_obj_key, sizeof(wot_td_data_obj_key)-1, slicer);
            _serialize_data_schema(receiver, tmp->data, lang, true, slicer);
        }
        if(tmp->cancellation != NULL){
            _previous_prop_check(receiver, has_previous_prop, slicer);
            has_previous_prop = true;
            _wot_td_fill_json_obj_key(receiver, wot_td_cancellation_obj_key, sizeof(wot_td_cancellation_obj_key)-1, slicer);
            _serialize_data_schema(receiver, tmp->cancellation, lang, true, slicer);
        }
        _previous_prop_check(receiver, has_previous_prop, slicer);
        _serialize_int_aff(receiver, tmp->int_affordance, lang, slicer);
        _wot_td_fill_json_receiver(receiver, "}", 1, slicer);
        if(tmp->next != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        }
        tmp = tmp->next;
    }
    _wot_td_fill_json_receiver(receiver, "}", 1, slicer);
}

void _serialize_link_array(wot_td_serialize_receiver_t receiver, wot_td_link_t *links, wot_td_ser_slicer_t *slicer){
    wot_td_link_t *tmp = links;
    _wot_td_fill_json_receiver(receiver, "[", 1, slicer);

    while(tmp != NULL){
        _wot_td_fill_json_receiver(receiver, "{", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, wot_td_href_obj_key, sizeof(wot_td_href_obj_key)-1, slicer);
        _wot_td_fill_json_uri(receiver, tmp->href, slicer);

        if(tmp->type != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
            _wot_td_fill_json_obj_key(receiver, wot_td_type_obj_key, sizeof(wot_td_type_obj_key)-1, slicer);
            _wot_td_fill_json_string(receiver, tmp->type, strlen(tmp->type), slicer);
        }

        if(tmp->rel != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
            _wot_td_fill_json_obj_key(receiver, wot_td_rel_obj_key, sizeof(wot_td_rel_obj_key)-1, slicer);
            _wot_td_fill_json_string(receiver, tmp->rel, strlen(tmp->rel), slicer);
        }

        if(tmp->anchor != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
            _wot_td_fill_json_obj_key(receiver, wot_td_anchor_obj_key, sizeof(wot_td_anchor_obj_key)-1, slicer);
            _wot_td_fill_json_uri(receiver, tmp->anchor, slicer);
        }

        _wot_td_fill_json_receiver(receiver, "}", 1, slicer);
        if(tmp->next != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        }
        tmp = tmp->next;
    }

    _wot_td_fill_json_receiver(receiver, "]", 1, slicer);
}

int wot_td_serialize_thing(wot_td_serialize_receiver_t receiver, wot_td_thing_t *thing, wot_td_ser_slicer_t *slicer){
    //Todo: Check for all necessary properties, before continue processing

    bool has_previous_prop = false;
    _wot_td_fill_json_receiver(receiver, "{", 1, slicer);

    has_previous_prop = true;
    _wot_td_fill_json_obj_key(receiver, wot_td_ser_obj_context_key, sizeof(wot_td_ser_obj_context_key)-1, slicer);

    if(thing->context != NULL){
        _wot_td_fill_json_receiver(receiver, "[", 1, slicer);
        _wot_td_fill_json_string(receiver, wot_td_ser_w3c_context_value, sizeof(wot_td_ser_w3c_context_value)-1, slicer);
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _serialize_context_array(receiver, thing->context, slicer);
        _wot_td_fill_json_receiver(receiver, "]", 1, slicer);
    }else{
        _wot_td_fill_json_string(receiver, wot_td_ser_w3c_context_value, sizeof(wot_td_ser_w3c_context_value)-1, slicer);
    }

    if(thing->security != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _serialize_security_array(receiver, thing->security, thing->default_language_tag, slicer);
    }else{
        return 1;
    }

    if(thing->type != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _serialize_type_array(receiver, thing->type, slicer);
    }

    if(thing->id != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _wot_td_fill_json_obj_key(receiver, wot_td_ser_id_obj_key, sizeof(wot_td_ser_id_obj_key)-1, slicer);
        _wot_td_fill_json_uri(receiver, thing->id, slicer);
    }

    if(thing->titles != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        has_previous_prop = true;
        _serialize_title_array(receiver, thing->titles, thing->default_language_tag, slicer);
    }

    if(thing->descriptions != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        has_previous_prop = true;
        _serialize_description_array(receiver, thing->descriptions, thing->default_language_tag, slicer);
    }

    if(thing->properties != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _serialize_prop_aff_array(receiver, thing->properties, thing->default_language_tag, slicer);
    }

    if(thing->actions != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _serialize_action_aff_array(receiver, thing->actions, thing->default_language_tag, slicer);
    }

    if(thing->events != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _serialize_event_aff_array(receiver, thing->events, thing->default_language_tag, slicer);
    }

    if(thing->links != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        _wot_td_fill_json_obj_key(receiver, wot_td_ser_links_obj_key, sizeof(wot_td_ser_links_obj_key)-1, slicer);
        _serialize_link_array(receiver, thing->links, slicer);
    }

    if(thing->base != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        _wot_td_fill_json_obj_key(receiver, wot_td_ser_base_obj_key, sizeof(wot_td_ser_base_obj_key)-1, slicer);
        _wot_td_fill_json_uri(receiver, thing->base, slicer);
    }

    if(thing->support != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, wot_td_ser_support_obj_key, sizeof(wot_td_ser_support_obj_key) - 1, slicer);
        _wot_td_fill_json_uri(receiver, thing->support, slicer);
    }

    if(thing->version != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, wot_td_ser_version_obj_key, sizeof(wot_td_ser_version_obj_key)-1, slicer);
        _wot_td_fill_json_receiver(receiver, "{", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, wot_td_ser_instance_obj_key, sizeof(wot_td_ser_instance_obj_key)-1, slicer);
        _wot_td_fill_json_string(receiver, thing->version->instance, strlen(thing->version->instance), slicer);
        _wot_td_fill_json_receiver(receiver, "}", 1, slicer);
    }

    if(thing->forms != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, wot_td_ser_form_obj_key, sizeof(wot_td_ser_form_obj_key)-1, slicer);
        _serialize_form_array(receiver, thing->forms, slicer);
    }

    if(thing->created != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, wot_td_created_obj_key, sizeof(wot_td_created_obj_key)-1, slicer);
        _wot_td_fill_json_date(receiver, thing->created, slicer);
    }

    if(thing->modified != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1, slicer);
        _wot_td_fill_json_obj_key(receiver, wot_td_modified_obj_key, sizeof(wot_td_modified_obj_key)-1, slicer);
        _wot_td_fill_json_date(receiver, thing->modified, slicer);
    }

    _wot_td_fill_json_receiver(receiver, "}", 1, slicer);

    return 0;
}
