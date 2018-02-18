#include <string.h>
#include <stdio.h>
#include "trowaSoft.hpp"
#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"
#include "Module_trigSeq.hpp"
#include "TSOSCSequencerOutputMessages.hpp"
#include "TSOSCCommon.hpp"

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// trigSeq::randomize()
// Only randomize the current gate/trigger steps.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void trigSeq::randomize()
{
	for (int s = 0; s < maxSteps; s++) 
	{
		triggerState[currentPatternEditingIx][currentChannelEditingIx][s] = (randomf() > 0.5);		
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
// trigSeq::step()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void trigSeq::step() {
	if (!initialized)
		return;

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
	TSSequencerModuleBase::getStepInputs(&pulse, &reloadMatrix, &valueModeChanged);
	if (valueModeChanged)
	{
		// Gate Mode has changed
		gateMode = static_cast<GateMode>((short)(selectedOutputValueMode));
		modeString = modeStrings[selectedOutputValueMode];
	}


	// Only send OSC if it is enabled, initialized, and we are in EDIT mode.
	sendOSC = useOSC && oscInitialized; //&& currentCtlMode == ExternalControllerMode::EditMode
	char addrBuff[50] = { 0 };
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
			if (triggerState[currentPatternEditingIx][currentChannelEditingIx][s])
			{
				gateLights[r][c] = 1.0f - stepLights[r][c];
				gateTriggers[s].state = SchmittTrigger::HIGH;				
			} 
			else
			{
				gateLights[r][c] = 0.0f; // Turn light off	
				gateTriggers[s].state = SchmittTrigger::LOW;
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
					sprintf(addrBuff, OSC_TOUCH_OSC_CHANGE_COLOR_FS, addrBuff);
					oscStream << osc::BeginMessage(addrBuff)
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
	else
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
			if (gateTriggers[s].process(params[ParamIds::CHANNEL_PARAM + s].value)) 
			{
				triggerState[currentPatternEditingIx][currentChannelEditingIx][s] = !triggerState[currentPatternEditingIx][currentChannelEditingIx][s];
				sendLightVal = sendOSC; // Value has changed.
			}
			r = s / this->numCols; // TROWA_SEQ_STEP_NUM_COLS;
			c = s % this->numCols; // TROWA_SEQ_STEP_NUM_COLS;
			stepLights[r][c] -= stepLights[r][c] / lightLambda / engineGetSampleRate();
			
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
				debug("Step changed %d (new val is %.2f), sending OSC %s", s, triggerState[currentPatternEditingIx][currentChannelEditingIx][s], addrBuff);
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
	
	// Set Outputs (16 triggers)	
	gOn = true;
	if (gateMode == TRIGGER)
		gOn = pulse;  // gateOn = gateOn && pulse;
	else if (gateMode == RETRIGGER)
		gOn = !pulse; // gateOn = gateOn && !pulse;		
	for (int g = 0; g < TROWA_SEQ_NUM_CHNLS; g++) 
	{
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
// trigSeqWidget
// Widget for the trowaSoft pad / trigger sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
trigSeqWidget::trigSeqWidget() : TSSequencerWidgetBase()
{
	trigSeq *module = new trigSeq();
	setModule(module);
	
	//////////////////////////////////////////////
	// Background
	//////////////////////////////////////////////	
	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/trigSeq.svg")));
		addChild(panel);
	}
	
	TSSequencerWidgetBase::addBaseControls(false);
	
	// (User) Input Pads ==================================================	
	int y = 115;
	int x = 79;
	int dx = 3;
	Vec lSize = Vec(50 - 2*dx, 50 - 2*dx);
	for (int r = 0; r < module->numRows; r++) //---------THE PADS
	{
		for (int c = 0; c < module->numCols; c++)
		{			
			// Pad buttons:
			TS_PadSquare* padBtn = dynamic_cast<TS_PadSquare*>(createParam<TS_PadSquare>(Vec(x, y), module, TSSequencerModuleBase::CHANNEL_PARAM + r * module->numCols + c, 0.0, 1.0, 0.0));
			padBtn->groupId = module->oscId; // For now this is unique.
			addParam(padBtn);
			// Keep a reference to our pad lights so we can change the colors
			module->padLightPtrs[r][c] = dynamic_cast<TS_LightSquare*>(TS_createColorValueLight<TS_LightSquare>(/*pos */ Vec(x+dx, y+dx), 
				/*module*/ module, 
				/*lightId*/ TSSequencerModuleBase::PAD_LIGHTS + r*module->numCols + c,
				/* size */ lSize, /* color */ module->voiceColors[module->currentChannelEditingIx]));
			addChild( module->padLightPtrs[r][c] );
			x+= 59;
		}		
		y += 59; // Next row
		x = 79;
	} // end loop through MxN grid	
	module->modeString = module->modeStrings[module->selectedOutputValueMode];
	module->initialized = true;
	return;
} // end trigSeqWidget()


