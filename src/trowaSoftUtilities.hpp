#ifndef TROWASOFT_UTILITIES_HPP
#define TROWASOFT_UTILITIES_HPP

#include <rack.hpp>
using namespace rack;

#include <string.h>
#include <vector>
#include <sstream> // std::istringstream
#include <algorithm> // for tolower() of string
#include <functional> 
#include <cctype>
#include <locale>


#include "math.hpp"

#define TROWA_DEBUG_LVL_HIGH		100
#define TROWA_DEBUG_LVL_MED			 50
#define TROWA_DEBUG_LVL_LOW			  1
#define TROWA_DEBUG_LVL_OFF			  0
#define TROWA_DEBUG_MSGS		TROWA_DEBUG_LVL_OFF

#define TROWA_PULSE_WIDTH			(1e-3)
#define TROWA_HORIZ_MARGIN			13	// Margin for element layout in Module widget
#define TROWA_VERT_MARGIN			13  // Margin for element layout

#define TROWA_INDEX_UNDEFINED		 -1 // Value for undefined index.
#define TROWA_DISP_MSG_SIZE			 30 // For local buffers of strings
#define TROWA_SEQ_NUM_PATTERNS		 64 // Number of patterns for sequencers.
#define TROWA_SEQ_PATTERN_MIN_V		-10 // Min voltage input / output for controlling pattern index and BPM
#define TROWA_SEQ_PATTERN_MAX_V	 	 10 // Max voltage input / output for controlling pattern index and BPM
#define TROWA_SEQ_NUM_NOTES			 12 // Num notes per octave (1 V per octave)
#define TROWA_SEQ_NOTES_MIN_V		-5 // OK, so back to -5 to +5V (from -4 to +6V), we'll just have a -1 Octave since apparently -1 Octave is a thing (MIDI 0-11)?
#define TROWA_SEQ_NOTES_MAX_V		 5 // OK, so back to -5 to +5V (from -4 to +6V), we'll just have a -1 Octave since apparently -1 Octave is a thing (MIDI 0-11)?
#define TROWA_SEQ_ZERO_OCTAVE		  4 // Octave for voltage 0 -- Was 5, now 4
#define TROWA_SEQ_NUM_OCTAVES	     10 // Number of total octaves
#define TROWA_SEQ_MIN_OCTAVE	((TROWA_SEQ_ZERO_OCTAVE - TROWA_SEQ_NUM_OCTAVES/2)) // Min octave (-1)
#define TROWA_SEQ_MAX_OCTAVE	((TROWA_SEQ_ZERO_OCTAVE + TROWA_SEQ_NUM_OCTAVES/2)) // Max octave (9)
#define TROWA_MIDI_NOTE_MIDDLE_C	 60 // MIDI note (middle C) - should correspond to C4 (Voltage 0)

#define TROWA_NUM_GLOBAL_EFFECTS	11

#define TROWA_ANGLE_STRAIGHT_UP_RADIANS			(1.5*NVG_PI) // Angle for straight up (svg angles start from positive x and go clockwise)
#define TROWA_ANGLE_STRAIGHT_DOWN_RADIANS		(0.5*NVG_PI) // Angle for straiclamght down

#define TROWA_BASE_FREQUENCY	261.626f // Base frequency for C4 (Voltage 0)


// Fonts:
#define TROWA_DIGITAL_FONT		"res/Fonts/Digital dream Fat.ttf"
#define TROWA_LABEL_FONT		"res/Fonts/ZeroesThree-Regular.ttf"
#define TROWA_MONOSPACE_FONT	"res/Fonts/larabieb.ttf"
#define TROWA_MATH_FONT			"res/Fonts/Math Symbols Normal.ttf"


// The draw layer (for drawLayer() in v2)
enum DrawLayer {
	ModuleWidgetShadowLayer = -1,
	DeprecatedDrawLayer = 0,
	LightLayer = 1,
	PlugsCablesLayer = 2,
	CableOutlineLayer = 3,
	CustomerLayerStart = 1000
};

// Signals (Replace dsp::SchmittTrigger::HIGH, LOW).
enum TriggerSignal
{
	LOW = 0,
	HIGH = 1
};

extern const char * TROWA_NOTES[TROWA_SEQ_NUM_NOTES]; // Our note labels.

// Given some input voltage, convert to our Pattern [1-64].
// [2018-11-20] Remove rounding so that each pattern is ~0.32 V.
inline int VoltsToPattern(float voltsInput)
{	
	// (float x, float xMin, float xMax, float yMin, float yMax)	
	return (int)clamp((int)(rescale(voltsInput, (float)TROWA_SEQ_PATTERN_MIN_V, (float)TROWA_SEQ_PATTERN_MAX_V, 1.0f, (float)TROWA_SEQ_NUM_PATTERNS)), 1, TROWA_SEQ_NUM_PATTERNS);
}
// Pattern index [0-63] to output voltage.
inline float PatternToVolts(int patternIx)
{
	return rescale(static_cast<float>(patternIx + 1),  1.0f, (float)(TROWA_SEQ_NUM_PATTERNS), (float)(TROWA_SEQ_PATTERN_MIN_V), (float)(TROWA_SEQ_PATTERN_MAX_V));
}
// Voltage [-5 to 5] to Octave -1 to 9
inline int VoltsToOctaveOld(float v)
{
	return (int)floorf(v + TROWA_SEQ_ZERO_OCTAVE);
}
inline int VoltsToOctave(float v)
{
	const float adj = 0.05f / TROWA_SEQ_NUM_NOTES;
	// Sometimes the rounding error makes this wrong add some extra
	return (int)floorf(v + TROWA_SEQ_ZERO_OCTAVE + adj);
}

// Octave -1 to 9 to Volts [-5 to 5]
inline float OctaveToVolts(int octave)
{
	return (float)(octave - TROWA_SEQ_ZERO_OCTAVE);
}
// Voltage to Note index 0 to 11 (to TROWA_NOTES array).
inline int VoltsToNoteIx(float v)
{
	// This doesn't work all the time.
	//(v - floorf(v))*TROWA_SEQ_NUM_NOTES
	// (-4.9 - -5) * 12 = 0.1*12 = int(1.2) = 1 [C#]
	// (-0.33 - -1) * 12 = 0.67*12 = int(8.04) = 8 [G#]
	// Insure that the result is positive... (add max voltage instead of zero octave)
	return (int)(round((v + TROWA_SEQ_PATTERN_MAX_V)*TROWA_SEQ_NUM_NOTES)) % TROWA_SEQ_NUM_NOTES;
}
// Note index 0 to 11 to Voltage
inline float NoteIxToVolts(int noteIx)
{
	return (float)(noteIx) / (float)(TROWA_SEQ_NUM_NOTES);
}
// Voltage to frequency (Hz).
inline float VoltageToFrequency(float v)
{
	return TROWA_BASE_FREQUENCY * powf(2.0f, v);
}
// Frequency (Hz) to Voltage.
inline float Frequency2Voltage(float f)
{
	return log2f(f / TROWA_BASE_FREQUENCY);
}

// Floating point hue [0-1.0] to color.
NVGcolor inline HueToColor(float hue)
{
	return nvgHSLA(hue, 1.0, 0.5, /*alpha 0-255*/ 0xff);
}
// Floating point hue [0-1.0] to color.
NVGcolor inline HueToColor(float hue, float sat, float light)
{
	return nvgHSLA(hue, sat, light, /*alpha 0-255*/ 0xff);
}

// Floating point hue [0-1.0] to color for our color gradient.
NVGcolor inline HueToColorGradient(float hue)
{
	return nvgHSLA(hue, 1.0, 0.5, /*alpha 0-255*/ 0xff);
}
NVGcolor inline ColorInvertToNegative(NVGcolor color)
{ // Keep alpha the same.
	return nvgRGBAf(1.0 - color.r, 1.0 - color.g, 1.0 - color.b, color.a);
}

// Split a string
std::vector<std::string> str_split(const std::string& s, char delimiter);

// Left trim (in place)
static inline void ltrim(std::string &s) {
    //s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {return !std::isspace(c); }));
}
// Right trim (in place)
static inline void rtrim(std::string &s) {
    //s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch);}).base(), s.end());
}
// Trim in place.
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}


struct TSColorHSL {
	union {
		float hsl[3];
		struct {
			float h, s, lum;
		};
	};
};
typedef struct TSColorHSL TSColorHSL;

namespace trowaSoft
{
	void TSColorToHSL(NVGcolor color, TSColorHSL* hsv);
}


struct GlobalEffect {
	NVGcompositeOperation compositeOperation = NVG_SOURCE_OVER;
	const char* label;
	GlobalEffect(const char* label, NVGcompositeOperation compositeOperation)
	{
		this->label = label;
		this->compositeOperation = compositeOperation;
		return;
	}
};



//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// ValueSequencerMode
// Information and methods for translating knob input voltages to output voltages
// and for display strings.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
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
	// If this amounts to just on/off.
	bool isBoolean = false;
	
	bool needsTranslationDisplay;
	bool needsTranslationOutput;
	
	float roundNearestDisplay = 0;
	float roundNearestOutput = 0;
	// If the display value should be integer
	bool displayIsInt = false;
	// Format string for the display value
	const char * displayFormatString;
	// The display name.
	const char * displayName;
	// The unit.
	const char * unit;
	
	float zeroValue = 0.0f;
	
	float snapValue = 0.0f;
	// True for boolean
	const char* bDisplayTrueStr = "On";
	// False for boolean
	const char* bDisplayFalseStr = "Off";
	
	ValueSequencerMode()
	{
		return;
	}
	
	ValueSequencerMode(const char* displayName, const char* unit, float minDisplayValue, float maxDisplayValue, float min_V, float max_V, 
		float outVoltageMin, float outVoltageMax,
		bool wholeNumbersOnly, float zeroPointAngle, const char * formatStr,
		float roundDisplay, float roundOutput, float zeroValue, bool outputIsBoolean = false, bool displayIsInt = false)
	{
		this->displayName = displayName;
		this->unit = unit; // add unit
		this->displayFormatString = formatStr;
		this->minDisplayValue = minDisplayValue; // I.e. 1
		this->maxDisplayValue = maxDisplayValue; // I.e. 64 
		this->voltageMin = min_V;  // I.e. -10 Volts
		this->voltageMax = max_V;  // I.e. +10 Volts
		this->outputVoltageMin = outVoltageMin;
		this->outputVoltageMax = outVoltageMax;
		this->wholeNumbersOnly = wholeNumbersOnly; // Force whole numbers
		this->zeroPointAngle_radians = zeroPointAngle;
		this->roundNearestDisplay = roundDisplay;
		this->roundNearestOutput = roundOutput;
		this->zeroValue = zeroValue;
		this->displayIsInt = displayIsInt;
		
		needsTranslationDisplay = minDisplayValue != voltageMin || maxDisplayValue != voltageMax;
		needsTranslationOutput = outputVoltageMin != voltageMin || outputVoltageMax != voltageMax;
		
		snapValue = (voltageMax - voltageMin)/(maxDisplayValue - minDisplayValue);
		
		isBoolean = outputIsBoolean;
		return;
	}

	// Given the output voltage, return the display string.
	virtual void GetDisplayString(/*in*/ float val, /*out*/ char* buffer)
	{
		float dVal = val;
		if (isBoolean)
		{
			// Allow for multiSeq to have -10 V as false (anything positive should be true, anything 0 or negative should be false).
			//dVal = (dVal > zeroValue) ? voltageMax : zeroValue;
			if (dVal > zeroValue) {
				strncpy(buffer, bDisplayTrueStr, 20);
			}
			else {
				strncpy(buffer, bDisplayFalseStr, 20);				
			}
		}
		else 
		{
			if (needsTranslationDisplay)
			{
				// Clip
				if (dVal < voltageMin)
					dVal = voltageMin;
				else if (dVal > voltageMax)
					dVal = voltageMax; 
				dVal = rescale(dVal, voltageMin, voltageMax, minDisplayValue, maxDisplayValue);
				//DEBUG("Value: %6.4f, Ending Value: %6.4f. (Out Range: %6.2f - %6.4f)", val, dVal, minDisplayValue, maxDisplayValue);				
			}
			if (roundNearestDisplay > 0)
			{
				dVal = static_cast<int>(round(dVal/roundNearestDisplay)) * roundNearestDisplay;
				//DEBUG("> Value: %6.4f, Ending Rounded: %6.4f", val, dVal);
			}
			// Need this now since I didn't notice the sprintf will automatically round, so our Pattern #'s were rounding up sometimes when they should always be floored...
			if (displayIsInt)
			{
				dVal = static_cast<int>(dVal);
				//DEBUG("> Value: %6.4f, Ending (forced to INT): %6.4f", val, dVal);
			}
			sprintf(buffer, displayFormatString, dVal);				
		}
		return;
	}

	virtual std::string GetDisplayString(float outVoltage)
	{
		char buffer[50];
		GetDisplayString(outVoltage, buffer);
		return std::string(buffer);
	}

	// Given the display string, return the output knob voltage.
	virtual float GetKnobValueFromString(std::string displayStr)
	{
		float val = 0.0f;
		if (isBoolean) 
		{
			if (displayStr.compare(bDisplayTrueStr) == 0 || displayStr.compare("1") == 0)
			{
				val = voltageMax;
			}
			else
			{
				val = voltageMin;
			}
		}
		else
		{
			float dVal = std::stof(displayStr);
			val = dVal;
			if (needsTranslationDisplay)
			{
				// Display value back to knob voltage:
				val = rescale(dVal, minDisplayValue, maxDisplayValue, voltageMin, voltageMax);			
			}			
		}
		return val;
	}
	
	virtual float GetOutputValue(float val)
	{
		float oVal = val;
		// Clip
		if (oVal < voltageMin)
			oVal = voltageMin;
		else if (oVal > voltageMax)
			oVal = voltageMax; 		
		if (needsTranslationOutput)
		{
			if (isBoolean)
			{
				// Allow for multiSeq to have -10 V as false (anything positive should be true, anything 0 or negative should be false).
				oVal = (oVal > zeroValue) ? outputVoltageMax : outputVoltageMin;
			}
			else 
			{
				oVal = rescale(val, voltageMin, voltageMax, outputVoltageMin, outputVoltageMax);				
			}
		}
		if (roundNearestOutput > 0)
		{ // Round this
			oVal = static_cast<int>(round(oVal  / roundNearestOutput)) * roundNearestOutput;
		}
		return oVal;
	}
};
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// NoteValueSequencerMode
// Special sequencer mode for displaying human friendly Note labels instead of voltages.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct NoteValueSequencerMode : ValueSequencerMode
{
	NoteValueSequencerMode(const char* displayName,
		float min_V, float max_V)
	{
		this->displayName = displayName;	
		this->unit = "note";
		//this->minDisplayValue = -TROWA_SEQ_ZERO_OCTAVE; // -4
		//this->maxDisplayValue = TROWA_SEQ_NUM_OCTAVES - TROWA_SEQ_ZERO_OCTAVE; // 10-4 = 6
		this->minDisplayValue = TROWA_SEQ_NOTES_MIN_V; // Back to -5V
		this->maxDisplayValue = TROWA_SEQ_NOTES_MAX_V; // Back to +5V

		this->voltageMin = min_V;  // I.e. -10 Volts
		this->voltageMax = max_V;  // I.e. +10 Volts
		
		this->outputVoltageMin = TROWA_SEQ_NOTES_MIN_V; // Now back to -5V. Was -4V: -TROWA_SEQ_ZERO_OCTAVE;
		this->outputVoltageMax = TROWA_SEQ_NOTES_MAX_V; // Now back to +5V. Was +6V: TROWA_SEQ_NUM_OCTAVES - TROWA_SEQ_ZERO_OCTAVE; // 10-4 = 6
		
		this->wholeNumbersOnly = false; // Force whole numbers
		// Zero is no longer straight up now that we are going -4 to +6
		// Knob goes from 0.67*NVG_PI to 2.33*NVG_PI (1 and 2/3 Pi)
		//this->zeroPointAngle_radians = 0.67*NVG_PI + TROWA_SEQ_ZERO_OCTAVE *1.67*NVG_PI / TROWA_SEQ_NUM_OCTAVES;
		//this->zeroValue = rescale(0, this->minDisplayValue, this->maxDisplayValue, min_V, max_V);
		// C4 is now zero, but we we returning to -5 to +5V. (So starts at C-1 instead of C0 and goes to C9 instead of C10).
		this->zeroPointAngle_radians = TROWA_ANGLE_STRAIGHT_UP_RADIANS;// 1.5*NVG_PI; // Straight up
		this->zeroValue = rescale(0, this->minDisplayValue, this->maxDisplayValue, min_V, max_V);
		this->roundNearestDisplay = 1.0/TROWA_SEQ_NUM_NOTES;
		this->roundNearestOutput = 1.0/TROWA_SEQ_NUM_NOTES;
		
		needsTranslationDisplay = minDisplayValue != voltageMin || maxDisplayValue != voltageMax;
		needsTranslationOutput = outputVoltageMin != voltageMin || outputVoltageMax != voltageMax;
		
		snapValue = 1/12.0f;
		return;
	}
	// Overriden display string to show notes instead of output voltage values.
	void GetDisplayString(/*in*/ float val, /*out*/ char* buffer) override
	{
		// Now octaves will go -1 to +9.
		int octave = VoltsToOctave(val);
		int noteIx = VoltsToNoteIx(val);
		if (noteIx > TROWA_SEQ_NUM_NOTES - 1)
			noteIx = TROWA_SEQ_NUM_NOTES - 1;
		else if (noteIx < 0)
			noteIx = 0;
		sprintf(buffer, "%s%d", TROWA_NOTES[noteIx], octave);
		return;
	}
	// Overriden getting note string to knob voltage.
	float GetKnobValueFromString(std::string displayStr) override
	{
		float val = 0.0f;
		int notesIx = 0;
		int octave = TROWA_SEQ_ZERO_OCTAVE;
		bool noteFound = false;
		
		// @diplayStr should be more than one character. 
		// If we only have the note, then we will default to octave 4.
		// If we only have the octave, then we will default to C (notesIx 0).
		std::string strKey = displayStr;
		std::transform(strKey.begin(), strKey.end(), strKey.begin(), ::toupper);

//DEBUG("Translate Value %s to Voltage...", strKey.c_str());		
		// Probably note the best parsing, but it should work
		std::string noteStr = std::string("");
		std::string octStr = std::string("");		
		for (int i = 0; i < (int)(strKey.length()); i++)
		{
			char n = strKey.at(i);
			if (std::isalpha(n) || n == '#')
			{
				noteStr += n;
//DEBUG("   str[%d] = %c = Note: %s", i, n, noteStr.c_str());
			}
			else if (std::isdigit(n) || n == '-')
			{
				octStr += n;
//DEBUG("   str[%d] = %c = Octave: %s", i, n, octStr.c_str());				
			}
		}
		
//DEBUG("Search for Note: %s", noteStr.c_str());		
		while (notesIx < TROWA_SEQ_NUM_NOTES && !noteFound)
		{
//DEBUG(" > %s compare %s = %d", noteStr.c_str(), TROWA_NOTES[notesIx], noteStr.compare(TROWA_NOTES[notesIx]));				
			if (noteStr.compare(TROWA_NOTES[notesIx]) == 0)
			{
				noteFound = true;
			}
			else
			{
				notesIx++;
			}
		}
		if (!octStr.empty())
		{
			octave = std::stoi(octStr);
		}
		if (!noteFound)
			notesIx = 0; // Default to C
		if (octave < TROWA_SEQ_MIN_OCTAVE)
			octave = TROWA_SEQ_MIN_OCTAVE;
		else if (octave > TROWA_SEQ_MAX_OCTAVE)
			octave = TROWA_SEQ_MAX_OCTAVE;
		
// DEBUG("Note %s = %5.3f Volts", TROWA_NOTES[notesIx], NoteIxToVolts(notesIx));
// DEBUG("Octave %d = %5.3f Volts Output", octave, OctaveToVolts(octave));		
		val = NoteIxToVolts(notesIx) + OctaveToVolts(octave);
		float kVal = rescale(val, minDisplayValue, maxDisplayValue, voltageMin, voltageMax);
//DEBUG("Voltage Output %5.3f == %5.3f Knob Voltage", val, kVal);		
		return kVal;
	}
};


#endif // end if not defined
