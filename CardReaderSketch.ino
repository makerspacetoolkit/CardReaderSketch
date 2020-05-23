// CardReaderSketch - Gerard Baron
// Code for Arduino Yun or Seeeduino Cloud. 
// Part of Makerspace Tool Kit. Add the IP address
// of a machine running mstk_doord in the SERVER definition. 
 

///////////////////
// Yun Bridge and HttpClient Library
#include <Bridge.h>
//#include <HttpClient.h>

///////////////////
// PN532 NFC Card Reader Library
#include <Wire.h>
#include <SPI.h>                // Required by Adafruit_PN532 library
#include <Adafruit_PN532.h>

#define PN532_IRQ   (6)         // Configured for YUN
#define PN532_RESET (3)         // Not connected by default on the NFC Shield

// Setup I2C connection:
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

///////////////////
// NeoPixel Library
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN      9
#define NUMPIXELS         1

// Setup NeoPixel
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);


void setup(void) {
  Serial.begin(115200);
  Serial.println("Hello!");

  ///////////////////
  // Configure Bridge
  Bridge.begin();

  ///////////////////
  // Configure NFC Reader
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }

  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

  // Enable timeout to otherwise blocking call to nfc.readPassiveTargetID()
  nfc.setPassiveActivationRetries(1);

  // configure board to read RFID tags
  nfc.SAMConfig();

  Serial.println("Waiting for an ISO14443A Card ...");

  ///////////////////
  // Configure NeoPixel
  pixels.begin(); // This initializes the NeoPixel library.
}


#define MAX_CONNECTION_TIMER 2000
boolean server_connected = false;
long connection_timer = MAX_CONNECTION_TIMER; 
void checkServerConnection() {    
  if(connection_timer > 0) {
      connection_timer--;
  } else {
    
    if(serverStatus()) {
      // server is UP! ... try again later
      // note: timer will decrement slowly 
      //       because loop will attempt cardread
      
      // approx. test every 150s ...
      connection_timer = MAX_CONNECTION_TIMER;
      server_connected = true;
    
      //Serial.println("Server Up");
 
    } else {
      // server is DOWN! ... try again soon
      // note: timer will decrement very quickly

      // approx. check every 3s..
      connection_timer = MAX_CONNECTION_TIMER;
      server_connected = false;

      //Serial.println("Server Down");
    }
  } 
}

// UX: used to offset breathing code after authenication
//     so that breathing always begins from off-state
float phasedelay = 0.0;

void loop(void) {

  // Periodically check server connection
  // If down, breath red and disable card reader until up
  checkServerConnection();
  if(!server_connected) {
    // breathing code ... server down
    float b = (exp(sin(millis() / 2000.0 * PI - phasedelay)) - 0.36787944) * 0.0;
    float r = (exp(sin(millis() / 2000.0 * PI - phasedelay)) - 0.36787944) * 25.0;
    float g = (exp(sin(millis() / 2000.0 * PI - phasedelay)) - 0.36787944) * 0.0;
    pixels.setPixelColor(0, pixels.Color(r, g, b)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
    return;    
  }
  
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)

  // Blocking if not configured with setPassiveActivationRetries()
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {

    // flash yellow
    pixels.setPixelColor(0, pixels.Color(40.0, 30.0, 0));
    pixels.show();

//TODO: check that MiFare Classic Card ... 4 not 7 bytes

    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("  UID Value (hex): ");
    nfc.PrintHex(uid, uidLength);

    // Convert to decimal value
    Serial.print("  UID Value (dec): ");
    unsigned long uid_dec = (((long)uid[0]) << 24);
    uid_dec |= (((long)uid[1]) << 16);    
    uid_dec |= (((long)uid[2]) << 8);
    uid_dec |= ((long)uid[3]);  
    Serial.println(uid_dec); 

    delay(1000);
    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    pixels.show();
    delay(500);

    if(authenticate(uid_dec)) {
      pixels.setPixelColor(0, pixels.Color(0, 255.0, 0));
      pixels.show();
      Serial.println("  Access: Granted\n"); 

    } else {
      pixels.setPixelColor(0, pixels.Color(255.0, 0, 0));
      pixels.show();
      Serial.println("  Access: Denied\n"); 
    }
    
    delay(1000);
    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    pixels.show();
    delay(1000);

    phasedelay = (millis() / 2000.0 * PI) + (PI / 2.0) ;
  }
  
  // breathing code ...
  float b = (exp(sin(millis() / 2000.0 * PI - phasedelay)) - 0.36787944) * 90.0;
  float r = (exp(sin(millis() / 2000.0 * PI - phasedelay)) - 0.36787944) * 0.0;
  float g = (exp(sin(millis() / 2000.0 * PI - phasedelay)) - 0.36787944) * 0.0;

  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}


