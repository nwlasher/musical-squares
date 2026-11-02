
# Purpose / Overview
This project was realized for project 3 of the University at Buffalo's EE526: Wearable and Implantable Sensors. The device consists of 3 TCS34725 RGB Color/Light sensors, a Class-D amplifier, some LEDs, an Arduino giga R1 Wifi, and some colorful tiles. In addition, a companion IOS app exists as a supplement for said colorful tiles. If I finish it, that will be a fun way to play songs using the system to scan and Ipad screen to play longer pre-programmed tracks. It's been a while since I have picked a simple idea, some sensors, and some embedded systems periphrals that I havenever used and made something. I figured I would use this class as an excuse.

Speaking of which, this system uses the color sensors to sense ambient color and react to specific colors by playing a corresponding tone on the included speakers. This simple system is a realization of a fun Idea I have had for a childrens toy for a while. Allowing kids to experiment with a simple set of rules and elements to create as they see fit. It is also analogous to the concept of engineering using first principles. ...ok maybe a bit of a stretch.

# References

For a full list of references see the 'Documentation/' folder. You'll find a technical document, a breif presentation deck and other fun stuff

# Directory Layout

Below is a tree that I will update with each polishing push outlining the directory layout.
Tests exist as separate Platformio projects that can be flashed individually to test the hardware and software at different levels. The main project software is contained within 'musical-squares/'. 

../
├── Code/
│   └── Testbenches/
│       ├── audio-debug/
│       ├── giga-blink/
│       └── rgb-sensor-debug/
├── Documentation/
│   ├── Presentation/
│   └── Technical-reference/
│       ├── Images/
│       │   ├── debug-out-config.jpeg
│       │   ├── debug-out.jpg
│       │   ├── GigaR1Wifi-pinout.png
│       │   └── IMG_2811.jpeg
│       ├── tech-docs-ms.pdf
│       └── tech-docs-ms.typ
├── map-repo.sh
└── README.md
