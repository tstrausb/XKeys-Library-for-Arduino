
#include "hid.h"
#include "hiduniversal.h"
#include "Usb.h"

#include <SPI.h>
#include <hidxkeysrptparser.h>

uint8_t counter = 0; 
uint8_t color = BL_BLUE;
uint8_t mode = MODE_ON;

/******************************************************************************
  Create a subclass of the XkeysReportParser and override the OnKey methods 
  to handle the key reports when they are received.
******************************************************************************/
class XKeys : public XkeysReportParser
{

public:
        XKeys();
protected:
  virtual void OnKeyUp(uint8_t keyId); 
  virtual void OnKeyDown(uint8_t keyId);
};

XKeys::XKeys() : XkeysReportParser() {}

XKeys XKs;

void XKeys::OnKeyDown(uint8_t keyId)
{
    Serial.print("Key down: ");
    Serial.println(keyId, DEC);
}

void XKeys::OnKeyUp(uint8_t keyId)
{
    Serial.print("Key Up: ");
    Serial.println(keyId, DEC);
}

void setup()
{
  /****Define your device details here.****/
  XKs.STICK = 0; // 0 for pad, 1 for stick
  XKs.PADSIZE = 24; // number of keys on your device
  /****************************************/
  
  Serial.begin( 115200 );
  while(!Serial); // waiting for serial port to connect
/******************************************************************************
    If you are running on boards like the Ethernet you might need 
    set the pin modes and write them to high to get the SPI bus to
    allow the USB Host Shield to work. You may also have to rewire 
    the USB host shield to use pin 7 instead of 10 for SPI. 
    Procedure on Circuits@Home's website.
******************************************************************************/
  pinMode(10, OUTPUT);
  //pinMode(4, OUTPUT);
  digitalWrite(10, HIGH);
  //digitalWrite(4, HIGH);
  
  Serial.println("Start");
  
  XKs.init();
  
  /*Place commands here to put your XKeys device into its desired initial state.*/
  /*************my particular example***************/
  for(uint8_t i = 0; i < 6; i++) {
    XKs.setRowBL((i + 1), BL_RED, MODE_OFF); // turning off all red backlighting for initial state on an XK24
  }
  for(uint8_t i = 0; i < 6; i++) {
    XKs.setRowBL((i + 1), BL_BLUE, MODE_OFF); // turning off all blue backlighting for initial state on an XK24
  }

}

void loop()
{
    // Excutes the USB.init() function to receive reports This MUST stay here!
    XKs.runLoop();
    XKs.indexSetBL(counter, color, mode);
    counter++;
    if(counter == 6) {
      counter = 8;
    }
    if(counter == 14) {
      counter = 16;
    }
    if(counter == 22) {
      counter = 24;
    }
    if((counter == 30) && (color == BL_BLUE) && (mode == MODE_ON)) {
      color = BL_RED;
      counter = 0;
    }
    if((counter == 30) && (color == BL_RED) && (mode == MODE_ON)) {
      mode = MODE_OFF;
      color = BL_BLUE;
      counter = 0;
    }
    if((counter == 30) && (color == BL_BLUE) && (mode == MODE_OFF)) {
      color = BL_RED;
      counter = 0;
    }
    if((counter == 30) && (color == BL_RED) && (mode == MODE_OFF)) {
      mode = MODE_ON;
      color = BL_BLUE;
      counter = 0;
    }
}
