#ifndef TROWASOFT_UTILITIES_HPP
#define TROWASOFT_UTILITIES_HPP

#include "math.hpp"

#define TROWA_DISP_MSG_SIZE		30
#define TROWASEQ_NUM_PATTERNS	16
#define TROWASEQ_PATTERN_MIN_V	-10   // Min voltage input / output for controlling pattern index
#define TROWASEQ_PATTERN_MAX_V	 10   // Max voltage input / output for controlling pattern index
#define TROWASEQ_NUM_NOTES			12 // Num notes per octave
#define TROWASEQ_ZERO_OCTAVE		5  // Octave for voltage 0

static const char * TROWA_NOTES[TROWASEQ_NUM_NOTES] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

// Given some input voltage, convert to our Pattern index (0-15).
static inline int VoltsToPattern(float voltsInput)
{	
	return clampi(roundf(rescalef(voltsInput, TROWASEQ_PATTERN_MIN_V, TROWASEQ_PATTERN_MAX_V, 1, TROWASEQ_NUM_PATTERNS)), 1, TROWASEQ_NUM_PATTERNS);
	//return (voltsInput - TROWASEQ_PATTERN_MIN_V) / (TROWASEQ_PATTERN_RANGE_V) *  (TROWASEQ_NUM_PATTERNS - 1);
}
// Pattern index (0-15) to output voltage.
static inline float PatternToVolts(int patternIx)
{
	return rescalef(patternIx + 1, 1, TROWASEQ_NUM_PATTERNS, TROWASEQ_PATTERN_MIN_V, TROWASEQ_PATTERN_MAX_V);
}
// Octave 0 to 10
static inline int VoltsToOctave(float v)
{
	return (int)(v + TROWASEQ_ZERO_OCTAVE);
}
// Note index 0 to 11
static inline int VoltsToNoteIx(float v)
{
	// This doesn't work all the time.
	//(v - floorf(v))*TROWASEQ_NUM_NOTES
	// (-4.9 - -5) * 12 = 0.1*12 = int(1.2) = 1 [C#]
	// (-0.33 - -1) * 12 = 0.67*12 = int(8.04) = 8 [G#]
	return (int)(round((v + TROWASEQ_ZERO_OCTAVE)*TROWASEQ_NUM_NOTES)) % TROWASEQ_NUM_NOTES;
}


struct ValueSequencerMode
{
	// Min value in voltage
	float voltageMin;
	// Max value in voltage
	float voltageMax;
	// Force whole / integer values 
	bool wholeNumbersOnly;
	
	// The angle that represents 0 radians.
	float zeroPointAngle_radians;
	// Output voltage 
	float outputVoltageMin;
	float outputVoltageMax;
	
	// Min value (what it means)
	float minDisplayValue;
	// Max value (what it means)
	float maxDisplayValue;
	
	bool needsTranslationDisplay;
	bool needsTranslationOutput;
	
	float roundNearestDisplay = 0;
	float roundNearestOutput = 0;
	// Format string for the display value
	const char * displayFormatString;
	// The display name.
	const char * displayName;
	
	float zeroValue;
	
	ValueSequencerMode()
	{
		return;
	}
	
	ValueSequencerMode(const char* displayName, float minDisplayValue, float maxDisplayValue, float min_V, float max_V, 
		float outVoltageMin, float outVoltageMax,
		bool wholeNumbersOnly, float zeroPointAngle, const char * formatStr,
		float roundDisplay, float roundOutput, float zeroValue)
	{
		this->displayName = displayName;
		this->displayFormatString = formatStr;
		this->minDisplayValue = minDisplayValue; // I.e. 0
		this->maxDisplayValue = maxDisplayValue; // I.e. 15 
		this->voltageMin = min_V;  // I.e. -10 Volts
		this->voltageMax = max_V;  // I.e. +10 Volts
		this->outputVoltageMin = outVoltageMin;
		this->outputVoltageMax = outVoltageMax;
		this->wholeNumbersOnly = wholeNumbersOnly; // Force whole numbers
		this->zeroPointAngle_radians = zeroPointAngle;
		this->roundNearestDisplay = roundDisplay;
		this->roundNearestOutput = roundOutput;
		this->zeroValue = zeroValue;
		
		needsTranslationDisplay = minDisplayValue != voltageMin || maxDisplayValue != voltageMax;
		needsTranslationOutput = outputVoltageMin != voltageMin || outputVoltageMax != voltageMax;
		return;
	}
	
	virtual void GetDisplayString(float val, char* buffer)
	{
		float dVal = val;
		if (needsTranslationDisplay)
		{
			dVal = rescalef(val, voltageMin, voltageMax, minDisplayValue, maxDisplayValue);
		}
		if (roundNearestDisplay > 0)
		{
			dVal = static_cast<int>(dVal  / roundNearestDisplay) * roundNearestDisplay;
		}
		sprintf(buffer, displayFormatString, dVal);	
		return;
	}
	
	virtual float GetOutputValue(float val)
	{
		float oVal = val;
		if (needsTranslationOutput)
		{
			oVal = rescalef(val, voltageMin, voltageMax, outputVoltageMin, outputVoltageMax);
		}
		if (roundNearestOutput > 0)
		{ // Round this
			oVal = static_cast<int>(round(oVal  / roundNearestOutput)) * roundNearestOutput;
		}
		return oVal;
	}
};
struct NoteValueSequencerMode : ValueSequencerMode
{
	NoteValueSequencerMode(const char* displayName,
		float min_V, float max_V)
	{
		this->displayName = displayName;
		//this->displayFormatString = formatStr;
		//this->minDisplayValue = minDisplayValue; // I.e. 0
		//this->maxDisplayValue = maxDisplayValue; // I.e. 15 
		
		this->minDisplayValue = -TROWASEQ_ZERO_OCTAVE;
		this->maxDisplayValue = TROWASEQ_ZERO_OCTAVE; // Not really used	
	
		this->voltageMin = min_V;  // I.e. -10 Volts
		this->voltageMax = max_V;  // I.e. +10 Volts
		
		this->outputVoltageMin = -TROWASEQ_ZERO_OCTAVE;
		this->outputVoltageMax = TROWASEQ_ZERO_OCTAVE;
		
		this->wholeNumbersOnly = false; // Force whole numbers
		this->zeroPointAngle_radians = 1.5*NVG_PI; // Straight up
		this->roundNearestDisplay = 1.0/TROWASEQ_NUM_NOTES;
		this->roundNearestOutput = 1.0/TROWASEQ_NUM_NOTES;
		this->zeroValue = (max_V+min_V) /2.0;
		
		needsTranslationDisplay = minDisplayValue != voltageMin || maxDisplayValue != voltageMax;
		needsTranslationOutput = outputVoltageMin != voltageMin || outputVoltageMax != voltageMax;
		return;
	}
	// Overriden display string to show notes instead of output voltage values.
	void GetDisplayString(float val, char* buffer) override
	{
		int octave = VoltsToOctave(val);
		int noteIx = VoltsToNoteIx(val);
		if (noteIx > TROWASEQ_NUM_NOTES - 1)
			noteIx = TROWASEQ_NUM_NOTES - 1;
		else if (noteIx < 0)
		{
			noteIx = 0;
		}
		sprintf(buffer, "%s%d", TROWA_NOTES[noteIx], octave);
		return;
	}
};


#endif // end if not defined