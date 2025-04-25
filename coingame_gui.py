#python coingame_gui.py

import zmq
import tkinter as tk
from tkinter import messagebox

SERVER_PUSH_ADDR = "tcp://benternet.pxl-ea-ict.be:24041"
SERVER_SUB_ADDR = "tcp://benternet.pxl-ea-ict.be:24042"
TOPIC_PREFIX = "Bjarni>CoinGame!>"

class CoinGameClient:
    def __init__(self, root):
        self.root = root
        self.root.title("CoinGame GUI")
        self.name = ""
        self.context = zmq.Context()

        # PUSH socket
        self.push_socket = self.context.socket(zmq.PUSH)
        self.push_socket.connect(SERVER_PUSH_ADDR)

        # SUB socket
        self.sub_socket = self.context.socket(zmq.SUB)
        self.sub_socket.connect(SERVER_SUB_ADDR)

        self.build_gui()

    def build_gui(self):
        tk.Label(self.root, text="Naam:").pack()
        self.name_entry = tk.Entry(self.root)
        self.name_entry.pack()

        tk.Label(self.root, text="Raad een getal (1-5):").pack()
        self.guess_entry = tk.Entry(self.root)
        self.guess_entry.pack()

        self.submit_button = tk.Button(self.root, text="Verzenden", command=self.send_guess)
        self.submit_button.pack()

        self.output_text = tk.Text(self.root, height=10, state="disabled")
        self.output_text.pack()

    def send_guess(self):
        self.name = self.name_entry.get()
        guess = self.guess_entry.get()

        if not guess.isdigit() or not (1 <= int(guess) <= 5):
            messagebox.showwarning("Fout", "Voer een getal in tussen 1 en 5.")
            return

        topic = TOPIC_PREFIX + self.name + ">"
        self.sub_socket.setsockopt_string(zmq.SUBSCRIBE, topic)

        message = f"Bjarni>CoinGame?>{self.name}>{guess}>"
        self.push_socket.send_string(message)

        self.output("Gok verzonden! Wachten op antwoord...")
        self.root.after(100, self.check_response)

    def check_response(self):
        try:
            msg = self.sub_socket.recv_string(flags=zmq.NOBLOCK)
            self.output("Server: " + msg)
        except zmq.Again:
            self.root.after(100, self.check_response)

    def output(self, text):
        self.output_text.configure(state="normal")
        self.output_text.insert(tk.END, text + "\n")
        self.output_text.configure(state="disabled")


if __name__ == "__main__":
    root = tk.Tk()
    app = CoinGameClient(root)
    root.mainloop()
