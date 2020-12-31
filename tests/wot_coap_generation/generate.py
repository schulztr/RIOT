import argparse
import json
import os
import sys
from typing import List, Tuple, TypedDict, IO, Any

PROPERTIES_NAME = 'properties'
ACTIONS_NAME = 'actions'
EVENTS_NAME = 'events'
AFFORDANCE_TYPES = [PROPERTIES_NAME, ACTIONS_NAME, EVENTS_NAME]
AFFORDANCE_TYPE_SPECIFIERS = {
    PROPERTIES_NAME: 'prop',
    ACTIONS_NAME: 'action',
    EVENTS_NAME: 'event'
}
CURRENT_DIRECTORY = os.getcwd()
CONFIG_DIRECTORY = f"{CURRENT_DIRECTORY}/config"
THING_DESCRIPTION_DIRECTORY = f"{CONFIG_DIRECTORY}/wot_td"
RESULT_FILE = f"{CURRENT_DIRECTORY}/wot_coap_config.c"
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
                                size_t maxlen, coap_link_encoder_ctx_t *context)
{{
    ssize_t res = gcoap_encode_link(resource, buf, maxlen, context);
    if (res > 0)
    {{
        if ({COAP_LINK_PARAMS_NAME}[context->link_pos] && (strlen({COAP_LINK_PARAMS_NAME}[context->link_pos]) < (maxlen - res)))
        {{
            if (buf)
            {{
                memcpy(buf + res, {COAP_LINK_PARAMS_NAME}[context->link_pos],
                       strlen({COAP_LINK_PARAMS_NAME}[context->link_pos]));
            }}
            return res + strlen({COAP_LINK_PARAMS_NAME}[context->link_pos]);
        }}
    }}

    return res;
}}'''


OPERATION_TYPES = {
    "readproperty": "FORM_OP_READ_PROPERTY",
    "writeproperty": "FORM_OP_WRITE_PROPERTY",
    "observeproperty": "FORM_OP_OBSERVE_PROPERTY",
    "unobserveproperty": "FORM_OP_UNOBSERVE_PROPERTY",
    "invokeaction": "FORM_OP_INVOKE_ACTION",
    "subscribeevent": "FORM_OP_SUBSCRIBE_EVENT",
    "unsubscribeevent": "FORM_OP_UNSUBSCRIBE_EVENT",
    "readallproperties": "FORM_OP_READ_ALL_PROPERTIES",
    "writeallproperties": "FORM_OP_WRITE_ALL_PROPERTIES",
    "readmultipleproperties": "FORM_OP_READ_MULTIPLE_PROPERTIES",
    "writemultipleproperties": "FORM_OP_WRITE_MULTIPLE_PROPERTIES"
}

JSON_TYPES = {
    "object": "JSON_TYPE_OBJECT",
    "array": "JSON_TYPE_ARRAY",
    "string": "JSON_TYPE_STRING",
    "number": "JSON_TYPE_NUMBER",
    "integer": "JSON_TYPE_INTEGER",
    "boolean": "JSON_TYPE_BOOLEAN",
    "null": "JSON_TYPE_NULL"
}

ALLOWED_OPERATIONS_BY_TYPE = {
    PROPERTIES_NAME: ["readproperty", "writeproperty", "observeproperty", "unobserveproperty", ],
    ACTIONS_NAME: ["invokeaction", ],
    EVENTS_NAME: ["subscribeevent", "unsubscribeevent", ],
}

used_affordance_keys: List[str] = []
header_files: List[str] = []
extern_functions: List[str] = []
resource_affordance_list: List[str] = []

ResourceDict = TypedDict(
    'ResourceDict', {'affordance_name': str, 'href': str, 'handler': str, "methods": List[str]})


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
    return f'extern ssize_t {handler_name}(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx);'


def validate_thing_json(thing_json: dict) -> None:
    # TODO: Expand thing validation
    assert thing_json['titles'], "ERROR: name in thing.json missing"
    assert thing_json['defaultLang'], "ERROR: name in thing.json missing"


class c_struct:
    def __init__(self, struct_type: str, struct_name: str, keywords: List[str] = []):
        first_line = self.__get_first_line(struct_type, struct_name, keywords)
        self.elements = [first_line]

    def __get_first_line(self, struct_type: str, struct_name: str, keywords: List[str]) -> str:
        return f"{self.__generate_keywords(keywords)}{struct_type} {struct_name} = {{"

    def __generate_keywords(self, keywords: List[str]) -> str:
        return ' '.join(keywords) + ' ' if keywords else ''

    def generate_struct(self):
        return f'\n{INDENT}'.join(self.elements) + "\n};"

    def __generate_field(self, field_name: str, field_value: str) -> str:
        return f".{field_name} = {field_value},"

    def add_field(self, field_name: str, field_value: str) -> None:
        field = self.__generate_field(field_name, field_value)
        self.elements.append(field)

    def insert_into(self, structs: List[str]) -> None:
        struct = self.generate_struct()
        structs.insert(0, struct)


def write_coap_resources(coap_resources: List[ResourceDict]) -> str:
    sorted_resources: List[ResourceDict] = sorted(
        coap_resources, key=lambda k: k['href'])

    result = f"const coap_resource_t {COAP_RESOURCES_NAME}[] = {{\n"
    for resource in sorted_resources:
        resource_affordance_list.append(resource['affordance_name'])
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
                resources: List[ResourceDict] = extract_coap_resources(
                    affordance_name, forms)
                coap_resources.extend(resources)
    return coap_resources


def assert_unique_affordance(affordance_name: str) -> None:
    assert affordance_name not in used_affordance_keys, "ERROR: Each coap affordance name has to be unique"


def extract_coap_resources(affordance_name: str, resources: List[dict]) -> List[ResourceDict]:
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
        dictionary = {'affordance_name': affordance_name,
                      'href': href,
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
    # struct()
    struct_elements = [f"static gcoap_listener_t {COAP_LISTENER_NAME} = {{"]
    struct_elements.append(f"&{COAP_RESOURCES_NAME}[0],")
    struct_elements.append(f"ARRAY_SIZE({COAP_RESOURCES_NAME}),")
    struct_elements.append(f"{COAP_LINK_ENCODER_NAME},")
    struct_elements.append(f"NULL,")
    struct_elements.append(f"NULL,")

    return generate_struct(struct_elements)


def generate_coap_handlers(coap_resources: List[ResourceDict]) -> str:
    handlers: List[str] = []

    for resource in coap_resources:
        wot_handler: str = get_handler_name_for_href(resource['href'])
        actual_handler: str = resource['handler']

        handler = f"static ssize_t {wot_handler}(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)\n"
        handler += "{\n"
        handler += INDENT
        handler += f"return {actual_handler}(pdu, buf, len, ctx);\n"
        handler += "}"
        handlers.append(handler)

        # TODO: Add validation

    return SEPERATOR.join(handlers)


def generate_coap_link_param(coap_resource: ResourceDict) -> str:
    return "NULL,"


def generate_coap_link_params(coap_resources: List[ResourceDict]) -> str:
    struct_elements = [f"static const char *{COAP_LINK_PARAMS_NAME}[] = {{"]

    for coap_resource in coap_resources:
        struct_elements.append(generate_coap_link_param(coap_resource))

    return generate_struct(struct_elements)


def get_affordance_type_specifier(affordance_type: str) -> str:
    return AFFORDANCE_TYPE_SPECIFIERS[affordance_type]


def get_affordance_struct_name(affordance_name: str) -> str:
    return f'wot_coap_{affordance_name}_affordance'


def generate_struct(struct_elements: List[str]) -> str:
    return f'\n{INDENT}'.join(struct_elements) + "\n};"


def insert_struct(structs: List[str], struct_elements: List[str]) -> None:
    struct = generate_struct(struct_elements)
    structs.insert(0, struct)


def generate_struct_field(field_name: str, field_value: str) -> str:
    return f".{field_name} = {field_value},"


def add_struct_field(struct_elements: List[str], field_name: str, field_value: str) -> None:
    struct_field = generate_struct_field(field_name, field_value)
    struct_elements.append(struct_field)


def add_operations_struct(structs: List[str], form_index: int, has_next: bool, op_type: str, affordance_name: str, affordance_type: str, op_index=0) -> None:
    assert op_type in ALLOWED_OPERATIONS_BY_TYPE[
        affordance_type], f"Operation {op_type} not allowed for affordance type {affordance_type}"

    op = c_struct("wot_td_form_op_t",
                  f"wot_td_{affordance_name}_form_{form_index}_op_{op_index}")
    op.add_field("op_type", OPERATION_TYPES[op_type])
    if has_next:
        next_op = f"wot_td_{affordance_name}_form_{form_index}_op_{op_index + 1}"
        op.add_field("next", next_op)
    else:
        op.add_field("next", "NULL")

    op.insert_into(structs)


def add_href_struct(structs: List[str], index: int, affordance_name: str) -> None:
    struct = f"wot_td_uri_t wot_td_{affordance_name}_aff_form_href_{index} = {{0}};"
    structs.insert(0, struct)


def add_extension_struct(structs: List[str], index: int, affordance_name: str) -> None:
    struct = f"wot_td_extension_t wot_td_{affordance_name}_form_coap_{index} = {{0}};"
    structs.insert(0, struct)


def generate_operations(structs: List[str], form: dict, index: int, affordance_type: str,  affordance_name: str, affordance: dict) -> None:
    operations = form["op"]
    if isinstance(operations, str):
        operations = [operations]
    for op_index, operation in enumerate(operations):
        has_next = len(operations) < op_index + 1
        add_operations_struct(structs, index, has_next,
                              operation, affordance_name, affordance_type)


def add_interaction_affordance_forms(structs: List[str], affordance_type: str,  affordance_name: str, affordance: dict) -> None:
    forms = affordance['forms']
    for index, form in enumerate(forms):
        struct = c_struct('wot_td_form_t',
                          f'wot_td_{affordance_name}_aff_form_{index}')
        if "op" in form:
            op = f'&wot_td_{affordance_name}_form_{index}_op_0'
            struct.add_field("op", op)
        if "contentType" in form:
            content_type = f'&wot_td_{affordance_name}_content_type_{index}'
            struct.add_field("content_type", content_type)
        assert "href" in form, f'ERROR: "href" is mandatory in "form" elements! ({affordance_name})'
        href = f'&wot_td_{affordance_name}_aff_form_href_{index}'
        struct.add_field("href", href)
        first_extension = f'&wot_td_{affordance_name}_form_coap_{index}'
        struct.add_field("extensions", first_extension)
        if index + 1 < len(forms):
            next_form = f"&wot_td_{affordance_name}_aff_form_{index + 1}"
            struct.add_field("next", next_form)
        else:
            struct.add_field("next", "NULL")

        struct.insert_into(structs)
        if "op" in form:
            generate_operations(structs, form, index,
                                affordance_type, affordance_name, affordance)
        add_href_struct(structs, index, affordance_name)
        add_extension_struct(structs, index, affordance_name)


def add_interaction_affordance(structs: List[str], affordance_type: str, affordance_name: str, affordance: dict) -> None:
    struct = c_struct("wot_td_int_affordance_t",
                      f'wot_{affordance_name}_int_affordance')
    struct.add_field("forms", f"&wot_td_{affordance_name}_aff_form_0")
    struct.insert_into(structs)
    add_interaction_affordance_forms(
        structs, affordance_type, affordance_name, affordance)


def get_c_boolean(boolean: bool) -> str:
    if boolean:
        return "true"
    else:
        return "false"


def add_requirements(structs: List[str], schema_name: str, requirements: List[str]) -> None:
    for requirement in requirements:
        struct = c_struct("wot_td_object_required_t",
                          f"wot_{schema_name}_{requirement}_required")
        struct.add_field("value", f'"{requirement}"')
        struct.insert_into(structs)


def add_data_schema_maps(structs: List[str], schema_name: str, properties: dict) -> None:
    for property_name, property in properties.items():
        struct = c_struct("wot_td_data_schema_map_t",
                          f"wot_{schema_name}_{property_name}_data_map")
        struct.add_field("key", f'"{property_name}"')
        data_schema = f'&wot_{schema_name}_{property_name}_data_schema'
        struct.add_field("value", data_schema)
        struct.insert_into(structs)

        generate_data_schema(
            structs, f'{schema_name}_{property_name}', property)


def get_required_properties(schema: dict) -> List[str]:
    required_properties = schema['required']
    if isinstance(required_properties, str):
        required_properties = [required_properties]
    for property in required_properties:
        assert property in schema['properties']

    return required_properties


def add_schema_object(structs: List[str], schema_name: str, schema: dict) -> None:
    properties = schema['properties']
    first_property = list(properties.keys())[0]
    required_properties: List[str] = get_required_properties(schema)

    struct = c_struct("wot_td_object_schema_t",
                      f"wot_{schema_name}_data_schema_obj")
    data_map = f'&wot_{schema_name}_{first_property}_data_map'
    struct.add_field('properties', data_map)

    if required_properties:
        required = f'&wot_{schema_name}_{required_properties[0]}_required'
        struct.add_field('required', required)

    struct.insert_into(structs)

    add_data_schema_maps(structs, schema_name, properties)
    if required_properties:
        add_requirements(structs, schema_name, required_properties)


def generate_data_schema(structs: List[str], schema_name: str, schema: dict) -> None:
    json_type = None
    if "properties" in schema:
        json_type = "object"
    elif "type" in schema:
        json_type = schema["type"]

    struct = c_struct("wot_td_data_schema_t", f"wot_{schema_name}_data_schema")
    if json_type is not None:
        struct.add_field("json_type", JSON_TYPES[json_type])
    if "readOnly" in schema:
        struct.add_field("read_only", get_c_boolean(schema["readOnly"]))
    if "writeOnly" in schema:
        struct.add_field("write_only", get_c_boolean(schema["writeOnly"]))
    if json_type == "object":
        struct.add_field("schema", f'&wot_{schema_name}_data_schema_obj')
    struct.insert_into(structs)

    if json_type == "object":
        add_schema_object(structs, schema_name, schema)


def add_specific_affordance(structs: List[str], affordance_type: str, affordance_name: str, affordance: dict) -> None:
    specifier = get_affordance_type_specifier(affordance_type)
    struct = c_struct(f'wot_td_{specifier}_affordance_t',
                      f'wot_{affordance_name}_affordance')
    struct.add_field("key", f'"{affordance_name}"')
    struct.add_field("int_affordance",
                     f'&wot_{affordance_name}_int_affordance')
    if PROPERTIES_NAME in affordance:
        assert affordance_type == PROPERTIES_NAME
        struct.add_field("data_schema", f'&wot_{affordance_name}_data_schema')
    struct.add_field("next", "NULL")

    struct.insert_into(structs)

    if PROPERTIES_NAME in affordance:
        generate_data_schema(structs, affordance_name, affordance)
    add_interaction_affordance(
        structs, affordance_type, affordance_name, affordance)


def generate_affordance_struct(affordance_type: str, affordance_name: str, affordance: dict) -> str:
    resource_index = resource_affordance_list.index(affordance_name)

    structs: List[str] = []
    struct_specifier = get_affordance_type_specifier(affordance_type)
    struct_name = get_affordance_struct_name(affordance_name)
    struct = c_struct(f"wot_td_coap_{struct_specifier}_affordance_t",
                      struct_name)
    struct.add_field("coap_resource",
                     f"&{COAP_RESOURCES_NAME}[{resource_index}]")
    struct.add_field("affordance", f"&wot_{affordance_name}_affordance")
    struct.add_field("form", f"&wot_td_{affordance_name}_aff_form_0")
    struct.insert_into(structs)

    add_specific_affordance(structs, affordance_type,
                            affordance_name, affordance)

    return SEPERATOR.join(structs)


def generate_json_serialization(coap_jsons: List[dict]) -> str:
    result_elements = []
    for coap_json in coap_jsons:
        for affordance_type in AFFORDANCE_TYPES:
            for affordance in coap_json[affordance_type].items():
                result_elements.append(generate_affordance_struct(affordance_type, affordance[0],
                                                                  affordance[1]))
    return SEPERATOR.join(result_elements)


def generate_affordance_entries(affordance_type: str, affordance_type_json: dict) -> str:
    result = ""
    specifier = get_affordance_type_specifier(affordance_type)
    for affordance_name in affordance_type_json:
        struct_name: str = get_affordance_struct_name(affordance_name)
        result += INDENT
        result += f'wot_td_coap_{specifier}_add(thing, &{struct_name});\n'

    return result


def generate_init_function(coap_jsons: List[dict]) -> str:
    result = "int wot_td_coap_config_init(wot_td_thing_t *thing)\n"
    result += "{\n"
    result += INDENT + f"gcoap_register_listener(&{COAP_LISTENER_NAME});\n"
    for coap_json in coap_jsons:
        for affordance_type in AFFORDANCE_TYPES:
            result += generate_affordance_entries(
                affordance_type, coap_json[affordance_type])

    result += INDENT + "return 0;\n"
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
    add_to_result(COAP_LINK_ENCODER, result_elements)
    add_to_result(generate_coap_listener(), result_elements)
    add_to_result(generate_json_serialization(coap_jsons), result_elements)
    add_to_result(generate_init_function(coap_jsons), result_elements)

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


if __name__ == '__main__':
    main()
