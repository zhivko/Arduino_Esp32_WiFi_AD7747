# D-duino-32(ESP32&0.96OLED display) & AD7747

This is simple Arduino 1.8 sketch that runs on https://www.tindie.com/products/lspoplove/d-duino-32esp32-and-096oled-display/
and uses:
graphic arduino lib for oled (https://github.com/squix78/esp8266-oled-ssd1306)
websockets library (https://github.com/bbx10/arduinoWebSockets)
hash library required by websockets (https://github.com/bbx10/Hash_tng.git)

Reads distance via capacity sensor AD7747:
(reference: http://opensourceecology.org/wiki/Paul_Log#Tue_Jul_30.2C_2013)
and streams data as standard websocket server that runs on esp32 on port 81.

