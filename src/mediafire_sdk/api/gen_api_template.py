#!/usr/bin/env python2

import argparse
import fnmatch
import json
import os
import re
import sys
import textwrap

default_http_input_method = 'POST'


class TSingle(object):
    pass


class TArray(object):
    pass


class TArrayFront(object):
    pass


class Optional(object):
    default_value = None

    def __init__(self, default_arg=None):
        self.default_value = default_arg


class Required(object):
    pass


class NoSessionToken(object):
    pass


class UnimplementedError(Exception):
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return repr(self.value)


class InvalidTType(Exception):
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return 'Is not a T* recognized type: ' + repr(self.value)


def pretty_json(json_obj):
    return json.dumps(json_obj, indent=4)


class InvalidParameter(Exception):
    def __init__(self, json_obj, key, value):
        self.json_obj = json_obj
        self.key = key
        self.value = value

    def __str__(self):
        return ('"' + str(self.key) + '" does not accept : ' +
                str(self.value) + ' in ' + pretty_json(self.json_obj))


class UnuniqueParameter(Exception):
    def __init__(self, json_obj, key):
        self.json_obj = json_obj
        self.key = key

    def __str__(self):
        return ('Duplicate parameter "' + str(self.key) +
                '" that should be unique to the API type in \n' +
                pretty_json(self.json_obj))


class RequiredParameter(Exception):
    def __init__(self, json_obj, value):
        self.json_obj = json_obj
        self.value = value

    def __str__(self):
        return ('Missing required parameter: \\"' + str(self.value) +
                '\\" in ' + pretty_json(self.json_obj))

# # Pull in API list
# execfile("templates/api_list.py")


def I(level):
    l = ''
    for i in range(level):
        l = l + '    '
    return l


def is_optional(opt):
    if opt is Optional or type(opt) is Optional:
        return True
    return False


def is_optional_no_default(opt):
    if (opt is Optional or
            type(opt) is Optional and
            opt.default_value is None):
        return True
    return False


def is_optional_with_default(opt):
    if (type(opt) is Optional and
            opt.default_value is not None):
        return True
    return False


def usage():
    print("Usage " + sys.argv[0] + " -path API_PATH")


def safe_json_get(json_obj, node_name):
    if node_name not in json_obj:
        raise RequiredParameter(json_obj, node_name)
    return json_obj[node_name]


def underscore_to_camelcase(value):
    '''Convert "an_underscore_string" to "AnUnderscoreString"'''
    value = str(value)

    def camelcase():
        while True:
            yield str.capitalize

    c = camelcase()
    return "".join(c.next()(x) if x else '_' for x in value.split("_"))


def get_api_path(api):
    (path_parts, path_parts_cpp_safe) = get_path_parts(api['api'])
    return '/' + '/'.join(path_parts)


def get_api_action_path(api):
    path_parts = []

    path_parts_raw = api['api'].split('/')

    for part in path_parts_raw:
        if len(part) == 0:
            pass
        elif not part[0].isdigit():
            path_parts.append(part)

    if len(path_parts) == 0:
        return
    if len(path_parts[0]) == 0:
        del path_parts[0]
    if len(path_parts) == 0:
        return
    if len(path_parts[-1]) == 0:
        del path_parts[-1]

    path = '/' + '/'.join(path_parts)
    if path.startswith('/api/'):
        return path[5:]
    return path


def parse_input_params(input_param_array, api):
    ret = list()
    for input_params in input_param_array:
        cpp_type = safe_json_get(input_params, "cpp_type")
        cpp_name = safe_json_get(input_params, "cpp_name")
        api_path = safe_json_get(input_params, "api_path")
        if 'http_method' in input_params:
            http_method = input_params['http_method']
        elif 'delivery_method' in api:
            # Use selected default
            http_method = api['delivery_method']
        else:
            http_method = default_http_input_method
        if 'description' in input_params:
            description = input_params['description']
        else:
            description = None
        ret.append((cpp_type, cpp_name, api_path, http_method, description))
    return ret


def parse_additional_input_params(input_param_array, api):
    ret = list()
    for input_params in input_param_array:
        cpp_type = safe_json_get(input_params, "cpp_type")
        cpp_name = safe_json_get(input_params, "cpp_name")
        api_path = safe_json_get(input_params, "api_path")
        if 'http_method' in input_params:
            http_method = input_params['http_method']
        elif 'delivery_method' in api:
            # Use selected default
            http_method = api['delivery_method']
        else:
            http_method = default_http_input_method
        ret.append((cpp_type, cpp_name, api_path, http_method))
    return ret


def parse_enum_params(enum_array):
    ret = list()
    for enum_json in enum_array:
        enum_cpp_name = safe_json_get(enum_json, "cpp_name")
        enum_members = safe_json_get(enum_json, "members")
        ret.append((enum_cpp_name, enum_members))
    return ret


def parse_enum_members(enum_members):
    ret = list()
    for enum_member in enum_members:
        enum_cpp_key = safe_json_get(enum_member, "cpp_name")
        enum_api_value = safe_json_get(enum_member, "api_value")

        additionals = []
        if "additional_api_values" in enum_member:
            additionals = enum_member["additional_api_values"]

        ret.append((enum_cpp_key, enum_api_value, additionals))
    return ret


def parse_return_struct(struct_list):
    ret = list()
    for struct in struct_list:
        struct_cpp_name = safe_json_get(struct, "cpp_name")
        struct_members = safe_json_get(struct, "members")
        ret.append((struct_cpp_name, struct_members))
    return ret


def get_version_string(version):
    return 'v' + str(version).replace('.', '_')


def get_cpp_template():
    return read_file("templates/api_template.cpp.txt")


def get_hpp_template():
    return read_file("templates/api_template.hpp.txt")


def get_all_hpp_template():
    return read_file("templates/all_hpp_template.hpp.txt")


def quote(s):
    return '\'' + str(s) + '\''


def dquote(s):
    return '"' + str(s) + '"'


def get_session_token_api_cpp_template(api):
    if not uses_session_token(api):
        return ''
    return read_file("templates/session_token_api.cpp.txt")


def get_session_token_api_hpp_template(api):
    if not uses_session_token(api):
        return ''
    return read_file("templates/session_token_api.hpp.txt")


def read_file(filepath):
    '''Read file to string.'''
    with open(filepath, "r") as f:
        data = f.read()
    return data


def write_contents(contents, filepath):
    '''Write a string to a file.'''

    # Make dir if it doesn't exist
    try:
        os.makedirs(os.path.dirname(filepath))
    except:
        pass

    try:
        old_contents = read_file(filepath)
        if contents == old_contents:
            return False
    except:
        pass

    with open(filepath, 'w') as f:
        f.write(contents)

    return True


def replace_by_dict(s, d):
    '''Fill out a template via key/value replacements.'''

    ret = s
    for k, v in d.items():
        ret = re.sub(k, v, ret)
    return ret


def get_path_parts(path):
    '''Split and sanitize the API path.'''

    path_parts_raw = path.split('/')

    path_parts = []
    path_parts_cpp_safe = []

    last_part = ''
    for part in path_parts_raw:
        if len(part) == 0:
            pass
        elif part[0].isdigit():
            path_parts.append(part)
        elif part == "delete":
            path_parts.append(part)  # This MUST stay as "delete"
            path_parts_cpp_safe.append(last_part + "_delete")
        elif part == "register":
            path_parts.append(part)  # This MUST stay as "register"
            path_parts_cpp_safe.append(last_part + "_register")
        else:
            path_parts.append(part)
            path_parts_cpp_safe.append(part)
        last_part = part

    if len(path_parts) == 0:
        return
    if len(path_parts[0]) == 0:
        del path_parts[0]
    if len(path_parts) == 0:
        return
    if len(path_parts[-1]) == 0:
        del path_parts[-1]

    return (path_parts, path_parts_cpp_safe)


def namespace_begin(base_parts):
    '''Header namespaces opening'''

    ret = []
    current_parts = []
    for part in base_parts:
        if part != 'api':
            current_parts.append(part)
            ret.append('/** API action path "' + '/'.join(current_parts) +
                       '" */')
        ret.append('namespace ' + part + ' {')
    return '\n'.join(ret)


def namespace_end(base_parts):
    '''Header namespaces closing'''

    ret = []
    for part in reversed(base_parts):
        ret.append('}  // namespace ' + part)
    return '\n'.join(ret)


def get_namespace_documentation(full_parts):
    action_path_parts = []
    for part in full_parts:
        if part != 'api':
            action_path_parts.append(part)
    return 'API action "' + '/'.join(action_path_parts) + '"'


def format_parameter_documentation(indent_level, parameter_name, description):
    start_description = '@param ' + parameter_name + ' '
    start_description_width = len(start_description)
    indent = ' ' * (indent_level * 4)
    line_start = indent + ' * '
    line_start_width = len(line_start)
    wrapped_lines = textwrap.wrap(description,
                                  80-line_start_width-start_description_width)
    line_num = 0
    ret = ''
    for line in wrapped_lines:
        if line_num == 0:
            ret = ret + '     * ' + start_description + line + '\n'
        else:
            ret = (ret + '     * ' + (' ' * start_description_width) + line +
                   '\n')
        line_num = line_num + 1
    return ret


def format_simple_documentation(indent_level, description):
    ret = ''

    indent = ' ' * (4*indent_level)

    comment = '/**'
    description_lines = textwrap.wrap(' ' + description,
                                      80 - len(indent) - len(comment))

    for i in range(len(description_lines)):
        line = indent + comment + description_lines[i]
        ret = ret + line
        if i == len(description_lines) - 1:
            # last line
            if len(line) > (80 - 3):
                ret = ret + '\n' + indent + ' */\n'
            else:
                ret = ret + ' */\n'
        else:
            ret = ret + '\n'
            comment = ' * '

    return ret


def get_cpp_optional_setters(api):
    '''Class member variable setter functions. These are used to modify the data
    'sent via REST.'''

    ret = ''
    if 'optional_input_params' in api:
        for optional_params in api['optional_input_params']:
            cpp_type = safe_json_get(optional_params, "cpp_type")
            cpp_name = safe_json_get(optional_params, "cpp_name")
            # api_path = safe_json_get(optional_params, "api_path")

            ret = ret + 'void Request::Set' + underscore_to_camelcase(cpp_name)
            ret = ret + '(' + cpp_type + ' ' + cpp_name + ')\n'
            ret = ret + '{\n'
            ret = ret + '    impl_->' + cpp_name + '_ = ' + cpp_name + ';\n'
            ret = ret + '}\n\n'
    return ret


def get_hpp_optional_setters(api):
    '''Class member variable setter functions. These are used to modify the data
    'sent via REST.'''

    ret = ''
    if 'optional_input_params' in api:
        for optional_params in api['optional_input_params']:
            cpp_type = safe_json_get(optional_params, "cpp_type")
            cpp_name = safe_json_get(optional_params, "cpp_name")
            api_path = safe_json_get(optional_params, "api_path")

            ret = ret + '    /**\n'
            ret = ret + '     * Optional API parameter "' + api_path + '"\n'
            ret = ret + '     *\n'

            if 'description' in optional_params:
                description = optional_params['description']
            else:
                description = ('Set parameter "' + api_path +
                               '" in API request.')

            ret = ret + format_parameter_documentation(1, cpp_name,
                                                       description)
            ret = ret + '     */\n'

            ret = ret + '    void Set' + underscore_to_camelcase(cpp_name)
            ret = ret + '(' + cpp_type + ' ' + cpp_name + ');\n\n'
    return ret


def uses_session_token(api):
    if 'session_token' in api and api['session_token'] is False:
        return False
    return True


def delivery_method(api):
    if 'delivery_method' in api:
        return api['delivery_method']

    # If has POST data must be POST
    if 'input_params' in api:
        for (cpp_type, cpp_name, api_path, http_method, description) \
                in parse_input_params(api['input_params'], api):
            if http_method == 'POST':
                return 'POST'
    if 'optional_input_params' in api:
        for (cpp_type, cpp_name, api_path, http_method, description) \
                in parse_input_params(api['optional_input_params'], api):
            if http_method == 'POST':
                return 'POST'

    # Session token types should deliver via POST if not overridden
    if uses_session_token(api):
        return 'POST'

    # If no parameters or all parameters are GET, then it is GET
    return 'GET'


def get_cpp_post_data_impl_decl(api):
    if delivery_method(api) == 'POST':
        return '''
    mf::http::SharedBuffer::Pointer GetPostData();

    mf::api::RequestMethod GetRequestMethod() const
    {
        return mf::api::RequestMethod::Post;
    }
'''
    else:
        return '''
    mf::api::RequestMethod GetRequestMethod() const
    {
        return mf::api::RequestMethod::Get;
    }
'''


def get_cpp_post_data_impl_def(api):
    if delivery_method(api) == 'GET':
        return ''

    ret = '''mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;

'''
    # if (full_description_)
    #     parts["full"] = *full_description_;
    params_set_count = 0

    if 'input_params' in api:
        lines = []
        for (cpp_type, cpp_name, api_path, http_method, description) \
                in parse_input_params(api['input_params'], api):
            if http_method == 'POST':
                params_set_count = params_set_count + 1
                line = 'parts["' + api_path + '"] = '
                if cpp_type == 'std::string':
                    line = line + cpp_name + '_;'
                else:
                    line = line + 'AsString(' + cpp_name + '_);'
                lines.append(line)
        if len(lines) > 0:
            ret = ret + '    ' + '\n    '.join(lines) + '\n'

    if 'optional_input_params' in api:
        lines = []
        for (cpp_type, cpp_name, api_path, http_method, description) \
                in parse_input_params(api['optional_input_params'], api):
            if http_method == 'POST':
                params_set_count = params_set_count + 1
                line = 'if (' + cpp_name + '_)\n'
                line = line + '        parts["' + api_path + '"] = '
                if cpp_type == 'std::string':
                    line = line + '*' + cpp_name + '_;'
                else:
                    line = line + 'AsString(*' + cpp_name + '_);'
                lines.append(line)
        if len(lines) > 0:
            ret = ret + '    ' + '\n    '.join(lines) + '\n'

    ret = ret + '''
    std::string post_data = MakePost(api_path + ".php", parts);
    AddDebugText(" POST data: " + post_data + "\\\\n");
    return mf::http::SharedBuffer::Create(post_data);
}

'''
    return ret


def get_hpp_post_data_template(api):
    if delivery_method(api) == 'POST':
        return '''    /** Requester optional method. */
    mf::http::SharedBuffer::Pointer GetPostData();

'''
    else:
        return ''


def get_cpp_post_data_template(api):
    if delivery_method(api) == 'POST':
        return '''mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

'''
    else:
        return ''


def get_class_member_vars(api):
    '''Class member variables for class declaration.'''

    ret = ''
    if 'input_params' in api:
        lines = []

        for (cpp_type, cpp_name, api_path, http_method, description) \
                in parse_input_params(api['input_params'], api):
            lines.append('    ' + cpp_type + ' ' + cpp_name + '_;')

        if len(lines) > 0:
            ret = ret + '\n'.join(lines) + '\n'
    if 'optional_input_params' in api:
        lines = []
        for optional_params in api['optional_input_params']:
            cpp_type = safe_json_get(optional_params, "cpp_type")
            cpp_name = safe_json_get(optional_params, "cpp_name")
            # api_path = safe_json_get(optional_params, "api_path")
            lines.append('    boost::optional<' + cpp_type + '> ' + cpp_name +
                         '_;')
        if len(lines) > 0:
            ret = ret + '\n'.join(lines) + '\n'
    return ret


def get_cpp_ctor_args(api):
    '''Constructor arguments for class definition.'''

    ret = []
    if 'input_params' in api:
        lines = []
        for (cpp_type, cpp_name, api_path, http_method, description) \
                in parse_input_params(api['input_params'], api):
            lines.append(cpp_type + ' ' + cpp_name)
        if len(lines) > 0:
            ret.append('\n        ' + ',\n        '.join(lines) + '\n    ')
    return '\n'.join(ret)


def get_cpp_ctor(api, cpp_variable_names):
    '''Additional CPP Constructor definition.'''

    input_args = ''
    lines = []
    for cpp_name in cpp_variable_names:
        cpp_type, api_path, http_method, description = \
                                    get_input_cpp_variable_info(api, cpp_name)
        lines.append(cpp_type + ' ' + cpp_name)
    if len(lines) > 0:
        input_args = '\n        ' + ',\n        '.join(lines) + '\n    '

    passthru_args = []
    for cpp_name in cpp_variable_names:
        passthru_args.append(cpp_name)

    ret = ''
    ret = ret + 'Request::Request(' + input_args + ') :\n'
    ret = ret + '    impl_(new Impl(' + ', '.join(passthru_args) + '))\n'

    ret = ret + '{\n}\n'
    return ret


def get_default_ctor_cpp_names(api):
    ret = []
    if 'input_params' in api:
        for (cpp_type, cpp_name, api_path, http_method, description) \
                in parse_input_params(api['input_params'], api):
            ret.append(cpp_name)
    return ret


def get_cpp_ctors(api):
    '''Constructors for class definition.'''
    if 'constructors' in api:
        ctors = []
        for additional_ctor in api['constructors']:
            cpp_variable_names = additional_ctor['cpp_variable_names']
            ctors.append(get_cpp_ctor(api, cpp_variable_names))
    else:
        ctors = [get_cpp_ctor(api, get_default_ctor_cpp_names(api))]
    return '\n\n'.join(ctors)


def get_impl_ctor(api, cpp_variable_names):
    '''CPP Constructor definition.'''

    input_args = ''
    lines = []
    for cpp_name in cpp_variable_names:
        cpp_type, api_path, http_method, description = \
                                    get_input_cpp_variable_info(api, cpp_name)
        lines.append(cpp_type + ' ' + cpp_name)
    if len(lines) > 0:
        input_args = '\n        ' + ',\n        '.join(lines) + '\n    '

    init_args = ''
    if len(cpp_variable_names) > 0:
        lines = []
        for cpp_name in cpp_variable_names:
            cpp_type, api_path, http_method, description = \
                                        get_input_cpp_variable_info(api,
                                                                    cpp_name)
            lines.append(cpp_name + '_(' + cpp_name + ')')
        init_args = ' :\n    ' + ',\n    '.join(lines)

    ret = ''
    ret = ret + 'Impl::Impl(' + input_args + ')'
    ret = ret + init_args + '\n'

    ret = ret + '{\n}\n'
    return ret


def get_impl_ctor_definitions(api):
    if 'constructors' in api:
        ctors = []
        for additional_ctor in api['constructors']:
            cpp_variable_names = additional_ctor['cpp_variable_names']
            ctors.append(get_impl_ctor(api, cpp_variable_names))
    else:
        ctors = [get_impl_ctor(api, get_default_ctor_cpp_names(api))]
    return '\n\n'.join(ctors)


def get_impl_passthru_args(api):
    '''Constructor arguments for Impl.'''

    names = []
    if 'input_params' in api:
        for (cpp_type, cpp_name, api_path, http_method, description) \
                in parse_input_params(api['input_params'], api):
            names.append(cpp_name)
    return ', '.join(names)


def get_hpp_ctor_args(api):
    '''The arguments of the CTOR in its declaration in the header.'''

    ret = []
    if 'input_params' in api:
        lines = []
        for (cpp_type, cpp_name, api_path, http_method, description) \
                in parse_input_params(api['input_params'], api):
            lines.append(cpp_type + ' ' + cpp_name)
        ret.append('\n            ' + ',\n            '.join(lines) +
                   '\n        ')
    return '\n'.join(ret)


def get_hpp_ctor_documentation(api):
    ret = '/**\n'
    ret = ret + '     * API request "' + get_api_action_path(api) + '"\n'

    if 'input_params' in api:
        ret = ret + '     *\n'
        for input_params in api['input_params']:
            # cpp_type = safe_json_get(input_params, "cpp_type")
            cpp_name = safe_json_get(input_params, "cpp_name")
            api_path = safe_json_get(input_params, "api_path")

            if 'description' in input_params:
                description = input_params['description']
            else:
                description = 'API parameter "' + api_path + '"'

            ret = ret + format_parameter_documentation(1, cpp_name,
                                                       description)

    ret = ret + '     */'

    return ret


def get_input_cpp_variable_info(api, cpp_name_to_find):
    if 'input_params' in api:
        for (cpp_type, cpp_name, api_path, http_method, description) \
                in parse_input_params(api['input_params'], api):
            if cpp_name == cpp_name_to_find:
                return (cpp_type, api_path, http_method, description)
    if 'optional_input_params' in api:
        for (cpp_type, cpp_name, api_path, http_method, description) \
                in parse_input_params(api['optional_input_params'], api):
            if cpp_name == cpp_name_to_find:
                return (cpp_type, api_path, http_method, description)
    return None


def get_hpp_ctor(api, cpp_variable_names, make_doc, class_name,
                 ctor_description):
    doc = '    /**\n'
    doc = doc + '     * API request "' + get_api_action_path(api) + '"\n'

    if ctor_description is not None:
        doc = doc + '     *\n'
        doc = doc + '     * ' + ctor_description + '\n'

    if len(cpp_variable_names) > 0:
        doc = doc + '     *\n'

    for cpp_name in cpp_variable_names:
        cpp_type, api_path, http_method, description = \
                                    get_input_cpp_variable_info(api, cpp_name)

        if description is None:
            description = 'API parameter "' + api_path + '"'

        doc = doc + format_parameter_documentation(1, cpp_name, description)
    doc = doc + '     */'

    args = ''
    lines = []
    for cpp_name in cpp_variable_names:
        cpp_type, api_path, http_method, description = \
                                    get_input_cpp_variable_info(api, cpp_name)
        lines.append(cpp_type + ' ' + cpp_name)
        args = ('\n            ' + ',\n            '.join(lines) +
                '\n        ')
    ret = ''
    if make_doc is True:
        ret = ret + doc + '\n'

    ret = (ret + '    ' + get_explicit(api) + class_name + '(' + args + ');')

    return ret


def get_hpp_ctors(api):
    if 'constructors' in api:
        ctors = []
        for additional_ctor in api['constructors']:
            cpp_variable_names = additional_ctor['cpp_variable_names']
            description = None
            if 'description' in additional_ctor:
                description = additional_ctor['description']
            ctors.append(get_hpp_ctor(api, cpp_variable_names,
                                      True, 'Request', description))
    else:
        ctors = [get_hpp_ctor(api, get_default_ctor_cpp_names(api), True,
                              'Request', None)]
    return '\n\n'.join(ctors)


def get_impl_ctor_declarations(api):
    if 'constructors' in api:
        ctors = []
        for additional_ctor in api['constructors']:
            cpp_variable_names = additional_ctor['cpp_variable_names']
            ctors.append(get_hpp_ctor(api, cpp_variable_names,
                                      False, 'Impl', None))
    else:
        ctors = [get_hpp_ctor(api, get_default_ctor_cpp_names(api), False,
                              'Impl', None)]
    return '\n\n'.join(ctors)


def get_url_creation(api):
    '''Create the url using required and optional input parameters.'''

    ret = []
    params_set_count = 0

    if 'input_params' in api:
        lines = []
        for (cpp_type, cpp_name, api_path, http_method, description) \
                in parse_input_params(api['input_params'], api):
            if http_method == 'GET':
                params_set_count = params_set_count + 1
                line = 'parts["' + api_path + '"] = '
                if cpp_type == 'std::string':
                    line = line + cpp_name + '_;'
                else:
                    line = line + 'AsString(' + cpp_name + '_);'
                lines.append(line)
        if len(lines) > 0:
            ret.append('    ' + '\n    '.join(lines) + '\n')

    if 'optional_input_params' in api:
        lines = []
        for (cpp_type, cpp_name, api_path, http_method, description) \
                in parse_input_params(api['optional_input_params'], api):
            if http_method == 'GET':
                params_set_count = params_set_count + 1
                line = 'if (' + cpp_name + '_)\n'
                line = line + '        parts["' + api_path + '"] = '
                if cpp_type == 'std::string':
                    line = line + '*' + cpp_name + '_;'
                else:
                    line = line + 'AsString(*' + cpp_name + '_);'
                lines.append(line)
        if len(lines) > 0:
            ret.append('    ' + '\n    '.join(lines) + '\n')

    if params_set_count > 0:
        return '''
    std::map<std::string, std::string> & parts = *query_parts;
''' + '\n'.join(ret)
    return ''


def get_contained_types(type_name_set):
    return_set = set()
    for type_name in type_name_set:
        extracted_name = re.sub(r".*<([^<>]*)>.*", r"\1", type_name)
        if len(extracted_name) > 0:
            return_set.add(extracted_name)
    return return_set


def get_as_string_funcs(api):
    '''Create AsString functions, but only if they are needed.'''

    if 'enums' not in api:
        return ''

    enum_names = get_enum_names(api)

    type_names = set()
    if 'input_params' in api:
        for (cpp_type, cpp_name, api_path, http_method, description) \
                in parse_input_params(api['input_params'], api):
            type_names.add(cpp_type)
    if 'optional_input_params' in api:
        for optional_params in api['optional_input_params']:
            cpp_type = safe_json_get(optional_params, "cpp_type")
            type_names.add(cpp_type)

    type_names = type_names.union(
        get_contained_types(type_names))

    enums_to_process = set()
    for type_name in type_names:
        if type_name in enum_names:
            enums_to_process.add(type_name)

    funcs = []
    for (enum_cpp_name, enum_members) in parse_enum_params(api['enums']):

        if enum_cpp_name not in enums_to_process:
            continue
        lines = []
        version_str = get_version_string(api['version'])

        n = version_str + '::' + enum_cpp_name
        lines.append('std::string AsString(const ' + n + ' & value)')
        lines.append('{')
        for (enum_cpp_key, enum_api_value, additional_api_values) \
                in parse_enum_members(enum_members):
            t = '::'.join([version_str, enum_cpp_name, enum_cpp_key])
            lines.append('    if (value == ' + t + ')')
            lines.append('        return "' + enum_api_value + '";')

        lines.append(
            '    return mf::utils::to_string(static_cast<uint32_t>(value));')
        lines.append('}')
        funcs.append('\n'.join(lines))

    if len(funcs) > 0:
        return '''
namespace {
''' + '\n'.join(funcs) + '''
}  // namespace
'''
    else:
        return ''


def create_content_struct_parse(api, ob, t, full_type_name, n, r, dt, optional,
                                pt):
    '''Parse return JSON when type is an struct.'''
    ret = ''

    if dt is TArray:
        ret = ret + '''
    // create_content_struct_parse TArray
    try {
        const boost::property_tree::wptree & branch =
            '''+pt+'''.get_child(L"'''+r+'''");
        ''' + ob + '''->''' + n + '''.reserve( '''+pt+'''.size() );

        for ( auto & it : branch )
        {
            ''' + full_type_name + ''' optarg;
            if ( ''' + t + '''FromPropertyBranch(
                    response, &optarg, it.second) )
                ''' + ob + '''->''' + n + '''.push_back(std::move(optarg));'''
        if optional is Required:
            ret = ret + '''
            else
                return;  // error set already'''
        ret = ret + '''
        }
    }
    catch(boost::property_tree::ptree_bad_path & err)
    {'''
        if optional is Required:
            ret = ret + '''
        // JSON response still has element if no files were returned.
        // This is really an error.
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \\"'''+r+'''\\"");'''
        else:
            ret = ret + '''
        // Is optional'''
        ret = ret + '''
    }'''

    elif dt is TSingle:
        ret = ret + '''
    // create_content_struct_parse TSingle
    try {
        const boost::property_tree::wptree & branch =
            '''+pt+'''.get_child(L"'''+r+'''");

        ''' + full_type_name + ''' optarg;
        if ( ''' + t + '''FromPropertyBranch(
                response, &optarg, branch) )
        {
            ''' + ob + '''->''' + n + ''' = std::move(optarg);
        }'''
        if optional is Required:
            ret = ret + '''
        else
        {
            // JSON response still has element if expected.
            // This is really an error.
            return_error(
                mf::api::api_code::ContentInvalidData,
                "missing \\"''' + r + '''\\"");
        }'''

        ret = ret + '''
    }
    catch(boost::property_tree::ptree_bad_path & err)
    {'''
        if optional is Required:
            ret = ret + '''
        // JSON response still has element if no files were returned.
        // This is really an error.
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \\"'''+r+'''\\"");'''
        else:
            ret = ret + '''
        // Is optional'''
        ret = ret + '''
    }'''
    elif dt is TArray:
        raise UnimplementedError(
            "Parsing struct as array front not implemented")
    else:
        raise InvalidTType(dt)

    return ret


def create_content_enum_parse(api, ob, t, n, r, dt, optional, pt):
    '''Parse return JSON when type is an enum.'''

    ret = '''
    {
        std::string optval;'''

    if dt is TSingle:
        ret = ret + '''
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                ''' + pt + ''',
                "''' + r + '''",
                &optval) )'''
    elif dt is TArrayFront:
        ret = ret + '''
        // create_content_enum_parse TArrayFront
        if ( GetIfExistsArrayFront(
                ''' + pt + ''',
                "''' + r + '''",
                &optval) )'''
    elif dt is TArray:
        raise UnimplementedError("Parsing enums in array is not implemented")
    else:
        raise InvalidTType(dt)

    ret = ret + '''
        {
'''
    cases = []
    for (enum_cpp_name, enum_members) in parse_enum_params(api['enums']):
        if enum_cpp_name == t:
            for (enum_cpp_key, enum_api_value, additional_api_values) \
                    in parse_enum_members(enum_members):
                values = [enum_api_value] + additional_api_values
                for value in values:
                    case = 'if ( optval == "' + value + '" )\n'
                    case = (case + '                ' + ob + '->' + n + ' = ' +
                            enum_cpp_name + '::' + enum_cpp_key + ';')
                    cases.append(case)
            ret = ret + '            ' + "\n            else ".join(cases)
            if optional is Required:
                ret = ret + '''
            else
                return_error(
                    mf::api::api_code::ContentInvalidData,
                    "invalid value in ''' + r + '''");'''

    ret = ret + '''
        }'''
    if optional is Required:
        ret = ret + '''
        else
            return_error(
                mf::api::api_code::ContentInvalidData,
                "no value in ''' + r + '''");'''
    ret = ret + '''
    }'''

    return ret


def create_content_parse_array(api, ob, t, n, r, dt, optional, pt):
    '''Parse return JSON when type is an array.'''

    ret = '''
    // create_content_parse_array TArray
    try {
        const boost::property_tree::wptree & branch =
            ''' + pt + '''.get_child(L"''' + r + '''");
        ''' + ob + '''->''' + n + '''.reserve( branch.size() );'''

    if optional is Required:
        ret = ret + '''
        if (branch.empty())
        {
            return_error(
                mf::api::api_code::ContentInvalidData,
                "missing value in ''' + r + '''");
        }'''

    ret = ret + '''
        for ( auto & it : branch )
        {
            ''' + t + ''' result;
            if ( GetValueIfExists(
                    it.second,
                    &result ) )
            {
                ''' + ob + '''->''' + n + '''.push_back(result);
            }
            else
            {'''

    if optional is Required:
        ret = ret + '''
                return_error(
                    mf::api::api_code::ContentInvalidData,
                    "invalid value in ''' + r + '''");'''
    else:
        ret = ret + '''
                break;'''

    ret = ret + '''
            }
        }
    }
    catch(boost::property_tree::ptree_bad_path & err)
    {'''

    if optional is Required:
        ret = ret + '''
        // JSON response still has element if expected.
        // This is really an error.
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \\"''' + r + '''\\"");'''
    else:
        ret = ret + '''
        // The value is optional.'''

    ret = ret + '''
    }'''

    return ret


def create_content_parse_array_front(api, ob, t, n, r, dt, optional, pt):
    '''Parse return JSON when type is an array where we only care about the
    first value.'''

    if is_optional_with_default(optional):
        return '''
    // create_content_parse_array_front optional with default
    GetIfExistsArrayFront(
            ''' + pt + ''',
            "''' + r + '''",
            &''' + ob + '''->''' + n + ''');'''
    if is_optional_no_default(optional):
        return '''
    // create_content_parse_array_front optional no default
    {
        ''' + t + ''' optarg;
        if ( GetIfExistsArrayFront(
                ''' + pt + ''',
                "''' + r + '''",
                &optarg) )
        {
            ''' + ob + '''->''' + n + ''' = optarg;
        }
    }'''
    else:
        return '''
    // create_content_parse_array_front required
    if ( ! GetIfExistsArrayFront(
            ''' + pt + ''',
            "''' + r + '''",
            &''' + ob + '''->''' + n + ''' ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \\"''' + r + '''\\"");'''


def create_content_parse_single(api, ob, t, n, r, dt, optional, pt):
    '''Parse return JSON when type is normal.'''

    if is_optional_with_default(optional):
        return '''
    // create_content_parse_single optional with default
    GetIfExists(
            ''' + pt + ''',
            "''' + r + '''",
            &''' + ob + '''->''' + n + ''');'''
    if is_optional_no_default(optional):
        return '''
    // create_content_parse_single optional no default
    {
        ''' + t + ''' optarg;
        if ( GetIfExists(
                ''' + pt + ''',
                "''' + r + '''",
                &optarg) )
        {
            ''' + ob + '''->''' + n + ''' = optarg;
        }
    }'''
    else:
        return '''
    // create_content_parse_single required
    if ( ! GetIfExists(
            ''' + pt + ''',
            "''' + r + '''",
            &''' + ob + '''->''' + n + ''' ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \\"''' + r + '''\\"");'''


def create_content_parse(api, ob, t, n, r, parse_data_type, optional, pt):
    '''Parse return JSON when type is not an enum.'''

    if parse_data_type is TArray:
        return create_content_parse_array(api, ob, t, n, r, parse_data_type,
                                          optional, pt)
    elif parse_data_type is TArrayFront:
        return create_content_parse_array_front(api, ob, t, n, r,
                                                parse_data_type, optional, pt)
    elif parse_data_type is TSingle:
        return create_content_parse_single(api, ob, t, n, r, parse_data_type,
                                           optional, pt)
    else:
        raise InvalidTType(parse_data_type)


def create_data_type(indent_level, cpp_type, cpp_name, r, parse_data_type,
                     optional, description):
    '''Generate a member of the api return data type.'''

    ret = ''

    indent = ' ' * (4*indent_level)

    ret = ret + format_simple_documentation(indent_level, description)

    if parse_data_type is TArray:
        ret = (ret + indent + 'std::vector<' + cpp_type + '> ' + cpp_name +
               ';\n')
    elif is_optional_no_default(optional):
        ret = (ret + indent + 'boost::optional<' + cpp_type + '> ' + cpp_name +
               ';\n')
    else:
        ret = ret + indent + cpp_type + ' ' + cpp_name + ';\n'

    return ret


def has_content_to_return(api):
    if 'return_params' not in api or len(api['return_params']) == 0:
        return False
    return True


def get_pr_opt_adt(api):
    '''ParseResponse argument name "response". Prevents unused variable C++
    warnings.'''

    if has_content_to_return(api) == False:
        return '/* response */'
    return 'response'


def get_data_ctor(api):
    '''Generate the constructor for the API return datatype if needed.'''

    if 'return_params' not in api or len(api['return_params']) == 0:
        return ''

    enum_names = get_enum_names(api)

    args_with_defaults = []
    for ret_data in api['return_params']:
        (cpp_type, cpp_name, api_path, json_type, optional, description) \
            = get_return_parameters(ret_data)
        if is_optional_with_default(optional):
            if cpp_type in enum_names:
                args_with_defaults.append((cpp_name, cpp_type + '::' +
                                           optional.default_value))
            else:
                args_with_defaults.append((cpp_name, optional.default_value))
    if len(args_with_defaults) == 0:
        return ''

    initializers = []
    for (n, v) in args_with_defaults:
        initializers.append(n + '(' + v + ')')

    return '''    ResponseData() :
        ''' + ',\n        '.join(initializers) + '''
    {}
'''


def get_data_types(api):
    '''Generate the members of the api return data type.'''

    if 'return_params' not in api or len(api['return_params']) == 0:
        return ''
    data_type_sections = []
    for ret_data in api['return_params']:
        (cpp_type, cpp_name, api_path, json_type, optional, description) \
            = get_return_parameters(ret_data)
        data_type_sections.append(create_data_type(1, cpp_type, cpp_name,
                                                   api_path, json_type,
                                                   optional, description))
    return '\n'.join(data_type_sections)


def get_data_type_struct(name, param_list):
    data_type_sections = []
    for ret_data in param_list:
        (cpp_type, cpp_name, api_path, json_type, optional, description) \
            = get_return_parameters(ret_data)
        data_type_sections.append(create_data_type(2, cpp_type, cpp_name,
                                                   api_path, json_type,
                                                   optional, description))
    inside = '\n'.join(data_type_sections)
    top = '    struct ' + name + '\n    {\n'
    bottom = '    };'
    return top + inside + bottom


def get_data_type_structs(api):
    '''Generate structs for the API data return type.'''

    if 'return_structs' not in api or len(api['return_structs']) == 0:
        return ''
    ret = []
    for (struct_cpp_name, struct_members) \
            in parse_return_struct(api['return_structs']):
        ret.append(get_data_type_struct(struct_cpp_name, struct_members))
    if len(ret) > 0:
        return '\n'.join(ret) + '\n'
    return ''


def get_data_type_struct_extractor(api, name, param_list):
    version_str = get_version_string(api['version'])

    lines = []
    lines.append(I(0) + '// get_data_type_struct_extractor begin')
    # lines.append(I(0) + 'using ' + version_str + '::' + datatype_name + ';')
    lines.append(I(0) + 'using namespace ' + version_str + ';  // NOLINT')
    lines.append(I(0) + 'bool ' + name + 'FromPropertyBranch(')
    lines.append(I(2) + 'Response * response,')
    lines.append(I(2) + 'ResponseData::' + name + ' * value,')
    lines.append(I(2) + 'const boost::property_tree::wptree & pt')
    lines.append(I(1) + ')')
    lines.append(I(0) + '{')
    lines.append(I(0) + '#   define return_error(error_type, error_message)' +
                 '                             \\')
    lines.append(I(1) + '{                                                 ' +
                 '                         \\')
    lines.append(I(2) + 'response->error_code = make_error_code( ' +
                 'error_type );' +
                 '                  \\')
    lines.append(I(2) + 'response->error_string = error_message;           ' +
                 '                     \\')
    lines.append(I(2) + 'return false;                                     ' +
                 '                     \\')
    lines.append(I(1) + '}')

    lines.append(I(1) + 'using mf::api::GetIfExists;')
    lines.append(I(1) + 'using mf::api::GetValueIfExists;')
    lines.append(I(1) + 'if (pt.size() == 0)  // Stop if branch is empty')
    lines.append(I(2) + 'return false;')

    top = '\n'.join(lines) + '\n'

    middle = content_parse_branch(api, 'value', 'pt', param_list) + '\n\n'

    lines = []
    lines.append(I(1) + '// get_data_type_struct_extractor conclusion')
    lines.append(I(1) + 'return true;')
    lines.append(I(0) + '#   undef return_error')
    lines.append(I(0) + '}')

    bottom = '\n'.join(lines)

    return top + middle + bottom


def get_data_type_struct_extractors(api):
    '''Create struct extractor functions.'''

    if 'return_structs' not in api:
        return ''

    ret = []

    for (struct_cpp_name, struct_members) \
            in parse_return_struct(api['return_structs']):
        ret.append(get_data_type_struct_extractor(api, struct_cpp_name,
                                                  struct_members))

    if len(ret) > 0:
        return '''
namespace {
''' + '\n'.join(ret) + '''
}  // namespace
'''
    else:
        return ''


def get_enum_names(api):
    '''Set of enum names.'''

    ret = set()
    if 'enums' not in api:
        return ret
    for (enum_cpp_name, enum_members) in parse_enum_params(api['enums']):
        ret.add(enum_cpp_name)
    return ret


def get_struct_names(api):
    '''Set of return struct names.'''

    ret = set()
    if 'return_structs' not in api:
        return ret
    for (struct_cpp_name, struct_members) \
            in parse_return_struct(api['return_structs']):
        ret.add(struct_cpp_name)
    return ret


def get_return_parameters(param):
    if 'cpp_type' not in param:
        raise RequiredParameter(param, 'cpp_type')
    cpp_type = param['cpp_type']

    if 'cpp_name' not in param:
        raise RequiredParameter(param, 'cpp_name')
    cpp_name = param['cpp_name']

    if 'api_path' not in param:
        raise RequiredParameter(param, 'api_path')
    api_path = param['api_path']

    json_type = TSingle
    if 'json_type' in param:
        if param['json_type'] == 'single':
            json_type = TSingle
        elif param['json_type'] == 'array':
            json_type = TArray
        elif param['json_type'] == 'array_front':
            json_type = TArrayFront
        else:
            raise InvalidParameter(param, 'json_type', param['json_type'])

    optional = Required

    if 'default_value' in param:
        optional = Optional(param['default_value'])

    if 'optional' in param and param['optional'] is True:
        optional = Optional

    if 'description' in param:
        description = param['description']
    else:
        description = 'API response field "' + api_path + '"'

    return (cpp_type, cpp_name, api_path, json_type, optional, description)


def content_parse_branch(api, ob, pt, return_param_list):
    '''Create code to parse return JSON.'''

    enum_names = get_enum_names(api)
    struct_names = get_struct_names(api)
    ret = []

    for ret_data in return_param_list:
        (cpp_type, cpp_name, api_path, json_type, optional, description) \
            = get_return_parameters(ret_data)

        if is_optional_with_default(optional):
            if cpp_type in enum_names:
                ret.append('    ' + ob + '->' + cpp_name + ' = ' + cpp_type +
                           '::' + optional.default_value + ';')
            else:
                ret.append('    ' + ob + '->' + cpp_name + ' = ' +
                           optional.default_value + ';')

    for ret_data in return_param_list:
        (cpp_type, cpp_name, api_path, json_type, optional, description) \
            = get_return_parameters(ret_data)

        if cpp_type in enum_names:
            ret.append(create_content_enum_parse(api, ob, cpp_type, cpp_name,
                                                 api_path, json_type, optional,
                                                 pt))

        elif cpp_type in struct_names:
            full_type_name = 'ResponseData::' + cpp_type
            ret.append(
                create_content_struct_parse(
                    api, ob, cpp_type, full_type_name, cpp_name, api_path,
                    json_type, optional, pt))
        else:
            ret.append(create_content_parse(api, ob, cpp_type, cpp_name,
                                            api_path, json_type, optional, pt))
    return '\n'.join(ret)


def get_content_parsing(api):
    '''Create code to parse return JSON.'''

    ret = ''

    if has_content_to_return(api) == True:
        ret = ret + """\
    ResponseData response_data;

    // For uniformity for code generation with the other content parsers.
    ResponseData * response_data_ptr = &response_data;
"""

    if 'return_params' in api:
        ret = ret + content_parse_branch(api, 'response_data_ptr', 'response->pt',
                                         api['return_params'])

    if has_content_to_return(api) == True:
        ret = ret + """

    // Only on success, return parsed data structure with response
    response->response_data = std::move(response_data); """

    return ret


def create_enum(enum_cpp_name, enum_members):
    '''Define an enum for the header.'''

    ret = ''

    ret = ret + 'enum class ' + enum_cpp_name + '\n'
    ret = ret + '{\n'

    indent = '    '

    members = parse_enum_members(enum_members)
    member_len = len(members)
    for i in range(member_len):
        (enum_cpp_key, enum_api_value, additional_api_values) = members[i]

        description = 'API value "' + enum_api_value + '"'

        ret = ret + format_simple_documentation(1, description)

        if i == member_len - 1:
            ret = ret + indent + enum_cpp_key + '\n'
        else:
            ret = ret + indent + enum_cpp_key + ',\n'

    ret = ret + '};\n'

    return ret


def get_enums(api):
    '''Define the enums for the header.'''

    if 'enums' not in api:
        return ''
    ret = []
    for (enum_cpp_name, enum_members) in parse_enum_params(api['enums']):
        ret.append(create_enum(enum_cpp_name, enum_members))
    return '\n' + '\n'.join(ret)


def get_hpp_namespaced_enums(api):
    '''Bring enums into the Request class namespace.'''

    if 'enums' not in api:
        return ''
    ret = []
    for (enum_cpp_name, enum_members) in parse_enum_params(api['enums']):
        ret.append('    using ' + enum_cpp_name + ' = enum ' + enum_cpp_name + ';')
    comment = '\n    // Enums in class namespace for usage with templates\n'
    return comment + '\n'.join(ret) + '\n\n'


def get_explicit(api):
    '''Class constructor should be explicit when only one parameter.'''

    if 'input_params' in api and len(api['input_params']) == 1:
        return 'explicit '
    return ''


def get_query_parts_arg(api):
    '''BuildUrl optional argument name. Prevents unused variable C++
    warnings.'''

    params_set_count = 0

    if 'input_params' in api:
        params_set_count = params_set_count + len(api['input_params'])
    if 'optional_input_params' in api:
        params_set_count = params_set_count + len(api['optional_input_params'])

    if params_set_count == 0:
        return '/* query_parts */'
    return 'query_parts'


def get_api_request_base(api):
    '''The base class for the Request class.'''

    if uses_session_token(api):
        base_class = 'SessionTokenApiBase<Response>'
    else:
        base_class = 'TokenlessApiBase<Response>'

    return base_class


def get_api_request_base_header(api):
    '''The base class header for the Request class.'''

    if uses_session_token(api):
        base_class_header = 'mediafire_sdk/api/session_token_api_base.hpp'
    else:
        base_class_header = 'mediafire_sdk/api/tokenless_api_base.hpp'

    return base_class_header


def get_version_documentation(api):
    return 'API path "' + get_api_path(api) + '"'


def get_additional_includes(api, name, create_n):
    '''Create includes.'''

    ret_str = ''

    if name in api:
        for header in api[name]:
            ret_str = create_n(header)

    return ret_str


def get_additional_cpp_system_includes(api):
    '''Create system cpp includes.'''

    return get_additional_includes(
        api,
        'system_cpp_includes',
        lambda header: '#include <' + header + '>\n')


def get_additional_cpp_local_includes(api):
    '''Create local cpp includes.'''

    includes = get_additional_includes(
        api,
        'local_cpp_includes',
        lambda header: '#include "' + header + '"\n')

    return includes


def get_additional_hpp_system_includes(api):
    '''Create system hpp includes.'''

    return get_additional_includes(
        api,
        'system_hpp_includes',
        lambda header: '#include <' + header + '>\n')


def get_additional_hpp_local_includes(api):
    '''Create local hpp includes.'''

    includes = get_additional_includes(
        api,
        'local_hpp_includes',
        lambda header: '#include "' + header + '"\n')

    if delivery_method(api) == 'POST':
        includes = (includes +
                    '#include "mediafire_sdk/http/shared_buffer.hpp"\n')

    return includes


def create_templates(api, target_path):
    '''Create the templates needed for a single api type.'''

    template_data = {}

    version_str = get_version_string(api['version'])
    template_data['version'] = version_str

    (path_parts, path_parts_cpp_safe) = get_path_parts(api['api'])
    # api_path = '/' + '/'.join(path_parts)
    relative_path = '/'.join(path_parts_cpp_safe[1:])
    # full_path = os.path.join(target_path, relative_path)
    template_data['path_parts_cpp_safe'] = path_parts_cpp_safe

    template_data['domain'] = relative_path

    cpp_path = relative_path + '/' + version_str + '.cpp'
    template_data['cpp'] = cpp_path

    hpp_path = relative_path + '/' + version_str + '.hpp'
    template_data['hpp'] = hpp_path

    filename = path_parts_cpp_safe[-1]
    cppsafe_name = path_parts_cpp_safe[-1]
    template_data['cppsafe_name'] = cppsafe_name

    # Create base for namespace
    base_parts = path_parts_cpp_safe[:-1]
    template_data['base_parts'] = base_parts

    relative_pathname = '/'.join(base_parts) + '/' + filename

    # namespace = "::".join(base_parts)

    constants = {}
    api['constants'] = constants

    replacements = {}
    replacements['__ADDITIONAL_CPP_LOCAL_INCLUDES__'] = (
        get_additional_cpp_local_includes(api))
    replacements['__ADDITIONAL_CPP_SYSTEM_INCLUDES__'] = (
        get_additional_cpp_system_includes(api))
    replacements['__ADDITIONAL_HPP_LOCAL_INCLUDES__'] = (
        get_additional_hpp_local_includes(api))
    replacements['__ADDITIONAL_HPP_SYSTEM_INCLUDES__'] = (
        get_additional_hpp_system_includes(api))
    replacements['__API_ACTION_PATH__'] = get_api_action_path(api)
    replacements['__API_PATH__'] = get_api_path(api)
    replacements['__API_REQUEST_BASE_HEADER__'] \
        = get_api_request_base_header(api)
    replacements['__API_REQUEST_BASE__'] = get_api_request_base(api)
    replacements['__AS_STRING_FUNCS__'] = get_as_string_funcs(api)
    replacements['__CLASS_MEMBER_VARS__'] = get_class_member_vars(api)
    replacements['__CONTENT_PARSING__'] = get_content_parsing(api)
    replacements['__CPPSAFE_NAME__'] = cppsafe_name
    replacements['__CPP_CTORS__'] = get_cpp_ctors(api)
    replacements['__CPP_CTOR_ARGS__'] = get_cpp_ctor_args(api)
    replacements['__CPP_FILENAME__'] = filename + '.cpp'
    replacements['__CPP_OPTIONAL_SETTERS__'] = get_cpp_optional_setters(api)
    replacements['__CPP_POST_DATA_IMPL_DECL__'] \
        = get_cpp_post_data_impl_decl(api)
    replacements['__CPP_POST_DATA_IMPL_DEF__'] \
        = get_cpp_post_data_impl_def(api)
    replacements['__CPP_POST_DATA_TEMPLATE__'] \
        = get_cpp_post_data_template(api)
    replacements['__CPP_SESSION_TOKEN_TEMPLATE__'] \
        = get_session_token_api_cpp_template(api)
    replacements['__DATA_TYPES__'] = get_data_types(api)
    replacements['__DATA_TYPE_CTOR__'] = get_data_ctor(api)
    replacements['__DATA_TYPE_STRUCTS__'] = get_data_type_structs(api)
    replacements['__DATA_TYPE_STRUCT_EXTRACTORS__'] \
        = get_data_type_struct_extractors(api)
    replacements['__ENUMS__'] = get_enums(api)
    replacements['__EXPLICIT__'] = get_explicit(api)
    replacements['__HPP_CTORS__'] = get_hpp_ctors(api)
    replacements['__HPP_CTOR_ARGS__'] = get_hpp_ctor_args(api)
    replacements['__HPP_FILENAME__'] = version_str + '.hpp'
    replacements['__HPP_RELATIVE_FILENAME__'] = relative_pathname + '.hpp'
    replacements['__HPP_NAMESPACED_ENUMS__'] = get_hpp_namespaced_enums(api)
    replacements['__HPP_OPTIONAL_SETTERS__'] = get_hpp_optional_setters(api)
    replacements['__HPP_POST_DATA_TEMPLATE__'] \
        = get_hpp_post_data_template(api)
    replacements['__HPP_SESSION_TOKEN_TEMPLATE__'] \
        = get_session_token_api_hpp_template(api)
    replacements['__IMPL_PASSTHRU_ARGS__'] = get_impl_passthru_args(api)
    replacements['__IMPL_CTOR_DECLARATIONS__'] = get_impl_ctor_declarations(
        api)
    replacements['__IMPL_CTOR_DEFINITIONS__'] = get_impl_ctor_definitions(api)
    replacements['__NAMESPACE_BEGIN__'] = namespace_begin(base_parts)
    replacements['__NAMESPACE_END__'] = namespace_end(base_parts)
    replacements['__NAMESPACE__'] = '::'.join(base_parts)
    replacements['__NAMESPACE_DOCUMENTATION__'] \
        = get_namespace_documentation(path_parts_cpp_safe)
    replacements['__PR_OPT_ADT__'] = get_pr_opt_adt(api)
    replacements['__QUERY_PARTS_ARG__'] = get_query_parts_arg(api)
    replacements['__URL_CREATION__'] = get_url_creation(api)
    replacements['__VERSION__'] = version_str
    replacements['__VERSION_DOCUMENTATION__'] = get_version_documentation(api)

    cpp = get_cpp_template()
    hpp = get_hpp_template()

    cpp = replace_by_dict(cpp, replacements)
    if write_contents(cpp, os.path.join(target_path, cpp_path)):
        print(cpp_path + ' modified')

    hpp = replace_by_dict(hpp, replacements)
    if write_contents(hpp, os.path.join(target_path, hpp_path)):
        print(hpp_path + ' modified')

    if 'default_version' in api and api['default_version'] is True:
        template_data['default_version'] = True
    else:
        template_data['default_version'] = False

    return template_data


def get_json_file_paths(src_path):
    json_files = []
    for root, dirnames, filenames in os.walk(src_path):
        for filename in fnmatch.filter(filenames, '*.json'):
            json_files.append(os.path.join(root, filename))
    return json_files


class ErrorFormatter:
    def __init__(self):
        self.errors_ = []
        self.intro_ = []

        self.json_error_count_list_ = []
        self.json_error_count_ = 0

    def error_list(self):
        return self.errors_

    def add(self, err):
        if len(self.intro_) == 0:
            self.errors_.append(err)
        else:
            self.errors_.append(self.intro() + ': ' + err)
        self.json_error_count_ = self.json_error_count_ + 1

    def reindent(self, depth, txt):
        new_indent = self.indent(depth)
        newline = '\n' + new_indent
        return newline.join(txt.split('\n'))

    def indent(self, depth):
        return '  ' * depth

    def intro(self):
        return ' '.join(self.intro_)

    def replace_intro(self, debug):
        self.intro_.pop()
        self.intro_.append(debug)

    def push_intro(self, debug):
        self.intro_.append(debug)
        self.json_error_count_list_.append(self.json_error_count_)
        self.json_error_count_ = 0

    def pop_intro(self, jsn=None):
        self.intro_.pop()
        if jsn is not None:
            if self.json_error_count_ > 0:
                self.errors_.append(self.reindent(1, pretty_json(jsn)))
        self.json_error_count_ = self.json_error_count_list_.pop()

    def empty(self):
        return (len(self.errors_) == 0)


class JsonAnalyser:
    def __init__(self):
        self.fields = {}
        self.arrays = {}
        pass

    def add_field(self, name, analyser=None, permitted_values=None,
                  is_id=False, required=False, validator=None,
                  fail_on_duplicate=False):
        self.fields[name] = {
            'analyser': analyser,
            'permitted_values': permitted_values,
            'is_id': is_id,
            'required': required,
            'validator': validator,
            'fail_on_duplicate': fail_on_duplicate}

    def add_array(self, name, analyser=None, required=False):
        self.arrays[name] = {
            'analyser': analyser,
            'required': required}

    def is_private_field(self, key):
        return key[0:1] == '_'

    def run(self, full_json, json, identifier=None, ef=None,
            duplicate_checker=None):
        if ef is None:
            ef = ErrorFormatter()

        if duplicate_checker is None:
            duplicate_checker = []

        # Determine if required fields get id if available
        required_keys = []
        all_keys = []
        for key, values in self.fields.iteritems():
            all_keys.append(key)
            if values['required'] is True:
                required_keys.append(key)
            if values['is_id'] is True:
                if key in json:
                    identifier = quote(str(json[key]))
        for key, values in self.arrays.iteritems():
            all_keys.append(key)
            if values['required'] is True:
                required_keys.append(key)

        if identifier is not None:
            ef.push_intro(identifier)

        # Determine if required field missing
        for key in required_keys:
            if key not in json:
                ef.add('Missing parameter "' + key + '"')

        # Ensure no excess members
        for key in json:
            if key not in all_keys and not self.is_private_field(key):
                ef.add('Unexpected parameter "' + key + '"')

        # Validate
        for key, values in self.fields.iteritems():
            if values['validator'] is not None and key in json:
                errors = values['validator'](full_json, json[key])
                for err in errors:
                    ef.add(err)
            if values['permitted_values'] is not None and key in json:
                val = json[key]
                if val not in values['permitted_values']:
                    ef.add(quote(key) + ' does not accept ' + quote(val))
                    quoted = []
                    for permitted in values['permitted_values']:
                        quoted.append(quote(permitted))
                    accepted = '    acceptable values: ' + ', '.join(quoted)
                    ef.add(accepted)
            if values['fail_on_duplicate'] and key in json:
                d = -1
                val = json[key]

                if key in duplicate_checker[d]:
                    if val in duplicate_checker[d][key]:
                        ef.add('Duplicate field ' + quote(key) +
                               ' with value ' + quote(val))
                    else:
                        duplicate_checker[d][key][val] = True
                else:
                    duplicate_checker[d][key] = {val: True}

        # Enter child arrays
        child_id = 0
        for key, values in self.arrays.iteritems():
            child_id = child_id + 1
            duplicate_checker.append({})
            if key in json and values['analyser'] is not None:
                for child in json[key]:
                    if identifier is None:
                        child_identifier = quote(key) + ' #' + str(child_id)
                        values['analyser'].run(full_json, child,
                                               child_identifier, ef,
                                               duplicate_checker)
                    else:
                        values['analyser'].run(full_json, child, None, ef,
                                               duplicate_checker)
            duplicate_checker.pop()

        if identifier is not None:
            ef.pop_intro(json)

        return ef.error_list()


def api_template_enum_analyser():
    ja = JsonAnalyser()
    members = JsonAnalyser()

    # Required "members"
    members.add_field('cpp_name', is_id=True, required=True,
                      fail_on_duplicate=True)
    members.add_field('api_value', required=True, fail_on_duplicate=True)

    # Optional "members"
    members.add_field('additional_api_values')

    ja.add_field('cpp_name', is_id=True, required=True, fail_on_duplicate=True)
    ja.add_array('members', members, required=True)

    return ja


def api_template_optional_input_analyser():
    ja = JsonAnalyser()

    # Required fields
    ja.add_field('cpp_name', is_id=True, required=True, fail_on_duplicate=True)
    ja.add_field('cpp_type', required=True)
    ja.add_field('api_path', required=True, fail_on_duplicate=True)

    # Optional fields
    ja.add_field('http_method', permitted_values=['GET', 'POST'])
    ja.add_field('description')

    return ja


def api_template_return_param_analyser():
    ja = JsonAnalyser()

    # Required fields
    ja.add_field('cpp_name', is_id=True, required=True,
                 fail_on_duplicate=True)
    ja.add_field('cpp_type', required=True)
    ja.add_field('api_path', required=True, fail_on_duplicate=True)

    # Optional fields
    ja.add_field('json_type', permitted_values=['single', 'array',
                                                'array_front'])
    ja.add_field('optional', permitted_values=[True])
    ja.add_field('default_value')
    ja.add_field('description')

    return ja


def api_template_input_param_analyser():
    ja = JsonAnalyser()

    # Required fields
    ja.add_field('cpp_name', is_id=True, required=True, fail_on_duplicate=True)
    ja.add_field('cpp_type', required=True)
    ja.add_field('api_path', required=True, fail_on_duplicate=True)

    # Optional fields
    ja.add_field('http_method', permitted_values=['GET', 'POST'])
    ja.add_field('description')

    return ja


def additional_variable_names_validator(full_json, cpp_names):
    ret = []
    for cpp_name in cpp_names:
        if get_input_cpp_variable_info(full_json, cpp_name) is None:
            ret.append('constructors cpp_variable_name ' +
                       'not a cpp variable name: ' + cpp_name)
    return ret


def api_template_additional_constructor_analyser():
    ja = JsonAnalyser()

    ja.add_field('cpp_variable_names',
                 validator=additional_variable_names_validator, required=True)
    ja.add_field('description')

    return ja


def api_template_return_struct_analyser():
    ja = JsonAnalyser()
    members = JsonAnalyser()

    # Required "members"
    members.add_field('cpp_name', is_id=True, required=True,
                      fail_on_duplicate=True)
    members.add_field('cpp_type', required=True)
    members.add_field('api_path', required=True, fail_on_duplicate=True)

    # Optional "members"
    members.add_field('json_type', permitted_values=['single', 'array',
                                                     'array_front'])
    members.add_field('default_value')
    members.add_field('optional', permitted_values=[True])
    members.add_field('description')

    members.add_field('api_path', required=True, fail_on_duplicate=True)

    ja.add_field('cpp_name', is_id=True, required=True, fail_on_duplicate=True)
    ja.add_array('members', members, required=True)

    return ja


def api_template_errors(json):
    ja = JsonAnalyser()

    # Required fields
    ja.add_field('api', required=True)
    ja.add_field('version', required=True)

    # Optional fields
    ja.add_field('session_token', permitted_values=[True, False])
    ja.add_field('delivery_method', permitted_values=['GET', 'POST'])
    ja.add_field('default_version', permitted_values=[True, False])

    # Optional arrays
    ja.add_array('enums', analyser=api_template_enum_analyser())
    ja.add_array('return_structs',
                 analyser=api_template_return_struct_analyser())
    ja.add_array('optional_input_params',
                 analyser=api_template_optional_input_analyser())
    ja.add_array('return_params',
                 analyser=api_template_return_param_analyser())
    ja.add_array('input_params', analyser=api_template_input_param_analyser())
    ja.add_array('constructors',
                 analyser=api_template_additional_constructor_analyser())
    ja.add_array('system_cpp_includes')
    ja.add_array('system_hpp_includes')

    return ja.run(json, json)


def generate_cmake_include(target_file, cpp_sources, hpp_sources):
    """Generate a CMake file to be included in the project to build the
    templates"""

    cpp_sources.sort()
    hpp_sources.sort()

    content = '''# This file is auto generated.  Do NOT edit by hand.

set(API_TEMPLATE_GENERATED_SOURCES
'''
    for name in cpp_sources:
        content = content + '    ' + name + "\n"

    content = content + ''')

set(API_TEMPLATE_GENERATED_HEADERS
'''

    for name in hpp_sources:
        content = content + '    ' + name + "\n"

    content = content + ''')
'''
    if write_contents(content, target_file):
        print(target_file + ' modified')


def common_header_includes(domain, domain_data):

    domain_parts = domain.split('/')

    # This looks nicer when sorted
    domain_data['headers'].sort()

    includes = ''
    for header in domain_data['headers']:
        header_parts = header.split('/')
        to_skip = 0
        for i in range(len(header_parts)):
            try:
                if domain_parts[i] == header_parts[i]:
                    to_skip = to_skip + 1
                else:
                    break
            except:
                break
        relative_path = '/'.join(header_parts[to_skip-1:])
        includes = includes + '#include "' + relative_path + '"\n'

    return includes


def create_common_headers(target_file, domain, domain_data):

    base_parts = domain_data['base_parts']

    if 'default_version' in domain_data:
        current_version = domain_data['default_version']
    else:
        domain_data['versions'].sort()
        current_version = domain_data['versions'][-1]

    replacements = {}

    replacements['__API_DOMAIN__'] = domain
    replacements['__CPPSAFE_NAME__'] = domain_data['cppsafe_name']
    replacements['__HPP_RELATIVE_FILENAME__'] = domain + '.hpp'
    replacements['__INCLUDES__'] = common_header_includes(domain, domain_data)
    replacements['__NAMESPACE_BEGIN__'] = namespace_begin(base_parts)
    replacements['__NAMESPACE_DOCUMENTATION__'] \
        = get_namespace_documentation(domain_data['path_parts_cpp_safe'])
    replacements['__NAMESPACE_END__'] = namespace_end(base_parts)
    replacements['__VERSION__'] = current_version

    hpp = replace_by_dict(get_all_hpp_template(), replacements)

    if write_contents(hpp, target_file):
        print(target_file + ' modified')


def main():
    parser = argparse.ArgumentParser(
        description='Convert API templates to C++.')
    parser.add_argument('-s', '--src',
                        dest='source_path',
                        default='templates/apis',
                        help='Directory with source templates')

    default_destination = os.path.dirname(os.path.abspath(__file__))
    parser.add_argument('-d', '--dest',
                        dest='destination_path',
                        default=default_destination,
                        help='Directory with source templates')
    args = parser.parse_args()

    json_apis = []
    for json_file_path in get_json_file_paths(args.source_path):
        json_text = read_file(json_file_path)
        try:
            api = json.loads(json_text)
            api_errors = api_template_errors(api)
            if len(api_errors) > 0:
                print("File contains errors: " + str(json_file_path))
                for err in api_errors:
                    print('   ' + err)
            else:
                json_apis.append(api)
        except ValueError as e:
            print('File is not proper JSON: ' + str(json_file_path))
            for arg in e.args:
                print('    ' + str(arg))

    source_files = []
    header_files = []

    domains = {}
    # default_by_domain = {}  # TODO

    for api in json_apis:
        meta = create_templates(api, args.destination_path)
        source_files.append(meta['cpp'])
        header_files.append(meta['hpp'])
        domain = meta['domain']
        if domain not in domains:
            domains[domain] = {}
            domains[domain]['base_parts'] = meta['base_parts']
            domains[domain]['cppsafe_name'] = meta['cppsafe_name']
            domains[domain]['path_parts_cpp_safe'] \
                = meta['path_parts_cpp_safe']
            domains[domain]['headers'] = []
            domains[domain]['versions'] = []

        domains[domain]['headers'].append(meta['hpp'])
        domains[domain]['versions'].append(meta['version'])

        if meta['default_version'] is True:
            if 'default_version' in domains[domain]:
                raise UnuniqueParameter(api, 'default_version')
            domains[domain]['default_version'] = meta['version']

    for domain, domain_data in domains.iteritems():
        common_hpp = domain + '.hpp'
        header_files.append(common_hpp)
        create_common_headers(os.path.join(args.destination_path, common_hpp),
                              domain, domain_data)

    generate_cmake_include(os.path.join(args.destination_path,
                                        'GeneratedList.txt'),
                           source_files, header_files)

if __name__ == "__main__":
    main()
