#include <rack.hpp>
using namespace rack;

#include <random.hpp>
#include <stdio.h>
#include "TSOSCCV_Common.hpp"
#include "Module_oscCVExpander.hpp"
#include "Module_oscCV.hpp"
#include "Widget_oscCVExpander.hpp"
#include "Widget_oscCV.hpp"

// Models for trowa oscCVExpanders (default 8 channel)
Model* modelOscCVExpanderInput = createModel<oscCVExpanderInput<TROWA_OSCCVEXPANDER_DEFAULT_NUM_CHANNELS>, oscCVExpanderInputWidget<TROWA_OSCCVEXPANDER_DEFAULT_NUM_CHANNELS>>(/*slug*/ "cvOSCcv-InputExpander");
Model* modelOscCVExpanderOutput = createModel<oscCVExpanderOutput<TROWA_OSCCVEXPANDER_DEFAULT_NUM_CHANNELS>, oscCVExpanderOutputWidget<TROWA_OSCCVEXPANDER_DEFAULT_NUM_CHANNELS>>(/*slug*/ "cvOSCcv-OutputExpander");
// Models for 16-channel expanders
Model* modelOscCVExpanderInput16 = createModel<oscCVExpanderInput<16>, oscCVExpanderInputWidget<16>>(/*slug*/ "cvOSCcv-InputExpander-16");
Model* modelOscCVExpanderOutput16 = createModel<oscCVExpanderOutput<16>, oscCVExpanderOutputWidget<16>>(/*slug*/ "cvOSCcv-OutputExpander-16");
// Models for 32-channel expanders.
Model* modelOscCVExpanderInput32 = createModel<oscCVExpanderInput<32>, oscCVExpanderInputWidget<32>>(/*slug*/ "cvOSCcv-InputExpander-32");
Model* modelOscCVExpanderOutput32 = createModel<oscCVExpanderOutput<32>, oscCVExpanderOutputWidget<32>>(/*slug*/ "cvOSCcv-OutputExpander-32");


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// oscCVExpander()
// Create a module with numChannels.
// @numChannels: (IN) Number of input or output 'channels'.
// @direction: (IN) The type of expander (INPUT or OUTPUT).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
oscCVExpander::oscCVExpander(int numChannels, TSOSCCVExpanderDirection direction)
{
	int numInputs = 0;
	int numOutputs = 0;
	this->numberChannels = numChannels;
	this->numberColumns = numberChannels / CVOSCCV_EXP_NUM_CHANNELS_PER_COL;
	if (direction == TSOSCCVExpanderDirection::Input)
	{
		numInputs = numChannels * 2;
		this->expanderType = TSOSCCVExpanderDirection::Input;
		inputChannels = new TSOSCCVInputChannel[numberChannels];
		inputTriggers = new dsp::SchmittTrigger[numberChannels];		
	}
	else
	{
		numOutputs = numChannels * 2;
		this->expanderType = TSOSCCVExpanderDirection::Output;		
		outputChannels = new TSOSCCVChannel[numberChannels];
		pulseGens = new dsp::PulseGenerator[numberChannels];		
	}
	
	config(NUM_PARAMS, NUM_INPUTS + numInputs, NUM_OUTPUTS + numOutputs, NUM_LIGHTS + numChannels * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL);
	
	if (direction == TSOSCCVExpanderDirection::Input)
	{
		// [Rack v2] Add labels for inputs and outputs (and lights)
		char buffer[100];	
		for (int i = 0; i < numberChannels; i++)
		{
			int inputId = i * 2;
			sprintf(buffer, "Ch %d Trigger Send", i + 1);
			configInput(InputIds::CH_INPUT_START + inputId, buffer);
			sprintf(buffer, "Ch %d Value", i + 1);
			configInput(InputIds::CH_INPUT_START + inputId + 1, buffer);
			// Configure the Light Also:
			sprintf(buffer, "Ch %d Message Sent", i + 1);			
			configLight(LightIds::CH_LIGHT_START + inputId, buffer);

			/// TODO: If we do ever want to chart the value history, turn history tracking on
			/// inputChannels[i].setStoreHistory(true);
		}
	}
	else
	{
		// [Rack v2] Add labels for inputs and outputs (and lights)
		char buffer[100];	
		for (int i = 0; i < numberChannels; i++)
		{
			int inputId = i * 2;
			sprintf(buffer, "Ch %d Received Trigger", i + 1);;
			configOutput(OutputIds::CH_OUTPUT_START + inputId, buffer);
			sprintf(buffer, "Ch %d Value Received", i + 1);
			configOutput(OutputIds::CH_OUTPUT_START + inputId + 1, buffer);
			// Configure the Light Also:			
			sprintf(buffer, "Ch %d Message Received", i + 1);			
			configLight(LightIds::CH_LIGHT_START + inputId + 1, buffer);

			/// TODO: If we do ever want to chart the value history, turn history tracking on
			/// outputChannels[i].setStoreHistory(true);
		}
	}
	
	// Can we see how far away a master is right now?	
	int baseChannels = 0;
	int lvlFromMaster = findMaster(0, baseChannels, masterModuleId);
	if (lvlFromMaster == -1)
		baseChannels = TROWA_OSCCV_DEFAULT_NUM_CHANNELS; // Master probably has this many channels.
	initChannels(baseChannels);
	
	_expID = (direction == TSOSCCVExpanderDirection::Input) ? "I" : "O";
	char buff[4]; 
	buff[0] = static_cast<char>('A' + random::uniform() * 26);
	buff[1] = static_cast<char>('A' + random::uniform() * 26);
	buff[2] = '-';
	_expID.append(buff, 3);	
	//_expID += static_cast<char>('A' + random::uniform() * 26) + static_cast<char>('A' + random::uniform() * 26) + "-";
	sprintf(buff, "%03u", static_cast<uint8_t>(random::uniform() * 100)); 
	_expID.append(buff, 3);
	
	displayName = _expID;
	return;
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Clean up or ram.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
oscCVExpander::~oscCVExpander()
{	
	if (inputChannels != NULL)
		delete[] inputChannels;
	if (outputChannels != NULL)
		delete[] outputChannels;
	

#if !USE_MODULE_STATIC_RX	
	// Message queue is now pointers, so delete
	rxMsgMutex.lock();	
	while (rxMsgQueue.size() > 0)
	{		
		TSOSCCVSimpleMessage* rxOscMsg = rxMsgQueue.front();
		rxMsgQueue.pop();
		delete rxOscMsg;
	} // end while (loop through message queue)
	rxMsgMutex.unlock();
#endif 	
	return;
} // end destructor
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// initializeChannels(void)
// Set channels to default values.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void oscCVExpander::initChannels(int baseChannel) {
	for (int i = 0; i < numberChannels; i++)
	{
		if (this->expanderType == TSOSCCVExpanderDirection::Input) {
			inputChannels[i].channelNum = i + 1 + baseChannel;
			inputChannels[i].path = "/ch/" + std::to_string(i + 1 + baseChannel);
			inputChannels[i].initialize();
		}
		else {
			outputChannels[i].channelNum = i + 1 + baseChannel;
			outputChannels[i].path = "/ch/" + std::to_string(i + 1 + baseChannel);
			outputChannels[i].initialize();
		}
	}
	// [Rack v2] Add labels for inputs and outputs
	renamePorts();
	return;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// renumberChannels()
// Renumber the channels only (does not reset advanced settings).
// Renames the ports too.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void oscCVExpander::renumberChannels()
{
	int baseChannel = 0;
	this->lvlFromMaster = findMaster(0, baseChannel, masterModuleId);
	if (lvlFromMaster == -1)
		baseChannel = TROWA_OSCCV_DEFAULT_NUM_CHANNELS; // Master probably has this many channels.

	oscMutex.lock();
	for (int i = 0; i < numberChannels; i++)
	{
		if (this->expanderType == TSOSCCVExpanderDirection::Input) {
			inputChannels[i].channelNum = i + 1 + baseChannel;
			inputChannels[i].path = "/ch/" + std::to_string(i + 1 + baseChannel);
		}
		else {
			outputChannels[i].channelNum = i + 1 + baseChannel;
			outputChannels[i].path = "/ch/" + std::to_string(i + 1 + baseChannel);
		}
	}
	oscMutex.unlock();

	// The tool tips for ports aren't used in OSC so we don't have to lock anything for this
	// [Rack v2] Add labels for inputs and outputs
	renamePorts();
	return;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// reset(void)
// Initialize values.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void oscCVExpander::onReset() {
	// Reset our values
	int baseChannels = 0;
	this->lvlFromMaster = findMaster(0, baseChannels, masterModuleId);
	if (lvlFromMaster == -1)
		baseChannels = TROWA_OSCCV_DEFAULT_NUM_CHANNELS; // Master probably has this many channels.
	this->sendChangeSensitivity = TROWA_OSCCV_CHANGE_THRESHHOLD_USE_PARENT;
	oscMutex.lock();
	initChannels(baseChannels);
	oscMutex.unlock();
	return;
} // end reset()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// findMaster()
// Find how many modules (of oscCVExpanders/oscCV) away a master oscCV module is.
// 1 away means we are right next to master.
// 2 away means there is one expander in between us and the master.
// -1 means no master found :-(.
// Also find the # channels away (now that expanders can have varying # channels).
// Next to the master = 8
// One away with a 16-channel between = 24
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
int oscCVExpander::findMaster(int n, int& nChannelsAway, int& masterId)
{
	try
	{
		if (this->expanderType == TSOSCCVExpanderDirection::Input)
		{
			// Input
			if (rightExpander.module)
			{
				if (rightExpander.module->model == modelOscCV)
				{
					masterId = dynamic_cast<oscCV*>(rightExpander.module)->oscId;
					nChannelsAway += TROWA_OSCCV_DEFAULT_NUM_CHANNELS; // 8
					return n + 1; // Master found					
				}
				else if ( (CVOSCCV_IS_EXPANDER_INPUT_MODEL(rightExpander.module->model)) // == modelOscCVExpanderInput
					&& dynamic_cast<oscCVExpander*>(rightExpander.module)->expanderType == this->expanderType)
				{
					nChannelsAway += dynamic_cast<oscCVExpander*>(rightExpander.module)->numberChannels;
					return dynamic_cast<oscCVExpander*>(rightExpander.module)->findMaster(n + 1, nChannelsAway, masterId);				
				}
			}
		}
		else 
		{
			// Output
			if (leftExpander.module)
			{
				if (leftExpander.module->model == modelOscCV)
				{
					masterId = dynamic_cast<oscCV*>(leftExpander.module)->oscId;
					nChannelsAway += TROWA_OSCCV_DEFAULT_NUM_CHANNELS; // 8
					return n + 1; // Master found					
				}
				else if (  (CVOSCCV_IS_EXPANDER_OUTPUT_MODEL(leftExpander.module->model)) // == modelOscCVExpanderOutput
					&& dynamic_cast<oscCVExpander*>(leftExpander.module)->expanderType == this->expanderType)
				{
					nChannelsAway += dynamic_cast<oscCVExpander*>(leftExpander.module)->numberChannels;
					return dynamic_cast<oscCVExpander*>(leftExpander.module)->findMaster(n + 1, nChannelsAway, masterId);				
				}
			}	
		}		
	} 
	catch (std::exception &e)
	{
		WARN("Error searching for master module.\n%s", e.what());
	}
	return -1; // Not found
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// findMaster()
// Find how many modules (of oscCVExpanders/oscCV) away a master oscCV module is.
// 1 away means we are right next to master.
// 2 away means there is one expander in between us and the master.
// -1 means no master found :-(.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
int oscCVExpander::findMaster(int n, int& nChannelsAway, Module* &master)
{
	try
	{
		if (this->expanderType == TSOSCCVExpanderDirection::Input)
		{
			// Input
			if (rightExpander.module)
			{
				if (rightExpander.module->model == modelOscCV)
				{
					master = dynamic_cast<oscCV*>(rightExpander.module);
					nChannelsAway += TROWA_OSCCV_DEFAULT_NUM_CHANNELS; // 8
					return n + 1; // Master found					
				}
				else if (CVOSCCV_IS_EXPANDER_INPUT_MODEL(rightExpander.module->model)
					&& dynamic_cast<oscCVExpander*>(rightExpander.module)->expanderType == this->expanderType)
				{
					nChannelsAway += dynamic_cast<oscCVExpander*>(rightExpander.module)->numberChannels;
					return dynamic_cast<oscCVExpander*>(rightExpander.module)->findMaster(n + 1, nChannelsAway, master);				
				}
			}
		}
		else 
		{
			// Output
			if (leftExpander.module)
			{
				if (leftExpander.module->model == modelOscCV)
				{
					master = dynamic_cast<oscCV*>(leftExpander.module);
					nChannelsAway += TROWA_OSCCV_DEFAULT_NUM_CHANNELS; // 8
					return n + 1; // Master found					
				}
				else if (CVOSCCV_IS_EXPANDER_OUTPUT_MODEL(leftExpander.module->model)
					&& dynamic_cast<oscCVExpander*>(leftExpander.module)->expanderType == this->expanderType)
				{
					nChannelsAway += dynamic_cast<oscCVExpander*>(leftExpander.module)->numberChannels;
					return dynamic_cast<oscCVExpander*>(leftExpander.module)->findMaster(n + 1, nChannelsAway, master);				
				}
			}	
		}		
	} 
	catch (std::exception &e)
	{
		WARN("Error searching for master module.\n%s", e.what());
	}
	return -1; // Not found
}			

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// process()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void oscCVExpander::process(const ProcessArgs &args)
{
	oscCV* master = NULL;
	Module* mod = NULL;
	int baseChannels = 0;
	lvlFromMaster = findMaster(0, baseChannels, mod);
	if (mod)
	{
		master = dynamic_cast<oscCV*>(mod);
	}
	if (lvlFromMaster > 0 && master != NULL)
	{
		lights[LightIds::MASTER_CONNECTED_LIGHT].value = 1.0f;
		int ix = 0;
		if (this->expanderType == TSOSCCVExpanderDirection::Input)
		{
			// Input
			lights[LightIds::RIGHT_CONNECTED_LIGHT].value = 1.0f;
			// Check if our left neighbor is of same type as us (input)
			lights[LightIds::LEFT_CONNECTED_LIGHT].value = (leftExpander.module && leftExpander.module->model == modelOscCVExpanderInput);			
			ix = -lvlFromMaster;
		}
		else
		{
			// Output
			lights[LightIds::LEFT_CONNECTED_LIGHT].value = 1.0f;	
			// Check if our right neighbor is of same type as us (output)
			lights[LightIds::RIGHT_CONNECTED_LIGHT].value = (rightExpander.module && rightExpander.module->model == modelOscCVExpanderOutput);
			ix = lvlFromMaster;
#if OSC2CV_EXP_HANDLE_OUTPUTS_DIRECTLY
			// Now I handle my own outputs instead of making the master do it.
			this->processOutputs(args.sampleTime);
#endif
		}
		thisColor = oscCVWidget::calcColor(ix);
		try
		{
			// Check if we are currently being configured by the master module.
			beingConfigured = master->expCurrentEditExpander == this;
			if (beingConfigured)
			{
				this->configureColIx = master->expCurrentEditPageCol;
				this->configureChannelIx = master->expCurrentEditChannelIx; // Track the channel being configured in Advanced Config
			}
		}
		catch (std::exception &e)
		{
			beingConfigured = false;
			this->configureColIx = 0;
			WARN("Error searching for master module.\n%s", e.what());
		}
	}
	else 
	{
		//------- NO CONNECTION --------
		thisColor = ColorNotConnected;
		// Make sure we signal any 'being configured' indicators to false for the widget
		beingConfigured = false;
		this->configureColIx = -1;
		this->configureChannelIx = -1;
		// If no master then don't bother showing connections on left / right.
		lights[LightIds::MASTER_CONNECTED_LIGHT].value = 0.0f;
		lights[LightIds::LEFT_CONNECTED_LIGHT].value = 0.0f;		
		lights[LightIds::RIGHT_CONNECTED_LIGHT].value = 0.0f;
	}
	return;
} // end step()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// processInputs()
// Process CV->OSC.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
void oscCVExpander::processInputs(std::string oscNamespace, bool oscInitialized, bool sendTime, float changeSensitivity, bool& packetOpened, std::mutex& sendMutex, osc::OutboundPacketStream& oscStream)
{
	/// TODO: If we make some osc base class, this could be built-in for oscCV and these expanders.
	try
	{
		if (this->expanderType == TSOSCCVExpanderDirection::Input) // doCVPort2OSC
		{
			// Read the channels and output to OSC
			char addressBuffer[512];

			// Use the parent/master change sensitivity given to us if we are set to negative.
			float changeThreshold = (this->sendChangeSensitivity < 0.0f) ? changeSensitivity : this->sendChangeSensitivity;

			for (int c = 0; c < this->numberChannels; c++)
			{		
				bool sendVal = false;
				if (oscInitialized && this->inputs[oscCVExpander::InputIds::CH_INPUT_START + c * 2 + 1].isConnected())
				{
					// Read in CV
					if (inputs[oscCVExpander::InputIds::CH_INPUT_START + c * 2 + 1].getChannels() > 1)
					{
						// Poly CV
						for (int i = 0; i < inputs[oscCVExpander::InputIds::CH_INPUT_START + c * 2 + 1].getChannels(); i++)
						{
							inputChannels[c].setValue(inputs[oscCVExpander::InputIds::CH_INPUT_START + c * 2 + 1].getVoltage(i), i);
						}
					}
					else
					{
						// Mono CV
						inputChannels[c].setValue(inputs[oscCVExpander::InputIds::CH_INPUT_START + c * 2 + 1].getVoltage());
					}
					
					if (this->inputs[oscCVExpander::InputIds::CH_INPUT_START + c * 2].isConnected()) // Input Trigger Port
					{
						sendVal = this->inputTriggers[c].process(this->inputs[oscCVExpander::InputIds::CH_INPUT_START + c * 2].getVoltage());
					} // end if trigger is active
					else
					{
						if (!inputChannels[c].doSend) {
							// Only send if changed enough. Maybe about 0.01 V? Defined on channel.
							// 1. Check for change:
							// Currently all channels have the same sensitivity, one day maybe we have this settable by channel.
							sendVal = inputChannels[c].valChanged(changeThreshold);
							// 2. Mark channel as needing to output
							if (sendVal) {
								inputChannels[c].doSend = true;
							}
						}
						// 3. Check if it is time to send out
						sendVal = sendTime && inputChannels[c].doSend;
					}
					//float outVal = inputChannels[c].translatedVal;
					if (sendVal)
					{
						lights[LightIds::CH_LIGHT_START + c * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL].value = 1.0f;
						sendMutex.lock();
						try
						{
							if (!packetOpened)
							{
								oscStream << osc::BeginBundleImmediate;
								packetOpened = true;
							}
							if (oscNamespace.empty()) // Allow empty namespaces
							{
								sprintf(addressBuffer, "%s", inputChannels[c].getPath().c_str());														
							}
							else
							{
								sprintf(addressBuffer, "/%s%s", oscNamespace.c_str(), inputChannels[c].getPath().c_str());							
							}
							oscStream << osc::BeginMessage(addressBuffer);							
							for (int j = 0; j < inputChannels[c].numVals; j++)
							{
								if (inputChannels[c].convertVals)
								{
									// Enforce Data Type:
									switch (inputChannels[c].dataType)
									{
									case TSOSCCVChannel::ArgDataType::OscInt:
										oscStream << static_cast<int>(inputChannels[c].translatedVals[j]);
										//outVal = static_cast<float>(static_cast<int>(inputChannels[c].translatedVal[j]));
										break;
									case TSOSCCVChannel::ArgDataType::OscBool:
										oscStream << static_cast<bool>(inputChannels[c].translatedVals[j]);
										//outVal = static_cast<float>(static_cast<bool>(inputChannels[c].translatedVals[j]));
										break;
									case TSOSCCVChannel::ArgDataType::OscFloat:
									default:
										oscStream << inputChannels[c].translatedVals[j];
										break;
									}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
									DEBUG("SEND OSC[%d,%d]: %s %7.3f", c, j, addressBuffer, inputChannels[c].translatedVals[j]);
#endif													
								}
								else
								{
									// Raw value out
									oscStream << inputChannels[c].vals[j];
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
									DEBUG("SEND OSC[%d]: %s %7.3f", c, addressBuffer, inputChannels[c].vals[j]);
#endif
								}							
							} // end loop through values.	
							// if (inputChannels[c].convertVals)
							// {
								// // Enforce Data Type:
								// switch (inputChannels[c].dataType)
								// {
								// case TSOSCCVChannel::ArgDataType::OscInt:
									// oscStream << static_cast<int>(inputChannels[c].translatedVal);
									// outVal = static_cast<float>(static_cast<int>(inputChannels[c].translatedVal));
									// break;
								// case TSOSCCVChannel::ArgDataType::OscBool:
									// oscStream << static_cast<bool>(inputChannels[c].translatedVal);
									// outVal = static_cast<float>(static_cast<bool>(inputChannels[c].translatedVal));
									// break;
								// case TSOSCCVChannel::ArgDataType::OscFloat:
								// default:
									// oscStream << inputChannels[c].translatedVal;
									// break;
								// }
							// }
							// else
							// {
								// // Raw value out
								// oscStream << inputChannels[c].val;
							// }
							oscStream << osc::EndMessage;
							
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
						DEBUG("SEND OSC[%d] (Expander): %s %7.3f", c, addressBuffer, inputChannels[c].getValCV2OSC());
#endif
						}
						catch (const std::exception& e)
						{
							WARN("Error %s.", e.what());
						}
						sendMutex.unlock();
						// Save our last sent values
						inputChannels[c].storeLastValues();
						inputChannels[c].doSend = false; // Reset
					} // end if send value 
				} // end if oscInitialied

				if (lights[LightIds::CH_LIGHT_START + c * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL].value > 0 && !sendVal) {
					lights[LightIds::CH_LIGHT_START + c * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL].value -= lightLambda;
				}
			} // end for loop				
		}
	}
	catch (const std::exception& e)
	{
		WARN("Error %s.", e.what());
	}	
	return;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// processOutputs()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void oscCVExpander::processOutputs(float sampleTime)
{
	//--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--
	// OSC ==> Rack Output Ports
	//--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--
	if (expanderType == TSOSCCVExpanderDirection::Output)
	{
		//------------------------------------------------------------
		// Look for OSC Rx messages --> Output to Rack
		//------------------------------------------------------------
		while (rxMsgQueue.size() > 0)
		{
			rxMsgMutex.lock();			
			TSOSCCVSimpleMessage* rxOscMsg = rxMsgQueue.front();
			rxMsgMutex.unlock();				
			int chIx = rxOscMsg->channelNum - 1;
			if (chIx > -1 && chIx < numberChannels)
			{
				// Process the message
				pulseGens[chIx].trigger(TROWA_PULSE_WIDTH); // Trigger (msg received)
				//outputChannels[chIx].setOSCInValue(rxOscMsg.rxVal);
				// Now we are using float array not vector for rxVals
				outputChannels[chIx].setOSCInValue(rxOscMsg->rxVals, rxOscMsg->rxLength);
				lights[LightIds::CH_LIGHT_START + chIx * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL + 1].value = 1.0f;
			} // end if valid channel			
			rxMsgMutex.lock();			
			rxMsgQueue.pop();
			rxMsgMutex.unlock();	
#if !USE_MODULE_STATIC_RX			
			delete rxOscMsg;
#endif	
		} // end while (loop through message queue)
		// ::: OUTPUTS :::
		float dt = sampleTime;
		for (int c = 0; c < numberChannels; c++)
		{
			// Output the value first
			// We should limit this value (-10V to +10V). Rack says nothing should be higher than +/- 12V.
			// Polyphonic output :::::::::::::::::::::::
			outputs[OutputIds::CH_OUTPUT_START + c * 2 + 1].setChannels(outputChannels[c].numVals);
			for (int j = 0; j < outputChannels[c].numVals; j++)
			{
				outputs[OutputIds::CH_OUTPUT_START + c * 2 + 1].setVoltage(/*v*/clamp(outputChannels[c].translatedVals[j], TROWA_OSCCV_MIN_VOLTAGE, TROWA_OSCCV_MAX_VOLTAGE), /*channel*/ j);				
			}
#if OSC2CV_EXP_STORE_HISTORY
			outputChannels[c].addValToBuffer(outputChannels[c].translatedVals[0]);
#endif
			// Then trigger if needed.
			bool trigger = pulseGens[c].process(dt);
			outputs[OutputIds::CH_OUTPUT_START + c * 2].setVoltage((trigger) ? TROWA_OSCCV_TRIGGER_ON_V : TROWA_OSCCV_TRIGGER_OFF_V);
			lights[LightIds::CH_LIGHT_START + c * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL + 1].value = clamp(lights[LightIds::CH_LIGHT_START + c * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL + 1].value - lightLambda, 0.0f, 1.0f);
		}
	}
	return;
} // end processOutputs()

NVGcolor oscCVExpander::getColor(int lvlFromMaster, bool left)
{	
	NVGcolor color;
	int add = 0;
	if (left)
	{
		if (lvlFromMaster < 2 && expanderType == TSOSCCVExpanderDirection::Output)
			color = TSColors::COLOR_WHITE;		
		else
		{
			add = (expanderType == TSOSCCVExpanderDirection::Input) ? 1 : 0;
			color = TSColors::CHANNEL_COLORS[(lvlFromMaster - 1 + add + TSColors::NUM_CHANNEL_COLORS) % TSColors::NUM_CHANNEL_COLORS];			
		}
	}
	else
	{
		if (lvlFromMaster < 2 && expanderType == TSOSCCVExpanderDirection::Input)
			color = TSColors::COLOR_WHITE;
		else
		{			
			add = (expanderType == TSOSCCVExpanderDirection::Input) ? 0 : 1;
			color = TSColors::CHANNEL_COLORS[(lvlFromMaster - 1 + add + TSColors::NUM_CHANNEL_COLORS) % TSColors::NUM_CHANNEL_COLORS];			
		}
	}
	return color;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// dataToJson(void)
// Serialize to json.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
json_t *oscCVExpander::dataToJson() {
	json_t* rootJ = json_object();

	// version
	json_object_set_new(rootJ, "version", json_integer(TROWA_INTERNAL_VERSION_INT));

	// General:
	json_object_set_new(rootJ, "type", json_integer((int)expanderType));
	json_object_set_new(rootJ, "expId", json_string(this->_expID.c_str()));
	json_object_set_new(rootJ, "displayName", json_string(this->displayName.c_str()));
	json_object_set_new(rootJ, "sendChangeSensitivity", json_real(sendChangeSensitivity)); // [v22: 2.0.8] Users can change the channel sensitivity now.


	// Channels
	json_object_set_new(rootJ, "numCh", json_integer(numberChannels));
	json_t* inputChannelsJ = json_array();
	json_t* outputChannelsJ = json_array();
	for (int c = 0; c < numberChannels; c++)
	{
		// Input
		if (inputChannels != NULL)
			json_array_append_new(inputChannelsJ, inputChannels[c].serialize());
		if (outputChannels != NULL)
			json_array_append_new(outputChannelsJ, outputChannels[c].serialize());
	}
	json_object_set_new(rootJ, "inputChannels", inputChannelsJ);
	json_object_set_new(rootJ, "outputChannels", outputChannelsJ);
	return rootJ;
} // end dataToJson()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// dataFromJson(void)
// Deserialize.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void oscCVExpander::dataFromJson(json_t *rootJ) {
	json_t* currJ = NULL;
	
	
	currJ = json_object_get(rootJ, "expId");
	if (currJ)
		this->_expID = json_string_value(currJ);
	currJ = json_object_get(rootJ, "displayName");
	if (currJ)
		this->displayName = json_string_value(currJ);
	// [v22: 2.0.8] Users can change the channel sensitivity now.
	// If users do not have a trigger, then the change needed to send.
	currJ = json_object_get(rootJ, "sendChangeSensitivity");
	if (currJ)
		this->sendChangeSensitivity = (float)(json_real_value(currJ));
	else
		this->sendChangeSensitivity = TROWA_OSCCV_CHANGE_THRESHHOLD_USE_PARENT;

	
	// Channels
	int nChannels = numberChannels;
	currJ = json_object_get(rootJ, "numCh");
	if (currJ)
	{
		nChannels = json_integer_value(currJ);
		if (nChannels > numberChannels)
			nChannels = numberChannels;
	}
	json_t* inputChannelsJ = json_object_get(rootJ, "inputChannels");
	json_t* outputChannelsJ = json_object_get(rootJ, "outputChannels");
	for (int c = 0; c < nChannels; c++)
	{
		// Input
		if (inputChannels && inputChannelsJ)
		{
			json_t* channelJ = json_array_get(inputChannelsJ, c);
			if (channelJ) {
				inputChannels[c].deserialize(channelJ);
			} // end if channel object
		} // end if there is an inputChannels array
		// Output
		if (outputChannels && outputChannelsJ)
		{
			json_t* channelJ = json_array_get(outputChannelsJ, c);
			if (channelJ) {
				outputChannels[c].deserialize(channelJ);
			} // end if channel object
		} // end if there is an outputChannels array
	} // end loop through channels
	
	// Rename our ports [Rack v2]
	renamePorts();
	return;
} // end dataFromJson() 
#if USE_MODULE_STATIC_RX
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// addRxMsgToQueue()
// Adds the message to the queue.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void oscCVExpander::addRxMsgToQueue(int chNum, float val)
{
	rxMsgMutex.lock();	
	TSOSCCVSimpleMessage* item = getRxMsgObj();
	item->SetValues(chNum, val);
	rxMsgQueue.push(item);
	rxMsgMutex.unlock();
	return;
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// addRxMsgToQueue()
// Adds the message to the queue.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void oscCVExpander::addRxMsgToQueue(int chNum, std::vector<float> vals)
{
	rxMsgMutex.lock();	
	TSOSCCVSimpleMessage* item = getRxMsgObj();
	item->SetValues(chNum, vals);
	rxMsgQueue.push(item);
	rxMsgMutex.unlock();
	return;
}
#endif