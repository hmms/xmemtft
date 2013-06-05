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
	 * JPEG decoder
	 */

	template<class TAccessMode>
	class JpegDecoder {

		typedef typename tLCD::template UnpackedColour<TColourDepth> UNPACKED_COLOUR;

		public:
			static void decode(const Point& pt,JpegDataSource& ds) {

				pjpeg_image_info_t imageInfo;
				UNPACKED_COLOUR cr;

				// initialise the decoder

				if(pjpeg_decode_init(&imageInfo,ds)!=0)
					return;

				int16_t mcu_x=0,mcu_y=0;

				for(;;) {

					if(pjpeg_decode_mcu()!=0)
						return;

					if(mcu_y>=imageInfo.m_MCUSPerCol)
						return;

					for(int y=0;y<imageInfo.m_MCUHeight;y+=8) {

						int by_limit=min(8,imageInfo.m_height-(mcu_y*imageInfo.m_MCUHeight+y));

						for(int x=0;x<imageInfo.m_MCUWidth;x+=8) {

							uint16_t src_ofs=(x*8U)+(y*16U);

							uint8_t *pSrcR=imageInfo.m_pMCUBufR+src_ofs;
							uint8_t *pSrcG=imageInfo.m_pMCUBufG+src_ofs;
							uint8_t *pSrcB=imageInfo.m_pMCUBufB+src_ofs;

							int bx_limit=min(8,imageInfo.m_width-(mcu_x*imageInfo.m_MCUWidth+x));

							tLCD::template moveTo<TOrientation>(
									Rectangle(pt.X+mcu_x*imageInfo.m_MCUWidth,pt.Y+mcu_y*imageInfo.m_MCUHeight,bx_limit,by_limit));

							tLCD::beginWriting();

							if(imageInfo.m_scanType==PJPG_GRAYSCALE) {

								for(int by=0;by<by_limit;by++) {

									for(int bx=0;bx < bx_limit;bx++) {
										tLCD::template unpackColourParts(*pSrcR,*pSrcR,*pSrcR,cr);
										tLCD::template writePixel(cr);
										pSrcR++;
									}

									pSrcR+=(8-bx_limit);
								}
							} else {
								for(int by=0;by<by_limit;by++) {

									for(int bx=0;bx<bx_limit;bx++) {

										tLCD::template unpackColourParts(*pSrcR,*pSrcG,*pSrcB,cr);

										pSrcR++;
										pSrcG++;
										pSrcB++;

										tLCD::writePixel(cr);
									}

									pSrcR+=(8-bx_limit);
									pSrcG+=(8-bx_limit);
									pSrcB+=(8-bx_limit);
								}
							}
						}
					}

					mcu_x++;

					if(mcu_x==imageInfo.m_MCUSPerRow) {
						mcu_x=0;
						mcu_y++;
					}
				}
			}
	};
}
