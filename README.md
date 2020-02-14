# Ad-hoc Messaging Service
### A service to exchange messages between the nodes of an ad hoc network in real-time

This project provides a real-time message exchange system for ad hoc networks. The communication between the nodes is based on TCP sockets. The implementation uses tools available on most POSIX-compliant systems. It was also tested on a wireless ad hoc network (WANET) consisted of 3 Raspberry Pi Zero W devices.

## Description

## Setup

* Step 1: Create an ad hoc network.
* Step 2: Assign a 4-digit ID to every device of the network.
* Step 3: Set the IP address of each device. If the ID of a device is XXYY, then its IP address must be 10.0.XX.YY.
* Step 4: Fill the *devices_info/device_ids.txt* file with the IDs of all devices in the network. ([example](devices_info/device_ids.txt))

## Compilation
If a device in the network has ID=XXYY, then the program for this device will be compiled as follows:

    make DEVICE_ID=XXYY
    
For example:

    make DEVICE_ID=8448
    
The executable will be located in the *bin/* directory.

## Execution
First navigage to the *bin/* directory.

    cd bin

If a device in the network has ID=XXYY, the name of the executable will be *main_XXYY*.
Run the command:

    ./main_XXYY t
    
where:
* t: the total time in seconds that the service will run 
  
For example:

    ./main_8448 7200
