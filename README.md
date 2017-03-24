

![Mini Relay Box](https://github.com/EasySensors/WallSocketInsertableNode/blob/master/pics/socket_25_03_top.png?raw=true)

# The Wall Socket Insertable Node  is extremly low profile wireless 220 Volts 10A Relay board. It is suitable for installing into wall socket compartment in between the socket and the wall.  Arduino IDE compatible (the Atmel ATMega328P) microcontroller with RFM 69 HW radio on board.  Onboard 220 Volts power supply. Secure athentication with ATSHA204A crypto-authentication. Best suitable for Home Automation, IOT. Current sensor can report power consumption. One Pixel LED installed and one regullar LED can be connected. External button connector. Check Specs below. It can replace this setup:

![](https://github.com/EasySensors/MiniRelayBox/blob/master/pics/replace.jpg?raw=true)

## Specification: ##
 - Authentication security - Atmel ATSHA204A Crypto Authentication Chip
 - External JDEC EPROM
 - RFM69-HW (high power version) 433 MHz Radio transceiver
 - ACS-712 current sensor. Range from 0,1 A up to 10 amperes.
 - External JDEC EPROM
 - Dualoptiboot bootloader. Implements over the air (OTA) firmware update ability
 - Pixel LED SK6812mini
 - Regullar LED can be directly connected
 - External button connector with JST 1.25 мм connector
 - Supply voltage 160-240 Volts AC
 - The Digital and Analog pins are 3.3 volts
 - Uncompromised Protection:<br>
	Overload varistor<br>
	Slow fuse<br>
	Thermal fuse<br>
 - Board Dimensions _36x38x18mm_
 - FTDI  header for programming


**Pin out:** 


Arduino Pins|	Description
------------|--------------
A7, A8 |	Available ARDUINO analog GPIO / DIGITAL GPIO 
D5, D3 |	Available ARDUINO digital GPIO 
Check picture to see both Analog and Digital pins | ![](https://github.com/EasySensors/WallSocketInsertableNode/blob/master/pics/Pins.jpg?raw=true)
D5 with 200 ohm resistor <br>connector and A2 as Relay button | In case you do not like soldered SK6812mini <br> you may use here regular LED for Relay visulal feedback <br> ![](https://github.com/EasySensors/WallSocketInsertableNode/blob/master/pics/LEDregular.jpg?raw=true)
A1 |	connected to ACS712 current sensor
A2 |	connected to JST  1.25 мм connector. Used as external relay switch in the sketch
A3 |	connected to  ATSHA204A
D6 |	connected to Pixel LED SK6812mini
D7 |	connected to the 10A Relay
D8 |	Connected to CS FLASH chip (OTA) M25P40
D9 |	connected to RFM69 reset pin
ANT |	RFM69 antenna


**Arduino IDE Settings**

![Arduino IDE Settings](https://github.com/EasySensors/ButtonSizeNode/blob/master/pics/IDEsettings.jpg?raw=true)



How to use it as home automation (IOT) node Relay
------------------------------------------------------


miniRelayBox.ino is the Arduino example sketch using [MySensors](https://www.mysensors.org/) API. 

Burn the miniRelayBox.ino sketch into it and it will became  one of the MySensors home automation network Relay Node. The Relay could be controlable from a smarthome controller web interface or smarphone App. 
To create Home Automation Network you need smarthome controller and at least two Nodes one as a Sensor, relay or actuator Node and the other one as “Gateway Serial” connected to the smarthome controller. I personally love [Domoticz](https://domoticz.com/) as smarthome conroller. Please check this [HowTo](https://github.com/EasySensors/ButtonSizeNode/blob/master/DomoticzInstallMySensors.md) to install Domoticz.

However, for no-controller setup, as example, you can use 3 nodes - first node as “Gateway Serial”, second node as the Mini Relay Box node and the last one as switch for the relay node. No controller needed then, keep the switch and the relay node on the same address and the switch will operate the relay node.

Things worth mentioning about the  [MySensors](https://www.mysensors.org/) Arduino sketch: 


Arduino Pins|	Description
------------|--------------
#define MY_RADIO_RFM69<br>#define MY_RFM69_FREQUENCY   RF69_433MHZ<br>#define MY_IS_RFM69HW|	Define which radio we use – here is RFM 69<br>with frequency 433 MHZ and it is HW<br>type – one of the most powerful RFM 69 radios.<br>If your radio is RFM69CW - comment out line<br>with // #define MY_IS_RFM69HW 
#define MY_NODE_ID 0xE0 | Define Node address (0xE0 here). I prefer to use static addresses<br> and in Hexadecimal since it is easier to identify the node<br> address in  [Domoticz](https://domoticz.com/) devices list after it<br> will be discovered by controller ( [Domoticz](https://domoticz.com/)).<br> However, you can use AUTO instead of the hardcoded number<br> (like 0xE0) though.  [Domoticz](https://domoticz.com/) will automatically assign node ID then.
#define MY_OTA_FIRMWARE_FEATURE<br>#define MY_OTA_FLASH_JDECID 0x2020 | Define OTA feature. OTA stands for “Over The Air firmware updates”.<br> If your node does not utilize Sleep mode you can send new “firmware”<br> (compiled sketch binary) by air. **Here is the link on how to do it.** <br>For OTA we use JDEC Flash chip where the node stores<br> new firmware and once it received and controlsum (CRC) is correct<br>  it reboots and flashes your new code into the node<br> controller. So we define it is "erase type" as 0x2020 here. 
#define MY_SIGNING_ATSHA204 | Define if you like to use Crypto Authentication to secure your nodes<br> from intruders or interference. After that, you have to “personalize”<br> all the nodes, which have those, defines enabled.<br> [**How to “personalize” nodes with encryption key**](https://github.com/EasySensors/ButtonSizeNode/blob/master/SecurityPersonalizationHowTo.md)
#define AdafruitNeoPixel | Enables onboard Pixel LED SK6812mini

Connect the Relay to FTDI USB adaptor, Select Pro Mini 8MHz board and burn the wallSocketInsertableNode.ino sketch.

**Done**

>For schematics lovers:

![enter image description here](https://github.com/EasySensors/MiniRelayBox/blob/master/pics/schematic.jpg?raw=true)

The board is created by  [Koresh](https://www.openhardware.io/user/143/projects/Koresh)
