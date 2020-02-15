# Ad-hoc Messaging Service
### A service to exchange messages between the nodes of an ad hoc network in real-time

This project provides a real-time message exchange system for ad hoc networks. The communication between the nodes is based on TCP sockets. The implementation uses tools available on most POSIX-compliant systems. It was also tested on a wireless ad hoc network (WANET) consisted of 3 Raspberry Pi Zero W devices.

## Description
The goal of this project is the implementation of a message exchange system which handles the flow of messages between the nodes of an ad hoc network. For the network nodes, real-time embedded systems could be used. \
This application runs on every device in the network and is responsible for transmitting and receiving messages through WiFi with every other device in its range. Every message contains the following fields: 
* Sender ID: `uint32`
* Recipient ID: `uint32`
* Creation time: Linux timestamp `uint64`
* Message body: ASCII `char[256]`

These fields are seperated by the symbol `_`. For example:

    7592_6998_1387909800_Test message
    
The application saves in a buffer the messages that it creates or receives. It also keeps track of the devices to which the messages have been sent. Every time that two devices connect with each other, an exchange of their new messages is performed. A device attempts to connect with other devices in its range, when it has new messages in the buffer.  

Every node in the network is characterized by a 4-digit ID. IP addresses are formed as `10.0.XX.YY`, where XX are the first 2 digits and YY the last 2 digits of the ID. 

For simulation purposes, every node generates messages to a random recipient in a random time interval of 1 to 5 minutes. 

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

## Experiment 
The service was tested on a wireless ad hoc network consisted of 3 Raspberry Pi Zero W devices. These devices were running Raspbian operating system. For each device the code was cross-compiled using the tools that can be found [here](https://github.com/abhiTronix/raspberry-pi-cross-compilers). Then, the programs were transferred to the network nodes via SCP. \
The logs that were collected for 2 hours of execution can be found [here](logs/). There is also a full [report](experiment_report/report.pdf) on the results (available only in greek). 
