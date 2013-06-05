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


	/**
	 * Specialisation of LDS285Orientation for the panel in LANDSCAPE mode.
	 * @tparam TAccessMode the access mode implementation, e.g. FSMC
	 */

	template<class TAccessMode,class TPanelTraits>
	class LDS285Orientation<LANDSCAPE,TAccessMode,TPanelTraits> {

		protected:
			uint16_t getMemoryAccessControl() const;

		public:
			int16_t getWidth() const;
			int16_t getHeight() const;
			void moveTo(const Rectangle& rc) const;

			void setScrollPosition(int16_t scrollPosition);
	};


	/**
	 * Get the register setting for memory access control
	 * @return The entry mode register setting for portrait
	 */

	template<class TAccessMode,class TPanelTraits>
	inline uint16_t LDS285Orientation<LANDSCAPE,TAccessMode,TPanelTraits>::getMemoryAccessControl() const {
		return 0xc0;
	}


	/**
	 * Get the width in pixels
	 * @return 240px
	 */

	template<class TAccessMode,class TPanelTraits>
	inline int16_t LDS285Orientation<LANDSCAPE,TAccessMode,TPanelTraits>::getWidth() const {
		return 320;
	}


	/**
	 * Get the height in pixels
	 * @return 320px
	 */

	template<class TAccessMode,class TPanelTraits>
	inline int16_t LDS285Orientation<LANDSCAPE,TAccessMode,TPanelTraits>::getHeight() const {
		return 240;
	}


	/**
	 * Move the display output rectangle
	 * @param rc The display output rectangle
	 */

	template<class TAccessMode,class TPanelTraits>
	inline void LDS285Orientation<LANDSCAPE,TAccessMode,TPanelTraits>::moveTo(const Rectangle& rc) const {

		TAccessMode::writeCommand(lds285::COLUMN_ADDRESS_SET);

		TAccessMode::writeData(rc.X >> 8);					// x=0..319
		TAccessMode::writeData(rc.X & 0xff);
		TAccessMode::writeData((rc.X+rc.Width-1)>>8);
		TAccessMode::writeData((rc.X+rc.Width-1) & 0xff);

		TAccessMode::writeCommand(lds285::ROW_ADDRESS_SET);

		TAccessMode::writeData(0);								// y=0..239
		TAccessMode::writeData(rc.Y);
		TAccessMode::writeData(0);
		TAccessMode::writeData(rc.Y+rc.Height-1);
	}


	/**
	 * Set a vertical scroll position
	 * @param scrollPosition The new scroll position
	 */

	template<class TAccessMode,class TPanelTraits>
	inline void LDS285Orientation<LANDSCAPE,TAccessMode,TPanelTraits>::setScrollPosition(int16_t scrollPosition) {

		// pull into range

		if(scrollPosition<0)
			scrollPosition+=320;
		else if(scrollPosition>319)
			scrollPosition-=320;

		// don't ask me why, but when the scroll position is zero either the optimiser breaks the code or
		// there is a panel timing issue that does not occur when the optimiser is disabled (-O0). Inserting
		// this zero delay call fixes it. <sigh>

		if(!scrollPosition)
			delay(0);

		// write to the register

		TAccessMode::writeCommand(lds285::VERTICAL_SCROLLING_START_ADDRESS);
		TAccessMode::writeData(scrollPosition >> 8);
		TAccessMode::writeData(scrollPosition & 0xff);
	}
}
