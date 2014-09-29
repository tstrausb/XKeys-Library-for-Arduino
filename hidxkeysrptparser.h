// (C) 2014 Thomas Strausbaugh
// See the README for additional credit where credit is due!

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; version 2
// of the License.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#if !defined(__HIDXKEYSRPTPARSER_H__)
#define __HIDXKEYSPARSER_H__

#if defined(ARDUINO) && ARDUINO >=100
#include "Arduino.h"
#else
#include <WProgram.h>
#endif

#include "hid.h"
#include "hiduniversal.h"
#include "Usb.h"

/****************************
Supported models are XK 4/8/16 
stick and XK 24/60/80/128. Modify 
definitions below for your device.
****************************/

#define RPT_XKEYS_LEN		18
#define OUT_RPT_LEN		35
#define BL_BLUE			0
#define BL_RED			1
#define MODE_OFF		0
#define MODE_ON			1
#define MODE_FLASH		2
#define LED_GREEN		6
#define LED_RED			7

class XkeysReportParser : public HIDReportParser
{
	uint8_t			oldRpt[RPT_XKEYS_LEN];
	uint32_t			oldButtons;

public:
        uint8_t rpt[OUT_RPT_LEN];
        uint8_t STICK;
        uint8_t PADSIZE;
          // define STICK and PADSIZE in the setup of the sketch

private:
        uint8_t		_blueIntensity;
        uint8_t		_redIntensity;
        uint8_t		_rowsBlue;
        uint8_t		_rowsRed;

public:

	/**********If the public methods are called from within the class
	(i.e. from the OnKey methods), you do not need to denote them with
	<class_instance>.method. See example.***************************/


	XkeysReportParser(HIDUniversal *p, USB *q);
	void init();
	virtual void Parse(HID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf);

	/**********BL stepping methods are only supported on XK68 or XK128************/
	void setBLSteps(uint8_t color, uint8_t *steps);
	  // use an array of 10 steps
	  // color: 0 for blue, 1 for red, 255 to reset all to factory default
	  // stepX: 0-255 for each
	void stepBLIntensity(uint8_t color, uint8_t incDec, uint8_t wrap);
	  // color: 0 for blue, 1 for red; incDec: 0 for lower, 1 for higher; wrap: 0 to wrap around, 1 to stop at limit
	/*****************************************************************************/

	void indexSetLEDs(uint8_t led, uint8_t state);
	void setFlashFreq(uint8_t freq); 
        // 0 is fastest, 255 is slowest (approx. 4 seconds)
	void indexSetBL(uint8_t keyId, uint8_t color, uint8_t mode); 
        // Xkeys sticks only have blue backlights--color is disregarded in this case but you must enter something
	void setRowBL(uint8_t row, uint8_t color, uint8_t mode); 
        // Xkeys sticks only have blue backlights--what you pass as color MUST evaluate to 0 and row must evaluate to 1
        // rows are 1 - 8 from top to bottom (entering row 0 will cause an error)
	void toggleBLs();
	  // any BLs that are on will go off. the same ones will come back on. Does not "reverse" the BLs.
	void setLEDs(boolean green, boolean red);
        // true for on and false for off
        // must use indexSetLEDs for flash mode
	void setBLintensity(uint8_t color, uint8_t intensity);
        // 0 is off and 255 is brightest
	void setUnitID(uint8_t unitID);
	  // 0 - 255
	void saveBLStateEEPROM(uint8_t save);
	  // anything other than 0 will save the current BL state as new startup default
	  // ********limited number of times this can be done! Use sparingly!**********
	void enableTimeStamp(uint8_t endis);
	  // 0 to disable, 1 to enable

protected:
	// Subclass XkeysReportParser and define these methods to receive events
	virtual void OnKeyUp(uint8_t keyId) {}; 
	virtual void OnKeyDown(uint8_t keyId) {};

private:
	HIDUniversal* pHid;
	USB* pUsb;
	void sendCommand();
	void wipeArray();

};

#endif // __HIDXKEYSRPTPARSER_H__
