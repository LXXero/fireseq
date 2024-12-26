# Fire Alarm Sequencer: Virtual Coded Pull Station Module
This module aims to recreate the notion of an antique mechanical fire pull station with code wheels, in the style of a modern addressable pull station module. It can also be used as a sync source and sequencer for various fire devices, such as tone modules, horn/strobes, and older electromechanical devices such as the autocall howe-transmitter.


## Features
- Support for well-known major codes and various march-time rates
- Replicates various codes from old panels and pull stations, such as the standard electric march time, and various siren code wheeels.
- Optional SSD1306 128x64 OLED to display status and the name of the current code.
- A shunt, which typically provides NC/NO "running" status of the pull station. Some models have a switch to allow the coded output to have independent power, or to pass-thru the shunt first, which allows some interesting combinations.
- **Reverse** and **Trigger** auxilary outputs are also provided. These have multiple uses at present:
  - controlling PWM motor controllers for doing sync protocols
  - tone module switching
  - triggering door chimes w/multiple notes

## User Interface
The fire sequencer has 3 sections of codes, which are accessed by 3 pairs of up/down buttons. Additionally, there's a run/stop toggle. No menu diving required.
- 7 momentary buttons:
  - Toggle
  - Fire Up/Down
  - March Up/Down
  - Code Up/Down


## Inputs/Outputs
- Zone input (Optocoupler/Pulldown)
  - gpio_zone_invert is typically set to true when using an optocoupler, and false otherwise.
- Doorbell Input (Pulldown, Relay Activated)
  - cycles through a dedicated list of "doorbell" codes upon each button press
  - can be attached to external relay for activation w/smart doorbells, etc
- Code output (Relay/FET)
- Shunt output (Relay/FET)
- Reverse output (Relay/FET)
- Trigger output (Relay/FET)


coder1.yaml, coder2.yaml, and coder2001.yaml are based on the standard esp32 38pin devkit and can drive any relays or mosfets with low voltage triggers (such as an L298 motor controller.)

coder3.yaml is based around the ESP32 Relay X2
https://devices.esphome.io/devices/ESP32-Relay-X2

coder4.yaml is for the AC/DC powered ESP32 Relay Board x4:
https://devices.esphome.io/devices/AC-DC-ESP32-Relay-x4

Other board configurations will be added once they can be confirmed.
