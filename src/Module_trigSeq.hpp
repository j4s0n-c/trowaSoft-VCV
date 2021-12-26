#ifndef MODULE_TRIGSEQ_HPP
#define MODULE_TRIGSEQ_HPP

#include <string.h>
#include <stdio.h>
//#include "trowaSoft.hpp"
//#include "dsp/digital.hpp"
#include "TSSequencerWidgetBase.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"

#define trigSeq_GATE_ON_OUTPUT	  10.0  // If gate is on, the value to output (port Voltage)
#define trigSeq_GATE_OFF_OUTPUT	   0.0  // If gate is off, the value to output (port Voltage)

// Single instance to the trigSeq Models.
// trigSeq (16-step) model
extern Model* modelTrigSeq;
// trigSeq (64-step) model
extern Model* modelTrigSeq64;


//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// trigSeq Module
// trowaSoft pad / trigger sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct trigSeq : TSSequencerModuleBase
{	
	trigSeq(int numSteps, int numRows, int numCols);
	trigSeq() : trigSeq(TROWA_SEQ_NUM_STEPS, TROWA_SEQ_STEP_NUM_ROWS, TROWA_SEQ_STEP_NUM_ROWS)
	{
		return;
	}	
	~trigSeq()
	{
		delete [] gateTriggers;	
		gateTriggers = NULL;
		return;
	}	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// process()
	// [Previously step(void)]
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void process(const ProcessArgs &args) override;
	// Only randomize the current gate/trigger steps.
	void onRandomize() override;
	// Get the toggle step value
	float getToggleStepValue(int step, float val, int channel, int pattern) override;
	// Calculate a representation of all channels for this step
	float getPlayingStepValue(int step, int pattern) override;
};

//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// trigSeq64 Module
// trowaSoft 64-step pad / trigger sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct trigSeq64 : trigSeq {
	trigSeq64() : trigSeq(N64_NUM_STEPS, N64_NUM_ROWS, N64_NUM_COLS)
	{		
		return;
	}
};


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// trigSeqWidget
// Widget for the trowaSoft pad / trigger sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct trigSeqWidget : TSSequencerWidgetBase {
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// trigSeqWidget()
	// Widget for the trowaSoft 16-step pad / trigger sequencer.
	// @seqModule : (IN) Pointer to the sequencer module.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	trigSeqWidget(trigSeq* seqModule);
};
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// trigSeq64Widget
// Widget for the trowaSoft 64-step sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct trigSeq64Widget : TSSequencerWidgetBase {
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// trigSeq64Widget()
	// Widget for the trowaSoft 64-step sequencer.
	// @seqModule : (IN) Pointer to the sequencer module.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	trigSeq64Widget(trigSeq* seqModule);
};


#endif