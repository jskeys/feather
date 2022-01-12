#!/usr/bin/env python

import socket
import struct
import time
import argparse

parser = argparse.ArgumentParser(description="Send a message over TCP/IP.")

parser.add_argument("-s", "--server", help="The server IP address.", default="192.168.86.35")
parser.add_argument("-v", "--value", help="The message to send.", type=str, default="")
parser.add_argument("-t", "--type", help="The message type", type=int, default = 0)
parser.add_argument("--led_on", dest="led", action="store_true")
parser.add_argument("--led_off", dest="led", action="store_false")
parser.set_defaults(led=False)

args = parser.parse_args()


class TLVMessage():
    def __init__(self, message_type, message_value):
        self._message = bytearray([0x5A, 0x55])
        self._message += struct.pack(">L", message_type)
        self._message += struct.pack(">L", len(message_value))
        self._message += message_value

    def as_bytes(self):
        return self._message

class FeatherClient():

    def __init__(self, server_ip_addr, server_port=7777):
        # Connect to server
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._socket.connect((server_ip_addr, server_port))

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, exc_traceback):
        pass

    def __del__(self):
        self._socket.close()

    def send(self, message):
        self._socket.send(message.as_bytes())

    def set_led_state(self, state):
        value = 1 if state else 0
        self.send(TLVMessage(2, struct.pack(">L", value)))

if __name__ == "__main__":
    with FeatherClient("192.168.86.101") as fc:
        fc.set_led_state(args.led)
