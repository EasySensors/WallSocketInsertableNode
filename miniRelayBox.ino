// Enable debug prints to serial monitor
#define MY_DEBUG

//#include <MemoryFree.h>

//#define AdafruitNeoPixel 


#ifdef  AdafruitNeoPixel
#include <Adafruit_NeoPixel.h>
// Which pin on the Arduino is connected to the NeoPixels?
#define PIN 6
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS   1
#define NEO_PTYPE  NEO_GRB 
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#endif

#include <avr/wdt.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif


// Enable and select radio type attached
//#define MY_RADIO_NRF24
#define MY_RADIO_RFM69
#define MY_RFM69_FREQUENCY   RF69_433MHZ
//#define MY_RFM69_FREQUENCY   RF69_868MHZ
//#define MY_RFM69_FREQUENCY   RF69_315MHZ
//#define MY_RFM69_FREQUENCY   RF69_915MHZ 

#define MY_IS_RFM69HW

#define MY_NODE_ID 100
//0xC9
//0xF3

#define MY_OTA_FIRMWARE_FEATURE
#define MY_OTA_FLASH_JDECID 0x2020
//#define11 MY_OTA_FLASH_JDECID 0x1C30

#define MY_SIGNING_ATSHA204
#define  MY_SIGNING_REQUEST_SIGNATURES

// Enable repeater functionality for this node
// #define MY_REPEATER_FEATURE

#include <MySensors.h>
#include <SimpleTimer.h>
#include <stdlib.h>

//--------------------- https://github.com/JonHub/Filters
#include <Filters.h> 

#define RELAY_pin 7 // Digital pin connected to relay

#define RELAY_sensor 1
#define Current_sensor 2
#define TEMP_sensor 3

#define RELAY_ON 1  // GPIO value to write to turn on attached relay
#define RELAY_OFF 0 // GPIO value to write to turn off attached relay

//#define SPIFLASH_BLOCKERASE_32K   0x52
#define SPIFLASH_BLOCKERASE_32K   0xD8
#define SPIFLASH_CHIPERASE        0x60
/*
RFM69::virtual bool sendWithRetry(uint8_t toAddress, const void* buffer, uint8_t bufferSize,
                             uint8_t retries=2, uint8_t retryWaitTime=
                                 10); //!< sendWithRetry (40ms roundtrip req for 61byte packets)
*/
MyMessage msg(RELAY_sensor, V_LIGHT);
MyMessage msg_current(Current_sensor, V_CURRENT);
MyMessage msg_temp(TEMP_sensor, V_TEMP);

//---------------- http://www.instructables.com/id/Simplified-Arduino-AC-Current-Measurement-Using-AC/?ALLSTEPS
float testFrequency = 50;                     // test signal frequency (Hz)
float windowLength = 20.0/testFrequency;     // how long to average the signal, for statistist
float intercept = -0.1129; // to be adjusted based on calibration testing
float slope = 0.0405; // to be adjusted based on calibration testing
float current_amps; // estimated actual current in amps

RunningStatistics inputStats;                 // create statistics to look at the raw test signal
SimpleTimer timer;
//RFM69 radio;


#define TACT_SWITCH
static uint8_t  value_ext_sw = HIGH, last_value_ext_sw = HIGH;
static uint8_t  temp_rfmPrevoiusReadings = 0;
static float ACS712AmpsPrevoiusReadings = 0;
unsigned long wdiDelay2  = 0;

void amps()
{
  
  float ACS712amps;
  char amps_txt[20];
  char temp_txt[10];
  
  
  // output sigma or variation values associated with the inputValue itsel
  Serial.print( "\tsigma: " ); Serial.println( inputStats.sigma() );

  int temp_rfm = (int)_radio.readTemperature(0);
  
  // convert signal sigma value to current in amps
  //current_amps = intercept + slope * inputStats.sigma();
//  ACS712Volts = 0.000794*inputStats.sigma() + 2.49; 
//  y=0.036781959552791x-0.30283767681053
  //Regression line equation: y=0.036509959786594x-0.25184447617508
  //Regression line equation: y=0.037054804045123x-0.35416182599077
  //Regression line equation: y=0.036380525770265x-0.2288321897403

  ACS712amps  = 0.05856*inputStats.sigma() - 0.2126;

  //ACS712amps  = 0.036380525770265*inputStats.sigma()-0.2288321897403;
  
  //ACS712Volts = 0.000794*inputStats.sigma() + 2.57; 
  //Serial.print( "\  ACS712Volts readings converted: " ); Serial.println( ACS712Volts );
  //current_amps = 10*ACS712Volts-25;
  //current_amps = current_amps < 0 ? 0:current_amps; 
  dtostrf(ACS712amps,0,2,amps_txt);   
  //dtostrf(current_amps,0,2,amps_txt);     
  current_amps = intercept + slope * inputStats.sigma();
  Serial.print( "\tACS712amps: " ); Serial.println( ACS712amps ); 
  Serial.print( "\tACS712AmpsPrevoiusReadings: " ); Serial.println( ACS712AmpsPrevoiusReadings ); 
    
  
  if (abs( (ACS712amps -  ACS712AmpsPrevoiusReadings)) > 0.1){
    Serial.print( "\tamps_txt: " ); Serial.println( amps_txt );
    Serial.print( "\current_amps: " ); Serial.println( ACS712amps );
    Serial.print( "\current_amps OLD: " ); Serial.println( current_amps );
    Serial.print( "\inputStats.sigma(): " ); Serial.println( inputStats.sigma() );

    ACS712AmpsPrevoiusReadings = ACS712amps;
    send(msg_current.set(amps_txt), true); // Send new state and request ack back
    wait(30);
    }
  if (abs(temp_rfmPrevoiusReadings - temp_rfm)>= 1){
    temp_rfmPrevoiusReadings = temp_rfm;
    Serial.print( "\tRFM temp: " ); Serial.println( temp_rfm );
    send(msg_temp.set(temp_rfm), true); // Send RFM module temp sensor readings
    }

}

void before() {
    wdt_enable(WDTO_8S);
    //wdt_disable();

    //RFM69 reset pin connected to digital pin 9
    pinMode(9, OUTPUT);  
    digitalWrite(9,LOW);
    
    /*
    //in case watchdog resets node - we do RFM69 reset here since VDD (power) is not disconnected while watchdog resets the node. Just in case!
    digitalWrite(9,HIGH);
    delay(10);
    digitalWrite(9,LOW);
    delay(10);
    */
    // external button
    pinMode(A2, INPUT_PULLUP);
    // external button
    digitalWrite(A2,HIGH);  
    
    
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
    
    //SPIFlash _flash(MY_OTA_FLASH_SS, MY_OTA_FLASH_JDECID);
    //Serial.println(_flash.readDeviceId());
    //Serial.println("reboot!!!!!");
    //_radio.readAllRegs();
}

void setup() {
  
}

void presentation() 
{  
  // Send the sketch version information to the gateway and Controller
  // char  SketchInfo[] = {"Relay node " && MY_NODE_ID};
  sendSketchInfo("Relay node f5","1.0");

  // Register all sensors to gw (they will be created as child devices)
  present(RELAY_sensor, S_LIGHT);
  present(Current_sensor, S_MULTIMETER);
  present(TEMP_sensor, S_TEMP);
}

unsigned long wdiDelay  = 0;

void loop()
{
  boolean loadedState;
  wdt_reset();
  //if (_radio.readRSSI() < CSMA_LIMIT){Serial.print(" canSend():"); Serial.println(_radio.readRSSI());}
  //Serial.print("RSSI "); Serial.println(_radio.readRSSI());
  
  #ifdef TACT_SWITCH
        value_ext_sw = digitalRead(A2);
        if (value_ext_sw == 0 && last_value_ext_sw == 1){ 
          loadedState = loadState(RELAY_sensor) == 1? true:false;
/*          if ( loadedState == 255){
            //Serial.println("A2 loadedState == 255");
            //Serial.println(value_ext_sw);
            //no state found in EPROM after new sketch leaded
            saveState(RELAY_sensor,RELAY_ON);
            last_value_ext_sw = value_ext_sw;
            send(msg.set(RELAY_ON), true);
            }
          else  {*/
            // Change relay state
            digitalWrite(RELAY_pin, !loadedState?RELAY_ON:RELAY_OFF);
            
            #ifdef  AdafruitNeoPixel
              noInterrupts();        
              pixels.setPixelColor(0,!loadedState ?pixels.Color(255,0,0):pixels.Color(0,255,0));
              pixels.show();
              wait(100);
              interrupts();
            #endif
            
            // Store state in eeprom
            saveState(RELAY_sensor, !loadedState?RELAY_ON:RELAY_OFF);
            send(msg.set(!loadedState?true:false), true);
            last_value_ext_sw = value_ext_sw;
//            }
          }
          else {
            last_value_ext_sw = value_ext_sw;
          }
          
  #else
        value_ext_sw = digitalRead(A2);
        if (last_value_ext_sw != value_ext_sw){
          last_value_ext_sw = value_ext_sw;
          //Serial.println("VALUE");
          //Serial.println(value_ext_sw);
          // Change relay state
          digitalWrite(RELAY_pin, value_ext_sw?RELAY_ON:RELAY_OFF);

          #ifdef  AdafruitNeoPixel
            noInterrupts();        
            pixels.setPixelColor(0,!loadedState ?pixels.Color(255,0,0):pixels.Color(0,255,0));
            pixels.show();
            wait(100);
            interrupts();
          #endif
          
          // Store state in eeprom
          saveState(RELAY_sensor, value_ext_sw?RELAY_ON:RELAY_OFF);
          send(msg.set(value_ext_sw ?true:false), true);
          }
  #endif
  

  //Serial.print("freeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeMemory()=");
  //Serial.println(freeMemory());

  /*
  if( millis() - wdiDelay > 10000 ){
  // Watchdog pulse
  wdiDelay = millis();
  digitalWrite(A4,1);
  delayMicroseconds(1);
  digitalWrite(A4,0);
  //MSerial.println("loop!!!!!");
  }*/
  

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
      noInterrupts();
      // Blink Blues light to see some message recevied
      pixels.setPixelColor(0,pixels.Color(0,0,255));
      pixels.show();
      interrupts();
      wait(100);
      noInterrupts();
      // Set Red for ON and green fo OFF
      pixels.setPixelColor(0,message.getBool() ?pixels.Color(255,0,0):pixels.Color(0,255,0));
      pixels.show();
      interrupts();
     #endif
     // Write some debug info
     //Serial.print(message.getBool()?pixels.Color(255,0,0):pixels.Color(0,255,0),HEX);
     Serial.print("Incoming change for sensor:");
     Serial.print(message.sensor);
     Serial.print(", New status: ");
     Serial.println(message.getBool());
   }
}
