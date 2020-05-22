## General settings
* Ubuntu 16.04
* Code tested and produces same results with [MMR](https://mbientlab.com/store/metamotionr/) and [MMR+](https://mbientlab.com/store/mmrp-metamotionrp/)

## Dependencies
* [MetaWear C++ SDK](https://github.com/mbientlab/MetaWear-SDK-Cpp), version [0.18.4](https://github.com/mbientlab/MetaWear-SDK-Cpp/tree/0.18.4)
* [Warble](https://github.com/mbientlab/Warble) ([version 1.1.5](https://github.com/mbientlab/Warble/tree/1.1.5)) for setting up Bluetooth LE connection
* [Boost](https://www.boost.org/) to help with UUIDs

## Current output from Terminal
```
Connecting to F2:DC:73:7D:AA:08
error 1590070706.778786: Error on line: 296 (src/blestatemachine.cc): Operation now in progress
Connected
Services disconvered
Characteristics discovered
Descriptors found
Connected to F2:DC:73:7D:AA:08
Enabling notification of characteristic: {326a9006-85cb-9195-d9dd-464cfbbae75a}
Succeeded in enabling notifications.
Reading characteristic: {00002a26-0000-1000-8000-00805f9b34fb}: 1.5.0
Reading characteristic: {00002a24-0000-1000-8000-00805f9b34fb}: 5
Reading characteristic: {00002a27-0000-1000-8000-00805f9b34fb}: 0.4
Reading characteristic: {00002a29-0000-1000-8000-00805f9b34fb}: MbientLab Inc
Reading characteristic: {00002a25-0000-1000-8000-00805f9b34fb}: 047D22
Writing characteristic: {326a9001-85cb-9195-d9dd-464cfbbae75a}: 1 128 
Writing characteristic: {326a9001-85cb-9195-d9dd-464cfbbae75a}: 2 128 
Writing characteristic: {326a9001-85cb-9195-d9dd-464cfbbae75a}: 3 128 
Writing characteristic: {326a9001-85cb-9195-d9dd-464cfbbae75a}: 4 128 
Writing characteristic: {326a9001-85cb-9195-d9dd-464cfbbae75a}: 5 128 
Writing characteristic: {326a9001-85cb-9195-d9dd-464cfbbae75a}: 6 128 
Writing characteristic: {326a9001-85cb-9195-d9dd-464cfbbae75a}: 7 128 
Writing characteristic: {326a9001-85cb-9195-d9dd-464cfbbae75a}: 8 128 
Writing characteristic: {326a9001-85cb-9195-d9dd-464cfbbae75a}: 9 128 
Writing characteristic: {326a9001-85cb-9195-d9dd-464cfbbae75a}: 10 128 
Writing characteristic: {326a9001-85cb-9195-d9dd-464cfbbae75a}: 11 128 
Writing characteristic: {326a9001-85cb-9195-d9dd-464cfbbae75a}: 12 128 
Writing characteristic: {326a9001-85cb-9195-d9dd-464cfbbae75a}: 13 128 
Writing characteristic: {326a9001-85cb-9195-d9dd-464cfbbae75a}: 15 128 
Error initialising board: 16
```
