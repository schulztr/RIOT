import json
import os
import argparse
import sys

AFFORDANCE_TYPES = ['properties', 'actions', 'events']
CURRENT_DIRECTORY = os.getcwd()
CONFIG_DIRECTORY = f"{CURRENT_DIRECTORY}/config"
THING_DESCRIPTION_DIRECTORY = f"{CONFIG_DIRECTORY}/wot_td"
RESULT_FILE = f"{CURRENT_DIRECTORY}/result.c"
AFFORDANCES_FILES = [".coap_affordances.json", ]
THING_FILES = [".thing.json", ]
SEPERATOR = "\n\n"
INDENT = "    "
COAP_LISTENER_NAME = "_coap_listener"
COAP_LINK_PARAMS_NAME = "_wot_link_params"
COAP_LINK_ENCODER_NAME = "_wot_encode_link"

DEFAULT_DEPENDENCIES = ['<stdint.h>', '<stdio.h>', '<stdlib.h>',
                        '<string.h>', '"net/gcoap.h"', '"od.h"', '"fmt.h"', ]

COAP_LINK_ENCODER = f'''static ssize_t {COAP_LINK_ENCODER_NAME}(const coap_resource_t *resource, char *buf,
                            size_t maxlen, coap_link_encoder_ctx_t *context) {{
    ssize_t res = gcoap_encode_link(resource, buf, maxlen, context);
    if (res > 0) {{
        if ({COAP_LINK_PARAMS_NAME}[context->link_pos]
                && (strlen({COAP_LINK_PARAMS_NAME}[context->link_pos]) < (maxlen - res))) {{
            if (buf) {{
                memcpy(buf+res, {COAP_LINK_PARAMS_NAME}[context->link_pos],
                       strlen({COAP_LINK_PARAMS_NAME}[context->link_pos]));
            }}
            return res + strlen({COAP_LINK_PARAMS_NAME}[context->link_pos]);
        }}
    }}

    return res;
}}'''


used_affordance_keys = []


def dict_raise_on_duplicates(ordered_pairs):
    """Reject duplicate keys."""
    d = {}
    for k, v in ordered_pairs:
        if k in d:
            raise ValueError("duplicate key: %r" % (k,))
        else:
            d[k] = v
    return d


def write_to_c_file(result):
    f = open(RESULT_FILE, "w")
    f.write(result)
    f.close()


def validate_coap_json(coap_jsons):
    # TODO: Add actual validator for (different types of) affordances
    assert coap_jsons['name'], "ERROR: name in coap_affordances.json missing"
    assert coap_jsons['url'], "ERROR: url in coap_affordances.json missing"
    assert coap_jsons['handler'], "ERROR: handler in coap_affordances.json missing"
    assert coap_jsons['method'], "ERROR: method in coap_affordances.json missing"


def validate_thing_json(thing_json):
    # TODO: Expand thing validation
    assert thing_json['titles'], "ERROR: name in thing.json missing"
    assert thing_json['defaultLang'], "ERROR: name in thing.json missing"


def write_coap_resources(coap_resources: list) -> str:
    sorted_resources = sorted(coap_resources, key=lambda k: k['href'])

    result = "const coap_resource_t _coap_resources[] = {\n"
    for resource in sorted_resources:
        result += f'    {{"{resource["href"]}", '
        result += " | ".join(resource['methods'])
        result += ", " + resource['handler'] + ", NULL},\n"
    result += "};"

    return result


def generate_coap_resources(coap_jsons: list) -> str:
    coap_resources = []
    for coap_json in coap_jsons:
        for affordance_type in AFFORDANCE_TYPES:
            for affordance_name, affordance in coap_json[affordance_type].items():
                assert_unique_affordance(affordance_name)
                used_affordance_keys.append(affordance_name)
                forms = affordance["forms"]
                resources = extract_coap_resources(forms)
                coap_resources.extend(resources)
    return coap_resources


def assert_unique_affordance(affordance_name: str):
    assert affordance_name not in used_affordance_keys, "ERROR: Each coap affordance name has to be unique"


def extract_coap_resources(resources: list) -> list:
    hrefs = []
    handlers = []
    methods = []
    for resource in resources:
        href = resource['href']
        handler_function = resource['handler_function']
        method_name = resource['cov:methodName']
        if href not in hrefs:
            hrefs.append(href)
            handlers.append(handler_function)
            methods.append([f"COAP_{method_name}"])
        else:
            index = hrefs.index(href)
            assert handlers[
                index] == handler_function, f"ERROR: Different handler function for {href}"
            assert method_name not in methods[
                index], f"ERROR: Method {method_name} already used for href {href}"
            methods[index].append(f"COAP_{method_name}")

    resource_list = []

    for index, href in enumerate(hrefs):
        dictionary = {}
        dictionary["href"] = href
        dictionary["handler"] = handlers[index]
        dictionary["methods"] = methods[index]
        resource_list.append(dictionary)

    return resource_list


def get_wot_json(file: str, directory=THING_DESCRIPTION_DIRECTORY, validation_function=None) -> str:
    path = f'{directory}/{file}'
    try:
        f = open(path)
        wot_json = json.loads(f.read())
        if validation_function is not None:
            validation_function(wot_json)
    except IOError:
        print(f"ERROR reading {path} is missing")
        sys.exit(0)
    except json.decoder.JSONDecodeError:
        print(f"ERROR: json in {path} is not valid")
        sys.exit(0)
    finally:
        f.close()

    return wot_json


def parse_command_line_arguments() -> list:
    parser = argparse.ArgumentParser(description='Web of Things helper script')
    parser.add_argument('--board', help='Define used board')
    parser.add_argument('--saul', action='store_true',
                        help='Define if WoT TD SAUL is used')
    parser.add_argument('--security', help='Define what security is used')
    return parser.parse_args()


def assert_command_line_arguments(args):
    assert args.board, "ERROR: Argument board has to be defined"
    assert args.security, "ERROR: Argument security has to be defined"


def generate_includes(coap_resources: list) -> str:
    dependencies = DEFAULT_DEPENDENCIES

    dependencies = [f'#include {dependency}' for dependency in dependencies]
    return "\n".join(dependencies)


def generate_coap_listener() -> str:
    result = f"static gcoap_listener_t {COAP_LISTENER_NAME} = {{\n"
    result += INDENT + "&_coap_resources[0],\n"
    result += INDENT + "ARRAY_SIZE(_coap_resources),\n"
    result += INDENT + f"{COAP_LINK_ENCODER_NAME},\n"
    result += INDENT + "NULL,\n"
    result += INDENT + "NULL\n"
    result += "};"

    return result


def generate_coap_handlers(coap_resources: list) -> str:
    handlers = []

    for resource in coap_resources:
        handler = f"static ssize_t {resource['handler']}(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)\n"
        handler += "{\n"
        handler += INDENT
        handler += "(void)ctx;\n"
        handler += INDENT
        handler += 'unsigned method_flag = coap_method2flag(coap_get_code_detail(pdu));\n\n'

        for index, method in enumerate(resource['methods']):
            handler += INDENT
            if index > 0:
                handler += 'else '
            handler += f'if (method_flag == {method})\n'
            handler += INDENT
            handler += "{\n"
            handler += INDENT + INDENT
            handler += 'puts("Hi.");\n'
            handler += INDENT + INDENT
            handler += 'return gcoap_response(pdu, buf, len, COAP_CODE_CHANGED);\n'
            handler += INDENT
            handler += "}\n"

        handler += "\n" + INDENT
        handler += "return 0;\n"
        handler += "}"

    handlers.append(handler)

    return SEPERATOR.join(handlers)


def generate_coap_link_param(coap_resource):
    return "NULL"


def generate_coap_link_params(coap_resources: list):
    result = f"static const char *{COAP_LINK_PARAMS_NAME}[] = {{\n"

    link_params = []
    for coap_resource in coap_resources:
        link_params.append(INDENT + generate_coap_link_param(coap_resource))

    result += ",\n".join(link_params)
    result += "\n}"
    return result


def generate_init_function() -> str:
    result = "void wot_td_coap_config_init(void)\n"
    result += "{\n"
    result += INDENT + f"gcoap_register_listener(&{COAP_LISTENER_NAME});\n"
    result += "}\n"

    return result


def assemble_results(coap_jsons: list, thing_jsons: list) -> list:
    coap_resources = generate_coap_resources(coap_jsons)

    result_elements = []
    result_elements.append(generate_includes(coap_resources))
    result_elements.append(generate_coap_handlers(coap_resources))
    result_elements.append(write_coap_resources(coap_resources))
    result_elements.append(generate_coap_link_params(coap_resources))
    result_elements.append(generate_coap_listener())
    result_elements.append(COAP_LINK_ENCODER)
    result_elements.append(generate_init_function())

    return result_elements


def get_result():
    coap_jsons = [get_wot_json(affordance_file)
                  for affordance_file in AFFORDANCES_FILES]
    thing_jsons = [get_wot_json(
        thing_file, validation_function=validate_thing_json) for thing_file in THING_FILES]

    result_elements = assemble_results(coap_jsons, thing_jsons)

    return SEPERATOR.join(result_elements)


def main():
    args = parse_command_line_arguments()
    assert_command_line_arguments(args)

    result = get_result()
    write_to_c_file(result)
    print(result)


if __name__ == '__main__':
    main()
