#include <string.h>
#include <stdio.h>
#include <exception>
#include "TSSequencerModuleBase.hpp"
#include "trowaSoft.hpp"
//#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "Module_trigSeq.hpp"
#include "TSOSCSequencerOutputMessages.hpp"
#include "TSOSCCommon.hpp"
#include "TSSequencerWidgetBase.hpp"


Model* modelTrigSeq = createModel<trigSeq, trigSeqWidget>(/*slug*/ "trigSeq");
Model* modelTrigSeq64 = createModel<trigSeq64, trigSeq64Widget>(/*slug*/ "trigSeq64");



trigSeq::trigSeq(int numSteps, int numRows, int numCols) : TSSequencerModuleBase(numSteps, numRows, numCols, false, TSSequencerModuleBase::ValueMode::VALUE_TRIGGER)
{
	// [v1.0.4] Set the value modes we support:
	numValueModesSupported = 3;
	valueModesSupported = new ValueMode[numValueModesSupported] { ValueMode::VALUE_TRIGGER, ValueMode::VALUE_RETRIGGER, ValueMode::VALUE_CONTINUOUS };	
	selectedOutputValueMode = defaultChannelValueMode;
	selectedOutputValueModeIx = getSupportedValueModeIndex(selectedOutputValueMode);
	lastOutputValueMode = selectedOutputValueMode;
	
	gateTriggers = new dsp::SchmittTrigger[numSteps]; // maxSteps
	// Configure Parameters:
	for (int s = 0; s < maxSteps; s++) 
	{
		configSwitch(TSSequencerModuleBase::CHANNEL_PARAM + s, 0, 1, defaultStateValue, "Step " + std::to_string(s+1), {"Off", "On"});;
		//configParam(TSSequencerModuleBase::CHANNEL_PARAM + s, 0.0, 1.0, defaultStateValue, /*label*/ "Step " + std::to_string(s+1));
	}
	
	this->reconfigureValueModeParamQty();
	return;
}


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// trigSeq::randomize()
// Only randomize the current gate/trigger steps.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void trigSeq::onRandomize(const RandomizeEvent& e)
{
	for (int s = 0; s < maxSteps; s++) 
	{
		triggerState[currentPatternEditingIx][currentChannelEditingIx][s] = (random::uniform() > 0.5);		
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
float trigSeq::getToggleStepValue(int step, float val, int channel, int pattern)
{
	return !(bool)(triggerState[pattern][channel][step]);
} // end getToggleStepValue()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Calculate a representation of all channels for this step
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
float trigSeq::getPlayingStepValue(int step, int pattern)
{
	/// TODO: REMOVE THIS, NOT USED ANYMORE
	int count = 0;
	for (int c = 0; c < TROWA_SEQ_NUM_CHNLS; c++)
	{
		count += (bool)(this->triggerState[pattern][c][step]);
	} // end for
	return (float)(count) / (float)(TROWA_SEQ_NUM_CHNLS);
} // end getPlayingStepValue()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// process()
// [Previously step(void)]
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void trigSeq::process(const ProcessArgs &args) 
{
	if (!initialized)
		return;	
	
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
	int r = 0;
	int c = 0;

	//------------------------------------------------------------
	// Get our common sequencer inputs
	//------------------------------------------------------------
	TSSequencerModuleBase::getStepInputs(args, &pulse, &reloadMatrix, &valueModeChanged, nextStep, clockTime);
	if (valueModeChanged)
	{
		// v1.0.4 - This should be OK for trigSeq (he uses the first 3 ValueModes).
		// Gate Mode has changed
		gateMode = static_cast<GateMode>((short)(selectedOutputValueMode));
		modeString = modeStrings[selectedOutputValueMode];		
		channelValueModes[currentChannelEditingIx] = selectedOutputValueMode;
	}


	// Only send OSC if it is enabled, initialized, and we are in EDIT mode.
	sendOSC = useOSC && oscInitialized; //&& currentCtlMode == ExternalControllerMode::EditMode
	char addrBuff[TROWA_SEQ_BUFF_SIZE] = { 0 };
	char colorAddrBuff[TROWA_SEQ_BUFF_SIZE] = { 0 }; // 2nd buffer to remove my lazy re-using of buffers (technically undefined behavior)
	//-- * Load the trigger we are editing into our button matrix for display:
	// This is what we are showing not what we playing
	int gridRow, gridCol; // for touchOSC grids
	if (reloadMatrix)
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
		// Load this gate and/or pattern into our 4x4 matrix
		this->currentStepMatrixColor = voiceColors[currentChannelEditingIx];
		for (int s = 0; s < maxSteps; s++) 
		{
			r = s / this->numCols; // TROWA_SEQ_STEP_NUM_COLS;
			c = s % this->numCols; // TROWA_SEQ_STEP_NUM_COLS;

			//padLightPtrs[r][c]->setColor(voiceColors[currentChannelEditingIx]);
			if (triggerState[currentPatternEditingIx][currentChannelEditingIx][s])
			{
				gateLights[r][c] = 1.0f - stepLights[r][c];
				gateTriggers[s].state = TriggerSignal::HIGH;
				paramQuantities[ParamIds::CHANNEL_PARAM + s]->setValue(1.0f);// Not momentary anymore
			} 
			else
			{
				gateLights[r][c] = 0.0f; // Turn light off	
				gateTriggers[s].state = TriggerSignal::LOW;
				paramQuantities[ParamIds::CHANNEL_PARAM + s]->setValue(0.0f);// Not momentary anymore
			}
			oscMutex.lock();
			if (sendOSC && oscInitialized)
			{
				if (s > 0 && s % 16 == 0) // There is a limit to client buffer size, so let's not make the bundles too large. Hopefully they can take 16-steps at a time.
				{
					// Send this bundle and then start a new one
					oscStream << osc::EndBundle;
					oscTxSocket->Send(oscStream.Data(), oscStream.Size());
					oscStream.Clear();
					// Start new bundle:
					oscStream << osc::BeginBundleImmediate;
				}
				if (this->oscCurrentClient == OSCClient::touchOSCClient)
				{
					// LED Color (current step LED):
					sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::PlayStepLed], s + 1);
					sprintf(colorAddrBuff, OSC_TOUCH_OSC_CHANGE_COLOR_FS, addrBuff);
					oscStream << osc::BeginMessage(colorAddrBuff)
						<< touchOSC::ChannelColors[currentChannelEditingIx]
						<< osc::EndMessage;
					// Step:
					touchOSC::stepIndex_to_mcRowCol(s, numRows, numCols, &gridRow, &gridCol);
					sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditTOSC_GridStep], gridRow, gridCol); // Grid's /<row>/<col> to accomodate touchOSC's lack of multi-parameter support.
				}
				else
				{
					// Step
					sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditStep], s + 1); // Changed to /<step> to accomodate touchOSC's lack of multi-parameter support.
				}
				oscStream << osc::BeginMessage(addrBuff)
					<< triggerState[currentPatternEditingIx][currentChannelEditingIx][s]
					<< osc::EndMessage;
			}
			oscMutex.unlock();
		} // end for
		oscMutex.lock();
		if (sendOSC && oscInitialized)
		{
			// Send color of grid:
			if (this->oscCurrentClient == OSCClient::touchOSCClient)
			{
				oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditStepGridColor])
					<< touchOSC::ChannelColors[currentChannelEditingIx]
					<< osc::EndMessage;
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
	}
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

		// Step buttons/pads (for this one Channel/gate) - Read Inputs
		for (int s = 0; s < maxSteps; s++) 
		{
			bool sendLightVal = false;
			//if (gateTriggers[s].process(params[ParamIds::CHANNEL_PARAM + s].getValue())) 
			// Now normal switches:
			if (triggerState[currentPatternEditingIx][currentChannelEditingIx][s] != (params[ParamIds::CHANNEL_PARAM + s].getValue() > 0))
			{
				triggerState[currentPatternEditingIx][currentChannelEditingIx][s] = !triggerState[currentPatternEditingIx][currentChannelEditingIx][s];
				sendLightVal = sendOSC; // Value has changed.
			}
			r = s / this->numCols; // TROWA_SEQ_STEP_NUM_COLS;
			c = s % this->numCols; // TROWA_SEQ_STEP_NUM_COLS;
			stepLights[r][c] -= stepLights[r][c] / lightLambda / args.sampleRate;
			
			gateLights[r][c] = (triggerState[currentPatternEditingIx][currentChannelEditingIx][s]) ? 1.0 - stepLights[r][c] : stepLights[r][c];
			lights[PAD_LIGHTS + s].value = gateLights[r][c];

			oscMutex.lock();
			// This step has changed and we are doing OSC
			if (sendLightVal && oscInitialized)
			{
				// Send the step value
				if (this->oscCurrentClient == OSCClient::touchOSCClient)
				{
					touchOSC::stepIndex_to_mcRowCol(s, numRows, numCols, &gridRow, &gridCol);
					sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditTOSC_GridStep], gridRow, gridCol); // Grid's /<row>/<col> to accomodate touchOSC's lack of multi-parameter support.
				}
				else
				{
					sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditStep], s + 1); // Changed to /<step> to accomodate touchOSC's lack of multi-parameter support.
				}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				DEBUG("Step changed %d (new val is %.2f), sending OSC %s", s, triggerState[currentPatternEditingIx][currentChannelEditingIx][s], addrBuff);
#endif
				oscStream << osc::BeginMessage(addrBuff)
					<< triggerState[currentPatternEditingIx][currentChannelEditingIx][s]
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
	} // end else (read buttons)
	
	// Set Outputs (16 Channels)	
	// gOn = true;
	// if (gateMode == TRIGGER)
		// gOn = pulse;  // gateOn = gateOn && pulse;
	// else if (gateMode == RETRIGGER)
		// gOn = !pulse; // gateOn = gateOn && !pulse;		
	for (int g = 0; g < TROWA_SEQ_NUM_CHNLS; g++) 
	{
		switch (channelValueModes[g]) // [v1.1] Each channel can have its own output mode (trigger/retrigger/gate).
		{
			case VALUE_TRIGGER:
				gOn = pulse;
				break;
			case VALUE_RETRIGGER:
				gOn = !pulse;
				break;
			default:
				gOn = true;
				break;
		}
		float gate = (running && gOn && (triggerState[currentPatternPlayingIx][g][index])) ? trigSeq_GATE_ON_OUTPUT : trigSeq_GATE_OFF_OUTPUT;
		outputs[CHANNELS_OUTPUT + g].value= gate;
		// Output lights (around output jacks for each gate/trigger):		
		lights[CHANNEL_LIGHTS + g].value = (running && triggerState[currentPatternPlayingIx][g][index]) ? 1.0 : 0;
	}	
	// Now we have to keep track of this for OSC...
	prevIndex = index;
	return;
} // end step()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// trigSeqWidget()
// Widget for the trowaSoft 16-step pad / trigger sequencer.
// @seqModule : (IN) Pointer to the sequencer module.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
trigSeqWidget::trigSeqWidget(trigSeq* seqModule) : TSSequencerWidgetBase(seqModule)
{
	// [02/24/2018] Adjusted for 0.60 differences. Main issue is possiblity of NULL module...
	bool isPreview = this->module == NULL; // If this is null, then this isn't a real module instance but a 'Preview'?	
	if (!isPreview && seqModule == NULL)
	{
		seqModule = dynamic_cast<trigSeq*>(this->module);
	}
	
	//////////////////////////////////////////////
	// Background
	//////////////////////////////////////////////	
	{
		SvgPanel *panel = new SvgPanel();
		panel->box.size = box.size;
		panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/trigSeq.svg")));
		addChild(panel);
	}
	
	this->TSSequencerWidgetBase::addBaseControls(false);
	
	// (User) Input Pads ==================================================	
	int y = 115;
	int x = 79;
	int dx = 3;
	Vec lSize = Vec(50 - 2*dx, 50 - 2*dx);
	NVGcolor lightColor = TSColors::COLOR_TS_RED; 
	numCols = TROWA_SEQ_STEP_NUM_COLS;
	numRows = TROWA_SEQ_STEP_NUM_ROWS;
	int groupId = 0;
	if (!isPreview)
	{
		numCols = seqModule->numCols;
		numRows = seqModule->numRows;
		lightColor = seqModule->voiceColors[seqModule->currentChannelEditingIx];
		groupId = seqModule->oscId; // Use this id for now since this is unique to each module instance.
	}
	int id = 0;
	padLightPtrs = new ColorValueLight**[numRows];	
	for (int r = 0; r < numRows; r++) //---------THE PADS
	{
		padLightPtrs[r] = new ColorValueLight*[numCols];		
		for (int c = 0; c < numCols; c++)
		{			
			// Pad buttons:
			TS_PadSquare* padBtn = dynamic_cast<TS_PadSquare*>(createParam<TS_PadSquare>(Vec(x, y), seqModule, TSSequencerModuleBase::CHANNEL_PARAM + id));//, 0.0, 1.0, 0.0));
			padBtn->groupId = groupId;
			padBtn->btnId = id;
			padBtn->isSequencerStep = true;
			addParam(padBtn);

			// Lights:
			TS_LightSquare* padLight = dynamic_cast<TS_LightSquare*>(TS_createColorValueLight<TS_LightSquare>(/*pos */ Vec(x + dx, y + dx),
				/*seqModule*/ seqModule,
				/*lightId*/ TSSequencerModuleBase::PAD_LIGHTS + id, // r * numCols + c
				/* size */ lSize, /* color */ lightColor));
			addChild(padLight);
			padLight->cornerRadius = 5.0f;
			padLightPtrs[r][c] = padLight;
			// if (seqModule != NULL)
			// {
				// // Keep a reference to our pad lights so we can change the colors
				// seqModule->padLightPtrs[r][c] = padLight;
			// }
			x+= 59;
			id++;
		}		
		y += 59; // Next row
		x = 79;
	} // end loop through MxN grid	
	if (seqModule != NULL)
	{
		seqModule->modeString = seqModule->modeStrings[seqModule->selectedOutputValueMode];
		seqModule->initialized = true;
	}
	return;
} // end trigSeqWidget()


