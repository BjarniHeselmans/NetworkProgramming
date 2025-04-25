import zmq
import tkinter as tk
from tkinter import messagebox

# Adressen zoals op de Benternet
SERVER_PUSH_ADDR = "tcp://benternet.pxl-ea-ict.be:24041"
SERVER_SUB_ADDR = "tcp://benternet.pxl-ea-ict.be:24042"
TOPIC_PREFIX = "Bjarni>SlotMachine!>"

class SlotMachineClient:
    def __init__(self, root):
        self.root = root
        self.root.title("🎰 SlotMachine Client")
        self.name = ""
        self.context = zmq.Context()

        # PUSH-socket voor verzoeken
        self.push_socket = self.context.socket(zmq.PUSH)
        self.push_socket.connect(SERVER_PUSH_ADDR)

        # SUB-socket voor reacties
        self.sub_socket = self.context.socket(zmq.SUB)
        self.sub_socket.connect(SERVER_SUB_ADDR)

        self.build_gui()

    def build_gui(self):
        # Naamveld
        tk.Label(self.root, text="Voer je naam in:").pack()
        self.name_entry = tk.Entry(self.root)
        self.name_entry.pack()

        # Speelknop
        self.play_button = tk.Button(self.root, text="🎰 Draai!", command=self.play_slot)
        self.play_button.pack(pady=10)

        # Output
        self.output_box = tk.Text(self.root, height=10, state="disabled")
        self.output_box.pack(pady=10)

    def play_slot(self):
        self.name = self.name_entry.get().strip()
        if not self.name:
            messagebox.showwarning("Fout", "Voer een geldige naam in.")
            return

        topic = TOPIC_PREFIX + self.name + ">"
        self.sub_socket.setsockopt_string(zmq.SUBSCRIBE, topic)

        # Stuur speelverzoek
        message = f"Bjarni>SlotMachine?>{self.name}>"
        self.push_socket.send_string(message)

        self.output(f"[Verzonden] Draai voor speler '{self.name}'...")
        self.root.after(100, self.poll_response)

    def poll_response(self):
        try:
            msg = self.sub_socket.recv_string(flags=zmq.NOBLOCK)
            self.output(f"[Resultaat] {msg}")
        except zmq.Again:
            self.root.after(100, self.poll_response)

    def output(self, tekst):
        self.output_box.config(state="normal")
        self.output_box.insert(tk.END, tekst + "\n")
        self.output_box.see(tk.END)
        self.output_box.config(state="disabled")


if __name__ == "__main__":
    root = tk.Tk()
    app = SlotMachineClient(root)
    root.mainloop()
