import json
import os
import argparse
import sys

currentDirectory = os.getcwd()
result_file = currentDirectory + "/result.c"
result = ""
coapJsons = []
thingJsons = []
multipleUsage = {
    "properties": [],
    "actions": [],
    "events": []
}
coapAffordances = {
    "properties": [],
    "actions": [],
    "events": []
}

parser = argparse.ArgumentParser(description='Web of Things helper script')
parser.add_argument('--board', help='Define used board')
parser.add_argument('--saul', action='store_true', help='Define if WoT TD SAUL is used')
parser.add_argument('--security', help='Define what security is used')

def add_content(content):
    global result
    result += content

def write_to_c_file(content):
    global result
    f = open(result_file, "w")
    f.write(result)
    f.close()

def validate_coap_json(coapJson):
    assert coapJson['name'], "ERROR: name in coap_affordances.json missing"
    assert coapJson['url'], "ERROR: url in coap_affordances.json missing"
    assert coapJson['handler'], "ERROR: handler in coap_affordances.json missing"
    assert coapJson['method'], "ERROR: method in coap_affordances.json missing"

def validate_thing_json(thingJson):
    assert thingJson['titles'], "ERROR: name in thing.json missing"
    assert thingJson['defaultLang'], "ERROR: name in thing.json missing"

def validate_unique_name(name, jsons, propName):
    count = 0
    for j in jsons:
        for aff in j[propName]:
            if aff[name] == name:
                count += 1
            assert not(count > 1), "ERROR: Each coap affordance has to be unique"

def validate_coap_affordances(coapJsons):
    for coapJson in coapJsons:
        properties = coapJson['properties']
        for prop in properties:
            name = prop['name']
            validate_unique_name(name, coapJsons, 'properties')

        events = coapJson['events']
        for event in events:
            name = event['name']
            validate_unique_name(name, coapJsons, 'events')

        actions = coapJson['actions']
        for action in actions:
            name = action['name']
            validate_unique_name(name, coapJsons, 'actions')

def find_all_coap_methods(handlerName, affName, coapJsons):
    methods = []
    for coapJson in coapJsons:
        for aff in coapJson[affName]:
            if aff['handler']['name'] == handlerName:
                methods.append(aff['method'])
    return methods

def write_coap_resources(coapResources):
    add_content("const coap_resource_t _coap_resources[] = {\n")
    #Fixme: sort before
    for resource in coapResources:
        add_content("{ \"" + resource['url'] +  "\", COAP_GET | COAP_MATCH_SUBTREE, _echo_handler, NULL }")
        i = len(resource['methods'])
        for method in resource['methods']:
            add_content(method)
            if i > 0:
                add_content(" | ")
        add_content(", " + resource['handler']['name'] + ", NULL }")
    

    add_content("\n};")

def generate_coap_resouces():
    coapRessources = []
    for coapJson in coapJsons:
        properties = coapJson['properties']
        for prop in properties:
            handler = prop['handler']
            if 0 == len(filter(lambda x: handler == x, coapRessources)):
                url = prop['url']
                methods = find_all_coap_methods(handler, 'properties', coapJsons)
                coapRessources.append({
                    'url': url,
                    'handler': handler,
                    'methods': methods
                })
    print(coapRessources)
    write_coap_resources(coapRessources)
     

if __name__ == '__main__':
    args = parser.parse_args()
    assert args.board, "ERROR: Argument board has to be defined"
    assert args.security, "ERROR: Argument security has to be defined"

    thingDefiniton = currentDirectory + "/config/wot_td/.thing.json"
    try:
        f = open(thingDefiniton)
        thingJson = json.loads(f.read())
    except IOError:
        print("ERROR: Thing definition in " + thingDefiniton + " is missing")
        sys.exit(0)
    except json.decoder.JSONDecodeError:
        print("ERROR: json in " + thingDefiniton + " is not valid")
        sys.exit(0)
    finally:
        f.close()
    
    coapAffordances = currentDirectory + "/config/wot_td/.coap_affordances.json"
    try:
        f = open(coapAffordances)
        coapJson = json.loads(f.read())
        validate_coap_affordances(coapJsons)
        coapJsons.append(coapJson)
    except IOError:
        print("INFO: Coap definition in " + coapAffordances + " not present")
    except json.decoder.JSONDecodeError:
        print("ERROR: json in " + coapAffordances + " is not valid")
        sys.exit(0)
    finally:
        f.close()
    generate_coap_resouces()
    print(result)