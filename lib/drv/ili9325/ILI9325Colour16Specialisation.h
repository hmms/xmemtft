/*
 * XMEM LCD Library for the Arduino
 * Copyright (c) 2012,2013 Andy Brown. All rights reserved.
 * This is open source software. Please see website for terms and conditions:
 *
 * http://andybrown.me.uk
 *
 * This notice may not be removed or altered from any source distribution.
 */

/*
 * Implementation for Adafruit TFT LCD copyright (c) 2013 Colin Irwin, see
 * http://aethersdr.org/ for details. Provided under original terms and
 * conditions.
 *
 */

#pragma once

namespace lcd {

	/**
	 * Template class holding the specialisation of ILI9325Colour for 16-bit colours
	 */

	template<class TAccessMode>
	class ILI9325Colour<COLOURS_16BIT,TAccessMode> {

		protected:
			uint16_t getInterfacePixelFormat() const;

			typedef uint32_t TColour;

			struct UnpackedColour {
					uint8_t first,second;
			};

		public:
			void unpackColour(TColour src,UnpackedColour& dest) const;
			void unpackColour(uint8_t red,uint8_t green,uint8_t blue,UnpackedColour& dest) const;
			void writePixel(const UnpackedColour& cr) const;
			void fillPixels(uint32_t numPixels,const UnpackedColour& cr) const;
			void allocatePixelBuffer(uint32_t numPixels,uint8_t*& buffer,uint32_t& bytesPerPixel) const;
			uint8_t getBytesPerPixel() const;
	};

	/**
	 * Get the register setting for 16-bit colours
	 * @return 16-bit interface pixel format register setting
	 */

	template<class TAccessMode>
	inline uint16_t ILI9325Colour<COLOURS_16BIT,TAccessMode>::getInterfacePixelFormat() const {
		return 0x0000;
	}

	/**
	 * Unpack the colour from rrggbb to the internal 5-6-5 format

	 * 00000000RRRRRRRRGGGGGGGGBBBBBBBB ->
	 * 0000000000000000RRRRRGGGGGGBBBBB

	 * @param src rrggbb
	 * @param dest The unpacked colour structure
	 */

	template<class TAccessMode>
	inline void ILI9325Colour<COLOURS_16BIT,TAccessMode>::unpackColour(TColour src,UnpackedColour& dest) const {
		uint8_t r,g,b;

		r=src >> 16;
		g=src >> 8;
		b=src;

		r&=0xf8;
		g&=0xfc;
		b&=0xf8;

		dest.first=r | (g >> 5);
		dest.second=(g << 3) | (b >> 3);
	}

	/**
	 * Unpack the colour from components to the internal format
	 * @param red
	 * @param green
	 * @param blue
	 * @param dest
	 */

	template<class TAccessMode>
	inline void ILI9325Colour<COLOURS_16BIT,TAccessMode>::unpackColour(uint8_t red,uint8_t green,uint8_t blue,UnpackedColour& dest) const {
		red&=0xf8;
		green&=0xfc;
		blue&=0xf8;

		dest.first=red | (green >> 5);
		dest.second=(green << 3) | (blue >> 3);
	}

	/**
	 * Write a single pixel to the current output position.
	 * Assumes that the caller has already issued the beginWriting() command.
	 * @param cr The pixel to write
	 */

	template<class TAccessMode>
	inline void ILI9325Colour<COLOURS_16BIT,TAccessMode>::writePixel(const UnpackedColour& cr) const {
		TAccessMode::writeData(cr.first);
		TAccessMode::writeData(cr.second);
	}

	/**
	 * Fill a block of pixels with the same colour. This operation will issue the
	 * beginWriting() command for you.
	 * @param numPixels how many
	 * @param cr The unpacked colour to write
	 */

	template<class TAccessMode>
	inline void ILI9325Colour<COLOURS_16BIT,TAccessMode>::fillPixels(uint32_t numPixels,const UnpackedColour& cr) const {
		TAccessMode::writeCommand(ILI932X_RW_GRAM);

		uint8_t first=cr.first;
		uint8_t second=cr.second;

		while(numPixels--) {
			TAccessMode::writeData(first);
			TAccessMode::writeData(second);
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

	template<class TAccessMode>
	inline void ILI9325Colour<COLOURS_16BIT,TAccessMode>::allocatePixelBuffer(uint32_t numPixels,uint8_t*& buffer,uint32_t& bytesPerPixel) const {
		buffer=(uint8_t *)malloc(numPixels * 2);
		bytesPerPixel=2;
	}

	/**
	 * Get the number of bytes per pixel
	 * @return 2
	 */

	template<class TAccessMode>
	inline uint8_t ILI9325Colour<COLOURS_16BIT,TAccessMode>::getBytesPerPixel() const {
		return 2;
	}
}