# WifiSignalLogger-ESP32

This project is meant to be a Wifi Strength manual data logger built with an ESP32-WROOM-32.

Made by:
- Gustavo Gomes ([Gustavo0022](https://github.com/Gustavo0022)) 
- Lucas Santana ([LucaskaSL](https://github.com/LucaskaSL))
- Irlan Felipe([])
- Paulo Medeiros([PauloBaja](https://github.com/PauloBaja))

# Used Libraries

- [Wifi.h](https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi)
- [SimpleFTPServer](https://github.com/xreef/SimpleFTPServer)
- [LittleFS](https://github.com/littlefs-project/littlefs)

# Usage

After downloading the required libraries to Arduino IDE, and opening [WiFiSignalLogger](./src/WiFiSignalLogger.ino), change the SSID and the password to the corresponding data of the network you want to measure strength. Then, compile and send the code to the board.

On serial monitor, the board will give connection status, file status and write status. Get the IP address provided on the serial monitor, and with a device connected on the same network as the board, connect to the board with a file explorer that supports FTP, or a separate software such as FileZilla. The file '**TesteWifi.csv**' will contain every measure made since the file was created.

To measure the WiFi strength at a point, press the white button (or the button connected to pin 15/buttonMeasurePoint). To save the current place's average signal strength, press the red button (or the button connected to 22/buttonChangePlace). To delete the file, press the button connected to pin 23 or buttonDelete.

While measuring, the LED connected to pin 21 (LEDWrite) will flash. Similarly, while deleting a file, the LED connected to pin 19 (LEDDelete). While measuring the average strength, both LEDS might flash.

# How does it work

(write tomorrow)

# Schematic
The main schematic is displayed below, showing every peripheral connected to the ESP32 board

![Schematic](./assets/circuit.png).

