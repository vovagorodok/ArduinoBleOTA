# 1 "/tmp/tmpewo24iz0"
#include <Arduino.h>
# 1 "/home/vova/projects/magic_chess_game/ArduinoBleOTA/examples/basic/main.ino"
#include <ArduinoBleOTA.h>
void setup();
void loop();
#line 3 "/home/vova/projects/magic_chess_game/ArduinoBleOTA/examples/basic/main.ino"
void setup() {
  ArduinoBleOTA.begin("ArduinoBleOTA", InternalStorage);
}

void loop() {
  ArduinoBleOTA.pull();
}