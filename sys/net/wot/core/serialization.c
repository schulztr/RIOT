#include "net/wot.h"
#include "net/wot/serialization.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    _reverse(s);
}

void _wot_td_fill_json_receiver(wot_td_serialize_receiver_t receiver, const char *string, uint32_t length){
    receiver(string, length);
}

void _wot_td_fill_json_string(wot_td_serialize_receiver_t receiver, const char *string, uint32_t length){
    _wot_td_fill_json_receiver(receiver, "\"", 1);
    _wot_td_fill_json_receiver(receiver, string, length);
    _wot_td_fill_json_receiver(receiver, "\"", 1);
}

void _wot_td_fill_json_uri(wot_td_serialize_receiver_t receiver, wot_td_uri_t *uri){
    _wot_td_fill_json_receiver(receiver, "\"", 1);
    _wot_td_fill_json_receiver(receiver, uri->schema, strlen(uri->schema));
    _wot_td_fill_json_receiver(receiver, uri->value, strlen(uri->value));
    _wot_td_fill_json_receiver(receiver, "\"", 1);
}

void _wot_td_fill_json_date(wot_td_serialize_receiver_t receiver, wot_td_date_time_t *date){
    _wot_td_fill_json_receiver(receiver, "\"", 1);
    char s[11];
    _itoa(date->year, s);
    _wot_td_fill_json_receiver(receiver, s, strlen(s));
    _wot_td_fill_json_receiver(receiver, "-", 1);
    _itoa(date->month, s);
    _wot_td_fill_json_receiver(receiver, s, strlen(s));
    _wot_td_fill_json_receiver(receiver, "-", 1);
    _itoa(date->day, s);
    _wot_td_fill_json_receiver(receiver, s, strlen(s));
    _wot_td_fill_json_receiver(receiver, "T", 1);
    _itoa(date->hour, s);
    _wot_td_fill_json_receiver(receiver, s, strlen(s));
    _wot_td_fill_json_receiver(receiver, ":", 1);
    _itoa(date->minute, s);
    _wot_td_fill_json_receiver(receiver, s, strlen(s));
    _wot_td_fill_json_receiver(receiver, ":", 1);
    _itoa(date->second, s);
    _wot_td_fill_json_receiver(receiver, s, strlen(s));
    _itoa(date->timezone_offset, s);
    _wot_td_fill_json_receiver(receiver, s, strlen(s));

    _wot_td_fill_json_receiver(receiver, "\"", 1);
}

void _wot_td_fill_json_obj_key(wot_td_serialize_receiver_t receiver, const char *string, uint32_t length){
    _wot_td_fill_json_string(receiver, string, length);
    _wot_td_fill_json_receiver(receiver, ":", 1);
}

void _wot_td_fill_json_bool(wot_td_serialize_receiver_t receiver, bool value){
    if(value){
        char c[] = "true";
        _wot_td_fill_json_receiver(receiver, c, sizeof(c)-1);
    }else{
        char c[] = "false";
        _wot_td_fill_json_receiver(receiver, c, sizeof(c)-1);
    }
}

void _previous_prop_check(wot_td_serialize_receiver_t receiver, bool has_previous_prop){
    if(has_previous_prop){
        return _wot_td_fill_json_receiver(receiver, ",", 1);
    }
}

void _serialize_context(wot_td_serialize_receiver_t receiver, json_ld_context_t *context){
    if(context->key != NULL){
        _wot_td_fill_json_receiver(receiver, "{", 1);

        _wot_td_fill_json_obj_key(receiver, context->key, strlen(context->key));
    }

    _wot_td_fill_json_string(receiver, context->value, strlen(context->value));

    if(context->key != NULL) {
        _wot_td_fill_json_receiver(receiver, "}", 1);
    }
}

void _serialize_context_array(wot_td_serialize_receiver_t receiver, json_ld_context_t *context){
    json_ld_context_t *tmp_ctx = context;

    while(tmp_ctx != NULL){
        _serialize_context(receiver, tmp_ctx);
        if(tmp_ctx != NULL && tmp_ctx->next != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1);
        }
        tmp_ctx = tmp_ctx->next;
    }
}

void _serialize_type_array(wot_td_serialize_receiver_t receiver, wot_td_type_t *type){
    char type_obj_key[] = "@type";
    _wot_td_fill_json_obj_key(receiver, type_obj_key, sizeof(type_obj_key)-1);
    _wot_td_fill_json_receiver(receiver, "[", 1);
    wot_td_type_t *tmp_type = type;
    while(tmp_type != NULL){
        _wot_td_fill_json_string(receiver, tmp_type->value, strlen(tmp_type->value));
        if(tmp_type != NULL && tmp_type->next != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1);
        }
        tmp_type = tmp_type->next;
    }
    _wot_td_fill_json_receiver(receiver, "]", 1);
}

void _serialize_lang(wot_td_serialize_receiver_t receiver, wot_td_multi_lang_t *lang){
    _wot_td_fill_json_obj_key(receiver, lang->tag, strlen(lang->tag));
    _wot_td_fill_json_string(receiver, lang->value, strlen(lang->value));
}

void _serialize_title_array(wot_td_serialize_receiver_t receiver, wot_td_multi_lang_t *titles, char *lang){
    wot_td_multi_lang_t *tmp = titles;
    wot_td_multi_lang_t *default_title = NULL;
    char title_obj_key[] = "titles";
    _wot_td_fill_json_obj_key(receiver, title_obj_key, sizeof(title_obj_key)-1);

    _wot_td_fill_json_receiver(receiver, "{", 1);
    while(tmp != NULL){
        if(lang == NULL || strcmp(tmp->tag, lang) != 0){
            _serialize_lang(receiver, tmp);
        }else{
            default_title = tmp;
        }

        if(tmp->next != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1);
        }
        tmp = tmp->next;
    }
    _wot_td_fill_json_receiver(receiver, "}", 1);

    if(default_title != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1);
        char key[] = "title";
        _wot_td_fill_json_obj_key(receiver, key, sizeof(key)-1);
        _wot_td_fill_json_string(receiver, default_title->value, strlen(default_title->value));
    }
}

void _serialize_description_array(wot_td_serialize_receiver_t receiver, wot_td_multi_lang_t *desc, char *lang){
    wot_td_multi_lang_t *tmp = desc;
    wot_td_multi_lang_t *default_description = NULL;
    char description_obj_key[] = "descriptions";
    _wot_td_fill_json_obj_key(receiver, description_obj_key, sizeof(description_obj_key)-1);
    _wot_td_fill_json_receiver(receiver, "{", 1);
    while(tmp != NULL){
        if(lang == NULL || strcmp(tmp->tag, lang) != 0){
            _serialize_lang(receiver, tmp);
        }else{
            default_description = tmp;
        }

        if(tmp->next != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1);
        }
        tmp = tmp->next;
    }
    _wot_td_fill_json_receiver(receiver, "}", 1);

    if(default_description != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1);
        char key[] = "description";
        _wot_td_fill_json_obj_key(receiver, key, sizeof(key)-1);
        _wot_td_fill_json_string(receiver, default_description->value, strlen(default_description->value));
    }
}

void _security_scheme_string(wot_td_serialize_receiver_t receiver, wot_td_sec_scheme_type_t scheme_type){
    switch (scheme_type) {
        case SECURITY_SCHEME_NONE:
            _wot_td_fill_json_string(receiver, "nosec", sizeof("nosec"));
            break;
        case SECURITY_SCHEME_BASIC:
            _wot_td_fill_json_string(receiver, "basic", sizeof("basic"));
            break;
        case SECURITY_SCHEME_DIGEST:
            _wot_td_fill_json_string(receiver, "digest", sizeof("digest"));
            break;
        case SECURITY_SCHEME_API_KEY:
            _wot_td_fill_json_string(receiver, "apikey", sizeof("apikey"));
            break;
        case SECURITY_SCHEME_BEARER:
            _wot_td_fill_json_string(receiver, "bearer", sizeof("bearer"));
            break;
        case SECURITY_SCHEME_PSK:
            _wot_td_fill_json_string(receiver, "psk", sizeof("psk"));
            break;
        case SECURITY_SCHEME_OAUTH2:
            _wot_td_fill_json_string(receiver, "oauth2", sizeof("oauth2"));
            break;
        default:
            _wot_td_fill_json_string(receiver, "nosec", sizeof("nosec"));
            break;
    }
}

void _security_schema_in_string(wot_td_serialize_receiver_t receiver, wot_td_sec_scheme_in_t in){
    switch(in){
        case SECURITY_SCHEME_IN_HEADER:
            _wot_td_fill_json_string(receiver, "header", sizeof("header"));
            break;
        case SECURITY_SCHEME_IN_QUERY:
            _wot_td_fill_json_string(receiver, "query", sizeof("query"));
            break;
        case SECURITY_SCHEME_IN_BODY:
            _wot_td_fill_json_string(receiver, "body", sizeof("body"));
            break;
        case SECURITY_SCHEME_IN_COOKIE:
            _wot_td_fill_json_string(receiver, "cookie", sizeof("cookie"));
            break;
        default:
            _wot_td_fill_json_string(receiver, "header", sizeof("header"));
    }
}

void _serialize_sec_scheme_basic(wot_td_serialize_receiver_t receiver, wot_td_basic_sec_scheme_t *scheme){
    char in_obj_key[] = "in";
    char name_obj_key[] = "name";


    _wot_td_fill_json_obj_key(receiver, in_obj_key, sizeof(in_obj_key)-1);
    _security_schema_in_string(receiver, scheme->in);

    _wot_td_fill_json_receiver(receiver, ",", 1);
    _wot_td_fill_json_obj_key(receiver, name_obj_key, sizeof(name_obj_key)-1);
    _wot_td_fill_json_string(receiver, scheme->name, strlen(scheme->name));
}

void _security_digest_qop_string(wot_td_serialize_receiver_t receiver, wot_td_digest_qop_t qop){
    if(qop == SECURITY_DIGEST_QOP_AUTH_INT){
        _wot_td_fill_json_string(receiver, "auth-int", sizeof("auth-int"));
    }else{
        _wot_td_fill_json_string(receiver, "auth", sizeof("auth"));
    }
}

void _serialize_sec_scheme_digest(wot_td_serialize_receiver_t receiver, wot_td_digest_sec_scheme_t *scheme){
    char in_obj_key[] = "in";
    char name_obj_key[] = "name";
    char qop_obj_key[] = "qop";

    _wot_td_fill_json_obj_key(receiver, qop_obj_key, sizeof(qop_obj_key)-1);
    _security_digest_qop_string(receiver, scheme->qop);

    _wot_td_fill_json_receiver(receiver, ",", 1);
    _wot_td_fill_json_obj_key(receiver, in_obj_key, sizeof(in_obj_key)-1);
    _security_schema_in_string(receiver, scheme->in);
    _wot_td_fill_json_receiver(receiver, ",", 1);
    _wot_td_fill_json_obj_key(receiver, name_obj_key, sizeof(name_obj_key)-1);
    _wot_td_fill_json_string(receiver, scheme->name, strlen(scheme->name));
}

//Todo: Implement
void _serialize_sec_scheme_api_key(wot_td_serialize_receiver_t receiver, wot_td_api_key_sec_scheme_t *scheme){
    (void)receiver;
    (void)scheme;
}

//Todo: Implement
void _serialize_sec_scheme_bearer(wot_td_serialize_receiver_t receiver, wot_td_bearer_sec_scheme_t *scheme){
    (void)receiver;
    (void)scheme;
}

//Todo: Implement
void _serialize_sec_scheme_psk(wot_td_serialize_receiver_t receiver, wot_td_psk_sec_scheme_t *scheme){
    (void)receiver;
    (void)scheme;
}

//Todo: Implement
void _serialize_sec_scheme_oauth2(wot_td_serialize_receiver_t receiver, wot_td_oauth2_sec_scheme_t *scheme){
    (void)receiver;
    (void)scheme;
}

void _serialize_security_schema(wot_td_serialize_receiver_t receiver, wot_td_sec_scheme_t *security){
    switch (security->scheme_type) {
        default:
            return;
        case SECURITY_SCHEME_BASIC:
            _serialize_sec_scheme_basic(receiver, (wot_td_basic_sec_scheme_t *) security->scheme);
            break;
        case SECURITY_SCHEME_DIGEST:
            _serialize_sec_scheme_digest(receiver, (wot_td_digest_sec_scheme_t *) security->scheme);
            break;
        case SECURITY_SCHEME_API_KEY:
            _serialize_sec_scheme_api_key(receiver, (wot_td_api_key_sec_scheme_t *) security->scheme);
            break;
        case SECURITY_SCHEME_BEARER:
            _serialize_sec_scheme_bearer(receiver, (wot_td_bearer_sec_scheme_t *) security->scheme);
            break;
        case SECURITY_SCHEME_PSK:
            _serialize_sec_scheme_psk(receiver, (wot_td_psk_sec_scheme_t *) security->scheme);
            break;
        case SECURITY_SCHEME_OAUTH2:
            _serialize_sec_scheme_oauth2(receiver, (wot_td_oauth2_sec_scheme_t *) security->scheme);
            break;
    }
}

//Todo: Full implementation
void _serialize_security_array(wot_td_serialize_receiver_t receiver, wot_td_security_t *security, char *lang){
    wot_td_security_t *tmp_sec = security;
    wot_td_sec_scheme_t *scheme = NULL;
    char schema_obj_key[] = "scheme";
    char security_def_obj_key[] = "securityDefinitions";
    _wot_td_fill_json_obj_key(receiver, security_def_obj_key, sizeof(security_def_obj_key)-1);
    _wot_td_fill_json_receiver(receiver, "{", 1);
    while (tmp_sec != NULL){
        scheme = tmp_sec->value;
        _wot_td_fill_json_obj_key(receiver, tmp_sec->key, strlen(tmp_sec->key));
        _wot_td_fill_json_receiver(receiver, "{", 1);
        _wot_td_fill_json_obj_key(receiver, schema_obj_key, sizeof(schema_obj_key)-1);
        _security_scheme_string(receiver, scheme->scheme_type);
        _wot_td_fill_json_receiver(receiver, ",", 1);
        _serialize_description_array(receiver, scheme->descriptions, lang);
        _wot_td_fill_json_receiver(receiver, ",", 1);
        _serialize_security_schema(receiver, scheme);
        _wot_td_fill_json_receiver(receiver, "}", 1);
        tmp_sec = tmp_sec->next;
    }
    _wot_td_fill_json_receiver(receiver, "}", 1);

    _wot_td_fill_json_receiver(receiver, ",", 1);
    char security_obj_key[] = "security";
    _wot_td_fill_json_obj_key(receiver, security_obj_key, sizeof(security_obj_key)-1);
    _wot_td_fill_json_receiver(receiver, "[", 1);
    tmp_sec = security;
    while (tmp_sec != NULL){
        scheme = tmp_sec->value;
        _security_scheme_string(receiver, scheme->scheme_type);
        tmp_sec = tmp_sec->next;
    }
    _wot_td_fill_json_receiver(receiver, "]", 1);
}

void _form_op_type_string(wot_td_serialize_receiver_t receiver, wot_td_form_op_type_t op_type){
    switch (op_type) {
        case FORM_OP_READ_PROPERTY:
            _wot_td_fill_json_string(receiver, "readproperty", sizeof("readproperty"));
            break;
        case FORM_OP_WRITE_PROPERTY:
            _wot_td_fill_json_string(receiver, "writeproperty", sizeof("writeproperty"));
            break;
        case FORM_OP_OBSERVE_PROPERTY:
            _wot_td_fill_json_string(receiver, "writeproperty", sizeof("writeproperty"));
            break;
        case FORM_OP_UNOBSERVE_PROPERTY:
            _wot_td_fill_json_string(receiver, "unobserveproperty", sizeof("unobserveproperty"));
            break;
        case FORM_OP_INVOKE_ACTION:
            _wot_td_fill_json_string(receiver, "invokeaction", sizeof("invokeaction"));
            break;
        case FORM_OP_SUBSCRIBE_EVENT:
            _wot_td_fill_json_string(receiver, "subscribeevent", sizeof("subscribeevent"));
            break;
        case FORM_OP_UNSUBSCRIBE_EVENT:
            _wot_td_fill_json_string(receiver, "unsubscribeevent", sizeof("unsubscribeevent"));
            break;
        case FORM_OP_READ_ALL_PROPERTIES:
            _wot_td_fill_json_string(receiver, "readallproperties", sizeof("readallproperties"));
            break;
        case FORM_OP_WRITE_ALL_PROPERTIES:
            _wot_td_fill_json_string(receiver, "writeallproperties", sizeof("writeallproperties"));
            break;
        case FORM_OP_READ_MULTIPLE_PROPERTIES:
            _wot_td_fill_json_string(receiver, "readmultipleproperties", sizeof("readmultipleproperties"));
            break;
        case FORM_OP_WRITE_MULTIPLE_PROPERTIES:
            _wot_td_fill_json_string(receiver, "writemultipleproperties", sizeof("writemultipleproperties"));
            break;
        default:
            _wot_td_fill_json_string(receiver, "", sizeof(""));
            break;
    }
}

void _serialize_op_array(wot_td_serialize_receiver_t receiver, wot_td_form_op_t *op){
    wot_td_form_op_t *tmp = op;
    _wot_td_fill_json_receiver(receiver, "[", 1);
    while(tmp != NULL){
        _form_op_type_string(receiver, tmp->op_type);
        tmp = tmp->next;
    }
    _wot_td_fill_json_receiver(receiver, "]", 1);
}

void _content_type_string(wot_td_serialize_receiver_t receiver, wot_td_content_type_t content_type){
    switch (content_type) {
        case CONTENT_TYPE_JSON:
            _wot_td_fill_json_string(receiver, "application/json", sizeof("application/json"));
            break;
        default:
            _wot_td_fill_json_string(receiver, "", sizeof(""));
            break;
    }
}

void _content_encoding_string(wot_td_serialize_receiver_t receiver, wot_td_content_encoding_type_t encoding){
    switch (encoding) {
        case CONTENT_ENCODING_GZIP:
            _wot_td_fill_json_string(receiver, "gzip", sizeof("gzip"));
            break;
        default:
            _wot_td_fill_json_string(receiver, "", sizeof(""));
            break;
    }
}

void _serialize_form_array(wot_td_serialize_receiver_t receiver, wot_td_form_t *form){
    wot_td_form_t *tmp = form;
    char op_obj_key[] = "op";
    char href_obj_key[] = "href";

    _wot_td_fill_json_receiver(receiver, "[", 1);
    while (tmp != NULL){
        _wot_td_fill_json_receiver(receiver, "{", 1);

        if(tmp->op != NULL){
            _wot_td_fill_json_obj_key(receiver, op_obj_key, sizeof(op_obj_key)-1);
            _serialize_op_array(receiver, tmp->op);
            _wot_td_fill_json_receiver(receiver, ",", 1);
        }
        _wot_td_fill_json_obj_key(receiver, href_obj_key, sizeof(href_obj_key)-1);
        _wot_td_fill_json_uri(receiver, tmp->href);

        if(tmp->content_type != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1);
            char content_type_obj_key[] = "contentType";
            _wot_td_fill_json_obj_key(receiver, content_type_obj_key, sizeof(content_type_obj_key)-1);
            _content_type_string(receiver, *tmp->content_type);
        }

        if(tmp->content_encoding != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1);
            char content_encoding_obj_key[] = "contentCoding";
            _wot_td_fill_json_obj_key(receiver, content_encoding_obj_key, sizeof(content_encoding_obj_key)-1);
            _content_encoding_string(receiver, *tmp->content_encoding);
        }

        if(tmp->sub_protocol != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1);
            char sub_protocol_obj_key[] = "subprotocol";
            _wot_td_fill_json_obj_key(receiver, sub_protocol_obj_key, sizeof(sub_protocol_obj_key)-1);
            _wot_td_fill_json_string(receiver, tmp->sub_protocol, strlen(tmp->sub_protocol));
        }

        if(tmp->security != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1);
            char security_obj_key[] = "security";
            _wot_td_fill_json_obj_key(receiver, security_obj_key, sizeof(security_obj_key)-1);
            wot_td_security_t *sec = tmp->security;
            _wot_td_fill_json_receiver(receiver, "[", 1);
            while (sec != NULL){
                _wot_td_fill_json_string(receiver, sec->key, strlen(sec->key));
                sec = sec->next;
            }
            _wot_td_fill_json_receiver(receiver, "]", 1);
        }

        if(tmp->scopes != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1);
            char scopes_obj_key[] = "scopes";
            _wot_td_fill_json_obj_key(receiver, scopes_obj_key, sizeof(scopes_obj_key)-1);
            wot_td_auth_scopes_t *scope = tmp->scopes;
            _wot_td_fill_json_receiver(receiver, "[", 1);
            while(scope != NULL){
                _wot_td_fill_json_string(receiver, scope->value, strlen(scope->value));
                scope = scope->next;
            }
            _wot_td_fill_json_receiver(receiver, "]", 1);
        }

        //Todo: Continue

        _wot_td_fill_json_receiver(receiver, "}", 1);
        tmp = tmp->next;
    }
    _wot_td_fill_json_receiver(receiver, "]", 1);
}

//Todo: Implement other properties
void _serialize_int_aff(wot_td_serialize_receiver_t receiver, wot_td_int_affordance_t *int_aff){
    char forms_obj_key[] = "forms";
    _wot_td_fill_json_obj_key(receiver, forms_obj_key, sizeof(forms_obj_key)-1);
    _serialize_form_array(receiver, int_aff->forms);
}

void _serialize_prop_aff_array(wot_td_serialize_receiver_t receiver, wot_td_prop_affordance_t *prop_aff){
    wot_td_prop_affordance_t *tmp = prop_aff;
    char prop_aff_obj_key[] = "properties";
    char observable_obj_key[] = "observable";

    _wot_td_fill_json_obj_key(receiver, prop_aff_obj_key, sizeof(prop_aff_obj_key)-1);
    _wot_td_fill_json_receiver(receiver, "{", 1);
    while(tmp != NULL){
        _wot_td_fill_json_obj_key(receiver, tmp->key, strlen(tmp->key));
        _wot_td_fill_json_receiver(receiver, "{", 1);
        _wot_td_fill_json_obj_key(receiver, observable_obj_key, sizeof(observable_obj_key)-1);
        _wot_td_fill_json_bool(receiver, tmp->observable);
        _wot_td_fill_json_receiver(receiver, ",", 1);
        _serialize_int_aff(receiver, tmp->int_affordance);
        _wot_td_fill_json_receiver(receiver, "}", 1);
        tmp = tmp->next;
    }

    _wot_td_fill_json_receiver(receiver, "}", 1);
}

void _serialize_data_schema(wot_td_serialize_receiver_t receiver, wot_td_data_schema_t *data_schema, char *lang);

void _serialize_data_schema_object(wot_td_serialize_receiver_t receiver, wot_td_object_schema_t *schema, char *lang){
    char properties_obj_key[] = "properties";
    _wot_td_fill_json_obj_key(receiver, properties_obj_key, sizeof(properties_obj_key)-1);
    wot_td_data_schema_map_t *property = schema->properties;
    _wot_td_fill_json_receiver(receiver, "{", 1);
    while (property != NULL){
        _wot_td_fill_json_obj_key(receiver, property->key, strlen(property->key));
        _serialize_data_schema(receiver, property->value, lang);
        property = property->next;
    }
    _wot_td_fill_json_receiver(receiver, "}", 1);
    _wot_td_fill_json_receiver(receiver, ",", 1);
    char required_obj_key[] = "required";
    _wot_td_fill_json_obj_key(receiver, required_obj_key, sizeof(required_obj_key)-1);
    wot_td_object_required_t *required = schema->required;
    _wot_td_fill_json_receiver(receiver, "[", 1);
    while (required != NULL){
        _wot_td_fill_json_string(receiver, required->value, strlen(required->value));
        required = required->next;
    }
    _wot_td_fill_json_receiver(receiver, "]", 1);
}

void _serialize_data_schema_array(wot_td_serialize_receiver_t receiver, wot_td_array_schema_t *schema, char *lang){
    if(schema->items != NULL){
        char item_obj_key[] = "items";
        _wot_td_fill_json_obj_key(receiver, item_obj_key, sizeof(item_obj_key)-1);
        wot_td_data_schemas_t *item = schema->items;
        _wot_td_fill_json_receiver(receiver, "[", 1);
        while (item != NULL){
            _serialize_data_schema(receiver, item->value, lang);
            item = item->next;
        }
        _wot_td_fill_json_receiver(receiver, "]", 1);
    }

    if(schema->min_items != NULL){
        if(schema->items != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1);
        }
        char min_items_obj_key[] = "minItems";
        _wot_td_fill_json_obj_key(receiver, min_items_obj_key, sizeof(min_items_obj_key) - 1);
        char min_items_output[32];
        _itoa(*schema->min_items, min_items_output);
        _wot_td_fill_json_string(receiver, min_items_output, strlen(min_items_output));
    }

    if(schema->max_items != NULL){
        if(schema->min_items != NULL || schema->items != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1);
        }
        char max_items_obj_key[] = "maxItems";
        _wot_td_fill_json_obj_key(receiver, max_items_obj_key, sizeof(max_items_obj_key)-1);
        char max_items_output[32];
        _itoa(*schema->max_items, max_items_output);
        _wot_td_fill_json_string(receiver, max_items_output, strlen(max_items_output));
    }
}

void _serialize_data_schema_number(wot_td_serialize_receiver_t receiver, wot_td_number_schema_t *schema){
    if(schema->minimum != NULL){
        char min_obj_key[] = "minimum";
        _wot_td_fill_json_obj_key(receiver, min_obj_key, sizeof(min_obj_key)-1);
        //Todo: Implement double to string conversion
    }
}

void _serialize_data_schema_int(wot_td_serialize_receiver_t receiver, wot_td_integer_schema_t *schema){
    if(schema->minimum != NULL){
        char min_obj_key[] = "minimum";
        _wot_td_fill_json_obj_key(receiver, min_obj_key, sizeof(min_obj_key)-1);
        char min_output[23];
        _itoa(*schema->minimum, min_output);
        _wot_td_fill_json_string(receiver, min_output, strlen(min_output));
        _wot_td_fill_json_receiver(receiver, ",", 1);

        char max_obj_key[] = "maximum";
        _wot_td_fill_json_obj_key(receiver, max_obj_key, sizeof(max_obj_key)-1);
        char max_output[23];
        _itoa(*schema->minimum, max_output);
        _wot_td_fill_json_string(receiver, max_output, strlen(max_output));
    }
}

void _serialize_data_schema_subclass(wot_td_serialize_receiver_t receiver, wot_td_data_schema_t *data_schema, char *lang){
    switch (*data_schema->json_type) {
        case JSON_TYPE_OBJECT:
            _serialize_data_schema_object(receiver, (wot_td_object_schema_t *) data_schema->schema, lang);
            break;
        case JSON_TYPE_ARRAY:
            _serialize_data_schema_array(receiver, (wot_td_array_schema_t *) data_schema->schema, lang);
            break;
        case JSON_TYPE_NUMBER:
            _serialize_data_schema_number(receiver, (wot_td_number_schema_t *) data_schema->schema);
            break;
        case JSON_TYPE_INTEGER:
            _serialize_data_schema_int(receiver, (wot_td_integer_schema_t *) data_schema->schema);
            break;
        default:
            return;
    }
}

//Todo: Implement
void _serialize_data_schema(wot_td_serialize_receiver_t receiver, wot_td_data_schema_t *data_schema, char *lang){
    bool has_previous_prop = false;
    _wot_td_fill_json_receiver(receiver, "{", 1);

    if(data_schema->type != NULL){
        has_previous_prop = true;
        wot_td_type_t *type = data_schema->type;
        char type_obj_key[] = "@type";
        _wot_td_fill_json_obj_key(receiver, type_obj_key, sizeof(type_obj_key)-1);

        while(type != NULL){
            _wot_td_fill_json_string(receiver, type->value, strlen(type->value));
            if(type->next != NULL){
                _wot_td_fill_json_receiver(receiver, ",", 1);
            }
            type = type->next;
        }
    }

    if(data_schema->titles != NULL){
        _previous_prop_check(receiver, has_previous_prop);
        has_previous_prop = true;
        _serialize_title_array(receiver, data_schema->titles, lang);
    }

    if(data_schema->descriptions != NULL){
        _previous_prop_check(receiver, has_previous_prop);
        has_previous_prop = true;
        _serialize_description_array(receiver, data_schema->descriptions, lang);
    }

    if(data_schema->json_type != NULL){
        _previous_prop_check(receiver, has_previous_prop);
        has_previous_prop = true;
        _serialize_data_schema_subclass(receiver, data_schema, lang);
    }

    if(data_schema->constant != NULL){
        _previous_prop_check(receiver, has_previous_prop);
        has_previous_prop = true;
        char constant_obj_key[] = "const";
        _wot_td_fill_json_obj_key(receiver, constant_obj_key, sizeof(constant_obj_key)-1);
        _wot_td_fill_json_string(receiver, data_schema->constant, strlen(data_schema->constant));
    }

    if(data_schema->unit != NULL){
        _previous_prop_check(receiver, has_previous_prop);
        has_previous_prop = true;
        char unit_obj_key[] = "unit";
        _wot_td_fill_json_obj_key(receiver, unit_obj_key, sizeof(unit_obj_key)-1);
        _wot_td_fill_json_string(receiver, data_schema->unit, strlen(data_schema->unit));
    }

    if(data_schema->one_of != NULL){
        _previous_prop_check(receiver, has_previous_prop);
        has_previous_prop = true;
        char one_of_obj_key[] = "oneOf";
        _wot_td_fill_json_obj_key(receiver, one_of_obj_key, sizeof(one_of_obj_key)-1);
        wot_td_data_schemas_t *tmp = data_schema->one_of;
        _wot_td_fill_json_string(receiver, "[", 1);
        while (tmp != NULL){
            _serialize_data_schema(receiver, tmp->value, lang);
            tmp = tmp->next;
        }
        _wot_td_fill_json_string(receiver, "]", 1);
    }

    if(data_schema->enumeration != NULL){
        _previous_prop_check(receiver, has_previous_prop);
        has_previous_prop = true;
        char enum_obj_key[] = "enum";
        _wot_td_fill_json_obj_key(receiver, enum_obj_key, sizeof(enum_obj_key)-1);
        wot_td_data_enums_t *tmp = data_schema->enumeration;
        _wot_td_fill_json_string(receiver, "[", 1);
        while (tmp != NULL){
            _wot_td_fill_json_string(receiver, tmp->value, strlen(tmp->value));
            tmp = tmp->next;
        }
        _wot_td_fill_json_string(receiver, "]", 1);
    }

    _previous_prop_check(receiver, has_previous_prop);
    char read_only_obj_key[] = "readOnly";
    _wot_td_fill_json_obj_key(receiver, read_only_obj_key, sizeof(read_only_obj_key)-1);
    _wot_td_fill_json_bool(receiver, data_schema->read_only);

    _wot_td_fill_json_receiver(receiver, ",", 1);
    char write_only_obj_key[] = "writeOnly";
    _wot_td_fill_json_obj_key(receiver, write_only_obj_key, sizeof(write_only_obj_key)-1);
    _wot_td_fill_json_bool(receiver, data_schema->write_only);

    if(data_schema->format != NULL){
        _wot_td_fill_json_string(receiver, ",", 1);
        char format_obj_key[] = "format";
        _wot_td_fill_json_obj_key(receiver, format_obj_key, sizeof(format_obj_key)-1);
        _wot_td_fill_json_string(receiver, data_schema->format, strlen(data_schema->format));
    }

    _wot_td_fill_json_receiver(receiver, "}", 1);
}

void _serialize_action_aff_array(wot_td_serialize_receiver_t receiver, wot_td_action_affordance_t *action_aff, char *lang){
    wot_td_action_affordance_t *tmp = action_aff;
    char action_aff_obj_key[] = "actions";
    char input_obj_key[] = "input";
    char output_obj_key[] = "output";
    char safe_obj_key[] = "safe";
    char idempotent_obj_key[] = "idempotent";

    _wot_td_fill_json_obj_key(receiver, action_aff_obj_key, sizeof(action_aff_obj_key)-1);
    _wot_td_fill_json_receiver(receiver, "{", 1);
    while (tmp != NULL){
        _wot_td_fill_json_obj_key(receiver, tmp->key, strlen(tmp->key));
        _wot_td_fill_json_receiver(receiver, "{", 1);
        _wot_td_fill_json_obj_key(receiver, input_obj_key, sizeof(input_obj_key)-1);
        _serialize_data_schema(receiver, tmp->input, lang);
        _wot_td_fill_json_receiver(receiver, ",", 1);
        _wot_td_fill_json_obj_key(receiver, output_obj_key, sizeof(output_obj_key)-1);
        _serialize_data_schema(receiver,tmp->output, lang);
        _wot_td_fill_json_receiver(receiver, ",", 1);
        _wot_td_fill_json_obj_key(receiver, safe_obj_key, sizeof(safe_obj_key)-1);
        _wot_td_fill_json_bool(receiver, tmp->safe);
        _wot_td_fill_json_receiver(receiver, ",", 1);
        _wot_td_fill_json_obj_key(receiver, idempotent_obj_key, sizeof(idempotent_obj_key)-1);
        _wot_td_fill_json_bool(receiver, tmp->idempotent);
        _wot_td_fill_json_receiver(receiver, ",", 1);
        _serialize_int_aff(receiver, tmp->int_affordance);
        _wot_td_fill_json_receiver(receiver, "}", 1);
        tmp = tmp->next;
    }

    _wot_td_fill_json_receiver(receiver, "}", 1);
}

void _serialize_event_aff_array(wot_td_serialize_receiver_t receiver, wot_td_event_affordance_t *event_aff, char *lang){
    wot_td_event_affordance_t *tmp = event_aff;
    char events_obj_key[] = "events";
    char subscription_obj_key[] = "subscription";
    char data_obj_key[] = "data";
    char cancellation_obj_key[] = "cancellation";
    bool has_previous_prop = false;

    _wot_td_fill_json_obj_key(receiver, events_obj_key, sizeof(events_obj_key)-1);
    _wot_td_fill_json_receiver(receiver, "{", 1);
    while (tmp != NULL){
        _wot_td_fill_json_obj_key(receiver, tmp->key, strlen(tmp->key));
        _wot_td_fill_json_receiver(receiver, "{", 1);
        if(tmp->subscription != NULL){
            has_previous_prop = true;
            _wot_td_fill_json_obj_key(receiver, subscription_obj_key, sizeof(subscription_obj_key)-1);
            _serialize_data_schema(receiver, tmp->subscription, lang);
        }
        if(tmp->data != NULL){
            _previous_prop_check(receiver, has_previous_prop);
            has_previous_prop = true;
            _wot_td_fill_json_obj_key(receiver, data_obj_key, sizeof(data_obj_key)-1);
            _serialize_data_schema(receiver, tmp->data, lang);
        }
        if(tmp->cancellation != NULL){
            _previous_prop_check(receiver, has_previous_prop);
            has_previous_prop = true;
            _wot_td_fill_json_obj_key(receiver, cancellation_obj_key, sizeof(cancellation_obj_key)-1);
            _serialize_data_schema(receiver, tmp->cancellation, lang);
        }
        _previous_prop_check(receiver, has_previous_prop);
        _serialize_int_aff(receiver, tmp->int_affordance);
        _wot_td_fill_json_receiver(receiver, "}", 1);
        if(tmp->next != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1);
        }
        tmp = tmp->next;
    }
    _wot_td_fill_json_receiver(receiver, "}", 1);
}

void _serialize_link_array(wot_td_serialize_receiver_t receiver, wot_td_link_t *links){
    wot_td_link_t *tmp = links;
    char href_obj_key[] = "href";
    char type_obj_key[] = "type";
    char rel_obj_key[] = "rel";
    char anchor_obj_key[] = "anchor";
    _wot_td_fill_json_receiver(receiver, "[", 1);

    while(tmp != NULL){
        _wot_td_fill_json_receiver(receiver, "{", 1);
        _wot_td_fill_json_obj_key(receiver, href_obj_key, sizeof(href_obj_key)-1);
        _wot_td_fill_json_uri(receiver, tmp->href);

        if(tmp->type != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1);
            _wot_td_fill_json_obj_key(receiver, type_obj_key, sizeof(type_obj_key)-1);
            _wot_td_fill_json_string(receiver, tmp->type, strlen(tmp->type));
        }

        if(tmp->rel != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1);
            _wot_td_fill_json_obj_key(receiver, rel_obj_key, sizeof(rel_obj_key)-1);
            _wot_td_fill_json_string(receiver, tmp->rel, strlen(tmp->rel));
        }

        if(tmp->anchor != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1);
            _wot_td_fill_json_obj_key(receiver, anchor_obj_key, sizeof(anchor_obj_key)-1);
            _wot_td_fill_json_uri(receiver, tmp->anchor);
        }

        _wot_td_fill_json_receiver(receiver, "}", 1);
        if(tmp->next != NULL){
            _wot_td_fill_json_receiver(receiver, ",", 1);
        }
        tmp = tmp->next;
    }

    return _wot_td_fill_json_receiver(receiver, "]", 1);
}

int wot_td_serialize_thing(wot_td_serialize_receiver_t receiver, wot_td_thing_t *thing){
    bool has_previous_prop = false;
    _wot_td_fill_json_receiver(receiver, "{", 1);

    has_previous_prop = true;
    char thing_context_key[] = "@context";
    _wot_td_fill_json_obj_key(receiver, thing_context_key, sizeof(thing_context_key)-1);
    char thing_context_value[] = "https://www.w3.org/2019/wot/td/v1";

    if(thing->context != NULL){
        _wot_td_fill_json_receiver(receiver, "[", 1);
        _wot_td_fill_json_string(receiver, thing_context_value, sizeof(thing_context_value)-1);
        _wot_td_fill_json_receiver(receiver, ",", 1);
        _serialize_context_array(receiver, thing->context);
        _wot_td_fill_json_receiver(receiver, "]", 1);
    }else{
        _wot_td_fill_json_string(receiver, thing_context_value, sizeof(thing_context_value)-1);
    }

    if(thing->security != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1);
        _serialize_security_array(receiver, thing->security, thing->default_language_tag);
    }else{
        return 1;
    }

    if(thing->type != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1);
        _serialize_type_array(receiver, thing->type);
    }

    if(thing->id != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1);
        char id_obj_key[] = "id";
        _wot_td_fill_json_obj_key(receiver, id_obj_key, sizeof(id_obj_key)-1);
        _wot_td_fill_json_uri(receiver, thing->id);
    }

    if(thing->titles != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1);
        has_previous_prop = true;
        _serialize_title_array(receiver, thing->titles, thing->default_language_tag);
    }

    if(thing->descriptions != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1);
        has_previous_prop = true;
        _serialize_description_array(receiver, thing->descriptions, thing->default_language_tag);
    }

    if(thing->properties != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1);
        _serialize_prop_aff_array(receiver, thing->properties);
    }

    if(thing->actions != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1);
        _serialize_action_aff_array(receiver, thing->actions, thing->default_language_tag);
    }

    if(thing->events != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1);
        _serialize_event_aff_array(receiver, thing->events, thing->default_language_tag);
    }

    if(thing->links != NULL){
        _previous_prop_check(receiver, has_previous_prop);
        char links_obj_key[] = "links";
        _wot_td_fill_json_obj_key(receiver, links_obj_key, sizeof(links_obj_key)-1);
        _serialize_link_array(receiver, thing->links);
    }

    if(thing->base != NULL){
        _previous_prop_check(receiver, has_previous_prop);
        char base_obj_key[] = "base";
        _wot_td_fill_json_obj_key(receiver, base_obj_key, sizeof(base_obj_key)-1);
        _wot_td_fill_json_uri(receiver, thing->base);
    }

    if(thing->support != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1);
        char support_obj_key[] = "support";
        _wot_td_fill_json_obj_key(receiver, support_obj_key, sizeof(support_obj_key) - 1);
        _wot_td_fill_json_uri(receiver, thing->support);
    }

    if(thing->version != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1);
        char version_obj_key[] = "version";
        _wot_td_fill_json_obj_key(receiver, version_obj_key, sizeof(version_obj_key)-1);
        char instance_obj_key[] = "instance";
        _wot_td_fill_json_receiver(receiver, "{", 1);
        _wot_td_fill_json_obj_key(receiver, instance_obj_key, sizeof(instance_obj_key)-1);
        _wot_td_fill_json_string(receiver, thing->version->instance, strlen(thing->version->instance));
        _wot_td_fill_json_receiver(receiver, "}", 1);
    }

    if(thing->forms != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1);
        char form_obj_key[] = "forms";
        _wot_td_fill_json_obj_key(receiver, form_obj_key, sizeof(form_obj_key)-1);
        _serialize_form_array(receiver, thing->forms);
    }

    if(thing->created != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1);
        char created_obj_key[] = "created";
        _wot_td_fill_json_obj_key(receiver, created_obj_key, sizeof(created_obj_key)-1);
        _wot_td_fill_json_date(receiver, thing->created);
    }

    if(thing->modified != NULL){
        _wot_td_fill_json_receiver(receiver, ",", 1);
        char modified_obj_key[] = "modified";
        _wot_td_fill_json_obj_key(receiver, modified_obj_key, sizeof(modified_obj_key)-1);
        _wot_td_fill_json_date(receiver, thing->modified);
    }

    _wot_td_fill_json_receiver(receiver, "}", 1);

    return 0;
}
