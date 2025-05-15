/*
 * EepromUtil.h
 *
 *  Created on: 03/04/2020
 *      Author: Ignacio Martínez Navajas
 *      E-mail: imnavajas@coit.es
 */

#ifndef EEPROMUTIL_H_
#define EEPROMUTIL_H_

#include <Arduino.h>
#include <EEPROM.h>

class EepromUtil {
public:
	static void eeprom_erase_all();
	static boolean eeprom_write_bytes(int startAddr, const byte* array,
			int numBytes);
	static boolean eeprom_write_string(int addr, const char* string);
	static boolean eeprom_read_string(int addr, char* buffer, int bufSize);
	static boolean eeprom_write_string(int Addr, String input);
	static String eeprom_read_string(int Addr, int length);

private:
	static boolean eeprom_is_addr_ok(int addr);
};
#endif /* EEPROMUTIL_H_ */
