#include <string.h>
#include <stdio.h>
#include <math.h> 
#include "trowaSoft.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"
#include "Module_voltSeq.hpp"



// Single model object? https://github.com/VCVRack/Rack/issues/258:
Model* modelVoltSeq = createModel<voltSeq, voltSeqWidget>(/*slug*/ "voltSeq");

ValueSequencerMode* voltSeq_DEFAULT_VALUE_MODE = new NoteValueSequencerMode(/*displayName*/ "NOTE",			
			/*inVoltageMin*/ voltSeq_STEP_KNOB_MIN, /*inVoltageMax*/ voltSeq_STEP_KNOB_MAX); 


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// voltSeq()
// @numSteps: (IN) Number of steps.
// @numRows: (IN) Number of rows for layout.
// @numCols: (IN) Number of columns for layout.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
voltSeq::voltSeq(int numSteps, int numRows, int numCols) : TSSequencerModuleBase(numSteps, numRows, numCols, /*default val*/ 0.0, /*default value mode*/ TSSequencerModuleBase::ValueMode::VALUE_VOLT) // Now default to 0 instead of -10
{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED	
	for (int p = 0; p < 64; p++)
	{
		float pVolt = PatternToVolts(p);
		int patternReturned = VoltsToPattern(pVolt);
		DEBUG("Pattern Index %d. Voltage = %10.8f. Pattern returned %d.", p, pVolt, patternReturned);
		// This seems OK but getting after being translated by our ValueSequenceMode object, I guess it gets funky (to 8 decimals it seems ok though).
		// So, just add a little adjustment in module base when we do VoltsToPattern (volts2PatternAdj).
	}
#endif 	
	
	numValueModesSupported = 3;
	valueModesSupported = new ValueMode[numValueModesSupported] { ValueMode::VALUE_VOLT, ValueMode::VALUE_MIDINOTE, ValueMode::VALUE_PATTERN };		
	selectedOutputValueMode = defaultChannelValueMode;
	selectedOutputValueModeIx = getSupportedValueModeIndex(selectedOutputValueMode);
	lastOutputValueMode = selectedOutputValueMode;
	
	numStructuredRandomPatterns = TROWA_SEQ_NUM_RANDOM_PATTERNS; // voltSeq can use the full range of random patterns.

	oscLastSentVals = new float[numSteps];	
	for (int s = 0; s < numSteps; s++)
	{
		// Configure step parameters:
		//configParam(TSSequencerModuleBase::CHANNEL_PARAM + s, voltSeq_STEP_KNOB_MIN, voltSeq_STEP_KNOB_MAX, /*default*/ defaultStateValue, /*label*/ "Step " + std::to_string(s+1));		
		configParam<TS_ValueSequencerParamQuantity>(TSSequencerModuleBase::CHANNEL_PARAM + s, voltSeq_STEP_KNOB_MIN, voltSeq_STEP_KNOB_MAX, /*default*/ defaultStateValue, /*label*/ "Step " + std::to_string(s+1));		
		oscLastSentVals[s] = voltSeq_STEP_KNOB_MIN - 1.0;
	}	
	this->reconfigureValueModeParamQty();	
	this->configValueModeParam();	
	// Populate labels.
	this->populateNotesPatternsLabels();
	return;
}
// Configure the value mode parameters on the steps.
void voltSeq::configValueModeParam()
{
	// v1.0.4 - selectedOutputValue is no longer an index into this array (subtract VALUE_VOLT).
	if (selectedOutputValueMode < ValueMode::VALUE_VOLT)
	{
		selectedOutputValueMode = (ValueMode)(selectedOutputValueMode + ValueMode::VALUE_VOLT);
		selectedOutputValueModeIx = getSupportedValueModeIndex(selectedOutputValueMode);
		DEBUG("voltSeq(%d) - FIXED selectedOutputValueMode = %d, index = %d.",  oscId, selectedOutputValueMode, selectedOutputValueModeIx);		
	}
	ValueSequencerMode* currOutputValueMode = ValueModes[selectedOutputValueModeIx];// ValueModes[selectedOutputValueMode - ValueMode::VALUE_VOLT];
	for (int s = 0; s < maxSteps; s++)
	{
		// Configure step parameters:
		TS_ValueSequencerParamQuantity* pQuantity = dynamic_cast<TS_ValueSequencerParamQuantity*>( this->paramQuantities[TSSequencerModuleBase::CHANNEL_PARAM + s] );
		pQuantity->setValueMode(currOutputValueMode);
	}
	return;
}


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// voltSeq::randomize()
// Only randomize the current gate/trigger steps.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void voltSeq::onRandomize(const RandomizeEvent& e)
{
	//int r, c;
	valuesChanging = true;
	for (int s = 0; s < maxSteps; s++) 
	{
		// random::uniform() - [0.0, 1.0)
		triggerState[currentPatternEditingIx][currentChannelEditingIx][s] = voltSeq_STEP_KNOB_MIN + random::uniform()*(voltSeq_STEP_KNOB_MAX - voltSeq_STEP_KNOB_MIN);		
		this->params[CHANNEL_PARAM + s].setValue(this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s]);
		//r = s / this->numCols; // TROWA_SEQ_STEP_NUM_COLS;
		//c = s % this->numCols; // TROWA_SEQ_STEP_NUM_COLS;		
		//knobStepMatrix[r][c]->setKnobValue(this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s]);			
	}	
	reloadEditMatrix = true;
	valuesChanging = false;
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
	triggerState[pattern][channel][step] = val;
	r = step / this->numCols;
	c = step % this->numCols;
	if (pattern == currentPatternEditingIx && channel == currentChannelEditingIx)
	{
		if (triggerState[pattern][channel][step])
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
		DEBUG("voltSeq:step() - Received a msg (s=%d, v=%0.2f, c=%d, p=%d), sending back (%s).",
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
		//this->knobStepMatrix[r][c]->setKnobValue(val);
		this->params[ParamIds::CHANNEL_PARAM + step].setValue(val);
	}
	return;
} // end setStepValue()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Shift all steps (+/-) some number of volts.
// @patternIx : (IN) The index into our pattern matrix (0-15). Or TROWA_INDEX_UNDEFINED for all patterns.
// @channelIx : (IN) The index of the channel (gate/trigger/voice) if any (0-15, or TROWA_SEQ_COPY_CHANNELIX_ALL/TROWA_INDEX_UNDEFINED for all).
// @volts: (IN) The number of volts to add.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void voltSeq::shiftValues(/*in*/ int patternIx, /*in*/ int channelIx, /*in*/ float volts)
{
	// Normal Range -10 to +10 V (20)
	// Midi: -5 to +5 V or -4 to + 6 V (10), so +1 octave will be +2V in 'Normal' range.
	float add = volts;

	if (selectedOutputValueMode == ValueMode::VALUE_MIDINOTE)
	{
		add = volts * 2.0;
	}
	else if (selectedOutputValueMode == ValueMode::VALUE_PATTERN)
	{
		add = (voltSeq_STEP_KNOB_MAX - voltSeq_STEP_KNOB_MIN) / TROWA_SEQ_NUM_PATTERNS * volts;
	}
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
		for (int channelIx = 0; channelIx < TROWA_SEQ_NUM_CHNLS; channelIx++)
		{
			for (int s = 0; s < maxSteps; s++)
			{
				float tmp = clamp(triggerState[patternIx][channelIx][s] + add, /*min*/ voltSeq_STEP_KNOB_MIN,  /*max*/ voltSeq_STEP_KNOB_MAX);
				triggerState[patternIx][channelIx][s] = tmp;
				if (patternIx == currentPatternEditingIx && channelIx == currentChannelEditingIx)
				{
					this->params[CHANNEL_PARAM + s].setValue(tmp);
					//int r = s / numCols;
					//int c = s % numCols;					
					//knobStepMatrix[r][c]->setKnobValue(tmp);
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
		for (int s = 0; s < maxSteps; s++)
		{
			float tmp = clamp(triggerState[patternIx][channelIx][s] + add, /*min*/ voltSeq_STEP_KNOB_MIN,  /*max*/ voltSeq_STEP_KNOB_MAX);
			DEBUG(" %d = %f + %fV (add %f) = %f", s, triggerState[patternIx][channelIx][s], volts, add, tmp);
			triggerState[patternIx][channelIx][s] = tmp;
			if (patternIx == currentPatternEditingIx && channelIx == currentChannelEditingIx)
			{
				this->params[CHANNEL_PARAM + s].setValue(tmp);
				//int r = s / numCols;
				//int c = s % numCols;				
				//knobStepMatrix[r][c]->setKnobValue(tmp);
			}
		}
		//if (patternIx == currentPatternEditingIx && channelIx == currentChannelEditingIx)
		//	this->reloadEditMatrix = true;
	}
	return;
} // end shiftValues()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// process()
// [Previously step(void)]
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void voltSeq::process(const ProcessArgs &args)
{
	
	// Now calculate some base items all the time:
	float clockTime = 0.0f;
	bool nextStep = isNewStep(args.sampleRate, &clockTime);	
#if TROWA_SEQ_USE_INTERNAL_DIVISOR	
	if (nextStep || resetTriggered || this->ctlMsgQueue.size() > 0)  // Eval if new step, reset triggered or OSC message
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

	TSSequencerModuleBase::getStepInputs(args, &pulse, &reloadMatrix, &valueModeChanged, nextStep, clockTime);
	int r = 0;
	int c = 0;
	ValueSequencerMode* currOutputValueMode = ValueModes[selectedOutputValueModeIx];// ValueModes[selectedOutputValueMode - ValueMode::VALUE_VOLT];
	if (valueModeChanged)
	{
		modeString = currOutputValueMode->displayName;
		channelValueModes[currentChannelEditingIx] = selectedOutputValueMode;
		for (int s = 0; s < maxSteps; s++)
		{
			paramQuantities[ParamIds::NUM_PARAMS + s]->defaultValue = currOutputValueMode->zeroValue;
		}
		// Knob Type change now handled in Widget
		// // Change our lights 
		// for (r = 0; r < this->numRows; r++)
		// {
			// for (c = 0; c < this->numCols; c++)
			// {				
				// dynamic_cast<TS_LightArc*>(padLightPtrs[r][c])->zeroAnglePoint = currOutputValueMode->zeroPointAngle_radians;
				// dynamic_cast<TS_LightArc*>(padLightPtrs[r][c])->valueMode = currOutputValueMode;
				// /// TODO: Move this to widget for when we are headless
				// //knobStepMatrix[r][c]->paramQuantity->defaultValue = currOutputValueMode->zeroValue;
// #if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				// //DEBUG("Setting Knob %d default value to %.2f", knobStepMatrix[r][c]->id, knobStepMatrix[r][c]->paramQuantity->getDefaultValue());
// #endif
			// }
		// }
		// Change our ParamQuantities
		this->configValueModeParam();
	}
	lastOutputValueMode = selectedOutputValueMode;
		
	// Only send OSC if it is enabled, initialized, and we are in EDIT mode.
	sendOSC = useOSC && currentCtlMode == ExternalControllerMode::EditMode && oscInitialized;
	//-- * Load the trigger we are editing into our button matrix for display:
	// This is what we are showing not what we playing
	char valOutputBuffer[20] = { 0 };
	char addrBuff[TROWA_SEQ_BUFF_SIZE] = { 0 };
	std::string stepStringAddr = std::string(oscAddrBuffer[SeqOSCOutputMsg::EditStepString]);
	if (reloadMatrix || reloadEditMatrix || valueModeChanged)
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
			//padLightPtrs[r][c]->setColor(voiceColors[currentChannelEditingIx]);
			gateLights[r][c] = 1.0 - stepLights[r][c];
			this->params[CHANNEL_PARAM + s].setValue(this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s]);
			//knobStepMatrix[r][c]->setKnobValue(this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s]);			
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
					sprintf(addrBuff, OSC_TOUCH_OSC_CHANGE_COLOR_FS, addrBuff);
					oscStream << osc::BeginMessage(addrBuff)
						<< touchOSC::ChannelColors[currentChannelEditingIx]
						<< osc::EndMessage;
					// LED Color (current step LED):
					sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::PlayStepLed], s + 1);
					sprintf(addrBuff, OSC_TOUCH_OSC_CHANGE_COLOR_FS, addrBuff);
					oscStream << osc::BeginMessage(addrBuff)
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
		const float threshold = TROWA_VOLTSEQ_KNOB_CHANGED_THRESHOLD;
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
				// voltSeq should send the actual values.
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
		// [v1.0.4] ValueMode isn't 0 starting for voltSeq anymore. 
		int k = getSupportedValueModeIndex(channelValueModes[g]);
		if (k < 0)
			k = 0;
		ValueSequencerMode* chMode = ValueModes[k];
		float gate = (running && gOn) ? chMode->GetOutputValue( triggerState[currentPatternPlayingIx][g][index] ) : 0.0; //***********VOLTAGE OUTPUT		
		outputs[CHANNELS_OUTPUT + g].value= gate;
		// Output lights (around output jacks for each gate/trigger):
		gateLightsOut[g] = (gate < 0) ? -gate : gate;
		lights[CHANNEL_LIGHTS + g].value = gate / chMode->outputVoltageMax;// currOutputValueMode->outputVoltageMax;
	}
	return;
} // end step()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// voltSeqWidget()
// Widget for the trowaSoft 16-step voltage/knobby sequencer.
// @seqModule : (IN) Pointer to the sequencer module.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
voltSeqWidget::voltSeqWidget(voltSeq* seqModule) : TSSequencerWidgetBase(seqModule)
{		
	bool isPreview = this->module == NULL; // If this is null, then this isn't a real module instance but a 'Preview'
	//////////////////////////////////////////////
	// Background
	//////////////////////////////////////////////
	{
		SvgPanel *panel = new SvgPanel();
		panel->box.size = box.size;
		panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/voltSeq.svg")));
		addChild(panel);
	}
	
	this->TSSequencerWidgetBase::addBaseControls(false);
	
	// (User) Input KNOBS ==================================================	
	int y = 115;
	int x = 79;
	int dx = 0;
	//int lightSize = 50 - 2*dx;
	Vec lSize = Vec(50, 50);
	int v = 0;
	numCols = 4;
	numRows = 4;
	NVGcolor lightColor = TSColors::COLOR_TS_RED;
	if (!isPreview) {
		numRows = seqModule->numRows;
		numCols = seqModule->numCols;
		// v1.0.4 - selectedOutputValue is no longer an index into this array (subtract VALUE_VOLT).
		currValueModePtr = seqModule->ValueModes[seqModule->selectedOutputValueModeIx];
		seqModule->modeString = currValueModePtr->displayName;
		lightColor = seqModule->voiceColors[seqModule->currentChannelEditingIx];
		lastMode = seqModule->selectedOutputValueMode;
	}
	else 
	{
		currValueModePtr = voltSeq_DEFAULT_VALUE_MODE;
	}
	lastValueModePtr = currValueModePtr;

	///////////// KNOBS //////////////////
	//knobStepMatrix = new TS_LightedKnob**[numRows];
	padLightPtrs = new ColorValueLight**[numRows];
	for (int r = 0; r < numRows; r++)
	{
		//knobStepMatrix[r] = new TS_LightedKnob*[numCols];		
		padLightPtrs[r] = new ColorValueLight*[numCols];
		for (int c = 0; c < numCols; c++)
		{	
			/// TODO: Combine the LightedKnob and LightArc now that we know more about the components
			// Pad Knob:
			//TS_LightedKnob* knobPtr = dynamic_cast<TS_LightedKnob*>(createParam<TS_LightedKnob>(Vec(x, y), seqModule, TSSequencerModuleBase::CHANNEL_PARAM + r*numCols + c));
			//knobPtr->id = r * numCols + c;
			//addParam(knobPtr);
			TS_VoltSeqLightKnob* knobPtr = dynamic_cast<TS_VoltSeqLightKnob*>(createParam<TS_VoltSeqLightKnob>(Vec(x, y), seqModule, TSSequencerModuleBase::CHANNEL_PARAM + r * numCols + c));
			knobPtr->id = r * numCols + c;
			knobPtr->seqModule = seqModule; // Typed pointer
			addParam(knobPtr);

			/// TODO: Add custom light knob where we can append context menu
			/// uniquely for each valuemode.
			
			// Keep a reference to our pad lights so we can change the colors			
			TS_LightArc* lightPtr = dynamic_cast<TS_LightArc*>(TS_createColorValueLight<TS_LightArc>(/*pos */ Vec(x+dx, y+dx), 
				/*seqModule*/ seqModule,
				/*lightId*/ TSSequencerModuleBase::PAD_LIGHTS + r*numCols + c,								
				/* size */ lSize, /* color */ lightColor));			
			lightPtr->paramWidget = knobPtr;
			lightPtr->currentAngle_radians = &(knobPtr->currentAngle);
			lightPtr->zeroAnglePoint = currValueModePtr->zeroPointAngle_radians;
			lightPtr->valueMode = currValueModePtr;
			this->padLightPtrs[r][c] = lightPtr;
			addChild( lightPtr );
			
			x+= 59;
			v++;
		}		
		y += 59; // Next row
		x = 79;
	} // end loop through 4x4 grid
	
	if (!isPreview)
		seqModule->initialized = true;
	return;
}

void voltSeqWidget::step()
{
	TSSequencerWidgetBase::step();
	
	if (this->module == NULL)
		return;
	
	voltSeq* seqModule =  dynamic_cast<voltSeq*>(this->module);
	// Switch the defaults on the knobs based on the output mode (if changed):
	if (lastMode != seqModule->selectedOutputValueMode)
	{
		ValueSequencerMode* currValueMode = seqModule->ValueModes[seqModule->selectedOutputValueModeIx];
		for (int r = 0; r < numRows; r++)
		{
			for (int c = 0; c < numCols; c++)
			{						
				TS_LightArc* stepLight = dynamic_cast<TS_LightArc*>(padLightPtrs[r][c]);
				stepLight->zeroAnglePoint = currValueMode->zeroPointAngle_radians;
				stepLight->valueMode = currValueMode;
			}
		}		
	}
	lastMode = seqModule->selectedOutputValueMode;
	return;
}

struct voltSeq_ShiftVoltageSubMenuItem : MenuItem {
	voltSeq* sequencerModule;
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

	voltSeq_ShiftVoltageSubMenuItem(std::string text, ShiftType target, float amount, voltSeq* seqModule)
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

struct voltSeq_ShiftVoltageSubMenu : Menu {
	voltSeq* sequencerModule;
	float amount = 1.0;

	voltSeq_ShiftVoltageSubMenu(float amount, voltSeq* seqModule)
	{
		this->box.size = Vec(200, 60);
		this->amount = amount;
		this->sequencerModule = seqModule;
		return;
	}

	void createChildren()
	{
		voltSeq_ShiftVoltageSubMenuItem* menuItem = new voltSeq_ShiftVoltageSubMenuItem(STR_CURR_EDIT_CHANNEL, voltSeq_ShiftVoltageSubMenuItem::ShiftType::CurrentChannelOnly, this->amount, this->sequencerModule);
		addChild(menuItem); //this->pushChild(menuItem);
		menuItem = new voltSeq_ShiftVoltageSubMenuItem(STR_CURR_EDIT_PATTERN, voltSeq_ShiftVoltageSubMenuItem::ShiftType::ThisPattern, this->amount, this->sequencerModule);
		addChild(menuItem);// this->pushChild(menuItem);
		menuItem = new voltSeq_ShiftVoltageSubMenuItem(STR_ALL_PATTERNS, voltSeq_ShiftVoltageSubMenuItem::ShiftType::AllPatterns, this->amount, this->sequencerModule);
		addChild(menuItem);// this->pushChild(menuItem);
		return;
	}
};
// First tier menu item. Create Submenu
struct voltSeq_ShiftVoltageMenuItem : MenuItem {
	voltSeq* sequencerModule;
	float amount = 1.0;

	voltSeq_ShiftVoltageMenuItem(std::string text, float amount, voltSeq* seqModule)
	{
		this->box.size.x = 200;
		this->text = text;
		this->amount = amount;
		this->sequencerModule = seqModule;
		return;
	}
	Menu *createChildMenu() override {
		voltSeq_ShiftVoltageSubMenu* menu = new voltSeq_ShiftVoltageSubMenu(amount, sequencerModule);
		menu->amount = this->amount;
		menu->sequencerModule = this->sequencerModule;
		menu->createChildren();
		menu->box.size = Vec(200, 60);
		return menu;
	}
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// voltSeqWidget
// Create context menu with the ability to shift 1 V (1 octave).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//Menu* voltSeq::createWidgetContextMenu()
void voltSeqWidget::appendContextMenu(ui::Menu *menu)
{
	TSSequencerWidgetBase::appendContextMenu(menu);
	//Menu *menu = TSSequencerWidgetBase::createContextMenu();
	

	// Add voltSeq specific options:
	//MenuLabel *spacerLabel = new MenuLabel();
	//menu->addChild(spacerLabel); //menu->pushChild(spacerLabel);
	menu->addChild(new ui::MenuSeparator);

	voltSeq* sequencerModule = dynamic_cast<voltSeq*>(module);
	assert(sequencerModule);

	//-------- Shift Values Up/Down ------- //
	// (Affects N steps).
	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "Shift Values";
	menu->addChild(modeLabel); //menu->pushChild(modeLabel);

	//voltSeq_ShiftVoltageMenuItem* menuItem = new voltSeq_ShiftVoltageMenuItem("> +1 V/Octave/Patt", 1.0, sequencerModule);
	menu->addChild(new voltSeq_ShiftVoltageMenuItem("> +1 V/Octave/Patt", 1.0, sequencerModule));// menu->pushChild(menuItem);
	//menuItem = new voltSeq_ShiftVoltageMenuItem("> -1 V/Octave/Patt", -1.0, sequencerModule);
	menu->addChild(new voltSeq_ShiftVoltageMenuItem("> -1 V/Octave/Patt", -1.0, sequencerModule));// menu->pushChild(menuItem);
}

// If sequencer step, handle CTRL-C, CTRL-V
void TS_VoltSeqLightKnob::onHoverKey(const HoverKeyEvent& e) {
	if (this->module == NULL)
		return;
	if (seqModule != NULL)
		controlSeqHandleStepKeyboardInput(e, this, seqModule);
	if (!e.isConsumed())
	{
		// Base method
		this->ParamWidget::onHoverKey(e);
	}
	return;
}


// Add enumeration menu if needed and copy/paste row.
void TS_VoltSeqLightKnob::appendContextMenu(ui::Menu* menu) {
	if (module == NULL || seqModule == NULL)
		return;
	ParamQuantity* pq = getParamQuantity();
	if (pq == NULL)
		return;
	controlAppendContextMenuSelect(menu, this, seqModule);
	controlAppendContextMenuCopyRowCol(menu, this, seqModule);
	return;
}


