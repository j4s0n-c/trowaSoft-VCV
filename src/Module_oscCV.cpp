#include "Module_oscCV.hpp"
#include <rack.hpp>
using namespace rack;
#include "TSOSCCV_Common.hpp"
#include "TSOSCCommunicator.hpp"
#include "Widget_oscCV.hpp"
#include <cmath>
#include "TSOSCCV_RxConnector.hpp"
#include <string>

// Model for trowa OSC2CV
Model* modelOscCV = createModel<oscCV, oscCVWidget>(/*slug*/ "cvOSCcv");



//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// oscCV()
// Create a module with numChannels.
// @numChannels: (IN) Number of input and output 'channels'. Each channel has two CV's.
// @cv2osc: (IN) True to do CV to OSC out.
// @osc2cv: (IN) True to do OSC to CV out.
// At least one of those flags should be true.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
oscCV::oscCV(int numChannels, bool cv2osc, bool osc2cv) // : Module(NUM_PARAMS + numChannels*2, NUM_INPUTS + numChannels*2, NUM_OUTPUTS + numChannels * 2, NUM_LIGHTS + numChannels * 2)
{
	config(NUM_PARAMS + numChannels*2, NUM_INPUTS + numChannels*2, NUM_OUTPUTS + numChannels * 2, NUM_LIGHTS + numChannels * 2);
	oscInitialized = false;
	oscId = TSOSCConnector::GetId();
	this->doOSC2CVPort = osc2cv;
	this->doCVPort2OSC = cv2osc;

	this->numberChannels = numChannels;
	char buffer[50];	
	if (doCVPort2OSC)
	{
		inputTriggers = new dsp::SchmittTrigger[numberChannels];
		inputChannels = new TSOSCCVInputChannel[numberChannels];
		// [Rack v2] Add labels for inputs and outputs
		int inputId = 0;
		DEBUG("oscCV - Setting Input Labels");
		for (int i = 0; i < numberChannels; i++)
		{
			inputId = i * 2;
			sprintf(buffer, "Ch %d Trigger Send", i + 1);
			configInput(InputIds::CH_INPUT_START + inputId, buffer);
			sprintf(buffer, "Ch %d Value", i + 1);
			configInput(InputIds::CH_INPUT_START + inputId + 1, buffer);			
		}		
	}
	if (doOSC2CVPort)
	{
		outputChannels = new TSOSCCVChannel[numberChannels];
		pulseGens = new dsp::PulseGenerator[numberChannels];
		// [Rack v2] Add labels for inputs and outputs
		int inputId = 0;
		DEBUG("oscCV - Setting Output Labels");		
		for (int i = 0; i < numberChannels; i++)
		{
			inputId = i * 2;
			sprintf(buffer, "Ch %d Received Trigger", i + 1);
			configOutput(OutputIds::CH_OUTPUT_START + inputId, buffer);
			sprintf(buffer, "Ch %d Value Received", i + 1);
			configOutput(OutputIds::CH_OUTPUT_START + inputId + 1, buffer);			
		}
	}
	initialChannels();
	
	// Configure parameters:
	// id, min, max, def
	// configParam(/*paramId*/ oscCV::ParamIds::OSC_SHOW_CONF_PARAM, /*minVal*/ 0.0f, /*maxVal*/ 1.0f, /*defVal*/ 0.0f);	
	// configParam(/*paramId*/ oscCV::ParamIds::OSC_AUTO_RECONNECT_PARAM, /*minVal*/ 0.0f, /*maxVal*/ 1.0f, /*defVal*/ 0.0f);	
	
	// [Rack v2] : configButton
	// Main Configuation Navigation Buttons:
	configButton(/*paramId*/ oscCV::ParamIds::OSC_SHOW_CONF_PARAM, "Show/Hide OSC Configuration");		
	configButton(/*paramId*/ oscCV::ParamIds::OSC_EXPANDER_CONFIG_PREV_PARAM, "<< Expander Config");
	configButton(/*paramId*/ oscCV::ParamIds::OSC_EXPANDER_CONFIG_NEXT_PARAM, "Expander Config >>");	
	
	// OSC Configuration Screen:
	configButton(/*paramId*/ oscCV::ParamIds::OSC_AUTO_RECONNECT_PARAM, "Automatically reconnect on load");		
	configButton(/*paramId*/ oscCV::ParamIds::OSC_SAVE_CONF_PARAM, "Enable/Disable OSC");	
	
#if TROWA_OSSCV_SHOW_ADV_CH_CONFIG	
	// configParam(/*paramId*/ oscCV::ParamIds::OSC_CH_SAVE_PARAM, /*minVal*/ 0.0f, /*maxVal*/ 1.0f, /*defVal*/ 0.0f);
	// configParam(/*paramId*/ oscCV::ParamIds::OSC_CH_TRANSLATE_VALS_PARAM, /*minVal*/ 0.0f, /*maxVal*/ 1.0f, /*defVal*/ 0.0f);			
	// configParam(/*paramId*/ oscCV::ParamIds::OSC_CH_CANCEL_PARAM, /*minVal*/ 0.0f, /*maxVal*/ 1.0f, /*defVal*/ 0.0f);
	configButton(/*paramId*/ oscCV::ParamIds::OSC_CH_SAVE_PARAM, "Save");
	configButton(/*paramId*/ oscCV::ParamIds::OSC_CH_TRANSLATE_VALS_PARAM, "Translate values");			
	configButton(/*paramId*/ oscCV::ParamIds::OSC_CH_CANCEL_PARAM, "Cancel");
	configButton(/*paramId*/ oscCV::ParamIds::OSC_CH_CLIP_CV_VOLT_PARM, "Clip Values");	
	
	// Channel parameters:	
	for (int ch = 0; ch < numberChannels; ch++)
	{
		int baseParamId = oscCV::ParamIds::CH_PARAM_START + ch*TSOSCCVChannel::BaseParamIds::CH_NUM_PARAMS;		
		//configParam(/*id*/ baseParamId + TSOSCCVChannel::BaseParamIds::CH_SHOW_CONFIG, /*minVal*/ 0.0f, /*maxVal*/ 1.0f, /*defVal*/ 0.0f);
		configButton(/*id*/ baseParamId + TSOSCCVChannel::BaseParamIds::CH_SHOW_CONFIG, "Configure Input Channel " +  std::to_string(ch + 1));
		configButton(/*id*/ baseParamId + numberChannels + TSOSCCVChannel::BaseParamIds::CH_SHOW_CONFIG, "Configure Output Channel " +  std::to_string(ch + 1));		
	}
#endif // Show Advanced Configuration on each Channel		
	
	
	
	return;
} // end constructor
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Clean up or ram.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
oscCV::~oscCV()
{
	oscInitialized = false;

	cleanupOSC();
	if (oscBuffer != NULL)
	{
		free(oscBuffer);
		oscBuffer = NULL;
	}

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
	
	if (inputChannels != NULL)
		delete[] inputChannels;
	if (outputChannels != NULL)
		delete[] outputChannels;
	if (pulseGens != NULL)
		delete[] pulseGens;
	if (inputTriggers != NULL)
		delete[] inputTriggers;
	return;
} // end destructor
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// reset(void)
// Initialize values.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void oscCV::onReset() {
	// Stop OSC while we reset the values
	cleanupOSC();

	setOscNamespace(TROWA_OSCCV_DEFAULT_NAMESPACE); // Default namespace.
	this->oscReconnectAtLoad = false;

	// Reset our values
	oscMutex.lock();
	initialChannels();
	this->currentOSCSettings.oscTxIpAddress = OSC_ADDRESS_DEF;
	this->currentOSCSettings.oscTxPort = OSC_OUTPORT_DEF;
	this->currentOSCSettings.oscRxPort = OSC_INPORT_DEF;
	oscMutex.unlock();

	this->oscShowConfigurationScreen = false;

	sendDt = 0.0f;
	sendFrequency_Hz = TROWA_OSCCV_DEFAULT_SEND_HZ;
	return;
} // end reset()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// initOSC()
// @ipAddres : (IN) The ip address of the OSC client / server.
// @outputPort : (IN) The Tx port.
// @inputPort : (IN) The Rx port.
// Initialize OSC on the given ip and ports.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void oscCV::initOSC(const char* ipAddress, int outputPort, int inputPort)
{
	oscMutex.lock();
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
	DEBUG("oscCV::initOSC() - Initializing OSC");
#endif
	try
	{
		bool portsRegistered = false;
		// Try to register these ports:
		if (!doCVPort2OSC) {
			// No output - Rx Port can't be shared
			portsRegistered = TSOSCConnector::RegisterPortRecv(oscId, inputPort, /*rxSharedAllowed*/ true);
		}
		else if (!doOSC2CVPort) {
			// No OSC Input - Tx Port can be shared:
			portsRegistered = TSOSCConnector::RegisterPortTrans(oscId, outputPort, /*sharingAllowed*/ true);
		}
		else {
			// New version - Tx Port can be shared:			
			portsRegistered = TSOSCConnector::RegisterPorts(oscId, outputPort, inputPort, /*txSharedAllowed*/ true, /*rxSharedAllowed*/ true);
		}

		if (portsRegistered)
		{
			oscError = false;
			this->currentOSCSettings.oscTxIpAddress = ipAddress;
			if (oscBuffer == NULL)
			{
				oscBuffer = (char*)malloc(OSC_CV_OUTPUT_BUFFER_SIZE * sizeof(char));
			}
			if (doCVPort2OSC) {
				// CV Port -> OSC Tx
				if (oscTxSocket == NULL)
				{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
					DEBUG("oscCV::initOSC() - Create TRANS socket at %s, port %d.", ipAddress, outputPort);
#endif
					oscTxSocket = new UdpTransmitSocket(IpEndpointName(ipAddress, outputPort));
					this->currentOSCSettings.oscTxPort = outputPort;
				}
			}
			oscInitialized = true;			
			if (doOSC2CVPort) 
			{
				// OSC Rx -> CV Port
				oscInitialized = OscCVRxConnector::StartListener(inputPort, this);
				this->currentOSCSettings.oscRxPort = inputPort;
			}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			DEBUG("oscCV::initOSC() - OSC Initialized");
#endif
		}
		else
		{
			oscError = true;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			DEBUG("oscCV::initOSC() - Ports in use already.");
#endif
		}
	}
	catch (const std::exception& ex)
	{
		oscError = true;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		WARN("oscCV::initOSC() - Error initializing: %s.", ex.what());
#endif
	}
	oscMutex.unlock();
	return;
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Clean up OSC.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void oscCV::cleanupOSC() 
{
	oscMutex.lock();
	try
	{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		DEBUG("oscCV::cleanupOSC() - Cleaning up OSC");
#endif
		oscInitialized = false;
		oscError = false;
		TSOSCConnector::ClearPorts(oscId, currentOSCSettings.oscTxPort, currentOSCSettings.oscRxPort);
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		DEBUG("oscCV::cleanupOSC() - Cleaning up RECV socket.");
#endif
		if (doOSC2CVPort)
		{
			OscCVRxConnector::StopListener(currentOSCSettings.oscRxPort, this);
		}


		if (oscTxSocket != NULL)
		{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			DEBUG("oscCV::cleanupOSC() - Cleanup TRANS socket.");
#endif
			delete oscTxSocket;
			oscTxSocket = NULL;
		}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		DEBUG("oscCV::cleanupOSC() - OSC cleaned");
#endif
	}
	catch (const std::exception& ex)
	{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		DEBUG("oscCV::cleanupOSC() - Exception caught:\n%s", ex.what());
#endif
	}
	oscMutex.unlock();
} // end cleanupOSC()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// dataToJson(void)
// Serialize to json.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
json_t *oscCV::dataToJson() {
	json_t* rootJ = json_object();

	// version
	json_object_set_new(rootJ, "version", json_integer(TROWA_INTERNAL_VERSION_INT));

	// OSC Parameters
	json_t* oscJ = json_object();
	json_object_set_new(oscJ, "IpAddress", json_string(this->currentOSCSettings.oscTxIpAddress.c_str()));
	json_object_set_new(oscJ, "TxPort", json_integer(this->currentOSCSettings.oscTxPort));
	json_object_set_new(oscJ, "RxPort", json_integer(this->currentOSCSettings.oscRxPort));
	json_object_set_new(oscJ, "Namespace", json_string(this->oscNamespace.c_str()));
	json_object_set_new(oscJ, "AutoReconnectAtLoad", json_boolean(oscReconnectAtLoad)); // [v11, v0.6.3]
	json_object_set_new(oscJ, "Initialized", json_boolean(oscInitialized)); // [v11, v0.6.3] We know the settings are good at least at the time of save
	json_object_set_new(rootJ, "osc", oscJ);

	// Channels
	json_object_set_new(rootJ, "numCh", json_integer(numberChannels));
	json_t* inputChannelsJ = json_array();
	json_t* outputChannelsJ = json_array();
	for (int c = 0; c < numberChannels; c++)
	{
		// Input
		json_array_append_new(inputChannelsJ, inputChannels[c].serialize());
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
void oscCV::dataFromJson(json_t *rootJ) {
	json_t* currJ = NULL;
	bool autoReconnect = false;
	// OSC Parameters
	json_t* oscJ = json_object_get(rootJ, "osc");
	if (oscJ)
	{
		currJ = json_object_get(oscJ, "IpAddress");
		if (currJ)
			this->currentOSCSettings.oscTxIpAddress = json_string_value(currJ);
		currJ = json_object_get(oscJ, "TxPort");
		if (currJ)
			this->currentOSCSettings.oscTxPort = (uint16_t)(json_integer_value(currJ));
		currJ = json_object_get(oscJ, "RxPort");
		if (currJ)
			this->currentOSCSettings.oscRxPort = (uint16_t)(json_integer_value(currJ));
		currJ = json_object_get(oscJ, "Namespace");
		if (currJ)
			setOscNamespace( json_string_value(currJ) );
		currJ = json_object_get(oscJ, "AutoReconnectAtLoad");
		if (currJ)
			oscReconnectAtLoad = json_boolean_value(currJ);
		if (oscReconnectAtLoad)
		{
			currJ = json_object_get(oscJ, "Initialized");
			autoReconnect = currJ && json_boolean_value(currJ);
		}
	} // end if OSC node

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
		if (inputChannelsJ)
		{
			json_t* channelJ = json_array_get(inputChannelsJ, c);
			if (channelJ) {
				inputChannels[c].deserialize(channelJ);
			} // end if channel object
		} // end if there is an inputChannels array
		// Output
		if (outputChannelsJ)
		{
			json_t* channelJ = json_array_get(outputChannelsJ, c);
			if (channelJ) {
				outputChannels[c].deserialize(channelJ);
			} // end if channel object
		} // end if there is an outputChannels array
	} // end loop through channels

	if (autoReconnect)
	{
		// Try to reconnect
		cleanupOSC();
		this->initOSC(this->currentOSCSettings.oscTxIpAddress.c_str(), this->currentOSCSettings.oscTxPort, this->currentOSCSettings.oscRxPort);

		if (oscError || !oscInitialized)
		{
			WARN("oscCV::dataFromJson(): Error on auto-reconnect OSC %s :%d :%d.", this->currentOSCSettings.oscTxIpAddress.c_str(), this->currentOSCSettings.oscTxPort, this->currentOSCSettings.oscRxPort);
		}
		else
		{
			INFO("oscCV::dataFromJson(): Successful auto-reconnection of OSC %s :%d :%d.", this->currentOSCSettings.oscTxIpAddress.c_str(), this->currentOSCSettings.oscTxPort, this->currentOSCSettings.oscRxPort);
		}
	}
	return;
} // end dataFromJson() 

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// process()
// [Previously step(void)]
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void oscCV::process(const ProcessArgs &args)
{
	//bool oscStarted = false; // If OSC just started to a new address this step.
	switch (this->oscCurrentAction)
	{
	case OSCAction::Disable:
		this->cleanupOSC(); // Try to clean up OSC
		break;
	case OSCAction::Enable:
		this->cleanupOSC(); // Try to clean up OSC if we already have something		
		this->initOSC(this->oscNewSettings.oscTxIpAddress.c_str(), this->oscNewSettings.oscTxPort, this->oscNewSettings.oscRxPort);
		//oscStarted = this->oscInitialized;
		break;
	case OSCAction::None:
	default:
		break;
	}
	this->oscCurrentAction = OSCAction::None;
	
	// Handle inputs:
	int numExpandersLeft = getNumExpansionModulesInput();
	int numExpandersRight = getNumExpansionModulesOutput();
	// 1. Main configuration button:
	if (this->oscConfigTrigger.process(this->params[oscCV::ParamIds::OSC_SHOW_CONF_PARAM].getValue()))
	{
		this->oscShowConfigurationScreen = !this->oscShowConfigurationScreen;
		if (!oscShowConfigurationScreen)
		{
			// Reset
			expCurrentEditExpanderIx = 0;
			expCurrentEditExpander = NULL;
		}
	}
	// 2. Previous Config
	else if (this->oscConfigPrevTrigger.process(this->params[oscCV::ParamIds::OSC_EXPANDER_CONFIG_PREV_PARAM].getValue()))
	{
		// Previous button hit
		expCurrentEditExpanderIx--;
		expCurrentEditExpander = getExpansionModule(expCurrentEditExpanderIx);
		if (expCurrentEditExpander)
		{
			this->oscShowConfigurationScreen = true;
		}
		else 
		{
			if (expCurrentEditExpanderIx < 0)
			{
				expCurrentEditExpanderIx = -numExpandersLeft; // Last one
				expCurrentEditExpander = getExpansionModule(expCurrentEditExpanderIx);				
			}
			else
			{
				expCurrentEditExpanderIx = 0;
				expCurrentEditExpander = NULL;
			}
		}
	}
	// 3. Next Config
	else if (this->oscConfigNextTrigger.process(this->params[oscCV::ParamIds::OSC_EXPANDER_CONFIG_NEXT_PARAM].getValue()))
	{
		// Previous button hit
		expCurrentEditExpanderIx++;
		expCurrentEditExpander = getExpansionModule(expCurrentEditExpanderIx);
		if (expCurrentEditExpander)
		{
			this->oscShowConfigurationScreen = true;
		}
		else 
		{
			if (expCurrentEditExpanderIx > 0)
			{
				expCurrentEditExpanderIx = numExpandersRight; // Last one
				expCurrentEditExpander = getExpansionModule(expCurrentEditExpanderIx);				
			}
			else
			{
				expCurrentEditExpanderIx = 0;				
				expCurrentEditExpander = NULL;				
			}
		}
	}
	if (expCurrentEditExpander)
	{
		try
		{
			expCurrentEditExpanderName = expCurrentEditExpander->displayName;
		}
		catch (const std::exception& expansionEx)
		{
			WARN("Error expander must be deleted...\n%s", expansionEx.what());	
			expCurrentEditExpanderName = "Connection Lost";
		}
	}
	
	lights[LightIds::OSC_CONFIGURE_PREV_LIGHT].value = (-numExpandersLeft < expCurrentEditExpanderIx) ? 1 : 0.2;
	lights[LightIds::OSC_CONFIGURE_NEXT_LIGHT].value = (numExpandersRight > expCurrentEditExpanderIx) ? 1 : 0.2;	
	

	// OSC is Enabled and Active light
	lights[LightIds::OSC_ENABLED_LIGHT].value = (oscInitialized) ? 1.0 : 0.0;

	//--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--
	// Rack Input Ports ==> OSC
	//--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--
	if (doCVPort2OSC) {
		// Timer for sending values
		bool sendTime = false;
		sendDt += sendFrequency_Hz / args.sampleRate;
		if (sendDt > 1.0f) {
			sendDt -= 1.0f;
			sendTime = true;
		}
		//------------------------------------------------------------
		// Look for inputs from Rack --> Send Out OSC
		//------------------------------------------------------------
		bool packetOpened = false;
		osc::OutboundPacketStream oscStream(oscBuffer, OSC_CV_OUTPUT_BUFFER_SIZE);
		char addressBuffer[512];
		// ### Our own channels ###
		for (int c = 0; c < this->numberChannels; c++)
		{			
			bool sendVal = false;
			if (oscInitialized && inputs[InputIds::CH_INPUT_START + c * 2 + 1].isConnected())
			{
				// Read Input
				if (inputs[InputIds::CH_INPUT_START + c * 2 + 1].getChannels() > 1)
				{
					// Poly CV
					for (int i = 0; i < inputs[InputIds::CH_INPUT_START + c * 2 + 1].getChannels(); i++)
					{
						inputChannels[c].setValue(inputs[InputIds::CH_INPUT_START + c * 2 + 1].getVoltage(i), i);
					}
				}
				else
				{
					// Mono CV
					inputChannels[c].setValue(inputs[InputIds::CH_INPUT_START + c * 2 + 1].getVoltage());
				}				
				
				// Check for sending the value:
				if (inputs[InputIds::CH_INPUT_START + c * 2].isConnected()) 
				{
					// Input Trigger Port
					sendVal = inputTriggers[c].process(inputs[InputIds::CH_INPUT_START + c * 2].getVoltage());
				} // end if trigger is active
				else
				{
					// See if value has changed enough 
					if (!inputChannels[c].doSend) {
						// Only send if changed enough. Maybe about 0.01 V? Defined on channel.
						// 1. Check for change:
						sendVal = inputChannels[c].valChanged();
						// 2. Mark channel as needing to output
						if (sendVal) {
							inputChannels[c].doSend = true;
						}
					}
					// 3. Check if it is time to send out
					sendVal = sendTime && inputChannels[c].doSend;
				} // end else check if value has changed enough
				
				if (sendVal)
				{
					lights[LightIds::CH_LIGHT_START + c * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL].value = 1.0f;
					oscMutex.lock();
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
						oscStream << osc::EndMessage;						
					}
					catch (const std::exception& e)
					{
						WARN("Error %s.", e.what());
					}
					oscMutex.unlock();
					// Save our last sent values
					//inputChannels[c].lastTranslatedVal = outVal;
					//inputChannels[c].lastVal = inputChannels[c].val;
					inputChannels[c].storeLastValues();
					inputChannels[c].doSend = false; // Reset
				} // end if send value 
			} // end if oscInitialied

			if (lights[LightIds::CH_LIGHT_START + c * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL].value > 0 && !sendVal) {
				lights[LightIds::CH_LIGHT_START + c * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL].value -= lightLambda;
			}
		} // end for loop
		
		// ### Expansion Modules ###		
		// Expansion CV -> OSC:
		try
		{
			// Input
			Expander* exp = &(this->leftExpander);
			while (exp != NULL && exp->module && exp->module->model == modelOscCVExpanderInput)
			{				
				dynamic_cast<oscCVExpander*>(exp->module)->processInputs(oscNamespace, oscInitialized, sendTime, packetOpened, oscMutex, oscStream);			
				exp = &(exp->module->leftExpander); // Go to next so we can see if that's another expander.
			}
		}
		catch (const std::exception& expansionEx)
		{
			WARN("Error with Expansion Module Input: %s.", expansionEx.what());
		}
		
		
		if (packetOpened)
		{
			oscMutex.lock();
			try
			{
				oscStream << osc::EndBundle;
				oscTxSocket->Send(oscStream.Data(), oscStream.Size());
			}
			catch (const std::exception& e)
			{
				WARN("Error %s.", e.what());
			}
			oscMutex.unlock();
		} // end if packet opened (close it)
		
	} // end Rack Input Ports ==> OSC Output

	//--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--
	// OSC ==> Rack Output Ports
	//--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--
	if (doOSC2CVPort)
	{
		//------------------------------------------------------------
		// Look for OSC Rx messages --> Output to Rack
		//------------------------------------------------------------
		while (rxMsgQueue.size() > 0)
		{
			try
			{
				rxMsgMutex.lock();				
				TSOSCCVSimpleMessage* rxOscMsg = rxMsgQueue.front();
				rxMsgMutex.unlock();
				if (rxOscMsg != NULL)
				{
					int chIx = rxOscMsg->channelNum - 1;
					if (chIx > -1 && chIx < numberChannels)
					{
						// Process the message
						pulseGens[chIx].trigger(TROWA_PULSE_WIDTH); // Trigger (msg received)
						//outputChannels[chIx].setValue(rxOscMsg.rxVal);
						//outputChannels[chIx].setOSCInValue(rxOscMsg->rxVals);
						outputChannels[chIx].setOSCInValue(rxOscMsg->rxVals, rxOscMsg->rxLength);							
						lights[LightIds::CH_LIGHT_START + chIx * 2 + 1].value = 1.0f;
					} // end if valid channel
					
#if DEBUG_MAC_OS_POINTER			
					DEBUG("oscCV::Deleting %d.", rxOscMsg);
#endif
#if !USE_MODULE_STATIC_RX
					// If we are not using a static buffer, then delete this.
					delete rxOscMsg;
					rxOscMsg = NULL
#endif 					
				}
			} 
			catch (std::exception& rxEx)
			{
				WARN("Error accessing received message.\n%s", rxEx.what());
			}
			rxMsgMutex.lock();				
			rxMsgQueue.pop();
			rxMsgMutex.unlock();			
		} // end while (loop through message queue)
		// ::: OUTPUTS :::
		float dt = args.sampleTime; //1.0 / engineGetSampleRate();
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
			outputChannels[c].addValToBuffer(outputChannels[c].translatedVals[0]);
			
			// Then trigger if needed.
			bool trigger = pulseGens[c].process(dt);
			outputs[OutputIds::CH_OUTPUT_START + c * 2].setVoltage((trigger) ? TROWA_OSCCV_TRIGGER_ON_V : TROWA_OSCCV_TRIGGER_OFF_V);
			lights[LightIds::CH_LIGHT_START + c * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL + 1].value = clamp(lights[LightIds::CH_LIGHT_START + c * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL + 1].value - lightLambda, 0.0f, 1.0f);			
		}
		
		// ### Expansion Modules ###		
		// Expansion OSC->CV:
		try
		{
			// Input
			Module::Expander* exp = &(this->rightExpander);
			while (exp != NULL && exp->module && exp->module->model == modelOscCVExpanderOutput)
			{				
				dynamic_cast<oscCVExpander*>(exp->module)->processOutputs(dt);			
				exp = &(exp->module->rightExpander); // Go to next so we can see if that's another expander.
			}
		}
		catch (const std::exception& expansionEx)
		{
			WARN("Error with Expansion Module Output: %s.", expansionEx.what());
		}				
	} // end OSC->CV
	return;
} // end step()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// setOscNamespace()
// @oscNamespace : (IN) The namespace (without /).
// Set the OSC namespace (thread safe-ish).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void oscCV::setOscNamespace(std::string oscNs)
{
	std::lock_guard<std::mutex> lock(oscMutex);
	if (!oscNs.empty() && oscNs.at(0) == '/')
		this->oscNamespace = oscNs.substr(1);
	else
		this->oscNamespace = oscNs;
	return;
} // end setOscNamespace()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// getOscNamespace()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
std::string oscCV::getOscNamespace()
{
	std::lock_guard<std::mutex> lock(oscMutex);
	return this->oscNamespace;
} // end getOscNamespace()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// getNumExpansionModules()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
int oscCV::getNumExpansionModules(TSOSCCVExpanderDirection dir)
{
	int n = 0;
	Module::Expander* exp = (dir == TSOSCCVExpanderDirection::Input) ? &(this->leftExpander) : &(this->rightExpander);
	while (exp != NULL && exp->module && 
		((dir == TSOSCCVExpanderDirection::Input && exp->module->model == modelOscCVExpanderInput) 
			|| (dir == TSOSCCVExpanderDirection::Output && exp->module->model == modelOscCVExpanderOutput)))
	{				
		n++;
		exp = (dir == TSOSCCVExpanderDirection::Input) ? &(exp->module->leftExpander) : &(exp->module->rightExpander);
	}	
	return n;
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// getExpansionModule()
// @index: 0 is this master module (invalid). Negative to the left. Positive to the right.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
oscCVExpander* oscCV::getExpansionModule(int index)
{
	oscCVExpander* item = NULL;
	if (index != 0)
	{
		int n = 0;
		bool left = false;
		int count = index;
		if (index < 0)
		{
			left = true;
			count = -index;
		}
		Module::Expander* exp = (left) ? &(this->leftExpander) : &(this->rightExpander);
		while (exp != NULL && exp->module && item == NULL && 
			((left && exp->module->model == modelOscCVExpanderInput) || (!left && exp->module->model == modelOscCVExpanderOutput))
			)
		{				
			n++;
			if (n == count)
			{
				item =  dynamic_cast<oscCVExpander*>(exp->module);
			}
			else
			{
				exp = (left) ? &(exp->module->leftExpander) : &(exp->module->rightExpander);				
			}
		}	
	}			
	return item;
}
#if USE_MODULE_STATIC_RX
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// addRxMsgToQueue()
// Adds the message to the queue.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void oscCV::addRxMsgToQueue(int chNum, float val)
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
void oscCV::addRxMsgToQueue(int chNum, std::vector<float> vals)
{
	rxMsgMutex.lock();	
	TSOSCCVSimpleMessage* item = getRxMsgObj();
	item->SetValues(chNum, vals);
	rxMsgQueue.push(item);
	rxMsgMutex.unlock();
	return;
}
#endif

