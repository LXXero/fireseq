# Fire Alarm Sequencer: Virtual Coded Pull Station Module
This module aims to recreate the notion of an antique mechanical fire pull station with code wheels, in the style of a modern addressable pull station module.

Currently, support for the major codes and various march BPMs are included, but future use cases may include simulating actual various station codes.

Additionally, a virtual "shunt" is provided, which can provide a NC/NO "running" status of the pull station, and both can be tied to "NAC" style inputs. Most prototypes built have had a switch to allow the code output to have independent power, or to pass-thru the shunt, which allows some interesting combinations.

The current code expect 7 momentary buttons, plus an optoisolated NAC input. There's a shunt toggle, plus an up/down button for each of Fire, March, and Code Wheel codesets. Each set of buttons will scroll up/down through that section of codes, accordingly.

Support for various displays is currently planned.

Current code is based around the ESP32 Relay X2, but additional configurations are also known to work with some tweaks.
https://www.amazon.com/Capacity-Programmable-Secondary-Development-Learning/dp/B0B8J9SNB5

Other supported board configurations will be added once they can be confirmed, AC/DC Relay X4 support is planned.
