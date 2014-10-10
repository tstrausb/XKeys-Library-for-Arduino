#include "Usb.h"
#include "hid.h"
#include "hiduniversal.h"
 
#include <SPI.h>
#include <Ethernet.h>
#include <hidxkeysrptparser.h>
#include <ATEM.h>
 
// MAC address and IP address for this *particular* Ethernet Shield!
// MAC address is printed on the shield
// IP address is an available address you choose on your subnet where the switcher is also present:
byte mac[] = { 
  0x90, 0xA2, 0xDA, 0x0F, 0x9A, 0xDC };  	// <= SETUP
IPAddress ip(10, 195, 51, 155);				// <= SETUP
 
// Connect to an ATEM switcher on this address and using this local port:
// The port number is chosen randomly among high numbers.
ATEM AtemSwitcher(IPAddress(10, 195, 51, 156), 56417);  // <= SETUP (the IP address of the ATEM switcher)

uint16_t currentInput = 0;
 
class XKeys : public XkeysReportParser
{
 
public:
        XKeys();
protected:
     	//virtual void OnKeyUp(uint8_t keyId); 
	virtual void OnKeyDown(uint8_t keyId);
};
 
XKeys::XKeys() : XkeysReportParser() {}
 
void XKeys::OnKeyDown(uint8_t keyId)
{
  if(keyId < 7) { // the first 7 keys (0 through 6) will switch between inputs 0 to 6 (0 is black)
    AtemSwitcher.changeProgramInput(keyId);
  }
  else if(keyId == 8) { // bars
    AtemSwitcher.changeProgramInput(1000);
  }
  else if(9 <= keyId <= 10) { // color 1 or 2
    AtemSwitcher.changeProgramInput(1992 + keyId);
  }
  else if(keyId == 11) { // media player 1
    AtemSwitcher.changeProgramInput(3010);
  }
  else if(keyId == 12) { // media player 2
    AtemSwitcher.changeProgramInput(3020);
  }
  // Keys 13 and 14 don't do anything. 15 is reserved for connection status.
  checkBLs();
  
}
 
XKeys XKs;

void checkBLs() 
{
  AtemSwitcher.delay(100);
  XKs.indexSetBL(currentInput, BL_BLUE, MODE_OFF);
  uint16_t tempInput = AtemSwitcher.getProgramInput();
  if(tempInput < 7) { // filtering "currentInput" to "keyId"
    currentInput = tempInput;
  } else if (tempInput == 1000) {
    currentInput = 8;
  } else if (2001 <= tempInput <= 2002) {
    currentInput = tempInput - 1992;
  } else if (tempInput == 3010) {
    currentInput = 11;
  } else if (tempInput == 3020) {
    currentInput = 12;
  }
  XKs.indexSetBL(currentInput, BL_BLUE, MODE_ON);
  
}

void reconnect()
{
  XKs.indexSetBL(15, BL_BLUE, MODE_FLASH); // going to flash mode to indicate loss of connection
  while(1)
  {
    Serial.println(F("Reconnecting"));
    AtemSwitcher.connect();
    // sleep while the connection is established
    delay(12000);
    // Do we have a connection?
    if(!AtemSwitcher.isConnectionTimedOut()) {
      XKs.indexSetBL(15, BL_BLUE, MODE_ON); // going back to on mode to indicate good connection again
      return;
    }
  }
}
  
 
void setup()
{
  /****Define your device details here.****/
  XKs.STICK = 1; // 0 for pad, 1 for stick
  XKs.PADSIZE = 16; // number of keys on your device
  /****************************************/
  
  // Start the Ethernet, Serial (debugging) and UDP:
  Ethernet.begin(mac,ip);
  
  Serial.begin( 115200 );
  //pinMode(10, OUTPUT);
  //pinMode(4, OUTPUT);
  //digitalWrite(10, HIGH);
  //digitalWrite(4, HIGH);
  
  Serial.println(F("Start"));
      
  // Initialize a connection to the switcher:
  AtemSwitcher.serialOutput(true);
  
  //Setup the XKs report parser
  XKs.init();  // do this before connecting to ATEM because it takes some time to stabilize
  
  AtemSwitcher.connect(); //assuming an Atem Television Studio
  
  for(uint8_t i = 1; i < 7; i++) {
    XKs.setRowBL(i, BL_BLUE, MODE_OFF); // turning off all blue backlighting for initial state on an XK16
  }
  
  AtemSwitcher.runLoop(); // keeping the connection alive since we haven't entered the loop yet
  
  XKs.indexSetBL(15, BL_BLUE, MODE_ON); // key 15 lit means we are connected to the ATEM
  
  checkBLs();

}
 
void loop()
{
    // Check for packets, respond to them etc. Keeping the connection alive!
    AtemSwitcher.runLoop();  
    XKs.runLoop();
    // Check to see if the Atem connection has timed out. If so attempt to reconnect.
    if(AtemSwitcher.isConnectionTimedOut())
      reconnect();

}
