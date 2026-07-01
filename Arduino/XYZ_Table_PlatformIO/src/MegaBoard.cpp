#include "MegaBoard.h"

#define FS(x) (__FlashStringHelper *)(x)

const char APP_NAME[]   PROGMEM = "XYZ-Table";
const char FW_VERSION[] PROGMEM = "v1.0.0";

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
    asm volatile("  jmp 0");
}

String MegaBoard::toJSON(String key, String value)
{
    return "{\"" + key + "\":" + value + "}";
}

uint32_t MegaBoard::FreeRam()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}
