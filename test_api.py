import subprocess
import time
import socket
import urllib.request
import urllib.error
import json
import sys
import os

# Port to run the test server on
PORT = 8085
SERVER_URL = f"http://localhost:{PORT}"

def run_command(cmd, shell=True):
    print(f"Running command: {cmd}")
    res = subprocess.run(cmd, shell=shell, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    return res.returncode, res.stdout, res.stderr

def test_endpoint(path, method="GET", data=None, headers=None, expected_code=200):
    url = f"{SERVER_URL}{path}"
    req = urllib.request.Request(url, method=method)
    if headers:
        for k, v in headers.items():
            req.add_header(k, v)
    
    if data:
        if isinstance(data, dict):
            req.data = json.dumps(data).encode('utf-8')
            req.add_header("Content-Type", "application/json")
        else:
            req.data = data.encode('utf-8')
            
    try:
        with urllib.request.urlopen(req, timeout=2) as response:
            status = response.status
            body = response.read().decode('utf-8')
            print(f"[{method}] {path} -> Status {status} (Expected {expected_code})")
            if status != expected_code:
                print(f"  FAILED: status got {status}, expected {expected_code}")
                return False, body
            return True, body
    except urllib.error.HTTPError as e:
        body = e.read().decode('utf-8') if e.fp else ""
        print(f"[{method}] {path} -> HTTP Error {e.code} (Expected {expected_code})")
        if e.code == expected_code:
            return True, body
        return False, body
    except Exception as e:
        print(f"[{method}] {path} -> Connection/General Error: {e}")
        return False, str(e)

def test_raw_socket(request_bytes):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(2)
    try:
        s.connect(("127.0.0.1", PORT))
        s.sendall(request_bytes)
        response = s.recv(4096)
        s.close()
        return True, response
    except Exception as e:
        return False, str(e)

def main():
    print("--- 1. Compiling Server ---")
    ret, out, err = run_command("g++ -std=c++11 -I./src -o PetSimulatorServer.exe src/main_server.cpp src/BattleResultCalculator.cpp -lws2_32")
    if ret != 0:
        print("Compilation failed!")
        print(err)
        sys.exit(1)
    print("Compilation successful!")

    print("\n--- 2. Starting Server ---")
    # Start server in background on test port
    server_process = subprocess.Popen(["PetSimulatorServer.exe", str(PORT)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    time.sleep(1.5) # Wait for server to initialize
    
    # Check if server is running
    if server_process.poll() is not None:
        print("Server failed to start!")
        sys.exit(1)
    print(f"Server started on port {PORT}")

    success_all = True

    try:
        print("\n--- 3. Testing API Endpoints ---")
        
        # GET /api/state
        ok, body = test_endpoint("/api/state")
        if not ok: success_all = False
        else:
            state = json.loads(body)
            print("  Initial Pets Count:", len(state.get("pets", [])))
            print("  Initial Backpack Items Count:", len(state.get("backpack", [])))
            
        # GET /api/pets
        ok, body = test_endpoint("/api/pets")
        if not ok: success_all = False
        
        # POST /api/pets (Create fire pet)
        ok, body = test_endpoint("/api/pets", method="POST", data={"type": "Fire", "name": "PythonFire"}, expected_code=201)
        if not ok: success_all = False
        
        # POST /api/pets (Create water pet)
        ok, body = test_endpoint("/api/pets", method="POST", data={"type": "Water", "name": "PythonWater"}, expected_code=201)
        if not ok: success_all = False

        # POST /api/pets (Create mech pet)
        ok, body = test_endpoint("/api/pets", method="POST", data={"type": "Mech", "name": "PythonMech"}, expected_code=201)
        if not ok: success_all = False

        # GET /api/pets/1
        ok, body = test_endpoint("/api/pets/1")
        if not ok: success_all = False

        # POST /api/pets/1/train
        ok, body = test_endpoint("/api/pets/1/train", method="POST")
        if not ok: success_all = False

        # GET /api/backpack
        ok, body = test_endpoint("/api/backpack")
        if not ok: success_all = False

        # POST /api/backpack (Add item)
        ok, body = test_endpoint("/api/backpack", method="POST", data={"name": "Magic Potion", "type": "Potion", "value": 50})
        if not ok: success_all = False

        # POST /api/backpack/sort
        ok, body = test_endpoint("/api/backpack/sort", method="POST")
        if not ok: success_all = False

        # POST /api/battle (Attacker 1 vs Defender 2)
        ok, body = test_endpoint("/api/battle", method="POST", data={"attacker": 0, "defender": 1})
        if not ok: success_all = False

        # POST /api/compare (Compare 1 vs 2)
        ok, body = test_endpoint("/api/compare", method="POST", data={"petA": 0, "petB": 1})
        if not ok: success_all = False

        # GET /api/monsters/random
        ok, body = test_endpoint("/api/monsters/random")
        if not ok: success_all = False

        # POST /api/monsters/battle
        ok, body = test_endpoint("/api/monsters/battle", method="POST", data={"petIndex": 0, "monsterName": "Wild Beast", "monsterType": "fire", "monsterLevel": 2})
        if not ok: success_all = False

        # POST /api/save
        ok, body = test_endpoint("/api/save", method="POST")
        if not ok: success_all = False

        print("\n--- 4. Testing Robustness (Path Traversal check) ---")
        # Send path traversal request to see if we can read save_data.json
        print("Attempting directory traversal to read save_data.json...")
        ok, body = test_endpoint("/../save_data.json", expected_code=404)
        if not ok:
            print("  CRITICAL: Path traversal read successful or returned unexpected status!")
            # If status was 200, then path traversal is definitely vulnerable!
            print("  Response body was:\n", body[:200])
            success_all = False
        else:
            print("  Path traversal to outside directory returned 404.")

        print("\n--- 5. Testing Robustness (Header Parsing Crash check) ---")
        # Send raw request with a header that has no value to see if server crashes
        print("Sending request with empty header value...")
        request_empty_header = b"GET /api/state HTTP/1.1\r\nHost: localhost\r\nMy-Empty-Header:   \r\n\r\n"
        ok_socket, response = test_raw_socket(request_empty_header)
        print("  Socket call success:", ok_socket)
        
        # Check if server is still alive
        if server_process.poll() is not None:
            print("  CRITICAL: Server CRASHED due to empty/spaces header!")
            success_all = False
        else:
            print("  Server is still alive after empty header.")
            # Verify we can still request /api/state
            ok_state, _ = test_endpoint("/api/state")
            if not ok_state:
                print("  CRITICAL: Server is unresponsive after empty header!")
                success_all = False

    finally:
        print("\n--- 6. Shutting Down Server ---")
        server_process.terminate()
        server_process.wait()
        print("Server shutdown completed.")

    if success_all:
        print("\nALL TESTS PASSED SUCCESSFULLY!")
        sys.exit(0)
    else:
        print("\nSOME TESTS FAILED!")
        sys.exit(1)

if __name__ == "__main__":
    main()
