#!/usr/bin/python3.5

import getopt

# traceback提供了一个标准接口来提取、格式化和打印 Python 程序的堆栈跟踪。当它打印堆栈跟踪时，它完全模仿 Python 解释器的行为。
import traceback
import os
import sys
from string import Template
from datetime import datetime

project_name = ""
proto_file = ""
out_project_path = "./"
bin_path = ""
conf_path = ""
test_client_path = ""
test_client_tool_path = ""
src_path = ""

# 初值为当前目录
generator_path = sys.path[0]

# 将字符串转换成驼峰命名法格式
def to_camel(input_s):
    if input_s.find('_') == -1:
        return input_s
    re = ''
    for s in input_s.split('_'):
        re += s.capitalize()
    return re

# 将字符串转换成下划线分割的格式
def to_underline(input_s):
    tmp = to_camel(input_s)
    re = ''
    for s in tmp:
        re += s if s.islower() else '_' + s.lower()
    re = re[0:]
    return re

def parseInput():
    # sys.argv[1:] 是要解析的参数列表，它包含了命令行中传递给程序的所有参数（不包括程序名称）。短格式选项字符串为 "hi:o:"，它定义了三个短格式选项：-h、-i 和 -o。冒号表示该选项需要一个参数。长格式选项列表为 ["help", "input=", "output="]，它定义了三个长格式选项：--help、--input 和 --output。等号表示该选项需要一个参数。
    # getopt.getopt 函数返回一个元组，其中包含两个列表：选项列表和非选项参数列表。
    opts, args = getopt.getopt(sys.argv[1:], "hi:o:", longopts=["help", "input=", "output="])
    
    for opts, arg in opts:
        if opts=="-h" or opts=="--help":
            printHelp()
            sys.exit(0)
        
        # 如果是-i，说明是输入，后面跟的是protobuf文件
        if opts=="-i" or opts=="--input":
            global proto_file 
            proto_file = arg 
        elif opts=="-o" or opts=="--output_path":
            global out_project_path
            out_project_path = arg
            # 默认是当前路径
            if out_project_path[-1] != '/':
                out_project_path = out_project_path + '/'
        else:
            raise Exception("invalid options: [" + opts + ": " + arg + "]")

    # 如果输入的protobuf文件不存在，就抛异常    
    if not os.path.exists(proto_file):
        raise Exception("Generate error, not exist protobuf file: " + proto_file)
    
    # 如果输入的protobuf文件不是以.proto结尾，那么也抛异常
    if ".proto" not in proto_file:
        raise Exception("Generate error, input file is't standard protobuf file:[" + proto_file + "]")
    
    # 如果都正常，就取出输入文件名，去掉文件名后缀“.proto”
    global project_name
    project_name = proto_file[0: -6]
    print("project name is " + project_name)

def printHelp():
    print('=' * 100)
    print('Welcome to use Rocket Generator, this is help document:\n')
    print('Run Environment: Python(version 3.6 or high version is better).')
    print('Run Platform: Linux Only(kernel version >= 3.9 is better).')
    print('Others: Only protobuf3 support, not support protobuf2.\n')
    print('Usage:')
    print('rocket_generator.py -[options][target]\n')
    print('Options:')
    print('-h, --help')
    print(('    ') + 'Print help document.\n')

    print('-i xxx.proto, --input xxx.proto')
    print(('    ') + 'Input the target proto file, must standard protobuf3 file, not support protobuf2.\n')

    print('-o dir, --output dir')
    print(('    ') + 'Set the path that your want to generate project, please give a dir param.\n')

    print('')
    print('For example:')
    print('rocket_generator.py -i order_server.proto -o ./')

    print('')
    print('=' * 100)  

def generate_dir():
    print('=' * 100)
    print('Begin to generate project dir')

    # 如果没有设置输出路径，就以当前目录为输出路径
    if out_project_path == "":
        proj_path = './' + project_name.strip()
    if out_project_path[-1] == '/':
        proj_path = out_project_path + project_name.strip()
    else:
        proj_path = out_project_path + './' + project_name.strip()
    
    global bin_path 
    bin_path = proj_path + '/bin'

    global conf_path
    conf_path = proj_path + '/conf'

    # 测试客户端路径
    global test_client_path 
    test_client_path = proj_path + '/test_client'

    # 测试用例路径
    global test_client_tool_path 
    test_client_tool_path = test_client_path + '/test_tool'

    log_path = proj_path + '/log'
    lib_path = proj_path + '/lib'
    obj_path = proj_path + '/obj'

    # 源代码路径
    global src_path
    src_path = proj_path + '/' + project_name
    src_interface_path = src_path + '/interface'
    src_service_path = src_path + '/service'
    src_pb_path = src_path + '/pb'
    src_stubs_path = src_path + '/stubs'
    src_comm_path = src_path + '/comm'

    dir_list = []
    dir_list.append(proj_path) 
    dir_list.append(bin_path) 
    dir_list.append(conf_path) 
    dir_list.append(log_path) 
    dir_list.append(lib_path) 
    dir_list.append(obj_path) 
    dir_list.append(test_client_path) 
    dir_list.append(test_client_tool_path) 
    dir_list.append(src_path) 
    dir_list.append(src_interface_path) 
    dir_list.append(src_service_path) 
    dir_list.append(src_pb_path) 
    dir_list.append(src_stubs_path) 
    dir_list.append(src_comm_path) 

    # 如果目录不存在，就去创建
    for each in dir_list:
        if not os.path.exists(each):
            os.mkdir(each)
            print("succ make dir in " + each)

    print('End generate project dir')
    print('=' * 100)

# 生成pb文件
def generate_pb():
    print('=' * 100)
    print('Begin generate protobuf file')
    # pb文件放在源代码目录下
    pb_path = src_path + '/pb/'
    # 将pb文件拷贝到pb路径下
    cmd = 'cp -r ' + proto_file + ' ' + pb_path
    # 编译pb文件，生成.h, .cc文件
    cmd += ' && cd ' + pb_path + ' && protoc --cpp_out=./ ' + proto_file 
    print('excute cmd: ' + cmd)

    if os.system(cmd) is not 0:
        raise Exception("execute cmd failed [" + cmd + "]")
    
    print('End generate protobuf file')
    print('=' * 100)

# 生成makefile文件
def generate_makefile():
    print('=' * 100)
    print('Begin to generate makefile')
    # makefile输出路径位于源代码目录之下
    out_file = src_path + '/makefile'
    # 只有第一次调用会生成makefile文件
    if os.path.exists(out_file):
        print('makefile exist, skip generate')
        print('End generate makefile')
        print('=' * 100)
        return
    
    # 打开makefile的模板文件
    template_file = open(generator_path + '/template/makefile.template','r')

    # 使用 Template 类的 safe_substitute 方法将模板中的占位符替换为实际值
    tmpl = Template(template_file.read())

    content = tmpl.safe_substitute(
        PROJECT_NAME = project_name,
        CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'))

    # 创建新文件，将替换后的内容写入该文件
    file = open(out_file, 'w')
    file.write(content)
    file.close()
    print('succ write to ' + out_file)
    print('End generate makefile')
    print('=' * 100)

# 根据模板文件生成运行脚本，并将其复制到指定目录下
def gen_run_script():
    print('=' * 100)
    print('Begin to generate run script')
    script = open(generator_path + '/template/conf.xml.template','r')
    dir_src = generator_path + '/template/'
    cmd = 'cp -r ' + dir_src + '*.sh ' + bin_path + "/" 
    print('excute cmd: ' + cmd)
    os.system(cmd)

    print('End generate run script')
    print('=' * 100)

# 生成配置文件
def gen_conf_file():
    print('=' * 100)
    file_name = "rocket.xml"
    print('Begin to generate tinyrpc conf file')
    out_file = conf_path + '/' + file_name
    if os.path.exists(out_file):
        print('config exist, skip generate')
        print('End generate ' + file_name)
        print('=' * 100)
        return 
    
    template_file = open(generator_path + '/template/conf.xml.template','r')
    # print(template_file.read())
    tmpl = Template(template_file.read())
    # 使用safe_substitute方法将模板中的占位符替换为实际的值，得到配置文件的内容
    content = tmpl.safe_substitute(
        PROJECT_NAME = project_name,
        FILE_NAME = file_name,
        CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'))

    file = open(out_file, 'w')
    file.write(content)
    file.close()
    print('succ write to ' + out_file)
    print('End generate rocket conf file')
    print('=' * 100)

# 生成框架代码
def generate_framework_code(): 
    print('=' * 100)
    print('Begin to generate myRocketRPC framework code')
    pb_head_file = src_path + '/pb/' + project_name + '.pb.h'
    file = open(pb_head_file, 'r')
    origin_text = file.read()
    
    # parse all rpc interface method from pb.h file 定位的是.pb.h文件中的虚析构函数
    begin = origin_text.find('virtual ~')
    i1 = origin_text[begin:].find('~') 
    i2 = origin_text[begin:].find('(') 
    service_name = origin_text[begin + i1 + 1 : begin + i2]
    print("service name is " + service_name)

    origin_text = origin_text[begin + i2: ] 
    method_list = []

    i1 = 0
    while 1:
        i1 = origin_text.find('virtual void')
        if (i1 == -1):
            break
        i2 = origin_text[i1:].find(');')
        method_list.append(origin_text[i1: i1 + i2 + 2])
        # print(origin_text[i1: i1 + i2 + 2])
        origin_text = origin_text[i1 + i2 + 3: ]

    
    print('=' * 100)
    print('Begin generate business_exception.h')
    exception_file = src_path + '/comm/' + 'business_exception.h'
    if not os.path.exists(exception_file):
        # generate business_exception.h
        # 这里根据模板生成exception文件
        exception_template = Template(open(generator_path + '/template/business_exception.h.template', 'r').read())
        exception_content = exception_template.safe_substitute(
            PROJECT_NAME = project_name,
            FILE_NAME = 'business_exception.cc',
            HEADER_DEFINE = project_name.upper() + '_COMM_BUSINESSEXCEPTION_H',
            CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
            INCLUDE_INTERFACEBASE_HEADER_FILE = '#include "' + project_name + '/interface/interface.h"',
        )
        out_exception_file = open(exception_file, 'w')
        out_exception_file.write(exception_content)
        out_exception_file.close()
    else:
        print("file: [" + exception_file + "] exist, skip")

    print('End generate business_exception.h')
    print('=' * 100)


    print('=' * 100)
    print('Begin generate server.h')
    # genneator server.h file
    class_name = to_camel(service_name) + 'Impl'
    head_file_temlate = Template(open(generator_path + '/template/server.h.template','r').read())
    head_file_content = head_file_temlate.safe_substitute(
        HEADER_DEFINE = project_name.upper() + '_SERVICE_' + project_name.upper() + '_H',
        FILE_NAME = project_name + '.h',
        PROJECT_NAME = project_name,
        CLASS_NAME = class_name,
        SERVICE_NAME = service_name,
        PB_HEAD_FILE = project_name + '/pb/' + project_name + '.pb.h', 
        CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
        INCLUDE_PB_HEADER = '#include "' + project_name + '/pb/' + project_name + '.pb.h"', 
    )

    i1 = head_file_content.find('${METHOD}') 
    pre_content = head_file_content[0: i1]
    next_content = head_file_content[i1 + 9: ]
    for each in method_list:
        each = each.replace('PROTOBUF_NAMESPACE_ID', 'google::protobuf')
        pre_content += '// override from ' + service_name + '\n  '
        pre_content += each
        pre_content += '\n\n  '
    content = pre_content + next_content
    out_head_file = open(src_path + '/service/' + project_name + '.h', 'w')
    out_head_file.write(content)
    out_head_file.close()

    print('End generate server.h')
    print('=' * 100)

    print('=' * 100)
    print('Begin generate server.cc')
    # genneator server.cc file
    cc_file_temlate = Template(open(generator_path + '/template/server.cc.template','r').read())
    cc_file_content = cc_file_temlate.safe_substitute(
        FILE_NAME = project_name + '.cc',
        PROJECT_NAME = project_name,
        INCLUDE_PB_HEADER = '#include "' + project_name + '/pb/' + project_name + '.pb.h"', 
        INCLUDE_BUSINESS_EXCEPTION_HEADER = '#include "' + project_name + '/comm/business_exception.h"',
        INCLUDE_SERVER_HEADER = '#include "' + project_name + '/service/' + project_name + '.h"', 
        CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    )


    method_i = cc_file_content.find('${METHOD}')
    pre_content = cc_file_content[0: method_i]
    next_content = cc_file_content[method_i + 9: ]
    interface_list = []

    for each in method_list:
        tmp = each.replace('PROTOBUF_NAMESPACE_ID', 'google::protobuf')
        i1 = tmp.find('void')
        tmp = tmp[i1:]

        i2 = tmp.find('(')
        method_name = tmp[5: i2]
        # print(method_name)
        interface_class_name = to_camel(method_name) + 'Interface'
        interface_file_name = to_underline(method_name)
        l = tmp.split(',')
        y = l[1].find('request')
        request_type = l[1][0: y - 1].replace('*', '').replace('const ', '').replace('\n', '').replace(' ', '')
        # print(request_type)

        y = l[2].find('response')
        response_type = l[2][0: y - 1].replace('*', '').replace('\n', '').replace(' ', '')
        # print(response_type)


        interface_list.append({
            'interface_name': interface_file_name,
            'method_name': method_name,
            'interface_class_name': interface_class_name,
            'request_type': request_type,
            'response_type': response_type
        })
        # print(interface_list)

        tmp = tmp[0: 5] + class_name + '::' + tmp[5:]
        tmp = tmp[0: -1] + '{\n\n  ' + 'CALL_RPC_INTERFACE(' + interface_class_name + ');\n}'
        # print(tmp)
        pre_content += tmp
        pre_content += '\n\n'

    include_str = ''
    for each in interface_list:
        include_str += '#include "' + project_name + '/interface/' + each['interface_name'] + '.h"\n'
    pre_content = pre_content.replace('${INCLUDE_SERVICE}', include_str)
    
    out_cc_file = open(src_path + '/service/' + project_name + '.cc', 'w')
    out_cc_file.write(pre_content + next_content)
    out_cc_file.close()

    print('End generate server.cc')
    print('=' * 100)


    print('=' * 100)
    print('Begin generate main.cc')
    # genneator main.cc file
    main_file = src_path + '/main.cc'
    if not os.path.exists(main_file):
        main_file_temlate = Template(open(generator_path + '/template/main.cc.template','r').read())
        main_file_content = main_file_temlate.safe_substitute(
            FILE_NAME = project_name + '.h',
            PROJECT_NAME = project_name,
            CLASS_NAME = project_name + "::" + class_name,
            INCLUDE_SERVER_HEADER = '#include "service/' + project_name + '.h"', 
            CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        )
        main_file_handler = open(main_file, 'w')
        main_file_handler.write(main_file_content)
        main_file_handler.close()
    else:
        print("file: [" + main_file + "] exist, skip")

    print('End generate main.cc')
    print('=' * 100)

    
    # genneator interface.h file
    interfase_base_h_file = src_path + '/interface/interface.h'
    if not os.path.exists(interfase_base_h_file):
        interface_base_h_file_temlate = Template(open(generator_path + '/template/interface_base.h.template','r').read())
        interfase_base_h_file_content = interface_base_h_file_temlate.safe_substitute(
            FILE_NAME = 'interface.h',
            PROJECT_NAME = project_name,
            HEADER_DEFINE = project_name.upper() + '_INTERFACE_H',
            CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        )
        interface_base_h_handler = open(interfase_base_h_file, 'w')
        interface_base_h_handler.write(interfase_base_h_file_content)
        interface_base_h_handler.close()
    else:
        print("file: [" + interfase_base_h_file + "] exist, skip")

    print('End generate interface.h')
    print('=' * 100)

    # genneator interface.cc file
    interfase_base_cc_file = src_path + '/interface/interface.cc'
    if not os.path.exists(interfase_base_cc_file):
        interface_base_cc_file_temlate = Template(open(generator_path + '/template/interface_base.cc.template','r').read())
        interfase_base_cc_file_content = interface_base_cc_file_temlate.safe_substitute(
            FILE_NAME = 'interface.cc',
            PROJECT_NAME = project_name,
            CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
            INCLUDE_INTERFACEBASE_HEADER_FILE = '#include "' + project_name + '/interface/interface.h"',
        )
        interface_base_cc_handler = open(interfase_base_cc_file, 'w')
        interface_base_cc_handler.write(interfase_base_cc_file_content)
        interface_base_cc_handler.close()
    else:
        print("file: [" + interfase_base_cc_file + "] exist, skip")

    print('End generate interface.cc')
    print('=' * 100)

    


    print('=' * 100)
    print('Begin generate each rpc method interface.cc & interface.h')
    # genneator each interface.cc and .h file
    interface_head_file_temlate = Template(open(generator_path + '/template/interface.h.template','r').read())
    interface_cc_file_temlate = Template(open(generator_path + '/template/interface.cc.template','r').read())
    interface_test_client_file_template = Template(open(generator_path + '/template/test_rocket_client.cc.template','r').read())

    stub_name = service_name + "_Stub"
    for each in interface_list:

        # interface.h 
        file = src_path + '/interface/' + each['interface_name'] + '.h'
        if not os.path.exists(file):
            header_content = interface_head_file_temlate.safe_substitute(
                PROJECT_NAME = project_name,
                INCLUDE_PB_HEADER = '#include "' + project_name + '/pb/' + project_name + '.pb.h"', 
                HEADER_DEFINE = project_name.upper() + '_INTERFACE_' + each['interface_name'].upper() + '_H',
                CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
                CLASS_NAME = each['interface_class_name'],
                REQUEST_TYPE = each['request_type'],
                RESPONSE_TYPE = each['response_type'],
                INCLUDE_INTERFACEBASE_HEADER_FILE = '#include "' + project_name + '/interface/interface.h"',
                FILE_NAME = each['interface_name'] + '.h'
            )
            out_interface_header_file = open(file, 'w')
            out_interface_header_file.write(header_content)
            out_interface_header_file.close()
        else:
            print("file: [" + file + "] exist, skip")

        # interface.cc 
        file = src_path + '/interface/' + each['interface_name'] + '.cc'
        if not os.path.exists(file):
            cc_file_content = interface_cc_file_temlate.safe_substitute(
                PROJECT_NAME = project_name,
                INCLUDE_PB_HEADER = '#include "' + project_name + '/pb/' + project_name + '.pb.h"', 
                INCLUDE_INTERFACE_HEADER_FILE = '#include "' + project_name + '/interface/' + each['interface_name'] + '.h"',
                INCLUDE_INTERFACEBASE_HEADER_FILE = '#include "' + project_name + '/interface/interface.h"',
                CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
                CLASS_NAME = each['interface_class_name'],
                REQUEST_TYPE = each['request_type'],
                RESPONSE_TYPE = each['response_type'],
                FILE_NAME = each['interface_name'] + '.cc'
            )
            out_interface_cc_file = open(file, 'w')
            out_interface_cc_file.write(cc_file_content)
            out_interface_cc_file.close()
        else:
            print("file: [" + file + "] exist, skip")
       
        # test_interface_client.cc 
        file = test_client_path + '/test_' + each['interface_name'] + '_client.cc'
        if not os.path.exists(file):
            cc_file_content = interface_test_client_file_template.safe_substitute(
                INCLUDE_PB_HEADER = '#include "' + project_name + '/pb/' + project_name + '.pb.h"', 
                CREATE_TIME = datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
                REQUEST_TYPE = each['request_type'],
                RESPONSE_TYPE = each['response_type'],
                STUBCLASS = stub_name,
                METHOD_NAME = each['method_name'],
                FILE_NAME = 'test_' + each['interface_name'] + '_client.cc',
            )
            out_interface_cc_file = open(file, 'w')
            out_interface_cc_file.write(cc_file_content)
            out_interface_cc_file.close()
        else:
            print("file: [" + file + "] exist, skip")
            
        

    print('End generate each interface.cc & interface.h & test_interface_client.h')
    print('=' * 100)
        
    print('End generate myRocketRPC framework code')
    print('=' * 100)


def generate_project():
    try:
        parseInput()

        print('=' * 150)
        print("Begin to generate myRocket rpc server")

        # 创建目录
        generate_dir()

        # 创建proto方法类
        generate_pb()

        # 生成makefile
        generate_makefile()

        gen_run_script()

        gen_conf_file()

        generate_framework_code()

        print('Succ generate myRocket project')
        print('=' * 150)

    except Exception as e:
        print("Failed to generate myRocket rpc server, err: " + str(e))
        print('=' * 150)

if __name__ == '__main__':
    generate_project()

      