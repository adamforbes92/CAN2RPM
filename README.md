# CAN2RPM
CAN2RPM is based on an ESP32 and is designed to provide an RPM signal for traditional coil based ignition systems.  It is perfect for conversions that don't have a dedicated RPM output and instead output RPM via. CAN.

It is a very lightweight module and is similar to Can2Cluster, but has considerably less functionality (RPM output ONLY).

![CAN2RPM Board](/Images/BoardOverview.png)

## Installation
Boards are supplied with 2x JST-XH cables: 1x 3-pin and 1x 2-pin.

> Coil - 'ignition' coil pulse on the cluster and is RPM output
> GND - chassis ground
> PWR - 12v ignition power

> CANH - CANBUS High
> CANL - CANBUS Low

## WiFi
A WiFi setup is available within the first 60 seconds after boot.  This is to minimise current draw while running.  Once booted, WiFi can be accessed by connecting and searching for 192.168.1.1 on your browser.

## Termination Resistor
A jumper is available for a termination resistor if there are other CAN devices on the network.  If there are, this can be removed.