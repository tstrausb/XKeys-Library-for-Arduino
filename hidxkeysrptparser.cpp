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

#include "hidxkeysrptparser.h"

XkeysReportParser::XkeysReportParser(HIDUniversal *p, USB *q) :
pHid(p), pUsb(q)
{
	oldButtons = 0;

  	for (uint8_t i=0; i<RPT_XKEYS_LEN; i++)
		oldRpt[i]	= 0xD;

	for (uint8_t i=0; i<OUT_RPT_LEN; i++)
		rpt[i]	= 0x0;

	_blueIntensity = 128;
	_redIntensity = 128;
	_rowsBlue = 255;
	_rowsRed = 0;
}

void XkeysReportParser::init()
{
  if (pUsb->Init() == -1)
            Serial.println("OSC did not start.");
            
  delay( 200 );

  if (!pHid->SetReportParser(0, (HIDReportParser*)this))
            ErrorMessage<uint8_t>(PSTR("SetReportParser"), 1  );

}

void XkeysReportParser::Parse(HID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf)
{
	bool match = true;

	// Checking if there are changes in report since the method was last called
	for (uint8_t i=0; i<RPT_XKEYS_LEN; i++)
		if (buf[i] != oldRpt[i])
		{
			match = false;
			break;
		}

	// Calling xkeys event handler
	if (!match)
	{
		for (uint8_t i=0; i<RPT_XKEYS_LEN; i++) oldRpt[i] = buf[i];
	}
	

	uint32_t buttons = (0x00000000 | buf[5]);
	buttons <<= 24;
	buttons |= ((uint32_t)buf[4] << 16);
	buttons |= ((uint32_t)buf[3] << 8);
        buttons |= ((uint32_t)buf[2] << 0);
	uint32_t changes = (buttons ^ oldButtons);
 

	// Calling Button Event Handler for every button changed
	if (changes)
	{
		for (uint8_t i=0; i<0x1e; i++)
		{
			uint32_t mask = ((uint32_t)0x01 << i);

			if (((mask & changes) > 0)) {
				uint8_t j = i/8 + 4*(i%8);
				if ((buttons & mask) > 0) {
					(STICK) ? OnKeyDown(j) : OnKeyDown(i);
                                } else {
					(STICK) ? OnKeyUp(j) : OnKeyUp(i);
                                }
			}
		}
		oldButtons = buttons;
	}
}

void XkeysReportParser::wipeArray()
{
  for (uint8_t i=0; i<OUT_RPT_LEN; i++)
		rpt[i]	= 0x00;
}

void XkeysReportParser::sendCommand()
{
  uint8_t ret = pHid->SndRpt(OUT_RPT_LEN, rpt);
  Serial.print("HID OUT Return: ");
  Serial.println(ret, HEX);
  uint32_t hold = millis() + 25;
  while(millis() < hold)
    pUsb->Task();

}

void XkeysReportParser::setBLSteps(uint8_t color, uint8_t *steps)
{
  wipeArray();
  rpt[0] = 172;
  rpt[1] = color;
  for(uint8_t i = 0; i < 10; i++) {
    rpt[i+2] = steps[i];
  }
  sendCommand();
}

void XkeysReportParser::stepBLIntensity(uint8_t color, uint8_t incDec, uint8_t wrap)
{
  wipeArray();
  rpt[0] = 173;
  rpt[1] = color;
  rpt[2] = incDec;
  rpt[3] = wrap;
  sendCommand();
}

void XkeysReportParser::indexSetLEDs(uint8_t led, uint8_t state)
{
  wipeArray();
  rpt[0] = 179;
  rpt[1] = led;
  rpt[2] = state;
  sendCommand();
}

void XkeysReportParser::setFlashFreq(uint8_t freq)
{
  wipeArray();
  rpt[0] = 180;
  rpt[1] = freq;
  sendCommand();
}

void XkeysReportParser::indexSetBL(uint8_t keyId, uint8_t color, uint8_t mode)
{
  wipeArray();
  uint8_t _index;
  rpt[0] = 181;
  if(!STICK) {
    if(color) {
      uint8_t _extra = (PADSIZE == 24) ? 32 : (PADSIZE);
      _index = keyId + _extra;
    } else {
      _index = keyId;
    }
  } else {
    _index = keyId + (2*(keyId/6));
  }
  rpt[1] = _index;
  rpt[2] = mode;
  sendCommand();
}

void XkeysReportParser::setRowBL(uint8_t row, uint8_t color, uint8_t onOff)
{
  wipeArray();
  rpt[0] = 182;
  rpt[1] = color;
  uint8_t _rows;
  if(!STICK) {
    _rows = (color) ? (_rowsRed) : (_rowsBlue);
    bitWrite(_rows, row - 1, onOff);
    (color) ? (_rowsRed = _rows) : (_rowsBlue = _rows);
  } else {
    _rows = (onOff * 255);
  }
  rpt[2] = _rows;
  sendCommand();
}

void XkeysReportParser::toggleBLs()
{
  wipeArray();
  rpt[0] = 184;
  sendCommand();
}

void XkeysReportParser::setLEDs(boolean green, boolean red)
{
  wipeArray();
  uint8_t _LEDs = (((uint8_t)red << 7) | ((uint8_t)green << 6));
  rpt[0] = 186;
  rpt[1] = _LEDs;
  sendCommand();
}

void XkeysReportParser::setBLintensity(uint8_t color, uint8_t intensity)
{
  wipeArray();
  rpt[0] = 187;
  (color) ? (_redIntensity = intensity) : (_blueIntensity = intensity);
  rpt[1] = _blueIntensity;
  rpt[2] = _redIntensity;
  sendCommand();
}

void XkeysReportParser::setUnitID(uint8_t unitID)
{
  wipeArray();
  rpt[0] = 189;
  rpt[1] = unitID;
  sendCommand();
}

void XkeysReportParser::saveBLStateEEPROM(uint8_t save)
{
  wipeArray();
  rpt[0] = 199;
  rpt[1] = save;
  sendCommand();
}

void XkeysReportParser::enableTimeStamp(uint8_t endis)
{
  wipeArray();
  rpt[0] = 210;
  rpt[1] = endis;
  sendCommand();
}

void XkeysReportParser::reboot()
{
  wipeArray();
  rpt[0] = 238;
  sendCommand();
}
