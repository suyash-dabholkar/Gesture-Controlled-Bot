import network
import socket
from time import sleep

def setup_sta():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    
    SSID = 'XX'
    PASSWORD = 'YY'
    
    print(f"Connecting to {SSID}...")
    wlan.connect(SSID, PASSWORD)
    
    while not wlan.isconnected():
        print(".", end="")
        sleep(1)
    
    ip_address = wlan.ifconfig()[0]
    print(f"\nConnected! IP address: {ip_address}")
    return wlan

if __name__ == "__main__":
    setup_sta()