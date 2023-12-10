40mHz Submarine Controller
=======

This repository contains example code to control a 40mHz submarine like the one available through the following link:
[LSRC-332 Mini RC Submarine](https://nl.aliexpress.com/item/1005005888269003.html)

# Software

## Carrier wave
The submarine used for this code uses a 40.680mHz carrier wave. Submarines of another type or bought on another date may use a
(slightly) different carrier wave. To verify the carrier wave, the delivered remote control can be taken apart. The correct frequency should be 
written on the frequency crystal in the remote control. 

For other carrier waves, the setup code for the SI5351 needs to be rewritten using the following guide: [Programming the SI5351](https://miscircuitos.com/clock-generator-si5351/). 

## IDE
The code can be openened in the Arduino IDE. Note that the library "Adafruit Si5351 Library" is needed. 

## Setup
### Components

- [Arduino Nano](https://nl.aliexpress.com/item/1005005967644175.html)
- [SI5351A (25mHz)](https://nl.aliexpress.com/item/4000040035169.html)

### Wiring
![WiringDiagram.png](WiringDiagram.png)
The above wiring diagram can be used to connect the components.
For the antenna, any piece of wire could be used. The antenna on the original remote control could also be connected in this place. 


## License
Copyright Niels Post 2023.
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
https://www.boost.org/LICENSE_1_0.txt)