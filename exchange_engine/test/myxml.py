import xml.etree.ElementTree as ET
import socket
import random


class XMLRequest:
    @staticmethod
    def create_account(account_id, balance):
        root = ET.Element("create")
        ET.SubElement(root, "account", attrib={
                      "id": str(account_id), "balance": str(balance)})
        return root

    @staticmethod
    def create_account_to_root(root, account_id, balance):
        ET.SubElement(root, "account", attrib={
                      "id": str(account_id), "balance": str(balance)})
        return root

    @staticmethod
    def create_sym(symbol, account_id, num):
        root = ET.Element("create")
        child = ET.SubElement(root, "symbol", attrib={"sym": symbol})
        ET.SubElement(child, "account", attrib={
                      "id": str(account_id)}).text = str(num)
        return root

    @staticmethod
    def create_sym_to_root(root, symbol, account_id, num):
        child = ET.SubElement(root, "symbol", attrib={"sym": symbol})
        ET.SubElement(child, "account", attrib={
                      "id": str(account_id)}).text = str(num)
        return root

    @staticmethod
    def transaction_order(account_id, symbol, amount, limit):
        root = ET.Element("transactions", attrib={"id": str(account_id)})
        ET.SubElement(root, "order", attrib={
                      "sym": symbol, "amount": str(amount), "limit": str(limit)})
        return root

    @staticmethod
    def transaction_order_to_root(root, symbol, amount, limit):
        ET.SubElement(root, "order", attrib={
                      "sym": symbol, "amount": str(amount), "limit": str(limit)})
        return root


class XMLTestGenerator:
    def __init__(self):
        self.account_num = 10
        self.init_balance = 100000000
        self.test_symbols = ["s1", "s2", "s3", "s4", "s5"]
        self.init_share = 100000000
        self.min_price = 1
        self.max_price = 10
        self.min_amount = 1
        self.max_amount = 100

    def setup(self):
        root = XMLRequest.create_account(0, self.init_balance)
        for account_id in range(1, self.account_num):
            XMLRequest.create_account_to_root(
                root, account_id, self.init_balance)
        for account_id in range(0, self.account_num):
            for symbol in self.test_symbols:
                XMLRequest.create_sym_to_root(
                    root, symbol, account_id, self.init_share)
        return ET.tostring(root, encoding="unicode", xml_declaration=True)

    def random_order(self):
        account_id = random.randint(0, self.account_num - 1)
        symbol = self.test_symbols[random.randint(
            0, len(self.test_symbols) - 1)]
        amount = random.randint(-self.max_amount, self.max_amount)
        limit = random.randint(self.min_price, self.max_price)
        root = XMLRequest.transaction_order(account_id, symbol, amount, limit)
        return ET.tostring(root, encoding="unicode", xml_declaration=True)


def netcat(hostname, port, content):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((hostname, port))
    s.sendall(content.encode())
    s.shutdown(socket.SHUT_WR)
    data = s.recv(10240)
    print("Received:", repr(data), flush=True)
    s.close()


def test(msg):
    msg = str(len(msg)) + "\n" + msg
    netcat("vcm-31876.vm.duke.edu", 12345, msg)


if __name__ == "__main__":
    gen = XMLTestGenerator()
    test(gen.setup())
    for i in range(10):
        print("sent transc " + str(i), end="; ", flush=True)
        test(gen.random_order())
