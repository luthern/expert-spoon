import asyncore
import socket
import sys
import time


class KVClient(asyncore.dispatcher):
    def __init__(self, host, buffer_, total_to_write):
        asyncore.dispatcher.__init__(self)
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connect((host, 80))
        self.packet_buffer = buffer_
        self.total_to_write = total_to_write
        self.total_writes = 0
        self.is_writable = True
        self.end_time = 0

    def handle_connect(self):
        pass

    def handle_close(self):
        self.end_time = time.time()
        self.close()

    def handle_read(self):
        self.recv(50)

    def writable(self):
        return self.is_writable

    def handle_write(self):
        if self.total_writes == self.total_to_write:
            self.is_writable = False
            return
        self.send(self.packet_buffer[self.total_writes])
        self.total_writes += 1

    def get_end_time(self):
        return self.end_time


filename = sys.argv[1]
number_of_messages = int(sys.argv[2])

file_pointer = open(filename, 'r')
lines = file_pointer.read().split('\n')
index = 0
packet_buffer = []
for line in lines:
    if not line:
        break
    line = line.split(':')
    operation = int(line[0])
    key = line[1]
    if len(line[2]) != 32:
        value = '\x00' * 32
    else:
        value = line[2]
    keep_alive = 0x1
    if index == number_of_messages - 1:
        keep_alive = 0x0
    packet_buffer.append(chr(operation) + chr(keep_alive) + key + value)
    index += 1
file_pointer.close()
clients = []

for c in range(0, 1000):
    clients.append(KVClient('192.168.0.151', packet_buffer[:
                                                           number_of_messages],
                            number_of_messages))

start = time.time()
asyncore.loop()
end = clients[0].get_end_time()

print("total_writes ", clients[100].total_writes)
print("total seconds ", end - start)
print("requests/second ", number_of_messages / (end - start))
