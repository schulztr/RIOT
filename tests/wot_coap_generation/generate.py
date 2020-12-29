import argparse
import json
import os
import sys
from typing import List, TypedDict, IO, Any

AFFORDANCE_TYPES = ['properties', 'actions', 'events']
CURRENT_DIRECTORY = os.getcwd()
CONFIG_DIRECTORY = f"{CURRENT_DIRECTORY}/config"
THING_DESCRIPTION_DIRECTORY = f"{CONFIG_DIRECTORY}/wot_td"
RESULT_FILE = f"{CURRENT_DIRECTORY}/result.c"
AFFORDANCES_FILES = [".coap_affordances.json", ]
THING_FILES = [".thing.json", ]
SEPERATOR = "\n\n"
INDENT = "    "
COAP_RESOURCES_NAME = "_wot_coap_resources"
COAP_LISTENER_NAME = "_wot_coap_listener"
COAP_LINK_PARAMS_NAME = "_wot_link_params"
COAP_LINK_ENCODER_NAME = "_wot_encode_link"

DEFAULT_DEPENDENCIES = ['<stdint.h>', '<stdio.h>', '<stdlib.h>',
                        '<string.h>', '"net/gcoap.h"', '"od.h"', '"fmt.h"',
                        '"net/wot.h"', '"net/wot/coap.h"', '"net/wot/coap/config.h"', ]


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


used_affordance_keys: List[str] = []
header_files: List[str] = []
extern_functions: List[str] = []

ResourceDict = TypedDict(
    'ResourceDict', {'href': str, 'handler': str, "methods": List[str]})


def dict_raise_on_duplicates(ordered_pairs):
    """Reject duplicate keys."""
    d = {}
    for k, v in ordered_pairs:
        if k in d:
            raise ValueError("duplicate key: %r" % (k,))
        else:
            d[k] = v
    return d


def write_to_c_file(result) -> None:
    f: IO[Any] = open(RESULT_FILE, "w")
    f.write(result)
    f.close()


def validate_coap_json(coap_jsons: dict) -> None:
    # TODO: Add actual validator for (different types of) affordances
    assert coap_jsons['name'], "ERROR: name in coap_affordances.json missing"
    assert coap_jsons['url'], "ERROR: url in coap_affordances.json missing"
    assert coap_jsons['handler'], "ERROR: handler in coap_affordances.json missing"
    assert coap_jsons['method'], "ERROR: method in coap_affordances.json missing"


def get_handler_name_for_href(href: str) -> str:
    return f'wot{href.replace("/", "_")}_handler'


def get_handler_function_header(handler_name: str) -> str:
    return f'extern ssize_t {handler_name}(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);'


def validate_thing_json(thing_json: dict) -> None:
    # TODO: Expand thing validation
    assert thing_json['titles'], "ERROR: name in thing.json missing"
    assert thing_json['defaultLang'], "ERROR: name in thing.json missing"


def write_coap_resources(coap_resources: List[ResourceDict]) -> str:
    sorted_resources: List[ResourceDict] = sorted(
        coap_resources, key=lambda k: k['href'])

    result = f"const coap_resource_t {COAP_RESOURCES_NAME}[] = {{\n"
    for resource in sorted_resources:
        href: str = resource["href"]
        methods: List[str] = resource['methods']
        handler_name: str = get_handler_name_for_href(href)

        result += f'    {{"{href}", '
        result += " | ".join(methods)
        result += f", {handler_name}, NULL}},\n"
    result += "};"

    return result


def generate_coap_resources(coap_jsons: list) -> List[ResourceDict]:
    coap_resources: List[ResourceDict] = []
    for coap_json in coap_jsons:
        for affordance_type in AFFORDANCE_TYPES:
            for affordance_name, affordance in coap_json[affordance_type].items():
                assert_unique_affordance(affordance_name)
                used_affordance_keys.append(affordance_name)
                forms: List[dict] = affordance["forms"]
                resources: List[ResourceDict] = extract_coap_resources(forms)
                coap_resources.extend(resources)
    return coap_resources


def assert_unique_affordance(affordance_name: str) -> None:
    assert affordance_name not in used_affordance_keys, "ERROR: Each coap affordance name has to be unique"


def extract_coap_resources(resources: List[dict]) -> List[ResourceDict]:
    hrefs: List[str] = []
    handlers: List[str] = []
    methods: List[List[str]] = []
    for resource in resources:
        href: str = resource['href']
        handler_function: str = resource['handler_function']
        method_name: str = resource['cov:methodName']
        if href not in hrefs:
            hrefs.append(href)
            handlers.append(handler_function)
            methods.append([f"COAP_{method_name}"])
        else:
            index: int = hrefs.index(href)
            assert handlers[
                index] == handler_function, f"ERROR: Different handler function for {href}"
            assert method_name not in methods[
                index], f"ERROR: Method {method_name} already used for href {href}"
            methods[index].append(f"COAP_{method_name}")

        header_file: str = resource.get("header_file", None)

        if header_file is not None and header_file not in header_files:
            header_files.append(header_file)
        elif header_file is None and handler_function not in extern_functions:
            extern_functions.append(handler_function)

    resource_list: List[ResourceDict] = []

    for index, href in enumerate(hrefs):
        dictionary = {'href': href,
                      "handler": handlers[index],
                      "methods": methods[index]
                      }  # type: ResourceDict
        resource_list.append(dictionary)

    return resource_list


def get_wot_json(file: str, directory=THING_DESCRIPTION_DIRECTORY, validation_function=None) -> dict:
    path = f'{directory}/{file}'
    try:
        f: IO[Any] = open(path)
        wot_json: dict = json.loads(f.read())
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


def parse_command_line_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='Web of Things helper script')
    parser.add_argument('--board', help='Define used board')
    parser.add_argument('--saul', action='store_true',
                        help='Define if WoT TD SAUL is used')
    parser.add_argument('--security', help='Define what security is used')
    return parser.parse_args()


def assert_command_line_arguments(args: argparse.Namespace) -> None:
    assert args.board, "ERROR: Argument board has to be defined"
    assert args.security, "ERROR: Argument security has to be defined"


def generate_includes() -> str:
    dependencies = DEFAULT_DEPENDENCIES + \
        [f'"{header_file}"' for header_file in header_files]

    dependencies = [f'#include {dependency}' for dependency in dependencies]
    return "\n".join(dependencies)


def generate_extern_functions() -> str:
    functions = [get_handler_function_header(x) for x in extern_functions]
    return "\n".join(functions)


def generate_coap_listener() -> str:
    result = f"static gcoap_listener_t {COAP_LISTENER_NAME} = {{\n"
    result += INDENT + f"&{COAP_RESOURCES_NAME}[0],\n"
    result += INDENT + f"ARRAY_SIZE({COAP_RESOURCES_NAME}),\n"
    result += INDENT + f"{COAP_LINK_ENCODER_NAME},\n"
    result += INDENT + "NULL,\n"
    result += INDENT + "NULL\n"
    result += "};"

    return result


def generate_coap_handlers(coap_resources: List[ResourceDict]) -> str:
    handlers: List[str] = []

    for resource in coap_resources:
        wot_handler: str = get_handler_name_for_href(resource['href'])
        actual_handler: str = resource['handler']

        handler = f"static ssize_t {wot_handler}(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)\n"
        handler += "{\n"
        handler += INDENT
        handler += f"return {actual_handler}(&pdu, &buf, len, &ctx);\n"
        handler += "}"

        continue

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
            handler += INDENT * 2
            handler += 'puts("Hi.");\n'
            handler += INDENT * 2
            handler += 'return gcoap_response(pdu, buf, len, COAP_CODE_CHANGED);\n'
            handler += INDENT
            handler += "}\n"

        handler += "\n" + INDENT
        handler += "return 0;\n"
        handler += "}"

    handlers.append(handler)

    return SEPERATOR.join(handlers)


def generate_coap_link_param(coap_resource: ResourceDict) -> str:
    return "NULL"


def generate_coap_link_params(coap_resources: List[ResourceDict]) -> str:
    result = f"static const char *{COAP_LINK_PARAMS_NAME}[] = {{\n"

    link_params = []
    for coap_resource in coap_resources:
        link_params.append(INDENT + generate_coap_link_param(coap_resource))

    result += ",\n".join(link_params)
    result += "\n}"
    return result


def generate_init_function() -> str:
    result = "void wot_td_coap_init(void)\n"
    result += "{\n"
    result += INDENT + f"gcoap_register_listener(&{COAP_LISTENER_NAME});\n"
    result += "}\n"

    return result


def add_to_result(result_element: str, result_elements: List[str]):
    if result_element:
        result_elements.append(result_element)


def assemble_results(coap_jsons: List[dict], thing_jsons: List[dict]) -> List[str]:
    coap_resources = generate_coap_resources(coap_jsons)

    result_elements: List[str] = []
    add_to_result(generate_includes(), result_elements)
    add_to_result(generate_extern_functions(), result_elements)
    add_to_result(generate_coap_handlers(coap_resources), result_elements)
    add_to_result(write_coap_resources(coap_resources), result_elements)
    add_to_result(generate_coap_link_params(coap_resources), result_elements)
    add_to_result(generate_coap_listener(), result_elements)
    add_to_result(COAP_LINK_ENCODER, result_elements)
    add_to_result(generate_init_function(), result_elements)

    return result_elements


def get_result() -> str:
    coap_jsons: List[dict] = [get_wot_json(affordance_file)
                              for affordance_file in AFFORDANCES_FILES]
    thing_jsons: List[dict] = [get_wot_json(thing_file, validation_function=validate_thing_json)
                               for thing_file in THING_FILES]

    result_elements: List[str] = assemble_results(coap_jsons, thing_jsons)

    return SEPERATOR.join(result_elements)


def main() -> None:
    args = parse_command_line_arguments()
    assert_command_line_arguments(args)

    result: str = get_result()
    write_to_c_file(result)
    print(result)


if __name__ == '__main__':
    main()
