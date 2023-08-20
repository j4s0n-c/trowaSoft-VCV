#include <string.h>
#include <stdio.h>
#include <math.h> 
#include "trowaSoft.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"
#include "Module_multiSeq.hpp"

//-------------------------
// Each Track
//-------------------------
// 1. Types:
//    a) Drum - 1 Channel for GATE or TRIGGER. Optional # of MOD channels -- Velocity, Filter Cut Off
//    b) Melody - 1 Channel for GATE (1 V/Oct). Optional # of MOD Channels.
//    c) Modulation - 1-N Channels for Voltages. Maybe option for triggered or repeating envelopes OR LFO.
// 2. # of Channels for Output (monophonic) and which channels (total 16 channels output to work with)
// 3. INPUTS:
// 	Step Length
// 	External Clock
// 	Reset
//  Pattern
// 4. Parameters (User Control)
//  Step Length (per Pattern and Track)
//  Internal Clock Division
//  Reset
//  Pattern
//  Pattern Sequencer (Pattern Length, Switch pattern)
//  Swing 
// 5. Tracks are color coded (so Track 1 DRUM is RED and the if you have two outputs for it, those become RED).
// 6. Tracks can also be named.
//---------------------------
// Master 


/// CHANGE ====== NEED COMPLETE REFACTOR OR TOTALLY NEW CODE
/// 1. STEP LENGTH PER CHANNEL
/// 2. RESET BY CHANNEL
/// 3. SONG MODE BY CHANNEL
/// 4. INTERNAL CLOCK BY CHANNEL
/// 5. PATTERN PER CHANNEL
/// 6. EXTERNAL CLOCK PER CHANNEL
/// 7. Swing per channel


Model* modelMultiSeq64 = createModel<multiSeq, multiSeqWidget>(/*slug*/ "multiSeq64");

#define TROWA_MULTISEQ_OSC_ROUND_VAL					 100   // Mult & Divisor for rounding.
#define TROWA_MULTISEQ_KNOB_CHANGED_THRESHOLD			0.01  // Value must change at least this much to send changed value over OSC


// Simple Pattern Value mode (basically 0-63, displayed 1 to 64).
// Doesn't try to use 'voltages'.
ValueSequencerMode* SimplePatternValueMode = new ValueSequencerMode(/*displayName*/ "Sequence",  /*unit*/ "",
	/*minDisplayValue*/ 1, /*maxDisplayValue*/ TROWA_SEQ_NUM_PATTERNS, 
	/*inVoltageMin*/ 0, /*inVoltageMax*/ TROWA_SEQ_NUM_PATTERNS - 1, 
	/*outVoltageMin*/ 0, /*outVoltageMax*/ TROWA_SEQ_NUM_PATTERNS - 1, 
	/*whole numbers*/ true, 
	/*zeroPointAngle*/ 0.67*NVG_PI, 
	/*display format String */ "%02.0f",
	/*roundDisplay*/ 0, /*roundOutput*/ 0,
	/*zeroValue*/ 0, /*outputIsBoolean*/ false, /*displayIsInt*/ true);
	
	
	
//=======================================================================================================================================
//
// multiSeq (Module)
//
//=======================================================================================================================================

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiSeq()
// Create a multiSeq with the given number of steps layed out in the number of rows
// and columns specified.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
multiSeq::multiSeq(int numSteps, int numRows, int numCols) : TSSequencerModuleBase(numSteps, numRows, numCols, /*default val*/ MULTISEQ_STEP_KNOB_MIN, /*default value mode*/ TSSequencerModuleBase::ValueMode::VALUE_TRIGGER, /*pattern sequencing*/ true)
{	
	numValueModesSupported = 6;
	valueModesSupported = new ValueMode[numValueModesSupported] { ValueMode::VALUE_TRIGGER, ValueMode::VALUE_RETRIGGER, ValueMode::VALUE_CONTINUOUS, ValueMode::VALUE_VOLT, ValueMode::VALUE_MIDINOTE, ValueMode::VALUE_PATTERN };		
	selectedOutputValueMode = defaultChannelValueMode;
	selectedOutputValueModeIx = getSupportedValueModeIndex(selectedOutputValueMode);
	lastOutputValueMode = selectedOutputValueMode;
	
	numStructuredRandomPatterns = TROWA_SEQ_NUM_RANDOM_PATTERNS; // multiSeq can use the full range of random patterns.	
		
	oscLastSentVals = new float[numSteps];	
	for (int s = 0; s < numSteps; s++)
	{
		// Configure step parameters:
		configParam<TS_ValueSequencerParamQuantity>(TSSequencerModuleBase::CHANNEL_PARAM + s, MULTISEQ_STEP_KNOB_MIN, MULTISEQ_STEP_KNOB_MAX, 
			/*default*/ defaultStateValue, /*label*/ "Step " + std::to_string(s+1));		
		oscLastSentVals[s] = MULTISEQ_STEP_KNOB_MIN - 1.0;
	}	
	
	this->reconfigureValueModeParamQty();	
	this->configValueModeParam();		

	// Populate labels.
	this->populateNotesPatternsLabels();
	return;
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiSeq::randomize()
// Only randomize the current gate/trigger steps by default OR the 
// the pattern sequences if that is shown.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void multiSeq::onRandomize(const RandomizeEvent& e)
{
	if (!showPatternSequencingConfig)
	{
		DEBUG("onRandomize() - showPatternConfig is false.");
		valuesChanging = true;	
		// If this is boolean or values.
		bool isBoolean = false;
		int ix = selectedOutputValueModeIx ;
		if (ix < 0 || ix > numValueModesSupported - 1)
			ix = 0;
		switch (valueModesSupported[ix])
		{
			case ValueMode::VALUE_TRIGGER:
			case ValueMode::VALUE_RETRIGGER:
			case ValueMode::VALUE_CONTINUOUS:
				isBoolean = true;
				break;
			default:
				isBoolean = false;
				break;
		}
		for (int s = 0; s < maxSteps; s++) 
		{
			// random::uniform() - [0.0, 1.0)
			if (isBoolean)
				triggerState[currentPatternEditingIx][currentChannelEditingIx][s] = (random::uniform() > 0.50) ? MULTISEQ_STEP_KNOB_MAX : MULTISEQ_STEP_KNOB_MIN;
			else
				triggerState[currentPatternEditingIx][currentChannelEditingIx][s] = MULTISEQ_STEP_KNOB_MIN + random::uniform()*(MULTISEQ_STEP_KNOB_MAX - MULTISEQ_STEP_KNOB_MIN);		
			this->params[CHANNEL_PARAM + s].setValue(this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s]);
		}	
		reloadEditMatrix = true;
		valuesChanging = false;
	}
	else 
	{
		// Showing pattern configuration, randomize the pattern config.
		randomizePatternSequence(false);		
	}
	return;
} // end randomize()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// getRandomValue()
// Get a random value for a step in this sequencer based on the channel.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
float multiSeq::getRandomValue(int channelIx)
{
	float v = 0.0f;
	switch (channelValueModes[channelIx])
	{
		case ValueMode::VALUE_TRIGGER:
		case ValueMode::VALUE_RETRIGGER:
		case ValueMode::VALUE_CONTINUOUS:
			v = (random::uniform() > 0.5f) ? MULTISEQ_STEP_KNOB_MAX : MULTISEQ_STEP_KNOB_MIN;
		break;
		default:
			v = MULTISEQ_STEP_KNOB_MIN + random::uniform()*(MULTISEQ_STEP_KNOB_MAX - MULTISEQ_STEP_KNOB_MIN);
		break;
	} // end switch	
	return v;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Toggle the single step value
// (i.e. this command probably comes from an external source)
// @step : (IN) The step number to edit (0 to maxSteps).
// @val : (IN) The step value.
// @channel : (IN) The channel to edit (0 to TROWA_SEQ_NUM_CHNLS - 1).
// @pattern: (IN) The pattern to edit (0 to TROWA_SEQ_NUM_PATTERNS - 1).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
float multiSeq::getToggleStepValue(int step, float val, int channel, int pattern)
{
	return -triggerState[pattern][channel][step];
} // end getToggleStepValue()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Calculate a representation of all channels for this step
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
float multiSeq::getPlayingStepValue(int step, int pattern)
{
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
void multiSeq::setStepValue(int step, float val, int channel, int pattern)
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
	triggerState[pattern][channel][step] = val;
	r = step / this->numCols;
	c = step % this->numCols;
	if (pattern == currentPatternEditingIx && channel == currentChannelEditingIx)
	{
		bool isOn = false;
		ValueSequencerMode* currOutputValueMode = ValueModes[selectedOutputValueModeIx];
		if (currOutputValueMode->isBoolean)
			isOn = triggerState[pattern][channel][step] > currOutputValueMode->zeroValue;
		else {
			isOn = true; // Values like -9 are still 'on'.
		}
		if (isOn) // (triggerState[pattern][channel][step])
		{
			gateLights[r][c] = 1.0f - stepLights[r][c];
			if (gateTriggers != NULL)
				gateTriggers[step].state = TriggerSignal::HIGH;
		}
		else
		{
			gateLights[r][c] = 0.0f; // Turn light off	
			if (gateTriggers != NULL)
				gateTriggers[step].state = TriggerSignal::LOW;
		}
	}
	oscMutex.lock();
	if (useOSC && oscInitialized)
	{
		// Send the result back
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
		DEBUG("multiSeq:step() - Received a msg (s=%d, v=%0.2f, c=%d, p=%d), sending back (%s).",
			step, val, channel, pattern,
			oscAddrBuffer[SeqOSCOutputMsg::EditStep]);
#endif
		char valOutputBuffer[20] = { 0 };
		char addrBuff[TROWA_SEQ_BUFF_SIZE] = { 0 };
		float val = roundValForOSC(triggerState[pattern][channel][step]);
		ValueSequencerMode* tmpMode = ValueModes[selectedOutputValueMode - ValueMode::VALUE_VOLT];
		tmpMode->GetDisplayString(tmpMode->GetOutputValue(triggerState[pattern][channel][step]), valOutputBuffer);

		sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditStep], step + 1);
		osc::OutboundPacketStream oscStream(oscBuffer, OSC_OUTPUT_BUFFER_SIZE);
		oscStream << osc::BeginBundleImmediate
			<< osc::BeginMessage(addrBuff)
			<< val // Rounded value for touchOSC
			<< osc::EndMessage;
		sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditStepString], step + 1);
		oscStream << osc::BeginMessage( addrBuff )
			<< valOutputBuffer // String version of the value (touchOSC needs this)
			<< osc::EndMessage
			<< osc::EndBundle;
		oscTxSocket->Send(oscStream.Data(), oscStream.Size());
	}
	oscMutex.unlock();

	// Set our knobs
	if (pattern == currentPatternEditingIx && channel == currentChannelEditingIx)
	{
		this->params[ParamIds::CHANNEL_PARAM + step].setValue(val);
	}
	return;
} // end setStepValue()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Shift all steps (+/-) some number of volts.
// @patternIx : (IN) The index into our pattern matrix (0-63). Or TROWA_INDEX_UNDEFINED for all patterns.
// @channelIx : (IN) The index of the channel (gate/trigger/voice) if any (0-15, or TROWA_SEQ_COPY_CHANNELIX_ALL/TROWA_INDEX_UNDEFINED for all).
// @volts: (IN) The number of volts to add.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void multiSeq::shiftValues(/*in*/ int patternIx, /*in*/ int channelIx, /*in*/ float volts)
{
	// Normal Range -10 to +10 V (20)
	// Midi: -5 to +5 V or -4 to + 6 V (10), so +1 octave will be +2V in 'Normal' range.
	float add = volts;
	
	// if (channelIx > TROWA_INDEX_UNDEFINED)
	// {
		// ValueMode chValueMode = channelValueModes[channelIx]; // Use the value mode for this channel
		// if (chValueMode == ValueMode::VALUE_MIDINOTE)//  (selectedOutputValueMode == ValueMode::VALUE_MIDINOTE)
		// {
			// add = volts * 2.0;
		// }
		// else if (chValueMode == ValueMode::VALUE_PATTERN)
		// {
			// add = (MULTISEQ_STEP_KNOB_MAX - MULTISEQ_STEP_KNOB_MIN) / TROWA_SEQ_NUM_PATTERNS * volts;
		// }		
	// }
	
	if (patternIx == TROWA_INDEX_UNDEFINED)
	{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED				
		DEBUG("shiftValues(ALL Patterns, %f) - Add %f", volts, add);
#endif		
		// All patterns:
		for (int p = 0; p < TROWA_SEQ_NUM_PATTERNS; p++)
		{
			shiftValues(p, TROWA_INDEX_UNDEFINED, volts); // All channels
		}
	}
	else if (channelIx == TROWA_INDEX_UNDEFINED)
	{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED						
		DEBUG("shiftValues(This Pattern, %f) - Add %f", volts, add);
#endif		
		// This pattern:
		for (int c = 0; c < TROWA_SEQ_NUM_CHNLS; c++)
		{
			ValueMode chValueMode = channelValueModes[c]; // Use the value mode for this channel
			if (chValueMode == ValueMode::VALUE_MIDINOTE)//  (selectedOutputValueMode == ValueMode::VALUE_MIDINOTE)
			{
				add = volts * 2.0;
			}
			else if (chValueMode == ValueMode::VALUE_PATTERN)
			{
				add = (MULTISEQ_STEP_KNOB_MAX - MULTISEQ_STEP_KNOB_MIN) / TROWA_SEQ_NUM_PATTERNS * volts;
			}
			else
			{
				add = volts;
			}
			
			for (int s = 0; s < maxSteps; s++)
			{
				float tmp = clamp(triggerState[patternIx][c][s] + add, /*min*/ MULTISEQ_STEP_KNOB_MIN,  /*max*/ MULTISEQ_STEP_KNOB_MAX);
				triggerState[patternIx][c][s] = tmp;
				if (patternIx == currentPatternEditingIx && c == currentChannelEditingIx)
				{
					this->params[CHANNEL_PARAM + s].setValue(tmp);
				}
			}
		}
		//this->reloadEditMatrix = true;
	}
	else
	{
		// Just this channel
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED		
		DEBUG("shiftValues(%d, %d, %f) - Add %f", patternIx, channelIx, volts, add);
#endif
		ValueMode chValueMode = channelValueModes[channelIx]; // Use the value mode for this channel
		if (chValueMode == ValueMode::VALUE_MIDINOTE)//  (selectedOutputValueMode == ValueMode::VALUE_MIDINOTE)
		{
			add = volts * 2.0;
		}
		else if (chValueMode == ValueMode::VALUE_PATTERN)
		{
			add = (MULTISEQ_STEP_KNOB_MAX - MULTISEQ_STEP_KNOB_MIN) / TROWA_SEQ_NUM_PATTERNS * volts;
		}
		else
		{
			add = volts;
		}
		for (int s = 0; s < maxSteps; s++)
		{
			float tmp = clamp(triggerState[patternIx][channelIx][s] + add, /*min*/ MULTISEQ_STEP_KNOB_MIN,  /*max*/ MULTISEQ_STEP_KNOB_MAX);
			DEBUG(" %d = %f + %fV (add %f) = %f", s, triggerState[patternIx][channelIx][s], volts, add, tmp);
			triggerState[patternIx][channelIx][s] = tmp;
			if (patternIx == currentPatternEditingIx && channelIx == currentChannelEditingIx)
			{
				this->params[CHANNEL_PARAM + s].setValue(tmp);
			}
		}
		//if (patternIx == currentPatternEditingIx && channelIx == currentChannelEditingIx)
		//	this->reloadEditMatrix = true;
	}
	return;
} // end shiftValues()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// process()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void multiSeq::process(const ProcessArgs &args)
{
	// Now calculate some base items all the time:
	float clockTime = 0.0f;
	bool nextStep = isNewStep(args.sampleRate, &clockTime);	
#if TROWA_SEQ_USE_INTERNAL_DIVISOR	
	if (nextStep || resetTriggered || this->ctlMsgQueue.size() > 0) // Eval if new step, reset triggered or OSC message
	{		
		idleCounter = 0;
	}
	else
	{
		// Skip evaluations if needed
		idleCounter = (idleCounter + 1) % IDLETHRESHOLD;
		if (idleCounter != 0)
		{				
			return;
		}
	}
#endif 	
	
	bool gOn = true;
	bool pulse = false;
	bool reloadMatrix = false;
	bool valueModeChanged =  false;
	bool sendOSC = useOSC && oscInitialized;
	
	bool isFirstLoad = firstLoad;
	TSSequencerModuleBase::getStepInputs(args, &pulse, &reloadMatrix, &valueModeChanged, nextStep, clockTime);
	
	int r = 0;
	int c = 0;
	ValueSequencerMode* currOutputValueMode = ValueModes[selectedOutputValueModeIx];// ValueModes[selectedOutputValueMode - ValueMode::VALUE_VOLT];
	if (valueModeChanged || isFirstLoad)
	{
		modeString = currOutputValueMode->displayName;
		channelValueModes[currentChannelEditingIx] = selectedOutputValueMode;
		for (int s = 0; s < maxSteps; s++)
		{
			paramQuantities[ParamIds::NUM_PARAMS + s]->defaultValue = currOutputValueMode->zeroValue;
		}
		// Change our ParamQuantities
		this->configValueModeParam();
	}
	lastOutputValueMode = selectedOutputValueMode;
	
	//// Pattern Sequencing Lights ////
	// INTERNAL PATTERN SEQUENCING ///////
	if (allowPatternSequencing)
	{
		if (patternSequencingOn)
		{
			// We are currently pattern sequencing  (song mode).
			for (int s = 0; s < numPatternsInSequence; s++)
			{			
				if (s != patternPlayHeadIx)
					lights[PATTERN_SEQ_LIGHT_START + s].value = 0.0f;
				else
				{
					float v = lights[PATTERN_SEQ_LIGHT_START + s].value - lights[PATTERN_SEQ_LIGHT_START + s].value / lightLambda / args.sampleRate / 20.f;
					lights[PATTERN_SEQ_LIGHT_START + s].value = (v < 0) ? 0 : v;				
				}
			}
			for (int s = numPatternsInSequence; s < maxSteps; s++)
			{
				lights[PATTERN_SEQ_LIGHT_START + s].value = 0;
			}			
		}
	} // end internal pattern sequencing
		
	// Only send OSC if it is enabled, initialized, and we are in EDIT mode.
	sendOSC = useOSC && currentCtlMode == ExternalControllerMode::EditMode && oscInitialized;
	//-- * Load the trigger we are editing into our button matrix for display:
	// This is what we are showing not what we playing
	char valOutputBuffer[20] = { 0 };
	char addrBuff[TROWA_SEQ_BUFF_SIZE] = { 0 };
	char colorAddrBuff[TROWA_SEQ_BUFF_SIZE] = { 0 }; // 2nd buffer to remove my lazy re-using of buffers (technically undefined behavior)
	std::string stepStringAddr = std::string(oscAddrBuffer[SeqOSCOutputMsg::EditStepString]);
	if (!valuesChanging && (reloadMatrix || reloadEditMatrix || valueModeChanged))
	{
		reloadEditMatrix = false;
		oscMutex.lock();
		osc::OutboundPacketStream oscStream(oscBuffer, OSC_OUTPUT_BUFFER_SIZE);
		if (sendOSC && oscInitialized)
		{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			DEBUG("Sending reload matrix: %s.", oscAddrBuffer[SeqOSCOutputMsg::EditStep]);
#endif
			oscStream << osc::BeginBundleImmediate;
		}
		oscMutex.unlock();
		// Load this channel into our 4x4 matrix
		this->currentStepMatrixColor = voiceColors[currentChannelEditingIx];
		for (int s = 0; s < maxSteps; s++) 
		{
			r = s / this->numCols; // TROWA_SEQ_STEP_NUM_COLS;
			c = s % this->numCols; // TROWA_SEQ_STEP_NUM_COLS;
			gateLights[r][c] = 1.0 - stepLights[r][c];			
			this->params[CHANNEL_PARAM + s].setValue(this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s]);
			lights[PAD_LIGHTS + s].value = gateLights[r][c];
			oscMutex.lock();
			if (sendOSC && oscInitialized)
			{
				// Each step may have up to 4-ish messages, so send 4 or 8 steps at a time.
				if (s > 0 && s % 8 == 0) // There is a limit to client buffer size, so let's not make the bundles too large. Hopefully they can take this many steps at a time.
				{
					// Send this bundle and then start a new one
					oscStream << osc::EndBundle;
					oscTxSocket->Send(oscStream.Data(), oscStream.Size());
					oscStream.Clear();
					// Start new bundle:
					oscStream << osc::BeginBundleImmediate;
				}
				oscLastSentVals[s] = roundValForOSC(triggerState[currentPatternEditingIx][currentChannelEditingIx][s]);
				currOutputValueMode->GetDisplayString(currOutputValueMode->GetOutputValue(triggerState[currentPatternEditingIx][currentChannelEditingIx][s]), valOutputBuffer);
				// Step value:
				sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditStep], s+1);
				oscStream << osc::BeginMessage(addrBuff)
					<< oscLastSentVals[s]
					<< osc::EndMessage;
				if (oscCurrentClient == OSCClient::touchOSCClient)
				{
					// Change color
					sprintf(colorAddrBuff, OSC_TOUCH_OSC_CHANGE_COLOR_FS, addrBuff);
					oscStream << osc::BeginMessage(colorAddrBuff)
						<< touchOSC::ChannelColors[currentChannelEditingIx]
						<< osc::EndMessage;
					// LED Color (current step LED):
					sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::PlayStepLed], s + 1);
					sprintf(colorAddrBuff, OSC_TOUCH_OSC_CHANGE_COLOR_FS, addrBuff);
					oscStream << osc::BeginMessage(colorAddrBuff)
						<< touchOSC::ChannelColors[currentChannelEditingIx]
						<< osc::EndMessage;
				}
				// Step String
				sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditStepString], s+1);
				oscStream << osc::BeginMessage( addrBuff )
					<< valOutputBuffer // String version of the value (touchOSC needs this)
					<< osc::EndMessage;
			}
			oscMutex.unlock();
		} // end for
		oscMutex.lock();
		if (sendOSC && oscInitialized)
		{
			if (oscCurrentClient == OSCClient::touchOSCClient)
			{
				// Also change color on the Channel control:
				sprintf(addrBuff, OSC_TOUCH_OSC_CHANGE_COLOR_FS, oscAddrBuffer[SeqOSCOutputMsg::EditChannel]);
				oscStream << osc::BeginMessage(addrBuff)
					<< touchOSC::ChannelColors[currentChannelEditingIx]
					<< osc::EndMessage;
			}

			// End last bundle and send:
			oscStream << osc::EndBundle;
			oscTxSocket->Send(oscStream.Data(), oscStream.Size());
		}
		oscMutex.unlock();
	} // end if reload edit matrix
	//-- * Read the buttons
	else if (!valuesChanging) // Only read in if another thread isn't changing the values
	{		
		oscMutex.lock();
		osc::OutboundPacketStream oscStream(oscBuffer, OSC_OUTPUT_BUFFER_SIZE);
		if (sendOSC && oscInitialized)
		{
			oscStream << osc::BeginBundleImmediate;
		}
		oscMutex.unlock();

		int numChanged = 0;
		const float threshold = TROWA_MULTISEQ_KNOB_CHANGED_THRESHOLD;
		// Channel step knobs - Read Inputs
		for (int s = 0; s < maxSteps; s++) 
		{
			bool sendLightVal = false;
			this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s] = this->params[ParamIds::CHANNEL_PARAM + s].getValue();
			float dv = roundValForOSC(this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s]) - oscLastSentVals[s];
			sendLightVal = sendOSC && (dv > threshold || -dv > threshold); // Let's not send super tiny changes
			r = s / this->numCols;
			c = s % this->numCols;			
			stepLights[r][c] -= stepLights[r][c] / lightLambda / args.sampleRate;
			gateLights[r][c] = stepLights[r][c];
			lights[PAD_LIGHTS + s].value = gateLights[r][c];	

			oscMutex.lock();
			// This step has changed and we are doing OSC
			if (sendLightVal && oscInitialized)
			{		
				oscLastSentVals[s] = roundValForOSC(triggerState[currentPatternEditingIx][currentChannelEditingIx][s]);
				// multiSeq should send the actual values.
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				DEBUG("Step changed %d (new val is %.4f), dv = %.4f, sending OSC %s", s, 
					oscLastSentVals[s],
					dv,
					oscAddrBuffer[SeqOSCOutputMsg::EditStep]);
#endif
				// Now also send the equivalent string:
				currOutputValueMode->GetDisplayString(currOutputValueMode->GetOutputValue( triggerState[currentPatternEditingIx][currentChannelEditingIx][s] ), valOutputBuffer);
				sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditStep], s + 1);
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED				
				DEBUG("Send: %s -> %s : %s", oscAddrBuffer[SeqOSCOutputMsg::EditStepString], addrBuff, valOutputBuffer);
#endif				
				oscStream << osc::BeginMessage(addrBuff)
					<< oscLastSentVals[s]
					<< osc::EndMessage;
				sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditStepString], s + 1);
				oscStream << osc::BeginMessage(addrBuff)
					<< valOutputBuffer // String version of the value (touchOSC needs this)
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
		//float gate = (running && gOn) ? currOutputValueMode->GetOutputValue( triggerState[currentPatternPlayingIx][g][index] ) : 0.0; //***********VOLTAGE OUTPUT
		// [v1.0.1] Each channel has its own output mode now
		// [v1.0.4] ValueMode isn't 0 starting for multiSeq anymore. 
		int k = getSupportedValueModeIndex(channelValueModes[g]);
		if (k < 0)
			k = 0;
		ValueSequencerMode* chMode = ValueModes[k];
		float gate = 0.0f;
		if (running && gOn)
		{
			switch (valueModesSupported[k])
			{
					case ValueMode::VALUE_TRIGGER:
						if (pulse)
							gate = chMode->GetOutputValue( triggerState[currentPatternPlayingIx][g][index] );
					break;
					case ValueMode::VALUE_RETRIGGER:
						if (!pulse)
							gate = chMode->GetOutputValue( triggerState[currentPatternPlayingIx][g][index] );					
					break;
					case ValueMode::VALUE_CONTINUOUS:
					case ValueMode::VALUE_VOLT:
					case ValueMode::VALUE_MIDINOTE:
					case ValueMode::VALUE_PATTERN:
					default:
						gate = chMode->GetOutputValue( triggerState[currentPatternPlayingIx][g][index] );					
					break;				
			}
			
		}		
		//float gate = (running && gOn) ? chMode->GetOutputValue( triggerState[currentPatternPlayingIx][g][index], pulse ) : 0.0; //***********VOLTAGE OUTPUT				
		
		outputs[CHANNELS_OUTPUT + g].value= gate;
		// Output lights (around output jacks for each gate/trigger):
		gateLightsOut[g] = (gate < 0) ? -gate : gate;
		lights[CHANNEL_LIGHTS + g].value = gate / chMode->outputVoltageMax;// currOutputValueMode->outputVoltageMax;
	}
	return;
} // end process()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Configure the value mode parameters on the steps.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void multiSeq::configValueModeParam()
{
	// v1.0.4 - selectedOutputValue is no longer an index into this array.
	ValueSequencerMode* currOutputValueMode = ValueModes[selectedOutputValueModeIx];
	for (int s = 0; s < maxSteps; s++)
	{
		// Configure step parameters:
		TS_ValueSequencerParamQuantity* pQuantity = dynamic_cast<TS_ValueSequencerParamQuantity*>( this->paramQuantities[TSSequencerModuleBase::CHANNEL_PARAM + s] );
		pQuantity->setValueMode(currOutputValueMode);
	}
	return;
}

//=======================================================================================================================================
//
// multiSeqWidget
//
//=======================================================================================================================================

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiSeqWidget()
// Widget for the trowaSoft 16-step voltage/knobby sequencer.
// @seqModule : (IN) Pointer to the sequencer module.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
multiSeqWidget::multiSeqWidget(multiSeq* seqModule, int nSteps, int nRows, int nCols) : TSSequencerWidgetBase(seqModule)
{		
	bool isPreview = seqModule == NULL; // If this is null, then this isn't a real module instance but a 'Preview'?	
	//////////////////////////////////////////////
	// Background
	//////////////////////////////////////////////
	{
		SvgPanel *panel = new SvgPanel();
		panel->box.size = box.size;
		panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/multiSeq.svg")));
		addChild(panel);
	}
	
	
	NVGcolor lightColor = TSColors::COLOR_TS_RED;
	int oscId = 0;	
	int basePatternParamId = 0;
	int basePatternLightId = 0;
	if (isPreview)
	{
		// Get the steps from the inputs in case we ever have another variation other than 64 steps:
		maxSteps = nSteps;
		numRows = nRows;
		numCols = nCols;
		currValueModePtr = voltSeq_DEFAULT_VALUE_MODE;				
		basePatternParamId = TSSequencerModuleBase::ParamIds::CHANNEL_PARAM + maxSteps * 2;
		basePatternLightId = TSSequencerModuleBase::LightIds::PAD_LIGHTS + maxSteps * 2;
	}
	else
	{
		// Read from the actual module, what we should be doing:
		maxSteps = seqModule->maxSteps;
		numRows = seqModule->numRows;
		numCols = seqModule->numCols;		
		// v1.0.4 - selectedOutputValue is no longer an index into this array (subtract VALUE_VOLT).
		currValueModePtr = seqModule->ValueModes[seqModule->selectedOutputValueModeIx];
		seqModule->modeString = currValueModePtr->displayName;
		lightColor = seqModule->voiceColors[seqModule->currentChannelEditingIx];
		//lastMode = seqModule->selectedOutputValueMode;
		oscId = seqModule->oscId;
		basePatternParamId = seqModule->PATTERN_SEQ_PARAM_START;
		basePatternLightId = seqModule->PATTERN_SEQ_LIGHT_START;		
	}
	lastValueModePtr = currValueModePtr;
	
	// Add the base controls (all sequencers have):
	this->TSSequencerWidgetBase::addBaseControls(true);
	// Set the labels
	this->labelArea->allowPatternSequencing = true;
	
	// (User) Input KNOBS ==================================================	
	int y = 0; //115;
	int x = 0; //79;
	// y = 115;
	// x = 314;
	// container width 314-79
	
	int dx = 0;
	Vec padSize = Vec(24, 24); // Pad Size
	Vec lSize = Vec(padSize.x - 2*dx, padSize.y - 2*dx);// Light Size
	//int v = 0;
	int spacing = padSize.x + 5; // Spacing between controls
	
	///////////// Step Controls    /////////////////
	///////////// Pattern Controls /////////////////	
	// Step Container object:
	stepContainer = new TSContainerWidget();
	stepContainer->visible = true;
	stepContainer->box.size = Vec(235, 235);
	stepContainer->box.pos = Vec(79, 115); // 79, 115
	addChild(stepContainer);
	// Pattern Container Object
	patternContainer = new TSContainerWidget();
	patternContainer->visible = false;
	patternContainer->box.size = Vec(235, 235);
	patternContainer->box.pos = Vec(79, 115); // 79, 115	
	addChild(patternContainer);
	
	// Step Controls and Lights:
	padPtrs = new TSSwitchKnob**[numRows];	
	padLightPtrs = new ColorValueLight**[numRows];	
	patternSeqPtrs = new TSSwitchKnob**[numRows];
	patternSeqLightPtrs = new TS_LightMeter**[numRows];
	int id = 0;
	for (int r = 0; r < numRows; r++)
	{
		padPtrs[r] = new TSSwitchKnob*[numCols];		
		padLightPtrs[r] = new ColorValueLight*[numCols];
		
		patternSeqPtrs[r] = new TSSwitchKnob*[numCols];
		patternSeqLightPtrs[r] = new TS_LightMeter*[numCols];
		for (int c = 0; c < numCols; c++)
		{	
			TSSwitchKnob* ctrlPtr = NULL;
			TS_LightMeter* lightPtr = NULL;
			
			//---------------------
			// Steps ==============
			//---------------------			
			ctrlPtr = dynamic_cast<TSSwitchKnob*>(createParam<TSSwitchKnob>(Vec(x, y), seqModule, TSSequencerModuleBase::CHANNEL_PARAM + r*numCols + c));
			ctrlPtr->id = id;
			ctrlPtr->groupId = 100 + oscId;
			ctrlPtr->box.size = padSize;
			ctrlPtr->seqModule = seqModule;
			ctrlPtr->allowRandomize = false;			
			ctrlPtr->isStepValue = true; // Control the normal steps
			stepContainer->addChild(ctrlPtr);			
			//params.push_back(ctrlPtr);
			padPtrs[r][c] = ctrlPtr;
			
			lightPtr = dynamic_cast<TS_LightMeter*>(TS_createColorValueLight<TS_LightMeter>(/*pos */ Vec(x+dx, y+dx), /*seqModule*/ seqModule, 
				/*lightId*/ TSSequencerModuleBase::PAD_LIGHTS + r*numCols + c, /*size*/ lSize, /*color*/ lightColor));
			lightPtr->valueMode = currValueModePtr;	
			lightPtr->paramWidget = ctrlPtr;
			lightPtr->id = id;
			padLightPtrs[r][c] = lightPtr;
			stepContainer->addChild(lightPtr);
			
			//--------------------------------
			// Pattern Sequence ==============
			//--------------------------------			
			ctrlPtr = dynamic_cast<TSSwitchKnob*>(createParam<TSSwitchKnob>(Vec(x, y), seqModule, basePatternParamId + r*numCols + c));
			ctrlPtr->id = id;
			ctrlPtr->groupId = 200 + oscId;
			ctrlPtr->snap = true;
			ctrlPtr->box.size = padSize;
			ctrlPtr->seqModule = seqModule;
			ctrlPtr->allowRandomize = false;
			ctrlPtr->isStepValue = false; // Control pattern steps (patternData)
			ctrlPtr->setFunctionType(TSSwitchKnob::FunctionType::Knob);
			patternContainer->addChild(ctrlPtr);
			//params.push_back(ctrlPtr);
			patternSeqPtrs[r][c] = ctrlPtr;
			
			
			// Make lights now : TS_PATTERN_SEQ_STATUS_COLOR
			// Was: TS_PATTERN_SEQ_STEP_COLOR
			lightPtr = dynamic_cast<TS_LightMeter*>(TS_createColorValueLight<TS_LightMeter>(/*pos */ Vec(x+dx, y+dx), /*seqModule*/ seqModule, 
				/*lightId*/ basePatternLightId + r*numCols + c, /*size*/ lSize, /*color*/ TS_PATTERN_SEQ_STATUS_COLOR));
			lightPtr->valueMode = SimplePatternValueMode;
			lightPtr->paramWidget = ctrlPtr;
			lightPtr->id = id;
			lightPtr->innerGlowAlphaAdj = 0.5f;
			lightPtr->setValueColor(TS_PATTERN_SEQ_STEP_COLOR);// = ctrlPtr;
			
			patternSeqLightPtrs[r][c] = lightPtr;
			patternContainer->addChild(lightPtr);
			
			// Increment
			x += spacing;
			id++;
		}
		y += spacing; // Next Row
		x = 0; // Column 0		
	} // end loop through 16x16 grid
	
	if (!isPreview)
		seqModule->initialized = true;	
	return;
} // end multiSeqWidget()

void multiSeqWidget::step()
{
	TSSequencerWidgetBase::step();
	
	if (this->module == NULL)
		return;
	
	multiSeq* seqModule =  dynamic_cast<multiSeq*>(this->module);
	ValueSequencerMode* currValueMode = seqModule->ValueModes[seqModule->selectedOutputValueModeIx];	
	// Switch the defaults on the knobs based on the output mode (if changed):
	if (lastMode != seqModule->selectedOutputValueMode)
	{
		TSSwitchKnob::FunctionType inputType = TSSwitchKnob::FunctionType::Switch;
		switch (seqModule->selectedOutputValueMode)
		{
			case TSSequencerModuleBase::ValueMode::VALUE_MIDINOTE:			
			case TSSequencerModuleBase::ValueMode::VALUE_VOLT:
			case TSSequencerModuleBase::ValueMode::VALUE_PATTERN:
				inputType = TSSwitchKnob::FunctionType::Knob;
				break;
			default:
				inputType = TSSwitchKnob::FunctionType::Switch;
				break;
		} // end switch
		bool inputModeChanged = padPtrs[0][0]->currentFunctionType != inputType;
		
		for (int r = 0; r < numRows; r++)
		{
			for (int c = 0; c < numCols; c++)
			{						
				TS_LightMeter* stepLight = dynamic_cast<TS_LightMeter*>(padLightPtrs[r][c]);
				stepLight->valueMode = currValueMode;
				
				if (inputModeChanged)
					padPtrs[r][c]->setFunctionType(inputType);
			}
		}		
	}
	display->sequencerValueModePtr = currValueMode;	
	if (seqModule->currentStepBeingEditedIx > -1)
	{
		display->currentView = TSSeqDisplay::SeqViewType::EditStepView;
	}
	else
	{
		display->currentView = TSSeqDisplay::SeqViewType::NormalView;
	}	
	lastMode = seqModule->selectedOutputValueMode;
	
	
	if (lastShowPatternConfig != seqModule->showPatternSequencingConfig)
	{
		// Show/Hide the different 
		stepContainer->setVisible(!seqModule->showPatternSequencingConfig);
		patternContainer->setVisible(seqModule->showPatternSequencingConfig);	
		
		/// Song Mode / Pattern Sequencer shown. Allow randomize if showing.
		for (int r = 0; r < numRows; r++)
		{
			for (int c = 0; c < numCols; c++)
			{
				patternSeqPtrs[r][c]->allowRandomize = seqModule->showPatternSequencingConfig;
			}		
		}		
	}
	lastShowPatternConfig = seqModule->showPatternSequencingConfig;
	return;
}

struct multiSeq_ShiftVoltageSubMenuItem : MenuItem 
{
	multiSeq* sequencerModule;
	float amount = 1.0;
	enum ShiftType {
		// Current Edit Pattern & Channel
		CurrentChannelOnly,
		// Current Edit Pattern, All Channels
		ThisPattern,
		// All patterns, all channels
		AllPatterns
	};
	ShiftType Target = ShiftType::CurrentChannelOnly;

	multiSeq_ShiftVoltageSubMenuItem(std::string text, ShiftType target, float amount, multiSeq* seqModule)
	{
		this->box.size.x = 200;
		this->text = text;
		this->Target = target;
		this->amount = amount;
		this->sequencerModule = seqModule;
	}

	void onAction(const event::Action &e) override {
		if (this->Target == ShiftType::AllPatterns)
		{
			sequencerModule->shiftValues(TROWA_INDEX_UNDEFINED, TROWA_SEQ_COPY_CHANNELIX_ALL, amount);
		}
		else if (this->Target == ShiftType::ThisPattern)
		{
			sequencerModule->shiftValues(sequencerModule->currentPatternEditingIx, TROWA_SEQ_COPY_CHANNELIX_ALL, amount);
		}
		else //if (this->Target == ShiftType::CurrentChannelOnly)
		{
			sequencerModule->shiftValues(sequencerModule->currentPatternEditingIx, sequencerModule->currentChannelEditingIx, amount);
		}
	}
	void step() override {
		//rightText = (seq3->gateMode == gateMode) ? "✔" : "";
	}
};

struct multiSeq_ShiftVoltageSubMenu : Menu {
	multiSeq* sequencerModule;
	float amount = 1.0;

	multiSeq_ShiftVoltageSubMenu(float amount, multiSeq* seqModule)
	{
		this->box.size = Vec(200, 60);
		this->amount = amount;
		this->sequencerModule = seqModule;
		return;
	}

	void createChildren()
	{
		multiSeq_ShiftVoltageSubMenuItem* menuItem = new multiSeq_ShiftVoltageSubMenuItem(STR_CURR_EDIT_CHANNEL, multiSeq_ShiftVoltageSubMenuItem::ShiftType::CurrentChannelOnly, this->amount, this->sequencerModule);
		addChild(menuItem); //this->pushChild(menuItem);
		menuItem = new multiSeq_ShiftVoltageSubMenuItem(STR_CURR_EDIT_PATTERN, multiSeq_ShiftVoltageSubMenuItem::ShiftType::ThisPattern, this->amount, this->sequencerModule);
		addChild(menuItem);// this->pushChild(menuItem);
		menuItem = new multiSeq_ShiftVoltageSubMenuItem(STR_ALL_PATTERNS, multiSeq_ShiftVoltageSubMenuItem::ShiftType::AllPatterns, this->amount, this->sequencerModule);
		addChild(menuItem);// this->pushChild(menuItem);
		return;
	}
};
// First tier menu item. Create Submenu
struct multiSeq_ShiftVoltageMenuItem : MenuItem {
	multiSeq* sequencerModule;
	float amount = 1.0;

	multiSeq_ShiftVoltageMenuItem(std::string text, float amount, multiSeq* seqModule)
	{
		this->box.size.x = 200;
		this->text = text;
		this->amount = amount;
		this->sequencerModule = seqModule;
		return;
	}
	Menu *createChildMenu() override {
		multiSeq_ShiftVoltageSubMenu* menu = new multiSeq_ShiftVoltageSubMenu(amount, sequencerModule);
		menu->amount = this->amount;
		menu->sequencerModule = this->sequencerModule;
		menu->createChildren();
		menu->box.size = Vec(200, 60);
		return menu;
	}
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiSeqWidget
// Create context menu with the ability to shift 1 V (1 octave).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void multiSeqWidget::appendContextMenu(ui::Menu *menu)
{
	TSSequencerWidgetBase::appendContextMenu(menu);
	
	// Add voltSeq specific options:
	//MenuLabel *spacerLabel = new MenuLabel();
	//menu->addChild(spacerLabel); //menu->pushChild(spacerLabel);
	menu->addChild(new ui::MenuSeparator);

	multiSeq* sequencerModule = dynamic_cast<multiSeq*>(module);
	assert(sequencerModule);

	//-------- Shift Values Up/Down ------- //
	// (Affects N steps).
	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "Shift Values";
	menu->addChild(modeLabel); //menu->pushChild(modeLabel);

	menu->addChild(new multiSeq_ShiftVoltageMenuItem("> +1 V/Octave/Patt", 1.0, sequencerModule));
	menu->addChild(new multiSeq_ShiftVoltageMenuItem("> -1 V/Octave/Patt", -1.0, sequencerModule));
}
