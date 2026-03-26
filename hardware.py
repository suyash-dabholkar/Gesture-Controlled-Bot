import network
import socket
from machine import Pin, PWM
import time

# Motor A (Left)
AIN1 = Pin(26, Pin.OUT)
AIN2 = Pin(25, Pin.OUT)
PWMA = PWM(Pin(33), freq=1000)

# Motor B (Right)
BIN1 = Pin(14, Pin.OUT)
BIN2 = Pin(12, Pin.OUT)
PWMB = PWM(Pin(13), freq=1000)

STDBY = Pin(27, Pin.OUT)

SSID = "NAME_OF_AP"     
PASSWORD = "PASSWORD_AP" 
UDP_PORT = 4210         

def setup_sta():
    """Initializes WiFi connection in Station mode and prints the local IP address."""
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(SSID, PASSWORD)
    
    print("Connecting to Wi-Fi...")
    while not wlan.isconnected():
        time.sleep(0.5)
        
    ip_address = wlan.ifconfig()[0]
    print(f"Connection Successful! Local IP Address: {ip_address}")
    return wlan


def run_motor(motor, spd, direction):
    STDBY.value(1)  
    
    dir_pin1 = 0    
    dir_pin2 = 1    
    if direction == 1:
        dir_pin1 = 1
        dir_pin2 = 0

    duty_cycle = int((spd / 255.0) * 65535)

    if motor == 0:  # Motor A
        AIN1.value(dir_pin1)
        AIN2.value(dir_pin2)
        PWMA.duty_u16(duty_cycle)
    else:           # Motor B
        BIN1.value(dir_pin1)
        BIN2.value(dir_pin2)
        PWMB.duty_u16(duty_cycle)

def move_left(spd):
    """Rotates the robot left in place."""
    run_motor(0, spd, 0)
    run_motor(1, spd, 1)

def move_right(spd):
    """Rotates the robot right in place."""
    run_motor(0, spd, 1)
    run_motor(1, spd, 0)

def move_forward(spd):
    """Moves the robot straight forward."""
    run_motor(0, spd, 0)
    run_motor(1, spd, 0)

def move_back(spd):
    """Moves the robot straight backward."""
    run_motor(0, spd, 1)
    run_motor(1, spd, 1)

def stop():
    """Stops both motors and puts the driver in standby mode."""
    STDBY.value(0)
    PWMA.duty_u16(0)
    PWMB.duty_u16(0)

def main():
    """Initializes network, sets up UDP server, and processes incoming commands."""
    setup_sta()
    
    # Setup UDP Socket server
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('0.0.0.0', UDP_PORT))

    sock.settimeout(0.1) 
    print(f"UDP server active on port {UDP_PORT}")

    while True:
        try:
            data, addr = sock.recvfrom(255)
            try:
                fingers_extended = int(data.decode('utf-8').strip())
                print(f"Received Command: {fingers_extended} fingers")

                if fingers_extended == 0:
                    stop()
                    print("Status: Stopped")
                elif fingers_extended == 1:
                    move_forward(255)
                    print("Status: Moving Forward")
                elif fingers_extended == 2:
                    move_back(255)
                    print("Status: Moving Backward")
                elif fingers_extended == 3:
                    move_left(255)
                    print("Status: Turning Left")
                elif fingers_extended == 4:
                    move_right(255)
                    print("Status: Turning Right")
                else:
                    stop()
                    print("Status: Command Not Recognized - Stopped")

            except (ValueError, UnicodeError):
                print("Error: Invalid or malformed packet received.")
                stop()

        except OSError:
            pass

if __name__ == "__main__":
    main()