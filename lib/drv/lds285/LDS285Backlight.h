/*
 * XMEM LCD Library for the Arduino
 * Copyright (c) 2012,2013 Andy Brown. All rights reserved.
 * This is open source software. Please see website for terms and conditions:
 *
 * http://andybrown.me.uk
 *
 * This notice may not be removed or altered from any source distribution.
 */

#pragma once


namespace lcd {


	/*
	 * Class to encapsulate a 0..100% backlight control on a Nokia 95 8Gb
	 * controller (LDS285) where the backlight is built in to the panel
	 * and managed by software.
	 */

	template<class TAccessMode>
	class LDS285Backlight {

		protected:
			uint8_t _currentPercentage;

		public:
			LDS285Backlight(uint8_t initialPercentage=0);
			void fadeTo(uint8_t newPercentage,int msPerStep);
			void setPercentage(uint8_t percentage);
	};


	/*
	 * Constructor - set to the user start value (default zero) but don't write out
	 * to the panel because we can't guarantee that the user has initialised the panel
	 * at the point this constructor is called.
	 */

	template<class TAccessMode>
	inline LDS285Backlight<TAccessMode>::LDS285Backlight(uint8_t initialPercentage) {
		_currentPercentage=initialPercentage;
	}


	/*
	 * fade up or down to the supplied percentage waiting
	 * for msPerStep millis between each step
	 */

	template<class TAccessMode>
	inline void LDS285Backlight<TAccessMode>::fadeTo(uint8_t newPercentage,int msPerStep) {

		int8_t direction;

		if(newPercentage==_currentPercentage)
			return;

		direction=newPercentage>_currentPercentage ? 1 : -1;

		while(newPercentage!=_currentPercentage) {
			setPercentage(_currentPercentage+direction);
			delay(msPerStep);
		}
	}


	/*
	 * Set a new backlight brightness level to a percentage
	 */

	template<class TAccessMode>
	inline void LDS285Backlight<TAccessMode>::setPercentage(uint8_t percentage) {

		uint8_t value;

		value=static_cast<uint8_t>((255*(uint16_t)percentage)/100);

		TAccessMode::writeCommand(lds285::WRITE_DISPLAY_BRIGHTNESS);
		TAccessMode::writeData(value);
		_currentPercentage=percentage;
	}
}
