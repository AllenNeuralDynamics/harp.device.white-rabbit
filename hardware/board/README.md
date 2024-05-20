## Overview

These schematic and layout designs have been vetted on prior devices.

High Level Specs:
  * USBC connector (ESD protected with TVS diode).
  * Audio jack connector for Harp synchronization signal (ESD protected with TVS diode).

Component Specs:
  * aqueous washable
  * commonly available components (no end-of-life or last-time-buy)

Layout Specs:
  * 4-layer PCB
      * signal (top), ground, power, signal (bottom)
  * 6-mil traces, 6-mil spacing (0.1524mm)
  * no microvias
  * smallest resistor/capacitor sizes are 0402.
  * Footprints default to "non-hand-soldering"-style.

MCU Schematic Specs:
  * 12MHz oscillator, +/-2.5ppm
  * 16MB flash, Winbond W25Q16JVUXIQ

### USB-Isolated-Only Template Specs
  * Electrically isolated from PC, Analog Devices ADUM4160

## Instructions

Pick a template folder (or none!) as a starting point and delete the remaining other folders.


Rename the folder and all subfolder files of the same name to the name of your project.


Delete the instructions in this file.
