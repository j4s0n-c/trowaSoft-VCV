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

// Model for trowa OSC2CV Expander
extern Model* modelOscCVExpanderInput;
extern Model* modelOscCVExpanderOutput;

#include "TSOSCCV_Common.hpp"
#include "TSColors.hpp"



// Expansion module for a master cvOSCcv module....
// The only problem with the native Expansion interface is the 1 sample delay when using the messaging interface.
// Try to work around this (may be a little hokey) by trying to implement some (hopefully) thread-safe manipulation of the Expanders
// directly from the master module? Or have the Expanders manipulate the cvOSCcv module? Easier for master to manipulate the slaves...
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
	int numberChannels = TROWA_OSCCV_DEFAULT_NUM_CHANNELS;	
	// Input CV (from Rack) ==> Needs to be output to OSC
	TSOSCCVInputChannel* inputChannels = NULL;
	// Input OSC (from External) ==> Needs to be translated to Rack output port CV
	TSOSCCVChannel* outputChannels = NULL;	
	dsp::SchmittTrigger* inputTriggers;	
	dsp::PulseGenerator* pulseGens = NULL;
	// The received messages.
	std::queue<TSOSCCVSimpleMessage> rxMsgQueue;
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
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-			
	int findMaster(int n, int& masterId);	
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// findMaster()
	// Find how many modules (of oscCVExpanders/oscCV) away a master oscCV module is.
	// 1 away means we are right next to master.
	// 2 away means there is one expander in between us and the master.
	// -1 means no master found :-(.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-			
	int findMaster(int n, Module* &master);		
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// calcMasterDistance()
	// Find how many modules (of oscCVExpanders/oscCV) away a master oscCV module is.
	// 1 away means we are right next to master.
	// 2 away means there is one expander in between us and the master.
	// -1 means no master found :-(.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-				
	void calcMasterDistance()
	{
		this->lvlFromMaster = findMaster(0, masterModuleId);
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
};

struct oscCVExpanderInput : oscCVExpander
{
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// oscCVExpander()
	// Create a module with numChannels.
	// @numChannels: (IN) Number of input or output 'channels'.
	// @direction: (IN) The type of expander (INPUT or OUTPUT).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	oscCVExpanderInput() : oscCVExpander(TROWA_OSCCV_DEFAULT_NUM_CHANNELS, TSOSCCVExpanderDirection::Input)
	{
		return;
	}
};
struct oscCVExpanderOutput : oscCVExpander
{
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// oscCVExpander()
	// Create a module with numChannels.
	// @numChannels: (IN) Number of input or output 'channels'.
	// @direction: (IN) The type of expander (INPUT or OUTPUT).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	oscCVExpanderOutput() : oscCVExpander(TROWA_OSCCV_DEFAULT_NUM_CHANNELS, TSOSCCVExpanderDirection::Output)
	{
		return;
	}
};

#endif // endif !MODULE_OSCCV_EXPANDER_HPP