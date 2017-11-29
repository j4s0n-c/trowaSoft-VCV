#ifndef MODULE_VOLTSEQ_HPP
#define MODULE_VOLTSEQ_HPP
#include <string.h>
#include <stdio.h>
#include "trowaSoft.hpp"
#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"

#define voltSeq_STEP_KNOB_MIN	  -10.0  // Minimum value from our knobs
#define voltSeq_STEP_KNOB_MAX	   10.0  // Maximum value from our knobs

//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// voltSeq Module
// trowaSoft knob / voltage sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
// [11/28/2017]: Change knobStepMatrix for allowing for > the standard # steps (16).
struct voltSeq : TSSequencerModuleBase	
{	
	// References to our pad knobs. 
	// We may have > 16 steps in the future, so no more static matrix.
	//TS_LightedKnob* knobStepMatrix[TROWA_SEQ_STEP_NUM_ROWS][TROWA_SEQ_STEP_NUM_COLS];
	TS_LightedKnob***  knobStepMatrix;
	
	ValueSequencerMode* ValueModes[TROWA_SEQ_NUM_MODES] = { 
		// Voltage Mode 
		new ValueSequencerMode(/*displayName*/ "VOLT",
			/*minDisplayValue*/ -10, /*maxDisplayValue*/ 10, 
			/*inVoltageMin*/ voltSeq_STEP_KNOB_MIN, /*inVoltageMax*/ voltSeq_STEP_KNOB_MAX, 
			/*outVoltageMin*/ -10, /*outVoltageMax*/ 10, 
			/*whole numbers*/ false, 
			/*zeroPointAngle*/ 1.5*NVG_PI, 
			/*display format String */ "%04.2f",
			/*roundDisplay*/ 0, /*roundOutput*/ 0,
			/*zeroValue*/ (voltSeq_STEP_KNOB_MAX+voltSeq_STEP_KNOB_MIN)/2.0),//
		
		// Note mode (1 octave per V; 10 octaves)
		new NoteValueSequencerMode(/*displayName*/ "NOTE",			
			/*inVoltageMin*/ voltSeq_STEP_KNOB_MIN, /*inVoltageMax*/ voltSeq_STEP_KNOB_MAX),
			
		// Sequence Mode (1-16 for the patterns)			
		new ValueSequencerMode(/*displayName*/ "PATT",
			/*minDisplayValue*/ 1, /*maxDisplayValue*/ TROWASEQ_NUM_PATTERNS, 
			/*inVoltageMin*/ voltSeq_STEP_KNOB_MIN, /*inVoltageMax*/ voltSeq_STEP_KNOB_MAX, 
			/*outVoltageMin*/ TROWASEQ_PATTERN_MIN_V, /*outVoltageMax*/ TROWASEQ_PATTERN_MAX_V, 
			/*whole numbers*/ false, 
			/*zeroPointAngle*/ 0.67*NVG_PI, 
			/*display format String */ "%02.0f",
			/*roundDisplay*/ 0, /*roundOutput*/ 0,
			/*zeroValue*/ voltSeq_STEP_KNOB_MIN)        
			
	};
	voltSeq(int numSteps, int numRows, int numCols) : TSSequencerModuleBase(numSteps, numRows, numCols, voltSeq_STEP_KNOB_MIN)
	{
		selectedOutputValueMode = VALUE_VOLT;
		lastOutputValueMode = selectedOutputValueMode;
		modeStrings[0] = "VOLT";
		modeStrings[1] = "NOTE";
		modeStrings[2] = "PATT";
		
		knobStepMatrix = new TS_LightedKnob**[numRows];
		for (int r = 0; r < numRows; r++)
		{
			knobStepMatrix[r] = new TS_LightedKnob*[numCols];
		}		
		return;
	}
	voltSeq() : voltSeq(TROWA_SEQ_NUM_STEPS, TROWA_SEQ_STEP_NUM_ROWS, TROWA_SEQ_STEP_NUM_ROWS)
	{
		return;
	}	
	~voltSeq()
	{
		for (int r = 0; r < numRows; r++)
		{
			delete[] knobStepMatrix[r];
			knobStepMatrix[r] = NULL;			
		}
		delete [] knobStepMatrix;
		knobStepMatrix = NULL;
	}
	void step() override;
	// Only randomize the current gate/trigger steps.
	void randomize() override;
};

#endif // end if not defined