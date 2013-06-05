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
	 * Template class holding the specialisation of LDS285Colour for 24-bit colours
	 * @tparam TAccessMode The access mode class (e.g. FSMC)
	 */

	template<class TAccessMode,class TPanelTraits>
	class LDS285Colour<COLOURS_24BIT,TAccessMode,TPanelTraits> {

		protected:
			uint16_t getInterfacePixelFormat() const;

			typedef uint32_t TColour;

			struct UnpackedColour {
				uint8_t r,g,b;
			};

		public:
			void unpackColour(TColour src,UnpackedColour& dest) const;
			void unpackColour(uint8_t red,uint8_t green,uint8_t blue,UnpackedColour& dest) const;

			void writePixel(const UnpackedColour& cr) const;
			void fillPixels(uint32_t numPixels,const UnpackedColour& cr) const;

			void allocatePixelBuffer(uint32_t numPixels,uint8_t*& buffer,uint32_t& bytesPerPixel) const;
			void rawFlashTransfer(uint32_t data,uint32_t numPixels) const;

			uint8_t getBytesPerPixel() const;
	};


	/**
	 * Get the register setting for 16-bit colours
	 * @return 16-bit interface pixel format register setting
	 */

	template<class TAccessMode,class TPanelTraits>
	inline uint16_t LDS285Colour<COLOURS_24BIT,TAccessMode,TPanelTraits>::getInterfacePixelFormat() const {
		return 0x77;
	}


	/**
	 * Unpack the colour from rrggbb to the interal 8-8-8 format
	 * 00000000RRRRRRRRGGGGGGGGBBBBBBBB ->
	 * RRRRRRRR,GGGGGGGG,BBBBBBBB
	 * @param src rrggbb
	 * @param dest The unpacked colour structure
	 */

	template<class TAccessMode,class TPanelTraits>
	inline void LDS285Colour<COLOURS_24BIT,TAccessMode,TPanelTraits>::unpackColour(TColour src,UnpackedColour& dest) const {
		dest.r=(src >> 16) & 0xff;
		dest.g=(src >> 8) & 0xff;
		dest.b=src & 0xff;
	}


	/**
	 * Unpack the colour from components to the internal format
	 * @param red
	 * @param green
	 * @param blue
	 * @param dest
	 */

	template<class TAccessMode,class TPanelTraits>
	inline void LDS285Colour<COLOURS_24BIT,TAccessMode,TPanelTraits>::unpackColour(uint8_t red,uint8_t green,uint8_t blue,UnpackedColour& dest) const {
		dest.r=red;
		dest.g=green;
		dest.b=blue;
	}


	/**
	 * Write a single pixel to the current output position.
	 * Assumes that the caller has already issued the beginWriting() command.
	 * @param cr The pixel to write
	 */

	template<class TAccessMode,class TPanelTraits>
	inline void LDS285Colour<COLOURS_24BIT,TAccessMode,TPanelTraits>::writePixel(const UnpackedColour& cr) const {
		TAccessMode::writeData(cr.r);
		TAccessMode::writeData(cr.g);
		TAccessMode::writeData(cr.b);
	}


	/**
	 * Fill a block of pixels with the same colour. This operation will issue the
	 * beginWriting() command for you.
	 * @param numPixels how many
	 * @param cr The unpacked colour to write
	 */

	template<class TAccessMode,class TPanelTraits>
	inline void LDS285Colour<COLOURS_24BIT,TAccessMode,TPanelTraits>::fillPixels(uint32_t numPixels,const UnpackedColour& cr) const {

		uint8_t b,g,r;

		TAccessMode::writeCommand(lds285::MEMORY_WRITE);

		b=cr.b;
		g=cr.g;
		r=cr.r;

		while(numPixels--) {
			TAccessMode::writeData(r);
			TAccessMode::writeData(g);
			TAccessMode::writeData(b);
		}
	}


	/**
	 * Allocate a buffer for pixel data. You supply the number of pixels and this allocates the buffer as a uint8_t[].
	 * Allocated buffers should be freed with free()
	 *
	 * @param numPixels The number of pixels to allocate
	 * @param buffer The output buffer
	 * @param bytesPerPixel Output the number of bytes per pixel
	 */

	template<class TAccessMode,class TPanelTraits>
	inline void LDS285Colour<COLOURS_24BIT,TAccessMode,TPanelTraits>::allocatePixelBuffer(uint32_t numPixels,uint8_t*& buffer,uint32_t& bytesPerPixel) const {
		buffer=(uint8_t *)malloc(numPixels*3);
		bytesPerPixel=3;
	}


	/**
	 * Bulk-copy some pixels from the memory buffer to the LCD. The pixels must already be formatted ready
	 * for transfer.
	 * @param buffer The memory buffer
	 * @param numPixels The number of pixels to transfer from the buffer
	 */

	template<class TAccessMode,class TPanelTraits>
	inline void LDS285Colour<COLOURS_24BIT,TAccessMode,TPanelTraits>::rawFlashTransfer(uint32_t data,uint32_t numPixels) const {

		TAccessMode::writeCommand(lds285::MEMORY_WRITE);
		TAccessMode::rawFlashTransfer(data,numPixels*3);
	}


	/**
	 * Get the number of bytes per pixel
	 * @return 3
	 */

	template<class TAccessMode,class TPanelTraits>
	inline uint8_t LDS285Colour<COLOURS_24BIT,TAccessMode,TPanelTraits>::getBytesPerPixel() const {
		return 3;
	}
}