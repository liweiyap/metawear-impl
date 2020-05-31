## Board Information
* Code tested and produces same results with [MMR](https://mbientlab.com/store/metamotionr/) and [MMR+](https://mbientlab.com/store/mmrp-metamotionrp/). Both devices have the same:
  * Hardware revision: `0.4`
  * Firmware revision: `1.5.0`
  * Model number: `5`

## Host Device Information
* Ubuntu 16.04
* [Linux Bluetooth library, version 5.54](http://www.bluez.org/)

## Description of Problem
1. Expected to print the message `Board initialised` from `initBoard()` with a status of `0` (`MBL_MW_STATUS_OK`).
2. Instead, observed the message `Error initialising board` from `initBoard()` with a status of `16` (`MBL_MW_STATUS_ERROR_TIMEOUT`).
3. The steps leading up to this behaviour in [`main.cpp`](https://github.com/liweiyap/metawear-impl/blob/master/main.cpp):
    1. A `WarbleGatt` object is first created using the device MAC address and the host HCI address.
    2. Connection to the device MAC address via `warble_gatt_connect_async` is successful.
    3. An `MblMwBtleConnection` object is created using our own implementations of the GATT operations.
    4. An `MblMwMetaWearBoard` object is instantiated using `mbl_mw_metawearboard_create`.
    5. Finally, `initBoard()` calls `mbl_mw_metawearboard_initialize`, which initialises the internal state of the `MblMwMetaWearBoard` object. This is where the observed message is printed.
4. As a result, `mbl_mw_sensor_fusion_set_mode(board, MBL_MW_SENSOR_FUSION_MODE_IMU_PLUS)` throws a `std::out_of_range` exception.

## SDK
* [MetaWear C++ SDK](https://github.com/mbientlab/MetaWear-SDK-Cpp), version [0.18.4](https://github.com/mbientlab/MetaWear-SDK-Cpp/tree/0.18.4)
* Ubuntu 16.04
* Self-contained piece of MetaWear SDK calls that replicates this issue is found in [`main.cpp`](https://github.com/liweiyap/metawear-impl/blob/master/main.cpp). The entire code can be copied/pasted and run, assuming that the following dependencies are also installed:
  * [Warble](https://github.com/mbientlab/Warble) ([version 1.1.5](https://github.com/mbientlab/Warble/tree/1.1.5)) for setting up Bluetooth LE connection
  * [Boost](https://www.boost.org/) to help with UUIDs
* All the output from `stdout`/`stderr` (with the message `Error initialising board` and the status of `16` for `MBL_MW_STATUS_ERROR_TIMEOUT` near the bottom):
<pre>
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
<b>Error initialising board: 16</b>
terminate called after throwing an instance of 'std::out_of_range'
  what():  _Map_base::at
Aborted (core dumped)
</pre>
* Output from `btmon` before and up till <code><b>Error initialising board: 16</b></code>:
```
Bluetooth monitor ver 5.54
= Note: Linux version 4.15.0-101-generic (x86_64)                      0.610727
= Note: Bluetooth subsystem version 2.22                               0.610729
= New Index: 5C:F3:70:99:2A:28 (Primary,USB,hci0)               [hci0] 0.610729
= Open Index: 5C:F3:70:99:2A:28                                 [hci0] 0.610729
= Index Info: 5C:F3:70:99:2A:28 (Broadcom Corporation)          [hci0] 0.610730
@ MGMT Open: bluetoothd (privileged) version 1.14             {0x0001} 0.610730
@ MGMT Open: btmon (privileged) version 1.14                  {0x0002} 0.610741
@ RAW Open: main                                              {0x0003} 8.572763
@ RAW Close: main                                             {0x0003} 8.572774
@ RAW Open: main                                              {0x0003} 8.572782
@ RAW Close: main                                             {0x0003} 8.572783
< HCI Command: LE Set Scan Parameters (0x08|0x000b) plen 7   #1 [hci0] 8.572834
        Type: Passive (0x00)
        Interval: 60.000 msec (0x0060)
        Window: 30.000 msec (0x0030)
        Own address type: Public (0x00)
        Filter policy: Ignore not in white list (0x01)
> HCI Event: Command Complete (0x0e) plen 4                  #2 [hci0] 8.573833
      LE Set Scan Parameters (0x08|0x000b) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2       #3 [hci0] 8.573846
        Scanning: Enabled (0x01)
        Filter duplicates: Enabled (0x01)
> HCI Event: Command Complete (0x0e) plen 4                  #4 [hci0] 8.574774
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
> HCI Event: LE Meta Event (0x3e) plen 43                    #5 [hci0] 9.029811
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Random (0x01)
        Address: F2:DC:73:7D:AA:08 (Static)
        Data length: 31
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (partial): 1 entry
          Vendor specific (326a9000-85cb-9195-d9dd-464cfbbae75a)
        Name (complete): MetaWear
        RSSI: -78 dBm (0xb2)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2       #6 [hci0] 9.029896
        Scanning: Disabled (0x00)
        Filter duplicates: Disabled (0x00)
> HCI Event: Command Complete (0x0e) plen 4                  #7 [hci0] 9.032811
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Create Connection (0x08|0x000d) plen 25    #8 [hci0] 9.032857
        Scan interval: 60.000 msec (0x0060)
        Scan window: 60.000 msec (0x0060)
        Filter policy: White list is not used (0x00)
        Peer address type: Random (0x01)
        Peer address: F2:DC:73:7D:AA:08 (Static)
        Own address type: Public (0x00)
        Min connection interval: 30.00 msec (0x0018)
        Max connection interval: 50.00 msec (0x0028)
        Connection latency: 0 (0x0000)
        Supervision timeout: 420 msec (0x002a)
        Min connection length: 0.000 msec (0x0000)
        Max connection length: 0.000 msec (0x0000)
> HCI Event: Command Status (0x0f) plen 4                    #9 [hci0] 9.033807
      LE Create Connection (0x08|0x000d) ncmd 1
        Status: Success (0x00)
> HCI Event: LE Meta Event (0x3e) plen 19                   #10 [hci0] 9.870797
      LE Connection Complete (0x01)
        Status: Success (0x00)
        Handle: 64
        Role: Master (0x00)
        Peer address type: Random (0x01)
        Peer address: F2:DC:73:7D:AA:08 (Static)
        Connection interval: 48.75 msec (0x0027)
        Connection latency: 0 (0x0000)
        Supervision timeout: 420 msec (0x002a)
        Master clock accuracy: 0x05
@ MGMT Event: Device Connected (0x000b) plen 44        {0x0002} [hci0] 9.870873
        LE Address: F2:DC:73:7D:AA:08 (Static)
        Flags: 0x00000000
        Data length: 31
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (partial): 1 entry
          Vendor specific (326a9000-85cb-9195-d9dd-464cfbbae75a)
        Name (complete): MetaWear
@ MGMT Event: Device Connected (0x000b) plen 44        {0x0001} [hci0] 9.870873
        LE Address: F2:DC:73:7D:AA:08 (Static)
        Flags: 0x00000000
        Data length: 31
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (partial): 1 entry
          Vendor specific (326a9000-85cb-9195-d9dd-464cfbbae75a)
        Name (complete): MetaWear
< HCI Command: LE Read Remote Used... (0x08|0x0016) plen 2  #11 [hci0] 9.871009
        Handle: 64
> HCI Event: Command Status (0x0f) plen 4                   #12 [hci0] 9.871800
      LE Read Remote Used Features (0x08|0x0016) ncmd 1
        Status: Success (0x00)
@ RAW Open: hciconfig (privileged) version 2.22               {0x0003} 9.876280
@ RAW Close: hciconfig                                        {0x0003} 9.876306
> HCI Event: LE Meta Event (0x3e) plen 12                  #13 [hci0] 10.077841
      LE Read Remote Used Features (0x04)
        Status: Success (0x00)
        Handle: 64
        Features: 0x01 0x00 0x00 0x00 0x00 0x00 0x00 0x00
          LE Encryption
< ACL Data TX: Handle 64 flags 0x00 dlen 11                #14 [hci0] 10.077961
      ATT: Read By Group Type Request (0x10) len 6
        Handle range: 0x0001-0xffff
        Attribute group type: Primary Service (0x2800)
> ACL Data RX: Handle 64 flags 0x02 dlen 24                #15 [hci0] 10.175877
      ATT: Read By Group Type Response (0x11) len 19
        Attribute data length: 6
        Attribute group list: 3 entries
        Handle range: 0x0001-0x0009
        UUID: Generic Access Profile (0x1800)
        Handle range: 0x000a-0x000d
        UUID: Generic Attribute Profile (0x1801)
        Handle range: 0x000e-0x0011
        UUID: Battery Service (0x180f)
< ACL Data TX: Handle 64 flags 0x00 dlen 11                #16 [hci0] 10.176084
      ATT: Read By Group Type Request (0x10) len 6
        Handle range: 0x0012-0xffff
        Attribute group type: Primary Service (0x2800)
> HCI Event: Number of Completed Packets (0x13) plen 5     #17 [hci0] 10.321794
        Num handles: 1
        Handle: 64
        Count: 2
> ACL Data RX: Handle 64 flags 0x02 dlen 12                #18 [hci0] 10.322006
      ATT: Read By Group Type Response (0x11) len 7
        Attribute data length: 6
        Attribute group list: 1 entry
        Handle range: 0x0012-0x001c
        UUID: Device Information (0x180a)
< ACL Data TX: Handle 64 flags 0x00 dlen 11                #19 [hci0] 10.322159
      ATT: Read By Group Type Request (0x10) len 6
        Handle range: 0x001d-0xffff
        Attribute group type: Primary Service (0x2800)
> ACL Data RX: Handle 64 flags 0x02 dlen 26                #20 [hci0] 10.467814
      ATT: Read By Group Type Response (0x11) len 21
        Attribute data length: 20
        Attribute group list: 1 entry
        Handle range: 0x001d-0xffff
        UUID: Vendor specific (326a9000-85cb-9195-d9dd-464cfbbae75a)
< ACL Data TX: Handle 64 flags 0x00 dlen 11                #21 [hci0] 10.468031
      ATT: Read By Type Request (0x08) len 6
        Handle range: 0x0001-0xffff
        Attribute type: Characteristic (0x2803)
> HCI Event: Number of Completed Packets (0x13) plen 5     #22 [hci0] 10.565792
        Num handles: 1
        Handle: 64
        Count: 2
> ACL Data RX: Handle 64 flags 0x02 dlen 27                #23 [hci0] 10.565843
      ATT: Read By Type Response (0x09) len 22
        Attribute data length: 7
        Attribute data list: 3 entries
        Handle: 0x0002
        Value: 0a0300002a
        Handle: 0x0004
        Value: 020500012a
        Handle: 0x0006
        Value: 020700042a
< ACL Data TX: Handle 64 flags 0x00 dlen 11                #24 [hci0] 10.566006
      ATT: Read By Type Request (0x08) len 6
        Handle range: 0x0007-0xffff
        Attribute type: Characteristic (0x2803)
> ACL Data RX: Handle 64 flags 0x02 dlen 27                #25 [hci0] 10.663372
      ATT: Read By Type Response (0x09) len 22
        Attribute data length: 7
        Attribute data list: 3 entries
        Handle: 0x0008
        Value: 020900a62a
        Handle: 0x000b
        Value: 200c00052a
        Handle: 0x000f
        Value: 121000192a
< ACL Data TX: Handle 64 flags 0x00 dlen 11                #26 [hci0] 10.663579
      ATT: Read By Type Request (0x08) len 6
        Handle range: 0x0010-0xffff
        Attribute type: Characteristic (0x2803)
> HCI Event: Number of Completed Packets (0x13) plen 5     #27 [hci0] 10.711777
        Num handles: 1
        Handle: 64
        Count: 2
> ACL Data RX: Handle 64 flags 0x02 dlen 27                #28 [hci0] 10.809071
      ATT: Read By Type Response (0x09) len 22
        Attribute data length: 7
        Attribute data list: 3 entries
        Handle: 0x0013
        Value: 021400292a
        Handle: 0x0015
        Value: 021600242a
        Handle: 0x0017
        Value: 021800252a
< ACL Data TX: Handle 64 flags 0x00 dlen 11                #29 [hci0] 10.809230
      ATT: Read By Type Request (0x08) len 6
        Handle range: 0x0018-0xffff
        Attribute type: Characteristic (0x2803)
> ACL Data RX: Handle 64 flags 0x02 dlen 20                #30 [hci0] 10.955284
      ATT: Read By Type Response (0x09) len 15
        Attribute data length: 7
        Attribute data list: 2 entries
        Handle: 0x0019
        Value: 021a00272a
        Handle: 0x001b
        Value: 021c00262a
< ACL Data TX: Handle 64 flags 0x00 dlen 11                #31 [hci0] 10.955518
      ATT: Read By Type Request (0x08) len 6
        Handle range: 0x001c-0xffff
        Attribute type: Characteristic (0x2803)
> HCI Event: Number of Completed Packets (0x13) plen 5     #32 [hci0] 11.052757
        Num handles: 1
        Handle: 64
        Count: 2
> ACL Data RX: Handle 64 flags 0x02 dlen 27                #33 [hci0] 11.101548
      ATT: Read By Type Response (0x09) len 22
        Attribute data length: 21
        Attribute data list: 1 entry
        Handle: 0x001e
        Value: 0e1f005ae7bafb4c46ddd99591cb8501906a32
< ACL Data TX: Handle 64 flags 0x00 dlen 11                #34 [hci0] 11.101731
      ATT: Read By Type Request (0x08) len 6
        Handle range: 0x001f-0xffff
        Attribute type: Characteristic (0x2803)
> ACL Data RX: Handle 64 flags 0x02 dlen 27                #35 [hci0] 11.199611
      ATT: Read By Type Response (0x09) len 22
        Attribute data length: 21
        Attribute data list: 1 entry
        Handle: 0x0020
        Value: 1221005ae7bafb4c46ddd99591cb8506906a32
< ACL Data TX: Handle 64 flags 0x00 dlen 11                #36 [hci0] 11.199796
      ATT: Read By Type Request (0x08) len 6
        Handle range: 0x0021-0xffff
        Attribute type: Characteristic (0x2803)
> HCI Event: Number of Completed Packets (0x13) plen 5     #37 [hci0] 11.247781
        Num handles: 1
        Handle: 64
        Count: 2
> ACL Data RX: Handle 64 flags 0x02 dlen 9                 #38 [hci0] 11.345194
      ATT: Error Response (0x01) len 4
        Read By Type Request (0x08)
        Handle: 0x0021
        Error: Attribute Not Found (0x0a)
< ACL Data TX: Handle 64 flags 0x00 dlen 11                #39 [hci0] 11.345410
      ATT: Read By Type Request (0x08) len 6
        Handle range: 0x0001-0xffff
        Attribute type: Client Characteristic Configuration (0x2902)
> ACL Data RX: Handle 64 flags 0x02 dlen 18                #40 [hci0] 11.443323
      ATT: Read By Type Response (0x09) len 13
        Attribute data length: 4
        Attribute data list: 3 entries
        Handle: 0x000d
        Value: 0000
        Handle: 0x0011
        Value: 0000
        Handle: 0x0022
        Value: 0000
< ACL Data TX: Handle 64 flags 0x00 dlen 11                #41 [hci0] 11.443508
      ATT: Read By Type Request (0x08) len 6
        Handle range: 0x0023-0xffff
        Attribute type: Client Characteristic Configuration (0x2902)
> HCI Event: Number of Completed Packets (0x13) plen 5     #42 [hci0] 11.491779
        Num handles: 1
        Handle: 64
        Count: 2
> ACL Data RX: Handle 64 flags 0x02 dlen 9                 #43 [hci0] 11.588979
      ATT: Error Response (0x01) len 4
        Read By Type Request (0x08)
        Handle: 0x0023
        Error: Attribute Not Found (0x0a)
< ACL Data TX: Handle 64 flags 0x00 dlen 9                 #44 [hci0] 11.589224
      ATT: Write Request (0x12) len 4
        Handle: 0x0022
          Data: 0100
> ACL Data RX: Handle 64 flags 0x02 dlen 5                 #45 [hci0] 11.735682
      ATT: Write Response (0x13) len 0
< ACL Data TX: Handle 64 flags 0x00 dlen 7                 #46 [hci0] 11.736139
      ATT: Read Request (0x0a) len 2
        Handle: 0x001c
> HCI Event: Number of Completed Packets (0x13) plen 5     #47 [hci0] 11.832773
        Num handles: 1
        Handle: 64
        Count: 2
> ACL Data RX: Handle 64 flags 0x02 dlen 10                #48 [hci0] 11.881487
      ATT: Read Response (0x0b) len 5
        Value: 312e352e30
< ACL Data TX: Handle 64 flags 0x00 dlen 7                 #49 [hci0] 11.881680
      ATT: Read Request (0x0a) len 2
        Handle: 0x0016
> ACL Data RX: Handle 64 flags 0x02 dlen 6                 #50 [hci0] 11.979432
      ATT: Read Response (0x0b) len 1
        Value: 35
< ACL Data TX: Handle 64 flags 0x00 dlen 7                 #51 [hci0] 11.979771
      ATT: Read Request (0x0a) len 2
        Handle: 0x001a
> HCI Event: Number of Completed Packets (0x13) plen 5     #52 [hci0] 12.125770
        Num handles: 1
        Handle: 64
        Count: 2
> ACL Data RX: Handle 64 flags 0x02 dlen 8                 #53 [hci0] 12.125788
      ATT: Read Response (0x0b) len 3
        Value: 302e34
< ACL Data TX: Handle 64 flags 0x00 dlen 7                 #54 [hci0] 12.126067
      ATT: Read Request (0x0a) len 2
        Handle: 0x0014
> ACL Data RX: Handle 64 flags 0x02 dlen 18                #55 [hci0] 12.223311
      ATT: Read Response (0x0b) len 13
        Value: 4d6269656e744c616220496e63
< ACL Data TX: Handle 64 flags 0x00 dlen 7                 #56 [hci0] 12.223508
      ATT: Read Request (0x0a) len 2
        Handle: 0x0018
> HCI Event: Number of Completed Packets (0x13) plen 5     #57 [hci0] 12.271775
        Num handles: 1
        Handle: 64
        Count: 2
> ACL Data RX: Handle 64 flags 0x02 dlen 11                #58 [hci0] 12.466460
      ATT: Read Response (0x0b) len 6
        Value: 303437443232
< ACL Data TX: Handle 64 flags 0x00 dlen 9                 #59 [hci0] 12.466786
      ATT: Write Command (0x52) len 4
        Handle: 0x001f
          Data: 0180
> ACL Data RX: Handle 64 flags 0x02 dlen 11                #60 [hci0] 12.612772
      ATT: Handle Value Notification (0x1b) len 6
        Handle: 0x0021
          Data: 01800000
< ACL Data TX: Handle 64 flags 0x00 dlen 9                 #61 [hci0] 12.612989
      ATT: Write Command (0x52) len 4
        Handle: 0x001f
          Data: 0280
> HCI Event: Number of Completed Packets (0x13) plen 5     #62 [hci0] 12.661763
        Num handles: 1
        Handle: 64
        Count: 2
> ACL Data RX: Handle 64 flags 0x02 dlen 13                #63 [hci0] 12.856482
      ATT: Handle Value Notification (0x1b) len 8
        Handle: 0x0021
          Data: 028000010300
< ACL Data TX: Handle 64 flags 0x00 dlen 9                 #64 [hci0] 12.856727
      ATT: Write Command (0x52) len 4
        Handle: 0x001f
          Data: 0380
> ACL Data RX: Handle 64 flags 0x02 dlen 11                #65 [hci0] 13.002770
      ATT: Handle Value Notification (0x1b) len 6
        Handle: 0x0021
          Data: 03800102
< ACL Data TX: Handle 64 flags 0x00 dlen 9                 #66 [hci0] 13.003045
      ATT: Write Command (0x52) len 4
        Handle: 0x001f
          Data: 0480
> HCI Event: Number of Completed Packets (0x13) plen 5     #67 [hci0] 13.051771
        Num handles: 1
        Handle: 64
        Count: 2
> ACL Data RX: Handle 64 flags 0x02 dlen 15                #68 [hci0] 13.149016
      ATT: Handle Value Notification (0x1b) len 10
        Handle: 0x0021
          Data: 0480010000030102
< ACL Data TX: Handle 64 flags 0x00 dlen 9                 #69 [hci0] 13.149240
      ATT: Write Command (0x52) len 4
        Handle: 0x001f
          Data: 0580
> ACL Data RX: Handle 64 flags 0x02 dlen 18                #70 [hci0] 13.295263
      ATT: Handle Value Notification (0x1b) len 13
        Handle: 0x0021
          Data: 0580000203030303010101
< ACL Data TX: Handle 64 flags 0x00 dlen 9                 #71 [hci0] 13.295483
      ATT: Write Command (0x52) len 4
        Handle: 0x001f
          Data: 0680
> HCI Event: Number of Completed Packets (0x13) plen 5     #72 [hci0] 13.392733
        Num handles: 1
        Handle: 64
        Count: 2
> ACL Data RX: Handle 64 flags 0x02 dlen 11                #73 [hci0] 13.538975
      ATT: Handle Value Notification (0x1b) len 6
        Handle: 0x0021
          Data: 06800000
< ACL Data TX: Handle 64 flags 0x00 dlen 9                 #74 [hci0] 13.539224
      ATT: Write Command (0x52) len 4
        Handle: 0x001f
          Data: 0780
> ACL Data RX: Handle 64 flags 0x02 dlen 11                #75 [hci0] 13.637008
      ATT: Handle Value Notification (0x1b) len 6
        Handle: 0x0021
          Data: 07800000
< ACL Data TX: Handle 64 flags 0x00 dlen 9                 #76 [hci0] 13.637235
      ATT: Write Command (0x52) len 4
        Handle: 0x001f
          Data: 0880
> HCI Event: Number of Completed Packets (0x13) plen 5     #77 [hci0] 13.685747
        Num handles: 1
        Handle: 64
        Count: 2
> ACL Data RX: Handle 64 flags 0x02 dlen 11                #78 [hci0] 13.782756
      ATT: Handle Value Notification (0x1b) len 6
        Handle: 0x0021
          Data: 08800000
< ACL Data TX: Handle 64 flags 0x00 dlen 9                 #79 [hci0] 13.782978
      ATT: Write Command (0x52) len 4
        Handle: 0x001f
          Data: 0980
> ACL Data RX: Handle 64 flags 0x02 dlen 12                #80 [hci0] 13.929529
      ATT: Handle Value Notification (0x1b) len 7
        Handle: 0x0021
          Data: 098000031c
< ACL Data TX: Handle 64 flags 0x00 dlen 9                 #81 [hci0] 13.929754
      ATT: Write Command (0x52) len 4
        Handle: 0x001f
          Data: 0a80
> HCI Event: Number of Completed Packets (0x13) plen 5     #82 [hci0] 14.026742
        Num handles: 1
        Handle: 64
        Count: 2
> ACL Data RX: Handle 64 flags 0x02 dlen 12                #83 [hci0] 14.027035
      ATT: Handle Value Notification (0x1b) len 7
        Handle: 0x0021
          Data: 0a8000001c
< ACL Data TX: Handle 64 flags 0x00 dlen 9                 #84 [hci0] 14.027204
      ATT: Write Command (0x52) len 4
        Handle: 0x001f
          Data: 0b80
> ACL Data RX: Handle 64 flags 0x02 dlen 16                #85 [hci0] 14.172784
      ATT: Handle Value Notification (0x1b) len 11
        Handle: 0x0021
          Data: 0b8000020800001000
< ACL Data TX: Handle 64 flags 0x00 dlen 9                 #86 [hci0] 14.173008
      ATT: Write Command (0x52) len 4
        Handle: 0x001f
          Data: 0c80
> HCI Event: Number of Completed Packets (0x13) plen 5     #87 [hci0] 14.221740
        Num handles: 1
        Handle: 64
        Count: 2
> ACL Data RX: Handle 64 flags 0x02 dlen 12                #88 [hci0] 14.270248
      ATT: Handle Value Notification (0x1b) len 7
        Handle: 0x0021
          Data: 0c80000008
< ACL Data TX: Handle 64 flags 0x00 dlen 9                 #89 [hci0] 14.270475
      ATT: Write Command (0x52) len 4
        Handle: 0x001f
          Data: 0d80
> ACL Data RX: Handle 64 flags 0x02 dlen 11                #90 [hci0] 14.416492
      ATT: Handle Value Notification (0x1b) len 6
        Handle: 0x0021
          Data: 0d800001
< ACL Data TX: Handle 64 flags 0x00 dlen 9                 #91 [hci0] 14.416730
      ATT: Write Request (0x12) len 4
        Handle: 0x001f
          Data: 0f80
> HCI Event: Number of Completed Packets (0x13) plen 5     #92 [hci0] 14.465744
        Num handles: 1
        Handle: 64
        Count: 2
> ACL Data RX: Handle 64 flags 0x02 dlen 5                 #93 [hci0] 14.562579
      ATT: Write Response (0x13) len 0
> ACL Data RX: Handle 64 flags 0x02 dlen 13                #94 [hci0] 14.563240
      ATT: Handle Value Notification (0x1b) len 8
        Handle: 0x0021
          Data: 0f8000020800
```
* Output from `btmon` at `terminate called after throwing an instance of 'std::out_of_range'`:
```
< HCI Command: Disconnect (0x01|0x0006) plen 3             #95 [hci0] 44.863170
        Handle: 64
        Reason: Remote User Terminated Connection (0x13)
> HCI Event: Command Status (0x0f) plen 4                  #96 [hci0] 44.864296
      Disconnect (0x01|0x0006) ncmd 1
        Status: Success (0x00)
> HCI Event: Disconnect Complete (0x05) plen 4             #97 [hci0] 44.984298
        Status: Success (0x00)
        Handle: 64
        Reason: Connection Terminated By Local Host (0x16)
@ MGMT Event: Device Disconnected (0x000c) plen 8     {0x0002} [hci0] 44.984312
        LE Address: F2:DC:73:7D:AA:08 (Static)
        Reason: Connection terminated by local host (0x02)
@ MGMT Event: Device Disconnected (0x000c) plen 8     {0x0001} [hci0] 44.984312
        LE Address: F2:DC:73:7D:AA:08 (Static)
        Reason: Connection terminated by local host (0x02)
```
