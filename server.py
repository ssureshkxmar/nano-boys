import socket
import threading
import os
import sys
import math
import random
import time
from flask import Flask, jsonify, render_template
from Crypto.Cipher import AES
from Crypto.Util.Padding import unpad

app = Flask(__name__)

system_state = {
    "decrypted": "Waiting...",
    "encrypted": "No Data",
    "key_id": "None",
    "entropy": "Initializing...",
    "timestamp": 0
}
reading_lock = threading.Lock()
current_session_key = None

class HyperChaosCrypto:
    def __init__(self, seed=123456789):
        self.state = seed & 0xFFFFFFFF
        
    def iterate(self):
        self.state = (self.state * 1664525 + 1013904223) & 0xFFFFFFFF
        return (self.state / 4294967296.0)

class LatticeCrypto:
    def __init__(self, n=4, q=255):
        self.n = n 
        self.q = q 
        self.chaos = HyperChaosCrypto()
        
    def gen_poly(self):
        return [int(self.chaos.iterate() * self.q) % self.q for _ in range(self.n)]

    def generate_keys(self):
        self.A = [self.gen_poly() for _ in range(self.n)] 
        self.S = self.gen_poly()
        self.E = self.gen_poly() 
        
        self.B = [0] * self.n
        for i in range(self.n):
            row_val = sum(self.A[i][j] * self.S[j] for j in range(self.n))
            self.B[i] = (row_val + self.E[i]) % self.q
            
        return self.B 

    def decapsulate(self, U, V):
        approx_k = [0] * self.n
        for i in range(self.n):
            s_dot_u = sum(self.S[j] * U[j] for j in range(self.n)) 
            approx_k[i] = (V[i] - s_dot_u) % self.q
            
        key_byte = sum(approx_k) % 256
        return bytes([key_byte] * 16) 

lwe_system = LatticeCrypto()

def encryption_server():
    global latest_reading, current_session_key
    s = socket.socket()
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    try:
        s.bind(('0.0.0.0', 8080))
        s.listen(1)
        print("HC-LEP Server listening on 8080...", flush=True)
    except Exception as e:
        print(f"CRITICAL: {e}", flush=True)
        os._exit(1)

    while True:
        try:
            conn, addr = s.accept()
            print(f"Connection from {addr}", flush=True)
            
            public_key_b = lwe_system.generate_keys()
            pk_str = ",".join(map(str, public_key_b)) + "\n"
            conn.sendall(pk_str.encode())
            
            data = conn.recv(32) 
            if data:
                key_gen_chaos = HyperChaosCrypto(seed=123456789) 
                key_gen_chaos.iterate() 
                
                chaos_secret = (key_gen_chaos.state >> 24) & 0xFF
                current_session_key = bytes([chaos_secret] * 16)
                
                cipher = AES.new(current_session_key, AES.MODE_ECB)
                try:
                    plain = cipher.decrypt(data[:16])
                    decoded_msg = plain.decode('utf-8', errors='ignore').rstrip('\x00')
                    print(f"Decrypted: {decoded_msg}", flush=True)
                    
                    # Parse D:XXXX,A:YYYY
                    dist = "---"
                    angle = "0"
                    if "," in decoded_msg:
                        parts = decoded_msg.split(",")
                        for p in parts:
                            if p.startswith("D:"): dist = p[2:]
                            if p.startswith("A:"): angle = p[2:]
                    elif decoded_msg.startswith("D:"):
                        dist = decoded_msg[2:]

                    with reading_lock:
                        system_state["decrypted"] = decoded_msg
                        system_state["distance"] = dist
                        system_state["angle"] = angle
                        system_state["encrypted"] = data[:16].hex().upper()
                        system_state["key_id"] = current_session_key.hex()[:8] + "..."
                        system_state["entropy"] = f"{key_gen_chaos.state / 4294967296.0:.6f}"
                        system_state["timestamp"] = time.time()
                        
                except Exception as e:
                    print(f"Decryption ERROR: {e}", flush=True)
                    pass
            
            conn.close()
        except Exception as e:
            print(f"Socket ERROR: {e}", flush=True)
            pass

@app.route('/')
def home():
    return render_template('index.html')

@app.route('/data')
def get_data():
    with reading_lock:
        response = jsonify(system_state)
        # Disable caching completely
        response.headers["Cache-Control"] = "no-cache, no-store, must-revalidate"
        response.headers["Pragma"] = "no-cache"
        response.headers["Expires"] = "0"
        return response

if __name__ == '__main__':
    t = threading.Thread(target=encryption_server, daemon=True)
    t.start()
    app.run(host='0.0.0.0', port=5000)
