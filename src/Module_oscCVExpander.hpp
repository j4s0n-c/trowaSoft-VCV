#ifndef MODULE_OSCCV_EXPANDER_HPP
#define MODULE_OSCCV_EXPANDER_HPP
//-----------------------------
// Expander for cvOSCcv
//-----------------------------
#include <rack.hpp>
using namespace rack;

#include "../lib/oscpack/osc/OscOutboundPacketStream.h"
#include "../lib/oscpack/ip/UdpSocket.h"
#include "../lib/oscpack/osc/OscReceivedElements.h"
#include "../lib/oscpack/osc/OscPacketListener.h"
#include <mutex>
#include <queue>

// Model for trowa OSC2CV Expander (default 8 channel)
extern Model* modelOscCVExpanderInput;
extern Model* modelOscCVExpanderOutput;
// Models for 16-channel expanders
extern Model* modelOscCVExpanderInput16;
extern Model* modelOscCVExpanderOutput16;
// Models for 32-channel expanders.
extern Model* modelOscCVExpanderInput32;
extern Model* modelOscCVExpanderOutput32;

// Check if the model is an expander input.
#define CVOSCCV_IS_EXPANDER_INPUT_MODEL(model)		((model == modelOscCVExpanderInput || model == modelOscCVExpanderInput16 || model == modelOscCVExpanderInput32))
// Check if the model is an expander output.
#define CVOSCCV_IS_EXPANDER_OUTPUT_MODEL(model)		((model == modelOscCVExpanderOutput || model == modelOscCVExpanderOutput16 || model == modelOscCVExpanderOutput32))

#include "TSOSCCV_Common.hpp"
#include "TSColors.hpp"

#define CV2OSC_LBL_IN_TRIG			"Send Trigger: "
#define CV2OSC_LBL_IN_VALUE			"Send Value: "
#define CV2OSC_LBL_LIGHT_MSG_TX		"Message Sent: "
#define OSC2CV_LBL_OUT_TRIG			"Received Trigger: "
#define OSC2CV_LBL_OUT_VALUE		"Received Value: "
#define OSC2CV_LBL_LIGHT_MSG_RX		"Message Received: "

#define CVOSCCV_EXP_NUM_CHANNELS_PER_COL	TROWA_OSCCV_DEFAULT_NUM_CHANNELS // Number of channels per column (for layout and paging)
// Master module only has this many rows to show, so more than this needs to go into another column.


#define OSC2CV_EXP_STORE_HISTORY				0 // Store history or not for expander received messages. Originally we were going to display this, but we never did, so don't keep the data.
#define OSC2CV_EXP_HANDLE_OUTPUTS_DIRECTLY		1 // If an OSCcv expander (output) should handle the outputs itself. Originally, master was doing everything.
// TEST: OSCcv handling messages: ~55.5%
// master = ~9.6%, OSCcv = 7.5%, OSCcv32 = 24%, OSCcv16 = 14.4%
// TEST: master handling messages: ~55.9% - 59.9%
// master = 52-56%, OSCcv = 0.8%, OSCcv32 = 1.2%, OSCcv16 = 1.9%
// So master handling everything is a little more? Basically the same... but maybe slightly better to have slaves handle their own
// messages since the Delivery thread is giving them messages anyway.

//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Expansion module for a master cvOSCcv module....
// The only problem with the native Expansion interface is the 1 sample delay when using the messaging interface.
// Try to work around this (may be a little hokey) by trying to implement some (hopefully) thread-safe manipulation of the Expanders
// directly from the master module.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct oscCVExpander : Module 
{
	// User control parameters
	enum ParamIds {
		NAME_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CH_INPUT_START,
		NUM_INPUTS = CH_INPUT_START // Add # channels *2 to this
	};
	enum OutputIds {
		CH_OUTPUT_START,
		NUM_OUTPUTS = CH_OUTPUT_START // Add # channels*2 to this Determined by # of channels
	};
	enum LightIds {
		MASTER_CONNECTED_LIGHT,
		LEFT_CONNECTED_LIGHT,
		RIGHT_CONNECTED_LIGHT,
		CH_LIGHT_START,
		NUM_LIGHTS = CH_LIGHT_START // Add # channels *2 to this
	};
	
	// Number of channels we have
	int numberChannels = TROWA_OSCCVEXPANDER_DEFAULT_NUM_CHANNELS;
	// Number of columns/pages for the channels.
	int numberColumns = 1;
	// Input CV (from Rack) ==> Needs to be output to OSC
	TSOSCCVInputChannel* inputChannels = NULL;
	// Input OSC (from External) ==> Needs to be translated to Rack output port CV
	TSOSCCVChannel* outputChannels = NULL;	
	dsp::SchmittTrigger* inputTriggers;	
	dsp::PulseGenerator* pulseGens = NULL;
	// The received messages.
	std::queue<TSOSCCVSimpleMessage*> rxMsgQueue;
	std::mutex rxMsgMutex; // Msg queue mutex
#if USE_MODULE_STATIC_RX	
	// Add static buffer (debug MAC OSC issues)
	TSOSCCVSimpleMessage rxMsgBuffer[OSC_RX_MSG_BUFFER_SIZE];
	int rxMsgBufferIx = 0;
#endif	
	const float lightLambda = 0.005f;	
	// Mutex for osc messaging.
	std::mutex oscMutex;		
	
	// Tracking
	int masterModuleId = -1;
	// 1 is right next to master, 2 is 2 away.
	int lvlFromMaster = 0;
	// Some kind of somewhat unique id for a master module to identify it.
	std::string _expID;
	// Some user defined name.
	std::string displayName;
	const NVGcolor ColorNotConnected = TSColors::COLOR_TS_GRAY;
	// This expander's color:
	NVGcolor thisColor = ColorNotConnected;
	// If this module is being configured.
	bool beingConfigured = false;
	// If this module is being configured, the column index being configured.
	int configureColIx = -1;
	// If this module is being configured and a certain channel is being configured, the channel index.
	int configureChannelIx = -1;
	
	// The expander type (INPUT/OUTPUT). Maybe we'll have 'both' someday? 
	TSOSCCVExpanderDirection expanderType = TSOSCCVExpanderDirection::Unknown;
	// Messaging has delay so chaining will get crazy.
	// // Right message. If this is INPUT, then a master should be on the right.
	// TSOSCCVExpanderMessage* rightMessage[2];
	// // Left message. If this is OUTPUT, then a master should be on the left.
	// TSOSCCVExpanderMessage* leftMessage[2];

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// oscCVExpander()
	// Create a module with numChannels.
	// @numChannels: (IN) Number of input or output 'channels'. (Maybe both in the future).
	// @direction: (IN) The type of expander (INPUT or OUTPUT). (Maybe both in the future).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	oscCVExpander(int numChannels, TSOSCCVExpanderDirection direction);	
	
	~oscCVExpander();

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// initChannels()
	// Initialize the channels.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void initChannels(int baseChannel);

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// renumberChannels()
	// Renumber the channels only (does not reset advanced settings).
	// Renames the ports too.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void renumberChannels();
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Rename the ports (i.e. after loading from save) after our actual
	// real channel addresses.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void renamePorts() {
		for (int i = 0; i < numberChannels; i++)
		{
			int portId = i *2;
			if (expanderType == TSOSCCVExpanderDirection::Input) {
				inputInfos[InputIds::CH_INPUT_START + portId]->PortInfo::name = CV2OSC_LBL_IN_TRIG + inputChannels[i].path;		  // Send Trigger
				inputInfos[InputIds::CH_INPUT_START + portId + 1]->PortInfo::name = CV2OSC_LBL_IN_VALUE + inputChannels[i].path;  // Send Value
				lightInfos[LightIds::CH_LIGHT_START + portId]->LightInfo::name = CV2OSC_LBL_LIGHT_MSG_TX + inputChannels[i].path; // Message Sent
			}
			else {
				outputInfos[OutputIds::CH_OUTPUT_START + portId]->PortInfo::name = OSC2CV_LBL_OUT_TRIG + outputChannels[i].path;	  // Received Trigger
				outputInfos[OutputIds::CH_OUTPUT_START + portId + 1]->PortInfo::name = OSC2CV_LBL_OUT_VALUE + outputChannels[i].path; // Received Value
				lightInfos[LightIds::CH_LIGHT_START + portId + 1]->LightInfo::name = OSC2CV_LBL_LIGHT_MSG_RX + outputChannels[i].path; // Message Received
			}
		}				
		return;
	}
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// reset(void)
	// Initialize values.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void onReset() override;	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// findMaster()
	// Find how many modules (of oscCVExpanders/oscCV) away a master oscCV module is.
	// 1 away means we are right next to master.
	// 2 away means there is one expander in between us and the master.
	// -1 means no master found :-(.
	// 
	// Also find the # channels away (now that expanders can have varying # channels).
	// Next to the master = 8
	// One away with a 16-channel between = 24
	//
	// @n : Current level.
	// @nChannelsAway : # channels away from the master.
	// @masterId : (OUT) The master module id.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-			
	int findMaster(int n, int& nChannelsAway, int& masterId);	
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// findMaster()
	// Find how many modules (of oscCVExpanders/oscCV) away a master oscCV module is.
	// 1 away means we are right next to master.
	// 2 away means there is one expander in between us and the master.
	// -1 means no master found :-(.
	// 
	// Also find the # channels away (now that expanders can have varying # channels).
	// Next to the master = 8
	// One away with a 16-channel between = 24
	//
	// @n : Current level.
	// @nChannelsAway : # channels away from the master.
	// @master : (OUT) The master module.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-			
	int findMaster(int n, int& nChannelsAway, Module* &master);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// calcMasterDistance()
	// Find how many modules (of oscCVExpanders/oscCV) away a master oscCV module is.
	// 1 away means we are right next to master.
	// 2 away means there is one expander in between us and the master.
	// -1 means no master found :-(.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-				
	void calcMasterDistance()
	{
		int baseChannels = 0;
		this->lvlFromMaster = findMaster(0, baseChannels, masterModuleId);
	}
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// process()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void process(const ProcessArgs &args) override;	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// processInputs()
	// Process CV->OSC.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
	void processInputs(std::string oscNamespace, bool oscInitialized, bool sendTime, bool& packetOpened, std::mutex& sendMutex, osc::OutboundPacketStream& oscStream);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// processOutputs()
	// Process OSC->CV (from msg queue).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
	void processOutputs(float sampleTime);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// getColor()
	// Get the color based on the position.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
	NVGcolor getColor(int lvlFromMaster, bool left);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// dataToJson(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	json_t *dataToJson() override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// dataFromJson(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void dataFromJson(json_t *rootJ) override;	
#if USE_MODULE_STATIC_RX	
	// This should really go into another class.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// getRxMsgObj()
	// Gets next msg object in circular buffer. 
	// RxConnector should use these objects.
	// Will overwrite if we are not fast enough reading. 
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	TSOSCCVSimpleMessage* getRxMsgObj()
	{
		if (rxMsgBufferIx >= OSC_RX_MSG_BUFFER_SIZE)
			rxMsgBufferIx = 0;
		return &(rxMsgBuffer[rxMsgBufferIx++]);
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// addRxMsgToQueue()
	// Adds the message to the queue.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void addRxMsgToQueue(int chNum, float val);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// addRxMsgToQueue()
	// Adds the message to the queue.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void addRxMsgToQueue(int chNum, std::vector<float> vals);	
#endif		
};

// Expander Input cvOSC - Default 8 channels.
template<int N = TROWA_OSCCVEXPANDER_DEFAULT_NUM_CHANNELS>
struct oscCVExpanderInput : oscCVExpander
{
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// oscCVExpanderInput()
	// Create a module with numChannels.
	// @numChannels: (IN) Number of input or output 'channels'.
	// @direction: (IN) The type of expander (INPUT or OUTPUT).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	oscCVExpanderInput() : oscCVExpander(N, TSOSCCVExpanderDirection::Input)
	{
		return;
	}
};
// Expander Output OSCcv - Default 8 channels.
template<int N = TROWA_OSCCVEXPANDER_DEFAULT_NUM_CHANNELS>
struct oscCVExpanderOutput : oscCVExpander
{
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// oscCVExpanderOutput()
	// Create a module with numChannels.
	// @numChannels: (IN) Number of input or output 'channels'.
	// @direction: (IN) The type of expander (INPUT or OUTPUT).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	oscCVExpanderOutput() : oscCVExpander(N, TSOSCCVExpanderDirection::Output)
	{
		return;
	}
};



#endif // endif !MODULE_OSCCV_EXPANDER_HPP