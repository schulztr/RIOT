import json
import os
import argparse
import sys

AFFORDANCE_TYPES = ['properties', 'actions', 'events']

current_directory = os.getcwd()
result_file = current_directory + "/result.c"
result = ""
coap_jsons = []
thing_jsons = []
multiple_usage = {
    "properties": [],
    "actions": [],
    "events": []
}
coap_affordances = {
    "properties": [],
    "actions": [],
    "events": []
}

parser = argparse.ArgumentParser(description='Web of Things helper script')
parser.add_argument('--board', help='Define used board')
parser.add_argument('--saul', action='store_true',
                    help='Define if WoT TD SAUL is used')
parser.add_argument('--security', help='Define what security is used')


def dict_raise_on_duplicates(ordered_pairs):
    """Reject duplicate keys."""
    d = {}
    for k, v in ordered_pairs:
        if k in d:
            raise ValueError("duplicate key: %r" % (k,))
        else:
            d[k] = v
    return d


def add_content(content):
    global result
    result += content


def write_to_c_file(content):
    global result
    f = open(result_file, "w")
    f.write(result)
    f.close()


def validate_coap_json(coap_jsons):
    assert coap_jsons['name'], "ERROR: name in coap_affordances.json missing"
    assert coap_jsons['url'], "ERROR: url in coap_affordances.json missing"
    assert coap_jsons['handler'], "ERROR: handler in coap_affordances.json missing"
    assert coap_jsons['method'], "ERROR: method in coap_affordances.json missing"


def validate_thing_json(thing_json):
    assert thing_json['titles'], "ERROR: name in thing.json missing"
    assert thing_json['defaultLang'], "ERROR: name in thing.json missing"


def write_coap_resources(coap_resources):
    add_content("const coap_resource_t _coap_resources[] = {\n")
    sorted_resources = sorted(coap_resources, key=lambda k: k['href'])
    for resource in sorted_resources:
        add_content(f'    {{"{resource["href"]}", ')
        for index, method in enumerate(resource['methods']):
            if index > 0:
                add_content(" | ")
            add_content(method)
        add_content(", " + resource['handler'] + ", NULL},\n")

    add_content("};")


def generate_coap_resources():
    coap_resources = []
    for coap_json in coap_jsons:
        for affordance_type in AFFORDANCE_TYPES:
            for affordance_name, affordance in coap_json[affordance_type].items():
                assert affordance_name not in multiple_usage[
                    affordance_type], "ERROR: Each coap affordance has to be unique"
                multiple_usage[affordance_type].append(affordance_name)
                forms = affordance["forms"]
                resources = extract_coap_resources(forms)
                coap_resources.extend(resources)
    write_coap_resources(coap_resources)


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


if __name__ == '__main__':
    args = parser.parse_args()
    assert args.board, "ERROR: Argument board has to be defined"
    assert args.security, "ERROR: Argument security has to be defined"

    thing_definiton = f"{current_directory}/config/wot_td/.thing.json"
    try:
        f = open(thing_definiton)
        thing_json = json.loads(f.read())
        validate_thing_json(thing_json)
    except IOError:
        print(f"ERROR: Thing definition in {thing_definiton} is missing")
        sys.exit(0)
    except json.decoder.JSONDecodeError:
        print(f"ERROR: json in {thing_definiton} is not valid")
        sys.exit(0)
    finally:
        f.close()

    coap_affordances = f"{current_directory}/config/wot_td/.coap_affordances.json"
    try:
        f = open(coap_affordances)
        coap_json = json.loads(
            f.read(), object_pairs_hook=dict_raise_on_duplicates)
        coap_jsons.append(coap_json)
    except IOError:
        print(f"INFO: Coap definition in {coap_affordances} not present")
    except json.decoder.JSONDecodeError:
        print(f"ERROR: json in {coap_affordances} is not valid")
        sys.exit(0)
    finally:
        f.close()
    generate_coap_resources()
    print(result)
