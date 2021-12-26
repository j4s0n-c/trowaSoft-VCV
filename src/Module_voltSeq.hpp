#ifndef MODULE_VOLTSEQ_HPP
#define MODULE_VOLTSEQ_HPP
#include <string.h>
#include <stdio.h>
//#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"
#include "TSSequencerWidgetBase.hpp"
#include "TSParamQuantity.hpp"

#define voltSeq_STEP_KNOB_MIN	  -10.0  // Minimum value from our knobs
#define voltSeq_STEP_KNOB_MAX	   10.0  // Maximum value from our knobs
#define TROWA_VOLTSEQ_OSC_ROUND_VAL					 100   // Mult & Divisor for rounding.
#define TROWA_VOLTSEQ_KNOB_CHANGED_THRESHOLD		0.01  // Value must change at least this much to send changed value over OSC


// voltSeq model.
extern Model *modelVoltSeq;

extern ValueSequencerMode* voltSeq_DEFAULT_VALUE_MODE;

// Round the value for OSC. We will match what VOLT mode shows
inline float roundValForOSC(float val) {
	return round(val * TROWA_VOLTSEQ_OSC_ROUND_VAL) / (float)(TROWA_VOLTSEQ_OSC_ROUND_VAL);
}

//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// voltSeq Module
// trowaSoft knob / voltage sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
// [11/28/2017]: Change knobStepMatrix for allowing for > the standard # steps (16).
struct voltSeq : TSSequencerModuleBase	
{	
	//int debugLastIndex = -1;

	/// TODO: Move to widget to separate UI from module.
	// References to our pad knobs. 
	// We may have > 16 steps in the future, so no more static matrix.
	//TS_LightedKnob***  knobStepMatrix;
	// Array of values of what we last sent over OSC (for comparison).
	float* oscLastSentVals = NULL;
	
	ValueSequencerMode* ValueModes[TROWA_SEQ_NUM_MODES] = { 
		// Voltage Mode 
		new ValueSequencerMode(/*displayName*/ "VOLT", /*unit*/ "V",
			/*minDisplayValue*/ -10, /*maxDisplayValue*/ 10, 
			/*inVoltageMin*/ voltSeq_STEP_KNOB_MIN, /*inVoltageMax*/ voltSeq_STEP_KNOB_MAX, 
			/*outVoltageMin*/ -10, /*outVoltageMax*/ 10, 
			/*whole numbers*/ false, 
			/*zeroPointAngle*/ TROWA_ANGLE_STRAIGHT_UP_RADIANS,
			/*display format String */ "%04.2f",
			/*roundDisplay*/ 0, /*roundOutput*/ 0,
			/*zeroValue*/ (voltSeq_STEP_KNOB_MAX+voltSeq_STEP_KNOB_MIN)/2.0),
		
		// Note mode (1 octave per V; 10 octaves)
		new NoteValueSequencerMode(/*displayName*/ "NOTE",			
			/*inVoltageMin*/ voltSeq_STEP_KNOB_MIN, /*inVoltageMax*/ voltSeq_STEP_KNOB_MAX),
			
		// Sequence Mode (1-64 for the patterns)			
		new ValueSequencerMode(/*displayName*/ "PATT",  /*unit*/ "pattern",
			/*minDisplayValue*/ 1, /*maxDisplayValue*/ TROWA_SEQ_NUM_PATTERNS, 
			/*inVoltageMin*/ voltSeq_STEP_KNOB_MIN, /*inVoltageMax*/ voltSeq_STEP_KNOB_MAX, 
			/*outVoltageMin*/ TROWA_SEQ_PATTERN_MIN_V, /*outVoltageMax*/ TROWA_SEQ_PATTERN_MAX_V, 
			/*whole numbers*/ false, 
			/*zeroPointAngle*/ 0.67*NVG_PI, 
			/*display format String */ "%02.0f",
			/*roundDisplay*/ 0, /*roundOutput*/ 0,
			/*zeroValue*/ voltSeq_STEP_KNOB_MIN)			
	};

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// voltSeq()
	// Create a voltSeq with the given number of steps layed out in the number of rows
	// and columns specified.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
	voltSeq(int numSteps, int numRows, int numCols);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// voltSeq()
	// Create a voltSeq with the default # steps and layout.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-			
	voltSeq() : voltSeq(TROWA_SEQ_NUM_STEPS, TROWA_SEQ_STEP_NUM_ROWS, TROWA_SEQ_STEP_NUM_ROWS)
	{
		return;
	}	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// ~voltSeq()
	// Destroy.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-				
	~voltSeq()
	{
		if (oscLastSentVals != NULL)
			delete [] oscLastSentVals;
	}	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// dataFromJson(void)
	// Read in our junk from json.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void dataFromJson(json_t *rootJ) override 
	{
		TSSequencerModuleBase::dataFromJson(rootJ);
		//// Check for old version and try to do graceful conversion from -5 to +5V 
		//// in Note Mode to -4 to +6V
		//if (saveVersion < 7 && selectedOutputValueMode == ValueMode::VALUE_MIDINOTE)
		//{
		//	this->shiftValues(TROWA_INDEX_UNDEFINED, TROWA_INDEX_UNDEFINED, 1.0);
		//}
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Set a single the step value
	// (i.e. this command probably comes from an external source).
	// >> Should set the control knob value too if applicable. <<
	// @step : (IN) The step number to edit (0 to maxSteps).
	// @val : (IN) The step value.
	// @channel : (IN) The channel to edit (0 to TROWA_SEQ_NUM_CHNLS - 1).
	// @pattern: (IN) The pattern to edit (0 to TROWA_SEQ_NUM_PATTERNS - 1).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void setStepValue(int step, float val, int channel, int pattern) override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// process()
	// [Previously step(void)]
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void process(const ProcessArgs &args) override;
	// Only randomize the current gate/trigger steps.
	void onRandomize() override;
	// Configure the value mode parameters on the steps.
	void configValueModeParam();

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// getRandomValue()
	// Get a random value for a step in this sequencer.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	float getRandomValue() override {
		return voltSeq_STEP_KNOB_MIN + random::uniform()*(voltSeq_STEP_KNOB_MAX - voltSeq_STEP_KNOB_MIN);
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// onShownStepChange()
	// If we changed a step that is shown on the matrix, then do something.
	// For voltSeq to adjust the knobs so we dont' read the old knob values again.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void onShownStepChange(int step, float val) override 
	{
		this->params[CHANNEL_PARAM + step].setValue(val);
		// int r = step / numRows;
		// int c = step % numRows;
		//knobStepMatrix[r][c]->setKnobValue(val);
		return;
	}
	// Get the toggle step value
	float getToggleStepValue(int step, float val, int channel, int pattern) override;
	// Calculate a representation of all channels for this step
	float getPlayingStepValue(int step, int pattern) override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Shift all steps (+/-) some number of volts.
	// @patternIx : (IN) The index into our pattern matrix (0-15). Or TROWA_INDEX_UNDEFINED for all patterns.
	// @channelIx : (IN) The index of the channel (gate/trigger/voice) if any (0-15, or TROWA_SEQ_COPY_CHANNELIX_ALL/TROWA_INDEX_UNDEFINED for all).
	// @volts: (IN) The number of volts to add.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void shiftValues(/*in*/ int patternIx, /*in*/ int channelIx, /*in*/ float volts);
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// ParamQuantity for our value modes.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TS_ValueSequencerParamQuantity : TS_ParamQuantity 
{
	// Most of this functionality was already done in ValueSequencerMode back in v0.5 or whatever, so use it.
	ValueSequencerMode* valueMode;	
	char buffer[50];
	TS_ValueSequencerParamQuantity() : TS_ParamQuantity()
	{
		return;
	}
	void setValueMode(ValueSequencerMode* vMode);
	// Returns a string representation of the display value 
	std::string getDisplayValueString() override;
	// Given the string make it the float voltage.
	void setDisplayValueString(std::string s) override;
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// voltSeqWidget
// Widget for the trowaSoft knob / voltage sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct voltSeqWidget : TSSequencerWidgetBase 
{
	// Last value output mode
	short lastMode = -1;
	// Current value mode pointer.
	ValueSequencerMode* currValueModePtr = NULL;
	ValueSequencerMode* lastValueModePtr = NULL;	
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// voltSeqWidget()
	// Widget for the trowaSoft 16-step voltage/knobby sequencer.
	// @seqModule : (IN) Pointer to the sequencer module.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	voltSeqWidget(voltSeq* seqModule);
	
	~voltSeqWidget()
	{
		return;
	}	
	
	void step() override;
	// Create context menu with shifting.
	//Menu *createContextMenu() override;
	/** Override to add context menu entries to your subclass.
	It is recommended to add a blank ui::MenuEntry first for spacing.
	*/
	void appendContextMenu(ui::Menu *menu) override;
};


#endif // end if not defined