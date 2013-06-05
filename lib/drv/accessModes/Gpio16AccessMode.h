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

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)


namespace lcd {


	/**
	 * Pin and port mappings for using the Gpio16AccessMode against the same pins that
	 * XMEM on the Arduino Mega would use. Here they are:
	 *
	 *  +---------+------+----------+
	 *  | Arduino | Port | Function |
	 *  +---------+------+----------+
	 *  | 22      | PA0  | D0/D8    |
	 *  | 23      | PA1	 | D1/D9    |
	 *  | 24      | PA2  | D2/D10   |
	 *  | 25      | PA3  | D3/D11   |
	 *  | 26      | PA4  | D4/D12   |
	 *  | 27      | PA5  | D5/D13   |
	 *  | 28      | PA6  | D6/D14   |
	 *  | 29      | PA7  | D7/D15   |
	 *  | 35      | PC2  | /RESET   |
	 *  | 37      | PC0  | RS       |
	 *  | 39      | PG2  | ALE      |
	 *  | 41      | PG0  | /WR      |
	 *  +---------+------+----------+
	 */

	struct Gpio16AccessModeXmemMapping {
		enum {

			// ports are the I/O index, not the physical address

			PORT_DATA  = 0x02,		// PORTA
			PORT_WR    = 0x14,		// PORTG
			PORT_RS    = 0x08,		// PORTC
			PORT_ALE   = 0x14,		// PORTG
			PORT_RESET = 0x08,		// PORTC

			// pins are the 0..7 port index, not the arduino numbers

			PIN_WR    = PIN0,
			PIN_RS    = PIN0,
			PIN_ALE   = PIN2,
			PIN_RESET = PIN2
		};
	};


	/**
	 * Access mode template for a GPIO-based LCD interface that uses an external latch to reduce
	 * the number of required pins by 7. An entire 8-bit port is eliminated at the expense
	 * of one additional pin for enabling the latch (ALE). See my website for full details
	 * and a write-up of how this interface works. 12 pins are required, of which 8 of them must
	 * be a full port for the data.
	 *
	 * This template includes a highly optimised assembly language multiple-pixel writer that can
	 * be used in 16-bit colour mode to push same-colour pixels out to the LCD at close to 8Mhz on
	 * a 16Mhz Arduino. When clearing a QVGA display the optimised version achieves 10.8ms whereas
	 * a tight loop around the single pixel writer achieves 100ms.
	 *
	 * I include driver typedefs that use the same pins as the XMEM interface for controlling the
	 * LCD. It should be trivial for you to work out how to supply your own pin mappings if your
	 * project has pin limitations.
	 *
	 * Finally, note that the optimised pixel-pusher ensures that global interrupts are off while
	 * it's running because it cannot have an IRQ handler modify any pin on the port where /WR lives
	 * while it's running. If you know this will never happen then you can remove that part of the
	 * assembly language to increase concurrency at the expense of worse pixel throughput if an
	 * IRQ goes off while it's running.
	 */

	template<typename TPinMappings>
	class Gpio16AccessMode {

		protected:
			static uint8_t _streamIndex;
			static void initOutputHigh(uint8_t port,uint8_t pin);

		public:
			static void initialise();
			static void hardReset();

			static void writeCommand(uint8_t lo8,uint8_t hi8=0);
			static void writeCommandData(uint8_t cmd,uint8_t data);
			static void writeData(uint8_t lo8,uint8_t hi8=0);
			static void writeMultiData(uint32_t howMuch,uint8_t lo8,uint8_t hi8=0);
			static void writeStreamedData(uint8_t data);
	};


	/**
	 * Initialise the static member
	 */

	template <typename TPinMappings>
	uint8_t Gpio16AccessMode<TPinMappings>::_streamIndex=0;


	/**
	 * Shortcut to write an 8-bit command and an 8-bit data parameter. This is a common scenario
	 * in programming the registers
	 * @param cmd The 8-bit command
	 * @param data The 8-bit data value
	 */

	template<class TPinMappings>
	__attribute__((always_inline))
	inline void Gpio16AccessMode<TPinMappings>::writeCommandData(uint8_t cmd,uint8_t data) {

		writeCommand(cmd,0);
		writeData(data,0);
	}


	/**
	 * Write a command to the XMEM interface
	 * @param lo8 The low 8 bits of the command to write
	 * @param hi8 The high 8 bits of the command to write. Many commands are 8-bits so this parameters defaults to zero.
	 */

	template<class TPinMappings>
	inline void Gpio16AccessMode<TPinMappings>::writeCommand(uint8_t lo8,uint8_t hi8) {

	  __asm volatile(
	  		"  sbi %1, %5   \n\t"			// ALE   = HIGH
				"  out %3, %7   \n\t"			// PORTA = lo8
				"  cbi %2, %6   \n\t"			// RS    = LOW
				"  cbi %1, %5   \n\t"			// ALE   = LOW
				"  out %3, %8   \n\t"  		// PORTA = hi8
				"  cbi %0, %6   \n\t"			// /WR   = LOW
				"  sbi %0, %6   \n\t"			// /WR   = HIGH

				:: "I" (TPinMappings::PORT_WR),			// %0
				   "I" (TPinMappings::PORT_ALE),		// %1
				   "I" (TPinMappings::PORT_RS),			// %2
				   "I" (TPinMappings::PORT_DATA),		// %3
				   "I" (TPinMappings::PIN_WR),			// %4
				   "I" (TPinMappings::PIN_ALE),			// %5
				   "I" (TPinMappings::PIN_RS),		  // %6
				   "r" (lo8),												// %7
				   "r" (hi8)												// %8
				  );
	}


	/**
	 * This is used by the LZG decompressor. Since the output is a byte stream and the decompressor doesn't
	 * know the colour depth of the target panel we use this 'stream' call that buffers incoming data until
	 * it's got a complete 'unit' to write to the interface. In this case we do half a transaction when the
	 * first byte arrives and finish it off when the second one shows up
	 * @param data The 8-bits
	 */

	template<class TPinMappings>
	inline void Gpio16AccessMode<TPinMappings>::writeStreamedData(uint8_t data) {

		if(_streamIndex==0) {

			__asm volatile(
	  		"  sbi %0, %3   \n\t"			// ALE   = HIGH
				"  out %2, %5   \n\t"			// PORTA = data
				"  sbi %1, %4   \n\t"			// RS    = HIGH

				:: "I" (TPinMappings::PORT_ALE),		// %0
				   "I" (TPinMappings::PORT_RS),			// %1
				   "I" (TPinMappings::PORT_DATA),		// %2
				   "I" (TPinMappings::PIN_ALE),			// %3
				   "I" (TPinMappings::PIN_RS),		  // %4
				   "r" (data)												// %5
				  );
		}
		else {
			__asm volatile(
				"  cbi %1, %4   \n\t"			// ALE   = LOW
				"  out %2, %5   \n\t"  		// PORTA = data
				"  cbi %0, %3   \n\t"			// /WR   = LOW
				"  sbi %0, %3   \n\t"			// /WR   = HIGH

				:: "I" (TPinMappings::PORT_WR),			// %0
				   "I" (TPinMappings::PORT_ALE),		// %1
				   "I" (TPinMappings::PORT_DATA),		// %2
				   "I" (TPinMappings::PIN_WR),			// %3
				   "I" (TPinMappings::PIN_ALE),			// %4
				   "r" (data)												// %5
				  );
		}

		// flip the position indicator

		_streamIndex^=1;
	}


	/**
	 * Write a data value to the XMEM interface
	 * @param lo8 The low 8 bits of the value to write
	 * @param hi8 The high 8 bits of the value to write. Many parameter values are 8-bits so this parameters defaults to zero.
	 */

	template<class TPinMappings>
	inline void Gpio16AccessMode<TPinMappings>::writeData(uint8_t lo8,uint8_t hi8) {

	  __asm volatile(
	  		"  sbi %1, %5   \n\t"			// ALE   = HIGH
				"  out %3, %7   \n\t"			// PORTA = lo8
				"  sbi %2, %6   \n\t"			// RS    = HIGH
				"  cbi %1, %5   \n\t"			// ALE   = LOW
				"  out %3, %8   \n\t"  		// PORTA = hi8
				"  cbi %0, %6   \n\t"			// /WR   = LOW
				"  sbi %0, %6   \n\t"			// /WR   = HIGH

				:: "I" (TPinMappings::PORT_WR),			// %0
				   "I" (TPinMappings::PORT_ALE),		// %1
				   "I" (TPinMappings::PORT_RS),			// %2
				   "I" (TPinMappings::PORT_DATA),		// %3
				   "I" (TPinMappings::PIN_WR),			// %4
				   "I" (TPinMappings::PIN_ALE),			// %5
				   "I" (TPinMappings::PIN_RS),		  // %6
				   "r" (lo8),												// %7
				   "r" (hi8)												// %8
				  );
	}


	/**
	 * Write a batch of the same data values to the XMEM interface using GPIO.
	 * @param howMuch The number of 16-bit values to write
	 * @param lo8 The low 8 bits of the value to write
	 * @param hi8 The high 8 bits of the value to write. Many parameter values are 8-bits so this parameters defaults to zero.
	 */

	template<class TPinMappings>
	inline void Gpio16AccessMode<TPinMappings>::writeMultiData(uint32_t howMuch,uint8_t lo8,uint8_t hi8) {

		__asm volatile(
				"	   sbi  %9, %7       \n\t"			// ALE   = HIGH
		    "    out  %1, %2       \n\t"			// PORTA = lo8
				"    sbi  %3, %8       \n\t"			// RS    = HIGH
		    "    cbi  %9, %7       \n\t"      // ALE   = LOW
		    "    out  %1, %4       \n\t"			// PORTA = hi8

				"    clr  r20          \n\t"			// r20 = global interrupt status
				"    brid intdis       \n\t"			// branch if global interrupts are off
				"    cli               \n\t"			// disable global interrupts (because we cannot let PORTG get modified by an IRQ)
				"    inc  r20          \n\t"			// r20 (global interrupts disabled) = 1
				"intdis:               \n\t"
				"    in   r18, %0      \n\t"			// get PORT_WR to r18 and r19
				"    mov  r19, r18     \n\t"
				"    cbr  r18, %6		   \n\t"			// clear WR in r18
				"    sbr  r19, %6      \n\t"			// set WR in r19
				"    cpi  %A5, 40      \n\t"			// if howMuch<40 then jump to lastlot
				"    cpc  %B5, r1      \n\t"
				"    cpc  %C5, r1      \n\t"
				"    brsh batchloop    \n\t"
				"    rjmp lastlot      \n\t"
				"batchloop:            \n\t"
				"    out  %0,  r18     \n\t"			// toggle /WR 40 times
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    subi %A5, 40      \n\t"				// subtract 40 from howMuch
				"    sbci %B5, 0       \n\t"
				"    sbci %C5, 0       \n\t"
				"    cpi  %A5, 40      \n\t"				// if howMuch >= 40 then go back for another batch
				"    cpc  %B5, r1      \n\t"
				"    cpc  %C5, r1      \n\t"
				"    brlo lastlot      \n\t"
				"    rjmp batchloop    \n\t"
				"lastlot:              \n\t"				// load index Z with the address of the end
				"    ldi  r31, pm_hi8(finished)   \n\t"
				"    ldi  r30, pm_lo8(finished)   \n\t"
				"    lsl  %A5          \n\t"				// multiply remaining by 2
				"    sub  r30, %A5     \n\t"				// subtract remaining*4 from Z
				"    sbci r31, 0       \n\t"
				"    ijmp              \n\t"				// jump to Z to finish off the writing
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"    out  %0,  r18     \n\t"
				"    out  %0,  r19     \n\t"
				"finished:             \n\t"
				"    cpi  r20, 0       \n\t"					// if global interrupts were enabled when we came in, restore them now
				"    breq skipinten    \n\t"
				"    sei               \n\t"
				"skipinten:            \n\t"

				:: "I" (TPinMappings::PORT_WR),			// %0
				   "I" (TPinMappings::PORT_DATA),		// %1
				   "r" (lo8),												// %2
				   "I" (TPinMappings::PORT_RS),			// %3
				   "r" (hi8),												// %4
				   "r" (howMuch),										// %C5,%B5,%A5
				   "I" (_BV(TPinMappings::PIN_WR)),	// %6
				   "I" (TPinMappings::PIN_ALE),			// %7
				   "I" (TPinMappings::PIN_RS),		  // %8
				   "I" (TPinMappings::PORT_ALE)		  // %9
				: "r18", "r19", "r20", "r30", "r31"
		);
	}


	/**
	 * Set up the pins for GPIO
	 */

	template<class TPinMappings>
	inline void Gpio16AccessMode<TPinMappings>::initialise() {

		// reset pin

		initOutputHigh(TPinMappings::PORT_RESET,TPinMappings::PIN_RESET);

		// control pins

		initOutputHigh(TPinMappings::PORT_RS,TPinMappings::PIN_RS);
		initOutputHigh(TPinMappings::PORT_ALE,TPinMappings::PIN_ALE);
		initOutputHigh(TPinMappings::PORT_WR,TPinMappings::PIN_WR);

		// data port

		*reinterpret_cast<volatile uint8_t *>(__SFR_OFFSET+TPinMappings::PORT_DATA-1)=0xff;
	}


	/**
	 * Initialise a pin to output and set it high
	 * @param port port I/O index
	 * @param pin pin 0..7 number in the port
	 */

	template<class TPinMappings>
	inline void Gpio16AccessMode<TPinMappings>::initOutputHigh(uint8_t port,uint8_t pin) {

	  // DDR is always at port address-1

		*reinterpret_cast<volatile uint8_t *>(__SFR_OFFSET+port-1)|=_BV(pin);

		// set the pin high on the port

		*reinterpret_cast<volatile uint8_t *>(__SFR_OFFSET+port)|=_BV(pin);
	}



	/**
	 * Perform a hard reset
	 */

	template<class TPinMappings>
	inline void Gpio16AccessMode<TPinMappings>::hardReset() {

		// pull reset low for 10ms

		*reinterpret_cast<volatile uint8_t *>(__SFR_OFFSET+TPinMappings::PORT_RESET)&=~_BV(TPinMappings::PIN_RESET);
		delay(10);
		*reinterpret_cast<volatile uint8_t *>(__SFR_OFFSET+TPinMappings::PORT_RESET)|=_BV(TPinMappings::PIN_RESET);
		delay(130);
	}


	/**
	 * Typedefs for easier access
	 */

	typedef Gpio16AccessMode<Gpio16AccessModeXmemMapping> DefaultMegaGpio16AccessMode;
}


#endif