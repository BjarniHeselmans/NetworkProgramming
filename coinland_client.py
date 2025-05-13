import zmq
import tkinter as tk
from tkinter import messagebox

SERVER_PUSH_ADDR = "tcp://benternet.pxl-ea-ict.be:24041"
SERVER_SUB_ADDR = "tcp://benternet.pxl-ea-ict.be:24042"

class CoinLandClient:
    def __init__(self, root):
        self.root = root
        self.root.title("CoinLand Client - CoinGame & SlotMachine")
        self.name = ""
        self.context = zmq.Context()

        # Sockets
        self.push_socket = self.context.socket(zmq.PUSH)
        self.push_socket.connect(SERVER_PUSH_ADDR)

        self.sub_socket = self.context.socket(zmq.SUB)
        self.sub_socket.connect(SERVER_SUB_ADDR)

        self.build_gui()

    def build_gui(self):
        tk.Label(self.root, text="Spelersnaam:").pack()
        self.name_entry = tk.Entry(self.root)
        self.name_entry.pack()

        # CoinGame
        tk.Label(self.root, text="Raad een getal (1-5) voor CoinGame:").pack()
        self.guess_entry = tk.Entry(self.root)
        self.guess_entry.pack()
        tk.Button(self.root, text="Speel CoinGame", command=self.play_coin_game).pack(pady=5)

        # SlotMachine
        tk.Button(self.root, text="Speel SlotMachine", command=self.play_slot_machine).pack(pady=10)

        self.output_box = tk.Text(self.root, height=12, state="disabled")
        self.output_box.pack(pady=10)

    def play_coin_game(self):
        self.name = self.name_entry.get().strip()
        guess = self.guess_entry.get().strip()
        if not self.name or not guess.isdigit():
            messagebox.showerror("Fout", "Vul een geldige naam en gok in.")
            return
        if not (1 <= int(guess) <= 5):
            messagebox.showerror("Fout", "Gok moet tussen 1 en 5 liggen.")
            return

        topic = f"Bjarni>CoinGame!>{self.name}>"
        self.sub_socket.setsockopt_string(zmq.SUBSCRIBE, topic)

        message = f"Bjarni>CoinGame?>{self.name}>{guess}>"
        self.push_socket.send_string(message)
        self.output(f"[CoinGame] Gok verzonden...")
        self.root.after(100, self.poll_response)

    def play_slot_machine(self):
        self.name = self.name_entry.get().strip()
        if not self.name:
            messagebox.showerror("Fout", "Voer een geldige naam in.")
            return

        topic = f"Bjarni>SlotMachine!>{self.name}>"
        self.sub_socket.setsockopt_string(zmq.SUBSCRIBE, topic)

        message = f"Bjarni>SlotMachine?>{self.name}>"
        self.push_socket.send_string(message)
        self.output(f"[SlotMachine] Draai gestart...")
        self.root.after(100, self.poll_response)

    def poll_response(self):
        try:
            msg = self.sub_socket.recv_string(flags=zmq.NOBLOCK)
            self.output(msg)
        except zmq.Again:
            self.root.after(100, self.poll_response)

    def output(self, tekst):
        self.output_box.config(state="normal")
        self.output_box.insert(tk.END, tekst + "\n")
        self.output_box.see(tk.END)
        self.output_box.config(state="disabled")


if __name__ == "__main__":
    root = tk.Tk()
    app = CoinLandClient(root)
    root.mainloop()