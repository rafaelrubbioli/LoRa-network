import socket 
import threading 

# FORMATO DA MENSAGEM = TIPO|ID|NUMERO_SEQUENCIA|PAYLOAD
nodes = {}
id_counter = 1
ip = '0.0.0.0'
port = 5000
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
ipConnections = []
recievedMessage = []

class Node:
    def __init__(self, id):
        self.sequence_number = 0
        self.id = id

    def showData(self, message):
        tipo = message[3]
        dado = message[4]
        print("Recebido do node ", self.id," : medida de ", message[0], " = ", message[1])


def setup():
    global ip, port, sock
    print("Inicializando o servidor ...")
    sock.bind((ip, port))
    print("IP: ", ip)
    print("Port: ", port)
    sock.listen(1)
    print("Esperando conexoes ...")
    messageThread = threading.Thread(target = recieveMessage, args = ())
    messageThread.daemon = True
    messageThread.start()
    connect()

def join():
    global id_counter, nodes
    nodes[id_counter] = Node(id_counter)
    id_counter += 1
    print("Novo node: ", nodes[id_counter].id)

def exit(id, reason):
    del id[id]
    print("Saida da rede de: ", id, " por motivo = ", reason)

def askForMeasure(id):
    message = "ASK|" + id + "|"
    # enviar mensagem para o node
    return

def ping():
    message = "PING|0|0|0"
    # Enviar mensagem para todos os nodes

def decode(message):
    global nodes
    mtype = message[0]
    id = message[1]
    seq_num = message[2]
    payload = message[3].split(":") 

    if nodes[id]:
        if seq_num <= nodes[id].sequence_number:
            return
        else:
            nodes[id].sequence_number = seq_num
            nodes[id].showData(payload)

def handleConnection(c, a):
    global ipConnections, recievedMessage
    while(True):
        data = c.recv(1024)
        if not data:
            ipConnections.remove(c)
            c.close()
            break
        else:
            stringdata = data.decode('utf-8')
            recievedMessage.append(stringdata)
            print(stringdata)
        

def connect():
    global ipConnections
    while(True):
        # conexao ip
        c, a = sock.accept()
        ipConnections.append(c)
        cThread = threading.Thread(target = handleConnection, args = (c,a))
        cThread.daemon = True
        cThread.start()


def recieveMessage():
    global recievedMessage
    while(True):
        # tratamento de mensagem
        if len(recievedMessage) > 0:
            print("RECEBI MENSAGEM", recievedMessage[0])
            msg = recievedMessage[0]
            del recievedMessage[0]
            message = msg.split("|")
            if message[0] == "JOIN":
                join()

            if message[0] == "MEASUREMENT":
                decode(message)

            if message[0] == "EXIT":
                exit(int(message[1]), message[3])


def main ():
    setup()

if __name__ == "__main__":
       main()