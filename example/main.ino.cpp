# 1 "/tmp/tmpqy9c06do"
#include <Arduino.h>
# 1 "/home/vova/projects/magic_chess_game/ArduinoBleOTA/example/main.ino"
#include <ArduinoBleOTA.h>
void setup();
void loop();
#line 3 "/home/vova/projects/magic_chess_game/ArduinoBleOTA/example/main.ino"
void setup() {
  ArduinoBleOTA.begin("ArduinoBleOTA", InternalStorage);
}

void loop() {
  ArduinoBleOTA.update();
}