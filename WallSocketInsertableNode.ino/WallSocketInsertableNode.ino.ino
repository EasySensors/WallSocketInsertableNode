// Enable debug prints to serial monitor
//#define MY_DEBUG

#define MY_NODE_ID 100
//0xF0

#define AdafruitNeoPixel 

#ifdef  AdafruitNeoPixel
#include <Adafruit_NeoPixel.h>
// Which pin on the Arduino is connected to the NeoPixels?
#define PIN 6
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS   1
#define NEO_PTYPE  NEO_GRB 1
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#endif

#include <avr/wdt.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif


// Enable and select radio type attached
#define MY_RADIO_RFM69
#define MY_RFM69_FREQUENCY   RF69_433MHZ

#define MY_IS_RFM69HW


#define MY_OTA_FIRMWARE_FEATURE
#define MY_OTA_FLASH_JDECID 0x2020

//#define MY_SIGNING_ATSHA204
//#define  MY_SIGNING_REQUEST_SIGNATURES

#include <MySensors.h>
#include <SimpleTimer.h>
#include <stdlib.h>

//--------------------- https://github.com/JonHub/Filters
#include <Filters.h> 
float testFrequency = 50;                     // test signal frequency (Hz)
float windowLength = 20.0/testFrequency;     // how long to average the signal, for statistist


#define RELAY_pin 7 // Digital pin connected to relay

#define RELAY_sensor 1
#define Current_sensor 2
#define TEMP_sensor 3

#define RELAY_ON 1  // GPIO value to write to turn on attached relay
#define RELAY_OFF 0 // GPIO value to write to turn off attached relay


//#define SPIFLASH_BLOCKERASE_32K   0x52
#define SPIFLASH_BLOCKERASE_32K   0xD8 // We redefine erase block and CHIPERASE commands here. so please keep these two lines AFTER #include <MySensors.h>
#define SPIFLASH_CHIPERASE        0x60

MyMessage msg(RELAY_sensor, V_LIGHT);
MyMessage msg_current(Current_sensor, V_CURRENT);
MyMessage msg_temp(TEMP_sensor, V_TEMP);

RunningStatistics inputStats;                 // create statistics to look at the raw test signal
SimpleTimer timer;

static uint8_t  value_ext_sw = HIGH, last_value_ext_sw = HIGH;
static uint8_t  temp_rfmPrevoiusReadings = 0;
static float ACS712AmpsPrevoiusReadings = 0;
unsigned long wdiDelay2  = 0;

#include <Bounce2.h>
Bounce debouncer = Bounce(); 

void amps()
{
  float ACS712amps;
  char amps_txt[20];
  char temp_txt[10];
  int temp_rfm = (int)_radio.readTemperature(0);

  ACS712amps  = 0.05856*inputStats.sigma() - 0.2126;
  dtostrf(ACS712amps,0,2,amps_txt);   
  
  if (abs( (ACS712amps -  ACS712AmpsPrevoiusReadings)) > 0.1){
    //Serial.print( "\tamps_txt: " ); Serial.println( amps_txt );
    //Serial.print( "\tACS712AmpsPrevoiusReadings: " ); Serial.println( ACS712AmpsPrevoiusReadings ); 
    //Serial.print( "\inputStats.sigma(): " ); Serial.println( inputStats.sigma() ); // sigma variation values of ACS712 value debug print
    ACS712AmpsPrevoiusReadings = ACS712amps;
    send(msg_current.set(amps_txt), true); // Send new state and request ack back
    wait(30);
    }
  if (abs(temp_rfmPrevoiusReadings - temp_rfm)>= 1){
    temp_rfmPrevoiusReadings = temp_rfm;
    //Serial.print( "\tRFM temp: " ); Serial.println( temp_rfm );
    send(msg_temp.set(temp_rfm), true); // Send RFM module temp sensor readings
    }
}

void before() {
    wdt_enable(WDTO_8S);     //wdt_disable();

    //RFM69 reset pin connected to digital pin 9
    pinMode(9, OUTPUT);  
    digitalWrite(9,LOW);
    
    //in case watchdog resets node - we do RFM69 reset here since VDD (power) is not disconnected while watchdog resets the node. Just in case!
    digitalWrite(9,HIGH);
    delay(10);
    digitalWrite(9,LOW);
    delay(10);
    // external button with JST connector
    pinMode(A2, INPUT_PULLUP);
    digitalWrite(A2,HIGH); 
    // Debouncing the button
    debouncer.attach(A2);
    debouncer.interval(20);

    #ifdef  AdafruitNeoPixel
      pixels.begin(); // This initializes the NeoPixel library.
      pixels.setPixelColor(0,pixels.Color(0,0,255)); // R G B 
      pixels.setBrightness(50); //0-255
      pixels.show();
    #endif
    
    // Call void amps() to update current and temperature readings.
    timer.setInterval(6000, amps);
    inputStats.setWindowSecs( windowLength );
    // Then set relay pins in output mode
    pinMode(RELAY_pin, OUTPUT);  
    // Set relay to last known state (using eeprom storage)
    digitalWrite(RELAY_pin, loadState(RELAY_sensor)?RELAY_ON:RELAY_OFF);

    #ifdef  AdafruitNeoPixel
      noInterrupts();
      pixels.setPixelColor(0,loadState(RELAY_sensor)?pixels.Color(255,0,0):pixels.Color(0,255,0));
      pixels.show();
      interrupts(); 
    #endif
    //_radio.readAllRegs();
}

void setup() {
  
}

void presentation() 
{  
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Insertable Relay node","2.0");

  // Register all sensors to gw (they will be created as child devices)
  present(RELAY_sensor, S_LIGHT);
  present(Current_sensor, S_MULTIMETER);
  present(TEMP_sensor, S_TEMP); // RFM 69 Radio have temp sensor. keep this if you like temp reported to a controller
}

unsigned long wdiDelay  = 0;

void loop()
{
  boolean loadedState;
  wdt_reset();
  //Serial.print("RSSI "); Serial.println(_radio.readRSSI());

  #ifdef TACT_SWITCH
        // read and debounce digitalRead(A2);
        debouncer.update();
        value_ext_sw = debouncer.read();
        if (value_ext_sw == 0 && last_value_ext_sw == 1){ 
          loadedState = loadState(RELAY_sensor) == 1? true:false;
            // Change relay state
            digitalWrite(RELAY_pin, !loadedState?RELAY_ON:RELAY_OFF);
            
            #ifdef  AdafruitNeoPixel
              noInterrupts();        
              pixels.setPixelColor(0,!loadedState ?pixels.Color(255,0,0):pixels.Color(0,255,0));
              pixels.show();
              wait(100);
              interrupts();
            #endif
            
            // Store state into eeprom
            saveState(RELAY_sensor, !loadedState?RELAY_ON:RELAY_OFF);
            send(msg.set(!loadedState?true:false), true);
            last_value_ext_sw = value_ext_sw;
          }
          else {
            last_value_ext_sw = value_ext_sw;
          }
          
  #else
        // read and debounce digitalRead(A2);
        debouncer.update();
        value_ext_sw = debouncer.read();
        if (last_value_ext_sw != value_ext_sw){
          last_value_ext_sw = value_ext_sw;
          //Serial.println("VALUE"); Serial.println(value_ext_sw);
          // Change relay state
          digitalWrite(RELAY_pin, value_ext_sw?RELAY_ON:RELAY_OFF);

          #ifdef  AdafruitNeoPixel
            noInterrupts();        
            pixels.setPixelColor(0,!loadedState ?pixels.Color(255,0,0):pixels.Color(0,255,0));
            pixels.show();
            wait(100);
            interrupts();
          #endif
          
          // Store state into eeprom
          saveState(RELAY_sensor, value_ext_sw?RELAY_ON:RELAY_OFF);
          send(msg.set(value_ext_sw ?true:false), true);
          }
  #endif
  

  inputStats.input(analogRead(A1));  // log to Stats function for ampers from A1 analog input
  timer.run();      
}

void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type==V_LIGHT) {
     // Change relay state
     digitalWrite(RELAY_pin, message.getBool()?RELAY_ON:RELAY_OFF);
     // Store state in eeprom
     saveState(message.sensor, message.getBool()?RELAY_ON:RELAY_OFF);
     #ifdef  AdafruitNeoPixel
      wait(100);
      // Blink Blues light to see radio communication with the gateway is 
      pixels.setPixelColor(0,pixels.Color(0,0,255));
      pixels.show();
      wait(100);
      // Set Red for ON and green fo OFF
      pixels.setPixelColor(0,message.getBool() ?pixels.Color(255,0,0):pixels.Color(0,255,0));
      pixels.show();
     #endif
     Serial.print("Incoming change for sensor:");
     Serial.print(message.sensor);
     Serial.print(", New status: ");
     Serial.println(message.getBool());
   }
}