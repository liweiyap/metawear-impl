## Board Information
* Code tested and produces same results with [MMR](https://mbientlab.com/store/metamotionr/) and [MMR+](https://mbientlab.com/store/mmrp-metamotionrp/). Both devices have the same:
  * Hardware revision: `0.4`
  * Firmware revision: `1.5.0`
  * Model number: `5`

## Host Device Information
* Ubuntu 16.04
* [Linux Bluetooth library, version 5.54](http://www.bluez.org/)

## Description of Problem
1. Expected to print the message `Board initialised` from `initBoard(board)` with a status of `0` (`MBL_MW_STATUS_OK`).
2. Instead, observed the message `Error initialising board` from `initBoard(board)` with a status of `16` (`MBL_MW_STATUS_ERROR_TIMEOUT`).
3. The steps leading up to this behaviour in `main.cpp`:
  * A `WarbleGatt` object is first created using the device MAC address and the host HCI address.
  * Connection to the device MAC address via `warble_gatt_connect_async` is successful.
  * An `MblMwBtleConnection` object is created using our own implementations of the GATT operations.
  * An `MblMwMetaWearBoard` object is instantiated using `mbl_mw_metawearboard_create`.
  * Finally, `initBoard(board)` calls `mbl_mw_metawearboard_initialize`, which initialises the internal state of the `MblMwMetaWearBoard` object. This is where the observed message is printed.

## SDK
* [MetaWear C++ SDK](https://github.com/mbientlab/MetaWear-SDK-Cpp), version [0.18.4](https://github.com/mbientlab/MetaWear-SDK-Cpp/tree/0.18.4)
* Ubuntu 16.04
* Self-contained piece of MetaWear SDK calls that replicates this issue is found in [`main.cpp`](https://github.com/liweiyap/metawear-impl/blob/master/main.cpp). The entire code can be copied/pasted and run, assuming that the following dependencies are also installed:
  * [Warble](https://github.com/mbientlab/Warble) ([version 1.1.5](https://github.com/mbientlab/Warble/tree/1.1.5)) for setting up Bluetooth LE connection
  * [Boost](https://www.boost.org/) to help with UUIDs
* All the output from `stdout`/`stderr`:
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
