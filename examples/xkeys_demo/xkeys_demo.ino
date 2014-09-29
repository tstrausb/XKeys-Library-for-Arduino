
#include "hid.h"
#include "hiduniversal.h"
#include "Usb.h"

#include <SPI.h>
#include <hidxkeysrptparser.h>

USB                             Usb;
HIDUniversal                    Hid(&Usb);

uint32_t now = millis() + 1000; // used during setup, Usb.Task must run a number of times before sending any commands to the XKeys device.

uint8_t counter = 0; //used for demo. Every key release will step through various commands.

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

XKeys::XKeys() : XkeysReportParser(&Hid, &Usb) {}

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
    switch(counter) { // tried to choose some "visual" commands here
      case 0: 
        indexSetLEDs(LED_RED, MODE_ON);
        counter++;
        break;
      case 1:
        indexSetBL(0, BL_BLUE, MODE_FLASH);
        counter++;
        break;
      case 2:
        setRowBL(2, BL_RED, MODE_ON);
        counter++;
        break;
      case 3:
        toggleBLs();
        counter++;
        break;
      case 4:
        toggleBLs();
        counter++;
        break;
      case 5:
        setBLintensity(BL_BLUE, 255);
        counter++;
        break;
      case 6:
        indexSetLEDs(LED_RED, MODE_OFF);
        indexSetBL(0, BL_BLUE, MODE_OFF);
        indexSetBL(0, BL_BLUE, MODE_ON);
        setRowBL(2, BL_RED, MODE_OFF);
        setBLintensity(BL_BLUE, 128);
        /****************************
        Too many rapid fire commands will 
        cause unpredictable behavior. Don't 
        go nuts! There is a small delay built 
        in to allow for the back and forth
        reports but you can overburden the
        host.
        ****************************/
        counter = 0;
        break;
    }
}

void setup()
{
  /************define these for your device****************/
  XKs.STICK = 0;      // 0 for pads, 1 for sticks
  XKs.PADSIZE = 24;   // enter number of keys on your pad (irrelevant if stick)
  /********************************************************/
  
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
  
  while(millis() < now) { // required: allows the host to initialize before any commands are sent
    Usb.Task();
  }
  
  /*Place commands here to put your XKeys device into its desired initial state.*/
  /*************my particular example***************/
  for(uint8_t i = 0; i < 6; i++) {
    XKs.setRowBL((i + 1), BL_RED, MODE_OFF); // turning off all red backlighting for initial state on an XK24
  }

}

void loop()
{
    // Excutes the USB.init() function to receive reports This MUST stay here!
    Usb.Task();

}
