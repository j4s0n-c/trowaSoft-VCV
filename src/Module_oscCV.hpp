#ifndef MODULE_OSCCV_HPP
#define MODULE_OSCCV_HPP

#include <rack.hpp>
using namespace rack;
#include "trowaSoft.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSOSCCommon.hpp"
#include "TSOSCCommunicator.hpp"
#include "TSOSCCV_Common.hpp"
#include "Module_oscCVExpander.hpp"
#include <thread> // std::thread
#include <mutex>
#include <string>
#include <queue>
#include <vector>

#include "../lib/oscpack/osc/OscOutboundPacketStream.h"
#include "../lib/oscpack/ip/UdpSocket.h"
#include "../lib/oscpack/osc/OscReceivedElements.h"
#include "../lib/oscpack/osc/OscPacketListener.h"

// Model for trowa OSC2CV
extern Model* modelOscCV;

#define TROWA_OSSCV_SHOW_ADV_CH_CONFIG			1 // Flag to showing advanced config or hiding it (while it is not finished)
#define OSC_CV_OUTPUT_BUFFER_SIZE			1024*128 // Hopefully large enough (may not be from adding poly cables).  


//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// oscCV
// OSC <=> CV (Open Sound Control <=> Control Voltage)
// Generic input port -> osc message (on change or on trigger)
// Generic osc message -> output port (on receive)
// Received messages can only have 1 argument (or only 1 will be parsed and used anyway).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct oscCV : Module {
	// User control parameters
	enum ParamIds {
		OSC_SAVE_CONF_PARAM, // ENABLE and Save the configuration for OSC
		OSC_DISABLE_PARAM,   // Disable OSC (ignore config values)
		OSC_SHOW_CONF_PARAM, // Configure OSC toggle
		OSC_SHOW_ADV_CONF_PARAM, // [TBI] Advanced configuration
		OSC_ADV_CONF_NEXT_PARAM, // [TBI] Advanced configuration Next >
		OSC_ADV_CONF_PREV_PARAM, // [TBI] Advanced configuration < Prev
		OSC_ADV_CONF_BACK_PARAM, // [TBI] Advanced configuration Back
		OSC_AUTO_RECONNECT_PARAM, // Automatically reconnect (if connection is active as of save) on re-load.
		OSC_CH_SAVE_PARAM, // Channel: Save changes and go back to main config. 
		OSC_CH_CANCEL_PARAM, // Channel: Save changes and go back to main config. 
		OSC_CH_OSC_DATATYPE_PARAM, // Channel: OSC data type.
		OSC_CH_TRANSLATE_VALS_PARAM, // Channel: Flag to translate/massage values
		OSC_CH_MIN_CV_VOLT_PARAM, // Channel: Minimum CV Voltage
		OSC_CH_MAX_CV_VOLT_PARAM, // Channel: Maximum CV Voltage
		OSC_CH_MIN_OSC_VAL_PARAM, // Channel: Minimum OSC Value
		OSC_CH_MAX_OSC_VAL_PARAM, // Channel: Maximum OSC Value
		OSC_CH_CLIP_CV_VOLT_PARM, // Channel: Clip/truncate the input voltage to the MIN and MAX if we are translating vals.		
		OSC_CH_SEND_FREQ_PARAM, // [TBI] Channel [INPUT->OSC only]: Send frequency (if trigger not active)
		OSC_CH_SEND_THRESHOLD_PARAM, // [TBI] Channell [INPUT->OSC only]: CV value change needed to trigger send (if trigger not active)
		OSC_EXPANDER_CONFIG_PREV_PARAM, // Expander: Configure Previous	(goes towards Left/Input)
		OSC_EXPANDER_CONFIG_NEXT_PARAM, // Expander: Configure Next (goes towards Right/Output)
		CH_PARAM_START,
		NUM_PARAMS = CH_PARAM_START // Add #channels * 2 to this
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
		OSC_CONFIGURE_LIGHT, // The light for configuring OSC.
		OSC_ENABLED_LIGHT, // Light for OSC enabled and currently running/active.
		OSC_CH_TRANSLATE_LIGHT, // Light for Channel Translate enabled.
		OSC_CONFIGURE_PREV_LIGHT, // Light if there are 'previous' modules to configure.
		OSC_CONFIGURE_NEXT_LIGHT, // Light if there are 'next' modules to configure.		
		CH_LIGHT_START,
		NUM_LIGHTS = CH_LIGHT_START // Add # channels *2 to this
	};

	// Flag for doing Input Rack CV -> OSC. 
	bool doCVPort2OSC = true;
	// Flag for Input OSC -> Rack CV
	bool doOSC2CVPort = true;
	// Number of channels we have
	int numberChannels = TROWA_OSCCV_DEFAULT_NUM_CHANNELS;
	// Input CV (from Rack) ==> Needs to be output to OSC
	TSOSCCVInputChannel* inputChannels = NULL;
	// Input OSC (from External) ==> Needs to be translated to Rack output port CV
	TSOSCCVChannel* outputChannels = NULL;
	dsp::PulseGenerator* pulseGens = NULL;
	// The received messages.
	std::queue<TSOSCCVSimpleMessage*> rxMsgQueue;
	dsp::SchmittTrigger* inputTriggers;
	std::mutex rxMsgMutex;
#if USE_MODULE_STATIC_RX	
	// Add static buffer (debug MAC OSC issues)
	TSOSCCVSimpleMessage rxMsgBuffer[OSC_RX_MSG_BUFFER_SIZE];
	int rxMsgBufferIx = 0;
#endif	
		
	int oscId;
	/// TODO: OSC members should be dumped into an OSC base class....
	// Mutex for osc messaging.
	std::mutex oscMutex;
	// Current OSC IP address and port settings.
	TSOSCConnectionInfo currentOSCSettings = { OSC_ADDRESS_DEF,  OSC_OUTPORT_DEF , OSC_INPORT_DEF };
	// OSC Configure trigger
	dsp::SchmittTrigger oscConfigTrigger;
	dsp::SchmittTrigger oscConnectTrigger;
	// Show the OSC configuration screen or not.
	bool oscShowConfigurationScreen = false;
	
	//----- EXPANDERS -----
	// [Expander] Current edit expander:
	oscCVExpander* expCurrentEditExpander = NULL;
	// [Expander] Current edit expander index. 0 is none. Negative is to the LEFT (input), Positive to the right (output).
	int expCurrentEditExpanderIx = 0;
	// [Expander] Current edit name.
	std::string expCurrentEditExpanderName;
	
	// Configure - Previous (goes Left or towards Inputs).
	dsp::SchmittTrigger oscConfigPrevTrigger;
	// Configure - Next (goes Right or towards Output).
	dsp::SchmittTrigger oscConfigNextTrigger;	

	float sendDt = 0.0f;
	int sendFrequency_Hz = TROWA_OSCCV_DEFAULT_SEND_HZ;

	// Flag to reconnect at load. IFF true and oscInitialized is also true.
	bool oscReconnectAtLoad = false;
	// Flag if OSC objects have been initialized
	bool oscInitialized = false;
	// If there is an osc error.
	bool oscError = false;
	// OSC output buffer.
	char* oscBuffer = NULL;
	// OSC namespace to use. Without the '/'.
	std::string oscNamespace = TROWA_OSCCV_DEFAULT_NAMESPACE;
	// Sending OSC socket
	UdpTransmitSocket* oscTxSocket = NULL;
	// OSC message listener
	//TSOSCCVSimpleMsgListener* oscListener = NULL;
	// Receiving OSC socket
	UdpListeningReceiveSocket* oscRxSocket = NULL;
	// The OSC listener thread
	std::thread oscListenerThread;
	// Prev step that was last turned off (when going to a new step).
	int oscLastPrevStepUpdated = TROWA_INDEX_UNDEFINED;
	// Settings for new OSC.
	TSOSCInfo oscNewSettings = { OSC_ADDRESS_DEF,  OSC_OUTPORT_DEF , OSC_INPORT_DEF };
	// OSC Mode action (i.e. Enable, Disable)
	enum OSCAction {
		None,
		Disable,
		Enable
	};
	// Flag for our module to either enable or disable osc.
	OSCAction oscCurrentAction = OSCAction::None;
	// If this has it controls configured.
	bool isInitialized = false;
	const float lightLambda = 0.005f;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// oscCV()
	// Create a module with numChannels.
	// @numChannels: (IN) Number of input and output 'channels'. Each channel has two CV's.
	// @cv2osc: (IN) True to do CV to OSC out.
	// @osc2cv: (IN) True to do OSC to CV out.
	// At least one of those flags should be true.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	oscCV(int numChannels, bool cv2osc, bool osc2cv);

	oscCV() : oscCV(TROWA_OSCCV_DEFAULT_NUM_CHANNELS, true, true) {
		return;
	}
	~oscCV();

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// initializeChannels(void)
	// Set channels to default values.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void initialChannels() {
		for (int i = 0; i < numberChannels; i++)
		{
			if (doCVPort2OSC) {
				inputChannels[i].channelNum = i + 1;
				inputChannels[i].path = "/ch/" + std::to_string(i + 1);
				inputChannels[i].initialize();
			}
			if (doOSC2CVPort) {
				outputChannels[i].channelNum = i + 1;
				outputChannels[i].path = "/ch/" + std::to_string(i + 1);
				outputChannels[i].initialize();
			}
		}
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// initOSC()
	// @ipAddres : (IN) The ip address of the OSC client / server.
	// @outputPort : (IN) The Tx port.
	// @inputPort : (IN) The Rx port.
	// Initialize OSC on the given ip and ports.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void initOSC(const char* ipAddress, int outputPort, int inputPort);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Clean up OSC.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void cleanupOSC();
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// setOscNamespace()
	// @oscNamespace : (IN) The namespace (without /).
	// Set the OSC namespace (thread safe-ish).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void setOscNamespace(std::string oscNamespace);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// getOscNamespace()
	// Get the osc namespace.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	std::string getOscNamespace();	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// process()
	// [Previously step(void)]
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void process(const ProcessArgs &args) override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// reset(void)
	// Initialize values.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void onReset() override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// dataToJson(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	json_t *dataToJson() override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// dataFromJson(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void dataFromJson(json_t *rootJ) override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// getNumExpansionModules()	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-			
	int getNumExpansionModules(TSOSCCVExpanderDirection dir);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// getNumExpansionModulesInput()
	// Get the number of input expanders (left).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-			
	int getNumExpansionModulesInput()
	{
		return getNumExpansionModules(TSOSCCVExpanderDirection::Input);
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// getNumExpansionModulesOutput()
	// Get the number of output expanders (right).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-			
	int getNumExpansionModulesOutput()
	{
		return getNumExpansionModules(TSOSCCVExpanderDirection::Output);
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// getExpansionModule()
	// @index: 0 is this master module (invalid). Negative to the left. Positive to the right.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
	oscCVExpander* getExpansionModule(int index);
#if USE_MODULE_STATIC_RX	
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

#endif // !MODULE_OSCCV_HPP
