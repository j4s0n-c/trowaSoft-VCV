#include <string.h>
#include <stdio.h>
#include <math.h> 
#include "trowaSoft.hpp"
#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"
#include "Module_voltSeq.hpp"

#define TROWA_VOLTSEQ_OSC_ROUND_VAL					 100   // Mult & Divisor for rounding.
#define TROWA_VOLTSEQ_KNOB_CHANGED_THRESHOLD		0.01  // Value must change at least this much to send changed value over OSC

// Round the value for OSC. We will match what VOLT mode shows
inline float roundValForOSC(float val) {
	return round(val * TROWA_VOLTSEQ_OSC_ROUND_VAL) / (float)(TROWA_VOLTSEQ_OSC_ROUND_VAL);
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// voltSeq::randomize()
// Only randomize the current gate/trigger steps.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void voltSeq::randomize()
{
	int r, c;
	for (int s = 0; s < maxSteps; s++) 
	{
		// randomf() - [0.0, 1.0)
		triggerState[currentPatternEditingIx][currentChannelEditingIx][s] = voltSeq_STEP_KNOB_MIN + randomf()*(voltSeq_STEP_KNOB_MAX - voltSeq_STEP_KNOB_MIN);		
		r = s / this->numCols; // TROWA_SEQ_STEP_NUM_COLS;
		c = s % this->numCols; // TROWA_SEQ_STEP_NUM_COLS;
		this->params[CHANNEL_PARAM + s].value = this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s];
		knobStepMatrix[r][c]->setKnobValue(this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s]);			
	}	
	reloadEditMatrix = true;
	return;
} // end randomize()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Toggle the single step value
// (i.e. this command probably comes from an external source)
// @step : (IN) The step number to edit (0 to maxSteps).
// @val : (IN) The step value.
// @channel : (IN) The channel to edit (0 to TROWA_SEQ_NUM_CHNLS - 1).
// @pattern: (IN) The pattern to edit (0 to TROWA_SEQ_NUM_PATTERNS - 1).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
float voltSeq::getToggleStepValue(int step, float val, int channel, int pattern)
{
	return -triggerState[pattern][channel][step];
} // end getToggleStepValue()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Calculate a representation of all channels for this step
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
float voltSeq::getPlayingStepValue(int step, int pattern)
{
	//// u16 : 65535
	//// u32: 2,147,483,647 (2.147E10)
	//// float (pos part): 3.402823E38.
	//uint32_t val = 0;
	//int max = (TROWA_SEQ_NUM_CHNLS > 32) ? 32 : TROWA_SEQ_NUM_CHNLS;
	//for (int c = 0; c < max; c++)
	//{
	//	val += (bool)(this->triggerState[pattern][c][step] > 0.05 || this->triggerState[pattern][c][step] < -0.05) << c;
	//} // end for

	// u16 : 65535
	// u32: 2,147,483,647 (2.147E10)
	// float (pos part): 3.402823E38.
	int count = 0;
	for (int c = 0; c < TROWA_SEQ_NUM_CHNLS; c++)
	{
		count += (this->triggerState[pattern][c][step] > 0.05 || this->triggerState[pattern][c][step] < -0.05);
	} // end for

	return (float)(count) / (float)(TROWA_SEQ_NUM_CHNLS);
} // end getPlayingStepValue()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Set a single the step value
// (i.e. this command probably comes from an external source)
// >> Should set the control knob value too if applicable. <<
// @step : (IN) The step number to edit (0 to maxSteps).
// @val : (IN) The step value.
// @channel : (IN) The channel to edit (0 to TROWA_SEQ_NUM_CHNLS - 1).
// @pattern: (IN) The pattern to edit (0 to TROWA_SEQ_NUM_PATTERNS - 1).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void voltSeq::setStepValue(int step, float val, int channel, int pattern)
{
	int r, c;
	if (channel == CURRENT_EDIT_CHANNEL_IX)
	{
		channel = currentChannelEditingIx;
	}
	if (pattern == CURRENT_EDIT_PATTERN_IX)
	{
		pattern = currentPatternEditingIx;
	}
	r = step / this->numCols;
	c = step % this->numCols;
	// Do base method
	TSSequencerModuleBase::setStepValue(step, val, channel, pattern);

	// Set our knobs
	if (pattern == currentPatternEditingIx && channel == currentChannelEditingIx)
	{
		this->knobStepMatrix[r][c]->setKnobValue(val);
	}
	return;
} // end setStepValue()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// voltSeq::step()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void voltSeq::step()
{
	if (!initialized)
		return;
	bool gOn = true;
	bool pulse = false;
	bool reloadMatrix = false;
	bool valueModeChanged =  false;
	bool sendOSC = useOSC && oscInitialized;

	TSSequencerModuleBase::getStepInputs(&pulse, &reloadMatrix, &valueModeChanged);
	int r = 0;
	int c = 0;

	// Current output value mode	
	ValueSequencerMode* currOutputValueMode = ValueModes[selectedOutputValueMode];
	if (valueModeChanged)
	{
		modeString = currOutputValueMode->displayName;
		// Change our lights 
		for (r = 0; r < this->numRows; r++)
		{
			for (c = 0; c < this->numCols; c++)
			{
				dynamic_cast<TS_LightArc*>(padLightPtrs[r][c])->zeroAnglePoint = currOutputValueMode->zeroPointAngle_radians;
				dynamic_cast<TS_LightArc*>(padLightPtrs[r][c])->valueMode = currOutputValueMode;
			}
		}
	}
	lastOutputValueMode = selectedOutputValueMode;
		
	// Only send OSC if it is enabled, initialized, and we are in EDIT mode.
	sendOSC = useOSC && currentCtlMode == ExternalControllerMode::EditMode && oscInitialized;
	//-- * Load the trigger we are editing into our button matrix for display:
	// This is what we are showing not what we playing
	if (reloadMatrix || valueModeChanged)
	{
		reloadEditMatrix = false;
		oscMutex.lock();
		osc::OutboundPacketStream oscStream(oscBuffer, OSC_OUTPUT_BUFFER_SIZE);
		if (sendOSC && oscInitialized)
		{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Sending reload matrix: %s.", oscAddrBuffer[SeqOSCOutputMsg::EditStep]);
#endif
			oscStream << osc::BeginBundleImmediate;
		}
		oscMutex.unlock();
		// Load this gate and/or pattern into our 4x4 matrix
		for (int s = 0; s < maxSteps; s++) 
		{
			r = s / this->numCols; // TROWA_SEQ_STEP_NUM_COLS;
			c = s % this->numCols; // TROWA_SEQ_STEP_NUM_COLS;
			padLightPtrs[r][c]->setColor(voiceColors[currentChannelEditingIx]);
			gateLights[r][c] = 1.0 - stepLights[r][c];
			this->params[CHANNEL_PARAM + s].value = this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s];
			knobStepMatrix[r][c]->setKnobValue(this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s]);			
			lights[PAD_LIGHTS + s].value = gateLights[r][c];
			oscMutex.lock();
			if (sendOSC && oscInitialized)
			{
				oscLastSentVals[s] = roundValForOSC(triggerState[currentPatternEditingIx][currentChannelEditingIx][s]);
				oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditStep])
					<< s << oscLastSentVals[s]
					<< osc::EndMessage;
			}
			oscMutex.unlock();
		} // end for
		oscMutex.lock();
		if (sendOSC && oscInitialized)
		{
			oscStream << osc::EndBundle;
			oscTxSocket->Send(oscStream.Data(), oscStream.Size());
		}
		oscMutex.unlock();
	} // end if reload edit matrix
	//-- * Read the buttons
	else
	{		
		oscMutex.lock();
		osc::OutboundPacketStream oscStream(oscBuffer, OSC_OUTPUT_BUFFER_SIZE);
#if OSC_UPDATE_CURRENT_STEP
		bool updateCurrentStepOSC = false;
#endif
		if (sendOSC && oscInitialized)
		{
			oscStream << osc::BeginBundleImmediate;
#if OSC_UPDATE_CURRENT_STEP
			updateCurrentStepOSC = oscLastPrevStepUpdated != prevIndex && index != prevIndex;
			if (updateCurrentStepOSC)
				oscLastPrevStepUpdated = prevIndex;

			updateCurrentStepOSC = false;
#endif
		}
		oscMutex.unlock();

		int numChanged = 0;
		const float threshold = TROWA_VOLTSEQ_KNOB_CHANGED_THRESHOLD;
		// Gate buttons (we only show one gate) - Read Inputs
		for (int s = 0; s < maxSteps; s++) 
		{
			bool sendLightVal = false;
			this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s] = this->params[ParamIds::CHANNEL_PARAM + s].value;
			float dv = roundValForOSC(this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s]) - oscLastSentVals[s];
			sendLightVal = sendOSC && (dv > threshold || -dv > threshold); // Let's not send super tiny changes
			r = s / this->numCols;
			c = s % this->numCols;			
			stepLights[r][c] -= stepLights[r][c] / lightLambda / engineGetSampleRate();
			gateLights[r][c] = stepLights[r][c];			
			lights[PAD_LIGHTS + s].value = gateLights[r][c];

			oscMutex.lock();
#if OSC_UPDATE_CURRENT_STEP
			// This step has changed or this is the current step or this is the previous step
			if ((updateCurrentStepOSC && (prevIndex == s || index == s)) || sendLightVal)
#else
			// This step has changed and we are doing OSC
			if (sendLightVal && oscInitialized)
#endif
			{		
				oscLastSentVals[s] = roundValForOSC(triggerState[currentPatternEditingIx][currentChannelEditingIx][s]);
				// voltSeq should send the actual values.
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("Step changed %d (new val is %.4f), dv = %.4f, sending OSC %s", s, 
					oscLastSentVals[s],
					dv,
					oscAddrBuffer[SeqOSCOutputMsg::EditStep]);
#endif
				oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditStep])
					<< s << oscLastSentVals[s]
					<< osc::EndMessage;
				numChanged++;
			} // end if send the value over OSC
			oscMutex.unlock();
		} // end loop through step buttons
		oscMutex.lock();
		if (sendOSC && oscInitialized && numChanged > 0)
		{
			oscStream << osc::EndBundle;
			oscTxSocket->Send(oscStream.Data(), oscStream.Size());
		}
		oscMutex.unlock();
	} // end else (read button matrix)
	
	// Set Outputs (16 triggers)	
	for (int g = 0; g < TROWA_SEQ_NUM_CHNLS; g++) 
	{		
		float gate = (running && gOn) ? currOutputValueMode->GetOutputValue( triggerState[currentPatternPlayingIx][g][index] ) : 0.0; //***********VOLTAGE OUTPUT
		outputs[CHANNELS_OUTPUT + g].value= gate;
		// Output lights (around output jacks for each gate/trigger):
		gateLightsOut[g] = (gate < 0) ? -gate : gate;
		lights[CHANNEL_LIGHTS + g].value = gate / currOutputValueMode->outputVoltageMax;
	}
	return;
} // end step()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// voltSeqWidget
// Widget for the trowaSoft knob / voltage sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
voltSeqWidget::voltSeqWidget() : TSSequencerWidgetBase()
{		
	voltSeq *module = new voltSeq();
	setModule(module);

	//////////////////////////////////////////////
	// Background
	//////////////////////////////////////////////
	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/voltSeq.svg")));
		addChild(panel);
	}
	
	TSSequencerWidgetBase::addBaseControls(false);
	
	// (User) Input KNOBS ==================================================	
	int y = 115;
	int x = 79;
	int dx = 0;
	//int lightSize = 50 - 2*dx;
	Vec lSize = Vec(50, 50);
	int v = 0;
	ValueSequencerMode* currValueMode = module->ValueModes[module->selectedOutputValueMode];
	module->modeString = currValueMode->displayName;
	for (int r = 0; r < module->numRows; r++) //---------THE KNOBS
	{
		for (int c = 0; c < module->numCols; c++)
		{						
			// Pad Knob:
			TS_LightedKnob* knobPtr = dynamic_cast<TS_LightedKnob*>(createParam<TS_LightedKnob>(Vec(x, y), module, 
				TSSequencerModuleBase::CHANNEL_PARAM + r*module->numCols + c, 
				/*min*/ voltSeq_STEP_KNOB_MIN,  /*max*/ voltSeq_STEP_KNOB_MAX, /*default*/ voltSeq_STEP_KNOB_MIN));
			module->knobStepMatrix[r][c] = knobPtr;
			knobPtr->value = voltSeq_STEP_KNOB_MIN;
			
			// Keep a reference to our pad lights so we can change the colors			
			TS_LightArc* lightPtr = dynamic_cast<TS_LightArc*>(TS_createColorValueLight<TS_LightArc>(/*pos */ Vec(x+dx, y+dx), 
				/*module*/ module,
				/*lightId*/ TSSequencerModuleBase::PAD_LIGHTS + r*module->numCols + c,								
				/* size */ lSize, /* color */ module->voiceColors[module->currentChannelEditingIx]));			
			lightPtr->numericValue = &(knobPtr->value);
			lightPtr->currentAngle_radians = &(knobPtr->currentAngle);
			lightPtr->zeroAnglePoint = currValueMode->zeroPointAngle_radians;
			lightPtr->valueMode = currValueMode;			
			
			module->padLightPtrs[r][c] = lightPtr;			
			addChild( module->padLightPtrs[r][c] );
			
			addParam(knobPtr);
			knobPtr->dirty = true;

			x+= 59;
			v++;
		}		
		y += 59; // Next row
		x = 79;
	} // end loop through 4x4 grid
	module->initialized = true;
	return;
}