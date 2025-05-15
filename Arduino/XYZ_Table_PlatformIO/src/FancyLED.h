/*
 FancyLED.h - - FancyLED library for Wiring/Arduino - Version 0.1

 Original library 		(0.1) by Carlyn Maw.

 */

// ensure this library description is only included once
#ifndef FancyLED_h
#define FancyLED_h

struct _led_t {
	uint8_t pinNumber;
	uint8_t defaultOnState;	//HIGH == pressed (1) or LOW == pressed (0)
	uint8_t state;	//HIGH == pressed (1) or LOW == pressed (0)
	// Pulsed Mode
	bool pulseMode;
	bool pulseForever;
	uint8_t pulseFSM;
	uint16_t pulsePeriod;
	uint8_t pulseDutyCycle;
	uint16_t pulseOnTime;
	uint16_t pulseOffTime;
	uint32_t pulseTimer;
	uint32_t pulseDelayedStart;
	uint8_t pulseCnt;
	uint8_t pulseCntMax;
};

class FancyLED {
public:
	// Constructors:
	FancyLED(uint8_t rLedPin, bool rDefaultOnState);
	FancyLED();

	void Begin(void);

	void SetCurrentTime(uint32_t rCurrentTime);
	void Update(uint32_t rCurrentTime);
	void Loop(void);

	int GetState(void);

	void TurnOn(void);
	void TurnOff(void);
	void Toggle(void);

	void PulseOneTime(void);
	void PulseNTimes(uint8_t rPulseRepetitions);
	void PulseForever(void);
	void StopPulses(void);

	void DelayedPulseNTimes(uint32_t rInitialDelay, uint8_t rPulseRepetitions);

	uint8_t GetDutyCycle(void);
	void SetLedPulseDutyCycle(uint8_t rDutyCycle);

	long GetLedPulsePeriod(void);
	void SetLedPulsePeriod(uint32_t rPulsePeriod);
private:
	uint32_t aLedCurrentTime;
	_led_t aLed;
	void Pulse(uint8_t rPulseRepetitions);
	void UpdateLedState(bool rLedStatus);
};
#endif

