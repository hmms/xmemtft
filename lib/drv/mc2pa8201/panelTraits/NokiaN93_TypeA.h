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

#include "drv/mc2pa8201/commands/AllCommands.h"


namespace lcd {

	/**
	 * Panel traits for a Type A Nokia N93 panel
	 *
	 * 1. Hardware scrolling is not supported
	 * 2. (page,column) addressing is not swapped for landscape mode
	 * 3. Memory access control is handled
	 */

	class NokiaN93_TypeA {
		public:
			static int16_t normaliseScrollPosition(int16_t scrollPosition);

			template<Orientation TOrientation>
			static uint8_t getColumnAddressCommand();

			template<Orientation TOrientation>
			static uint8_t getPageAddressCommand();

			template<Orientation TOrientation>
			static uint8_t getMemoryAccessControl();

			template<Orientation TOrientation>
			static bool hasHardwareScrolling();

			static uint16_t getScrollHeight();
	};


	/**
	 * Get the maximum scroll height of the display
	 * @return 320
	 */

	inline uint16_t NokiaN93_TypeA::getScrollHeight() {
		return 320;
	}


	/*
	 * Hardware scrolling is not supported
	 */

	template<Orientation TOrientation>
	inline bool NokiaN93_TypeA::hasHardwareScrolling() {
		return false;
	}


	/*
	 * Normalise the scroll position for the setScrollPosition() call.
	 * Scrolling is not supported (yet...) on the N93
	 */

	inline int16_t NokiaN93_TypeA::normaliseScrollPosition(int16_t scrollPosition) {
		return scrollPosition;
	}


	/**
	 * Get the command used to set column address in landscape mode
	 * @return mc2pa8201::COLUMN_ADDRESS_SET
	 */

	template<>
	inline uint8_t NokiaN93_TypeA::getColumnAddressCommand<LANDSCAPE>() {
		return mc2pa8201::COLUMN_ADDRESS_SET;
	}


	/**
	 * Get the command used to set column address in landscape mode
	 * @return mc2pa8201::PAGE_ADDRESS_SET
	 */

	template<>
	inline uint8_t NokiaN93_TypeA::getPageAddressCommand<LANDSCAPE>() {
		return mc2pa8201::PAGE_ADDRESS_SET;
	}


	/**
	 * Get the parameter to the MEMORY_ACCESS_CONTROL command
	 * @return 0x50
	 */

	template<>
	inline uint8_t NokiaN93_TypeA::getMemoryAccessControl<PORTRAIT>() {
		return 0x50;
	}


	/**
	 * Get the parameter to the MEMORY_ACCESS_CONTROL command
	 * @return 0x50
	 */

	template<>
	inline uint8_t NokiaN93_TypeA::getMemoryAccessControl<LANDSCAPE>() {
		return 0x60;
	}
}