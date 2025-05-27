/**
 * ===============================================================
 *  MegaBoard.cpp
 *  XYZ Camera Positioning System - Board Abstraction Layer
 * ===============================================================
 *  Description:
 *  - Implements low-level board operations such as reboot, free RAM checking, and version display.
 *  - Initializes the main serial interface used for CLI and debugging.
 *  - Includes helper for formatting simple JSON key-value responses.
 *
 *  Created on: 03/04/2020
 *  Author: Ignacio Martínez Navajas
 *  E-mail: imnavajas@coit.es
 * ===============================================================
 */

 #include "MegaBoard.h"

#define FS(x) (__FlashStringHelper *)(x)

/* Initialize static constants and variables */
const char APP_NAME[] PROGMEM = "XYZ-Table";
const char FW_VERSION[] PROGMEM = "v1.0";

void MegaBoard::Begin(void)
{
	BOARD_SERIAL.begin(BOARD_SERIAL_BAUDRATE);
}

void MegaBoard::Version(void)
{
	MegaBoard::Print(FS(APP_NAME));
	MegaBoard::Print(F("_"));
	MegaBoard::Println(FS(FW_VERSION));
}

void MegaBoard::Reboot(void)
{
	// Reboot sequence (http://forum.arduino.cc/index.php?topic=49581.0)
	asm volatile("  jmp 0");
}

String MegaBoard::toJSON(String key, String value)
{
	return "{\"" + key + "\":" + value + "}";
}

/* Private methods */
// Function to check the available RAM memory
uint32_t MegaBoard::FreeRam()
{
	extern int __heap_start, *__brkval;
	int v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}
