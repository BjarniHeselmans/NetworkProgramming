import zmq
import tkinter as tk
from tkinter import messagebox
from tkinter import font as tkfont
import random
import re

SERVER_PUSH_ADDR = "tcp://benternet.pxl-ea-ict.be:24041"
SERVER_SUB_ADDR = "tcp://benternet.pxl-ea-ict.be:24042"

class CoinLandClient:
    def __init__(self, root):
        self.root = root
        self.root.title("🎰 CoinLand Client")
        self.name = ""
        self.munten = 0
        self.context = zmq.Context()
        self.debug_visible = True

        self.symbol_map = {
            "CHERRY": "🍒",
            "LEMON": "🍋",
            "BELL": "🔔",
            "STAR": "⭐",
            "DIAMOND": "💎"
        }

        self.slot_images = list(self.symbol_map.values())

        # Sockets
        self.push_socket = self.context.socket(zmq.PUSH)
        self.push_socket.connect(SERVER_PUSH_ADDR)

        self.sub_socket = self.context.socket(zmq.SUB)
        self.sub_socket.connect(SERVER_SUB_ADDR)

        self.build_gui()

        # Start continue luisteren naar antwoorden
        self.root.after(100, self.poll_response)

    def build_gui(self):
        self.root.configure(bg="#1e1e1e")
        header = tk.Label(self.root, text="🎰 CoinLand", fg="gold", bg="#1e1e1e", font=("Helvetica", 20, "bold"))
        header.pack(pady=10)

        self.balance_label = tk.Label(self.root, text="💰 Muntjes: 0", fg="lightgreen", bg="#1e1e1e", font=("Helvetica", 14, "bold"))
        self.balance_label.pack()

        form_frame = tk.Frame(self.root, bg="#1e1e1e")
        form_frame.pack(pady=10)

        tk.Label(form_frame, text="Naam:", bg="#1e1e1e", fg="white", font=("Helvetica", 12)).grid(row=0, column=0, sticky="e")
        self.name_entry = tk.Entry(form_frame, font=("Helvetica", 12))
        self.name_entry.grid(row=0, column=1, padx=5, pady=5)

        # Sync knop toegevoegd
        self.sync_button = tk.Button(form_frame, text="🔄 Sync", command=self.sync_balance, bg="blue", fg="white", font=("Helvetica", 12, "bold"))
        self.sync_button.grid(row=0, column=2, padx=10)

        tk.Label(form_frame, text="Gok (1-5):", bg="#1e1e1e", fg="white", font=("Helvetica", 12)).grid(row=1, column=0, sticky="e")
        self.guess_entry = tk.Entry(form_frame, font=("Helvetica", 12))
        self.guess_entry.grid(row=1, column=1, padx=5, pady=5)

        tk.Button(form_frame, text="🎯 Speel CoinGame", command=self.play_coin_game, bg="orange", fg="black", font=("Helvetica", 12)).grid(row=2, column=0, columnspan=3, pady=10)

        tk.Label(form_frame, text="Inzet (SlotMachine):", bg="#1e1e1e", fg="white", font=("Helvetica", 12)).grid(row=3, column=0, sticky="e")
        self.bet_entry = tk.Entry(form_frame, font=("Helvetica", 12))
        self.bet_entry.insert(0, "2")
        self.bet_entry.grid(row=3, column=1, padx=5, pady=5)

        tk.Button(form_frame, text="🎰 Draai SlotMachine", command=self.play_slot_machine, bg="red", fg="white", font=("Helvetica", 12, "bold")).grid(row=4, column=0, columnspan=3, pady=10)

        self.slot_frame = tk.Frame(self.root, bg="black")
        self.slot_frame.pack(pady=10)
        self.slot_labels = [tk.Label(self.slot_frame, text="❔", font=("Helvetica", 48), fg="gold", bg="black", width=4) for _ in range(3)]
        for label in self.slot_labels:
            label.pack(side=tk.LEFT, padx=15)

        self.output_box = tk.Text(self.root, height=8, bg="black", fg="lime", font=("Courier", 12), state="disabled")
        self.output_box.pack(padx=10, pady=10, fill=tk.BOTH, expand=True)

    def sync_balance(self):
        naam = self.name_entry.get().strip()
        if not naam:
            messagebox.showerror("Fout", "Vul eerst een naam in voor synchronisatie.")
            return

        self.name = naam
        # Abonneer op sync topic voor deze naam
        topic = f"Bjarni>Sync!>{self.name}>"
        self.sub_socket.setsockopt_string(zmq.SUBSCRIBE, topic)

        # Verstuur sync verzoek
        message = f"Bjarni>Sync?>{self.name}>"
        self.push_socket.send_string(message)
        self.output(f"🔄 Sync verzoek verstuurd voor {self.name}...")

    def play_coin_game(self):
        guess = self.guess_entry.get().strip()
        if not self.name_entry.get().strip() or not guess.isdigit():
            messagebox.showerror("Fout", "Vul een geldige naam en gok in.")
            return
        if not (1 <= int(guess) <= 5):
            messagebox.showerror("Fout", "Gok moet tussen 1 en 5 liggen.")
            return

        self.name = self.name_entry.get().strip()
        # Abonneer op CoinGame antwoorden voor deze naam
        topic = f"Bjarni>CoinGame!>{self.name}>"
        self.sub_socket.setsockopt_string(zmq.SUBSCRIBE, topic)

        message = f"Bjarni>CoinGame?>{self.name}>{guess}>"
        self.push_socket.send_string(message)
        self.output(f"🎯 Gok {guess} verzonden...")

    def play_slot_machine(self):
        inzet = self.bet_entry.get().strip()
        if not inzet.isdigit() or int(inzet) <= 0:
            messagebox.showerror("Fout", "Voer een geldige inzet in.")
            return

        if self.munten < int(inzet):
            messagebox.showerror("Fout", f"Te weinig muntjes ({self.munten}) voor inzet van {inzet}.")
            return

        self.name = self.name_entry.get().strip()
        # Abonneer op SlotMachine antwoorden voor deze naam
        topic = f"Bjarni>SlotMachine!>{self.name}>"
        self.sub_socket.setsockopt_string(zmq.SUBSCRIBE, topic)

        message = f"Bjarni>SlotMachine?>{self.name}|{inzet}>"
        self.push_socket.send_string(message)
        self.output(f"🎰 Spin verzonden met inzet {inzet}...")

    def poll_response(self):
        try:
            while True:
                msg = self.sub_socket.recv_string(flags=zmq.NOBLOCK)
                self.output("📩 " + msg)
                self.update_from_msg(msg)
        except zmq.Again:
            pass
        self.root.after(100, self.poll_response)

    def update_from_msg(self, msg):
        # Update muntjes als sync antwoord of ander bericht met saldo
        if "Je hebt nog" in msg or "Je hebt nu" in msg:
            coins = re.findall(r"Je hebt(?: nog| nu) (\d+) muntjes", msg)
            if coins:
                self.munten = int(coins[0])
                self.balance_label.config(text=f"💰 Muntjes: {self.munten}")

        # Update slotmachine symbolen
        if "Spin:" in msg:
            match = re.search(r"Spin: ([A-Z]+) ([A-Z]+) ([A-Z]+)", msg)
            if match:
                for i in range(3):
                    sym = match.group(i+1)
                    self.slot_labels[i].config(text=self.symbol_map.get(sym, "❔"))

    def output(self, tekst):
        self.output_box.config(state="normal")
        self.output_box.insert(tk.END, tekst + "\n")
        self.output_box.see(tk.END)
        self.output_box.config(state="disabled")

if __name__ == "__main__":
    root = tk.Tk()
    app = CoinLandClient(root)
    root.mainloop()
