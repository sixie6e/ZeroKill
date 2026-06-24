import serial
import time
import sys
import os

port = '/dev/ttyUSB0'  # check with 'dmesg | grep tty'
baud = 115200
logs = "zero_kill.csv"

if not os.path.exists(logs):
    with open(logs, "w") as f:
        f.write("Timestamp,Sensor_Value,System_Status\n")

try:
    esp32 = serial.Serial(port, baud, timeout=1)
    time.sleep(2) 
    print("Connected to ESP32-C3.")
except Exception as e:
    print(f"Failed: {e}")
    sys.exit(1)

def esp32comm(command):
    try:
        esp32.write(command.encode('utf-8'))
        response = esp32.readline().decode('utf-8').strip()
        return response
    except Exception as e:
        return f"Error:{e}"

try:
    last_poll = 0
    
    while True:
        current_time = time.time()
        if current_time - last_poll > 2:
            response = esp32comm('R')
            if response.startswith("DATA:"):
                try:
                    # format: DATA:Value,Status
                    p = response.split(":")[1]
                    sensor_val_str, status_str = p.split(",")
                    sensor_val = int(sensor_val_str)
                    
                    print(f"[{time.strftime('%Y-%m-%d %H:%M:%S')}] Metrics: {sensor_val} | Status: {status_str}")

                    with open(logs, "a") as log:
                        log.write(f"{current_time},{sensor_val},{status_str}\n")
                    
                    if status_str == "OK":
                        if sensor_val < 300:
                            cmd_res = esp32comm('P')
                        elif sensor_val > 600:
                            cmd_res = esp32comm('p')
                    else:
                        print("Warning: ESP32 safety lockout active.")
                        
                except (ValueError, IndexError):
                    print("Malformed serial packet.")
            
            elif "error" in response:
                print(f"Error: {response}")
                def alert:
					pass
                def sms:
					pass
					
            last_poll = current_time
            
        time.sleep(0.1) 

except KeyboardInterrupt:
    print("\nShutting down. ESP32 safety loops remain active.")
    esp32.close()
