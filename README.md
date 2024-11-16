# Fire Alarm Sequencer: Virtual Coded Pull Station Module
This module aims to recreate the notion of an antique mechanical fire pull station with code wheels, in the style of a modern addressable pull station module. It can also be used as a sync source and sequencer for various fire devices, such as tone modules, horn/strobes, and older electromechanical devices such as the autocall howe-transmitter.

Currently, support for well-known major codes and various march-time rates are included, but other fun things have also been included, such as replications of a few famous code wheels, such as the standard electric march time, and various siren code wheeels.

Additionally, a virtual "shunt" is provided, which can provide a NC/NO "running" status of the pull station, and both can be tied to "NAC" style inputs. Most prototypes built have had a switch to allow the code output to have independent power, or to pass-thru the shunt, which allows some interesting combinations. A "reverse" and "trigger" output are also present, originally meant for controlling PWM motor controllers for doing sync protocols, but get re-used/re-purposed for tone module & door chime control where more outputs are needed.

The current code expect 7 momentary buttons, plus an optoisolated NAC input. There's a shunt toggle, plus an up/down button for each of Fire, March, and Code Wheel codesets. Each set of buttons will scroll up/down through that section of codes, accordingly.

It also supports a SSD1306 128x64 OLED to display status and the name of the current code.

coder1.yaml and coder2.yaml are based on the standard esp32 38pin devkit and can drive any relays or mosfets with low voltage triggers (such as an L298 motor controller.)

coder3.yaml is based around the ESP32 Relay X2
https://devices.esphome.io/devices/ESP32-Relay-X2

coder4.yaml is for the AC/DC powered ESP32 Relay Board x4:
https://devices.esphome.io/devices/AC-DC-ESP32-Relay-x4

Other board configurations will be added once they can be confirmed.
