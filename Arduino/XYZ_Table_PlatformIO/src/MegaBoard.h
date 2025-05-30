/**
 * ===============================================================
 *  MegaBoard.h
 *  XYZ Camera Positioning System - Board Abstraction Layer
 * ===============================================================
 *  Description:
 *  - Provides hardware abstraction for serial communication and system-level utilities.
 *  - Offers methods for initialization, version reporting, rebooting, and memory diagnostics.
 *  - Includes templated helpers for consistent serial output formatting.
 *
 *  Created on: 03/04/2020
 *  Author: Ignacio Martínez Navajas
 *  E-mail: imnavajas@coit.es
 * ===============================================================
 */

#ifndef MEGABOARD_H_
#define MEGABOARD_H_

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#include "Cmd.h"

#define BOARD_SERIAL CMD_SERIAL
#define BOARD_SERIAL_BAUDRATE 115200
#define SERIAL_EOL "\n"


class MegaBoard {
public:
	static void Begin(void);
	
	template <typename T>
    static void Print(const T& value) {
        BOARD_SERIAL.print(value);
    }
	
	template <typename T>
    static void Println(const T& value) {
        BOARD_SERIAL.print(value);
        BOARD_SERIAL.print(SERIAL_EOL);
		BOARD_SERIAL.print((char)0x3); // ETX for the server
    }

	static void Println() {
		BOARD_SERIAL.print(SERIAL_EOL);
	}

	static String toJSON(String key, String value);

	/* HW related functions */
	static void Version(void);
	static void Reboot(void);
	static uint32_t FreeRam(void);

private:

};

#endif /* MEGABOARD_H_ */
