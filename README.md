# CAN2RPM
CAN2RPM is based on an ESP32 and is designed to provide an RPM signal for traditional coil based ignition systems.  It is perfect for conversions that don't have a dedicated RPM output and instead output RPM via. CAN.

It is a very lightweight module and is similar to Can2Cluster, but has considerably less functionality (RPM output ONLY).

![CAN2RPM Board](/Images/BoardOverview.png)

## Installation
Boards are supplied with 2x JST-XH cables: 1x 3-pin and 1x 2-pin.

### Power and Outputs
> Coil - 'ignition' coil pulse on the cluster and is RPM output

> GND - chassis ground

> PWR - 12v ignition power

### CANBUS Inputs
> CANH - CANBUS High

> CANL - CANBUS Low

NB: The PCB is marked incorrectly! CANH/L are switched(!).  To be revised(!).

### Das Blinken Lightsen
The main red power LED will be illuminated during power.
The onboard blue LED will flash if there is an ERROR - i.e., no incoming CANBUS signals.  The onboard blue LED will stay off during normal operation.

## WiFi
A WiFi setup is available within the first 30 seconds after boot.  This is to minimise current draw while running.  Once booted, WiFi can be accessed by connecting and searching for 192.168.1.1 on your browser.

Typical setups include the addition of needle sweep or changing RPM scaling/range.

## Termination Resistor
A jumper is available for a termination resistor IF there are other CAN devices on the network.  If there are, this can be removed.

## Circuit Design
Traditional ignition coils create a high-voltage 'spike' when the current is removed from the OEM coil.  The concept of this circuit is to re-create this by charging and collapsing the 'ignition coil'.  A "DR125-124-R" inductor is used as this shares a similar 'resistance' and very low inductance value which mimicks an OEM ignition coil.

The ESP32 reads CANBUS data and captures the RPM value.  It is based on VAG ECUs and therefore the CAN ID for RPM is in 'Motor1' - 0x280.  Aftermarket ECUs can be added if required.

The RPM is then converted into a lower frequency pulse (default ranging typically from 0Hz to 230Hz: 0 to 7000RPM).  
![CAN2RPM Schematic](/Images/Schematic.png)
