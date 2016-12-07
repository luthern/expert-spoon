import asyncore
import socket
import sys
import time


class KVClient(asyncore.dispatcher):
    def __init__(self, host, file_name, total_to_write):
        asyncore.dispatcher.__init__(self)
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connect((host, 80))
        self.file_name = file_name
        self.file_pointer = open(file_name, 'r')
        self.total_writes = 0
        self.total_to_write = total_to_write
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
        line = self.file_pointer.readline()
        if not line:
            self.is_writable = False
            return
        line = line.split(':')
        operation = int(line[0])
        key = line[1]
        if len(line[2]) != 32:
            value = '\x00' * 32
        else:
            value = line[2]

        keep_alive = 0x1
        if self.total_writes == self.total_to_write - 1:
            keep_alive = 0x0
        buffer = chr(operation) + chr(keep_alive) + key + value
        self.send(buffer)
        self.total_writes += 1
        # print(self.total_writes)

    def get_end_time(self):
        return self.end_time


filename = sys.argv[1]
number_of_messages = int(sys.argv[2])

client = KVClient('192.168.0.151', filename, number_of_messages)
start = time.time()
asyncore.loop()
end = client.get_end_time()

print("total seconds ", end - start)
print("requests/second ", number_of_messages / (end - start))
