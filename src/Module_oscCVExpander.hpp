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

// Model for trowa OSC2CV Expander
extern Model* modelOscCVExpanderInput;
extern Model* modelOscCVExpanderOutput;

#include "TSOSCCV_Common.hpp"
#include "TSColors.hpp"

//=== DEBUG MacOS ====
//#define USE_MODULE_STATIC_RX					1 // Debug MAC OS issues. Start keeping a static buffer of msg objects for each module.
//#define OSC_RX_MSG_BUFFER_SIZE				   40 // Debug MAC OS issues. Start keeping a static buffer of msg objects for each module.
// NOTES:
// User reported crash on Osc RECV (Poly) on Mac.
// Tested: Intermittent crashes on Windows, Definite crashes on Mac.
// Tried:
// 1. Forgot to implement copy constructor on message. Did that. Fixed intermittent crashes on Windows, but not Mac.
// 2. Thought it was the still somehow the message going out of scope even though copy is in modules, queue. Changed to pointers (dynamic heap msg).
//    Still crashed on Mac (though lasted a lot longer before crashing).
// 3. Noticed lots of memory issues (growing) on Mac. Changed vector<float> to float* and created new float[N]. Memory didn't grow as much but Mac still crashed.
//    Since normally deleted heap memory isn't released back to OS, this may be too many messages created constantly?
// 4. Changed to static array float[16]. Mac still crashed.
// 5. Added mutex for message queue access. Mutex on message queue seemed to fix crash on Mac. But RAM usage still seemed high.
// 6. RAM USAGE: Added static buffer on the oscCV module. Items will be reused. No checks for 'overwriting' a message that has not been handled yet though.
//    Just making the queue fairly large.
// ? Static buffer may help with speed too (not as many new & deletes) and possibly RAM usage.
// * Runtime Test * (dynamic message vs static buffer messages) - Ran for 1 hourish straight:
// Dynamic Message Creation vs Static Buffer (Size 30).
// 1 multiWave, 1 Merge (to make 3 waveforms into a Poly channel), 1 cvOSCcv (sending and receiving Poly (3-value array)).
// Send & Receive @ 500 Hz.
// Rack Mac <==> Win talking to each other.
//- Result -
// Windows 10: No difference in RAM usage (RAM usage 71-81 MB). 
// MacOS (Mojave 10.14.6): 320-350 MB dynamic messages (65 minutes) vs 273 - 294 MB static message buffer (60 minutes)
// Both Win & Mac seem to grow in RAM slowly. Need to find what the hell is leaking. Thought it was the messages, but apparently not because it grows even with a 
// static buffer of messages.
// The difference is Windows seems to somehow recover RAM from Rack.exe as the RAM usage will reduce periodically?
// Does Mac use a crap ton more RAM because it lacks a video card? Is that the Intel HD graphics taking up 150 MB?
// Or is Mac running everything in sandbox/vm so that is the overhead???
// >> RAM seems to grow slowly with just multiWave + MERGE module. Maybe it's in multiWave.



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