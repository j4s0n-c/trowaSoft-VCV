#ifndef MODULE_TRIGSEQ_HPP
#define MODULE_TRIGSEQ_HPP

#include <string.h>
#include <stdio.h>
#include "trowaSoft.hpp"
#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"

#define trigSeq_GATE_ON_OUTPUT	  10.0  // If gate is on, the value to output (port Voltage)
#define trigSeq_GATE_OFF_OUTPUT	   0.0  // If gate is off, the value to output (port Voltage)


//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// trigSeq Module
// trowaSoft pad / trigger sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct trigSeq : TSSequencerModuleBase
{	
	SchmittTrigger* gateTriggers;
	trigSeq() : TSSequencerModuleBase(TROWA_SEQ_NUM_STEPS, TROWA_SEQ_STEP_NUM_ROWS, TROWA_SEQ_STEP_NUM_ROWS, false)
	{
		gateTriggers = new SchmittTrigger[maxSteps];
		selectedOutputValueMode = VALUE_TRIGGER;
		lastOutputValueMode = selectedOutputValueMode;
		modeStrings[0] = "TRIG";
		modeStrings[1] = "RTRG";
		modeStrings[2] = "CONT";		
		return;
	}
	trigSeq(int numSteps, int numRows, int numCols) : TSSequencerModuleBase(numSteps, numRows, numCols, false)
	{
		gateTriggers = new SchmittTrigger[maxSteps];
		selectedOutputValueMode = VALUE_TRIGGER;
		lastOutputValueMode = selectedOutputValueMode;
		modeStrings[0] = "TRIG";
		modeStrings[1] = "RTRG";
		modeStrings[2] = "CONT";		
		return;
	}
	
	~trigSeq()
	{
		delete [] gateTriggers;		
		return;
	}
	void step() override;
	// Only randomize the current gate/trigger steps.
	void randomize() override;
};

#endif