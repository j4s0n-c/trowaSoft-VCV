#ifndef TSOSCCV_COMMON_HPP
#define TSOSCCV_COMMON_HPP

#include <rack.hpp>
using namespace rack;
#include "trowaSoft.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSOSCCommon.hpp"
#include <mutex>
#include <vector>

#define TROWA_OSCCVEXPANDER_DEFAULT_NUM_CHANNELS	8 // Default # channels for expander. Fixed 2023-01-02. Was 16 since I forgot to change it to 8.

#define TROWA_OSCCV_DEFAULT_NUM_CHANNELS		8 // Default number of channels for cvOSCcv (main module).
#define TROWA_OSCCV_NUM_PORTS_PER_INPUT			2 // Each input port should have a trigger input and actual value input.
#define TROWA_OSCCV_DEFAULT_NAMESPACE		   "" // Default namespace for this module (should not be the same as the sequencers). Now Blank was 'trowacv'.
#define TROWA_OSCCV_MAX_VOLTAGE				 10.0 // Max output voltage
#define TROWA_OSCCV_MIN_VOLTAGE				-10.0 // Min output voltage
#define TROWA_OSCCV_VAL_BUFFER_SIZE			  512 // Buffer size for value history
#define TROWA_OSCCV_TRIGGER_ON_V			 10.0 // Trigger on/high output voltage
#define TROWA_OSCCV_TRIGGER_OFF_V			  0.0 // Trigger off/low output voltage
#define TROWA_OSCCV_MIDI_VALUE_MIN_V		     -5 // -5v : Midi Value 0 (C-1)
#define TROWA_OSCCV_MIDI_VALUE_MAX_V		5.58333 // +5.5833v : Midi Value 127
#define TROWA_OSCCV_MIDI_VALUE_MIN		         0 // Midi value 0
#define TROWA_OSCCV_MIDI_VALUE_MAX			   127 // Midi value 127
#define TROWA_OSCCV_DEFAULT_SEND_HZ			   100 // If no trigger input, bang out OSC when val changes this many times per second.
#define TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL		2

#define TROWA_OSCCV_VECTOR_MAX_SIZE			engine::PORT_MAX_CHANNELS // Now with polyphonic cables, there can be 16 channels sent in one CV input/output

//=== DEBUG MacOS ====
// for cvOSCcv
#define DEBUG_MAC_OS_POINTER					 0
#define USE_STATIC_ARRAY						 1 // Use a STATIC array for OSC Message since it seems MAC OS doesn't handle new and delete[] when it's very fast (and vector errors too).
#define USE_MODULE_STATIC_RX					 1 // Debug MAC OS issues. Start keeping a static buffer of msg objects for each module.
#define OSC_RX_MSG_BUFFER_SIZE				    50 // Debug MAC OS issues. Start keeping a static buffer of msg objects for each module.
// Was 40, increased to 50 for 32-channel expanders.

// Available options for send frequency (Hz)
#define TROWA_OSCCV_NUM_SEND_HZ_OPTS		    6  // Number of send options in our simple array. Add quick & dirty simple run-time config of sending frequency.
extern const int TROWA_OSCCV_Send_Freq_Opts_Hz[TROWA_OSCCV_NUM_SEND_HZ_OPTS];

// A channel for OSC.
struct TSOSCCVChannel {
	// Base param ids for the channel
	enum BaseParamIds {
		CH_SHOW_CONFIG,
		CH_NUM_PARAMS
	};
	// Path for this channel. Must start with '/'.
	std::string path;
	// The value
	float val = 0.0;
	// The translated value
	float translatedVal = 0.0;
	uint32_t uintVal = 0;
	
	int numVals = 1;
	// Values
	std::vector<float> vals;
	// Translated values.
	std::vector<float> translatedVals;
	
	// Channel number (1-based)
	int channelNum;

	// What our parameter type should be. We can't really translate strings to voltage, so that is not available.
	enum ArgDataType : int {
		OscFloat = 1,
		OscInt = 2,
		OscBool = 3,
		// An OSC Midi message -- Not sure what actually supports this natively.
		OscMidi = 20
	};
	ArgDataType dataType = ArgDataType::OscFloat;
	// Message received.
	//float msgReceived = 0.0;

	// Flag to store history values or not.
	bool storeHistory = false;
	// Value buffer
	float* valBuffer = NULL;  	//float valBuffer[TROWA_OSCCV_VAL_BUFFER_SIZE] = { 0.0 };
	// Value buffer current index to insert into.
	int valBuffIx = 0;
	// The frame index.
	int frameIx = 0;

	// Show channel configuration for this channel.
	dsp::SchmittTrigger showChannelConfigTrigger;
	

	/// TODO: Configuration for conversion & use the conversion stuff.
	/// TODO: Eventually allow strings? Basically user would have to enumerate and we should have an index into the array of strings.
	const float VoltageUnused = -100.f;

	// Min Rack input or output voltage
	float minVoltage = TROWA_OSCCV_MIN_VOLTAGE;
	// Max Rack input or output voltage
	float maxVoltage = TROWA_OSCCV_MAX_VOLTAGE;
	// Min OSC input or output value.
	float minOscVal = 0;
	// Max OSC input or output value.
	float maxOscVal = 127;
	// If we should translate between the min and max values.
	bool convertVals = false;
	// If we are converting values, if we should clip them.
	bool clipVals = false;

	std::mutex mutPath;

	TSOSCCVChannel()
	{
		numVals = 1;
		vals.push_back(0.f);
		translatedVals.push_back(0.f);
		return;
	}
	//TSOSCCVChannel(int chNum, std::string path) : TSOSCCVChannel()
	//{
	//	this->channelNum = chNum;
	//	this->path = path;
	//	initialize();
	//	return;
	//}

	~TSOSCCVChannel() {
		vals.clear();
		translatedVals.clear();
		if (valBuffer != NULL)
		{
			delete[] valBuffer;
			valBuffer = NULL;
		}
		return;
	}
	// Set if we should store history or not.
	// Since expanders don't get displays for shown, let's default to NO and have cvOSCcv specifically set it to true.
	void setStoreHistory(bool storeVals)
	{
		this->storeHistory = storeVals;
		if (storeHistory)
		{
			if (valBuffer == NULL)
				valBuffer = new float[TROWA_OSCCV_VAL_BUFFER_SIZE];
		}
		else if (valBuffer != NULL)
		{
			delete[] valBuffer;
			valBuffer = NULL;
		}
		return;
	}

	virtual void initialize() {
		this->convertVals = false;
		this->val = 0.0;
		this->translatedVal = getValOSC2CV();
		this->dataType = ArgDataType::OscFloat;
		// Min Rack input or output voltage
		minVoltage = TROWA_OSCCV_MIDI_VALUE_MIN_V;
		// Max Rack input or output voltage
		maxVoltage = TROWA_OSCCV_MIDI_VALUE_MAX_V;
		// Min OSC input or output value.
		minOscVal = 0;
		// Max OSC input or output value.
		maxOscVal = 127;
		if (storeHistory)
		{
			if (valBuffer == NULL)
				valBuffer = new float[TROWA_OSCCV_VAL_BUFFER_SIZE];
			for (int i = 0; i < TROWA_OSCCV_VAL_BUFFER_SIZE; i++)
			{
				valBuffer[i] = 0.0f;
			}
		}
		valBuffIx = 0;
		convertVals = false;
		return;
	} // end initialize()


	// Get the value translated from OSC to CV voltage.
	float getValOSC2CV(int ix = 0) {
		float v = (ix < numVals) ? vals[ix] : 0;
		float tVal = v;
		if (convertVals) {
			if (clipVals)
			{
				v = clamp(v, minOscVal, maxOscVal); // [v1.0.2] Add clipping to input if set to do so.
			}			
			tVal = rescale(v, minOscVal, maxOscVal, minVoltage, maxVoltage);
		}
		return tVal;
	}
	// Get the value translated from CV voltage to OSC value.
	float getValCV2OSC(int ix = 0) {
		float v = (ix < numVals) ? vals[ix] : 0;
		float tVal = v;
		if (convertVals) {
			if (clipVals)
			{
				v = clamp(v, minVoltage, maxVoltage); // [v1.0.2] Add clipping to input if set to do so.
			}
			tVal = rescale(v, minVoltage, maxVoltage, minOscVal, maxOscVal);
			switch (this->dataType)
			{
			case TSOSCCVChannel::ArgDataType::OscInt:
				tVal = static_cast<float>(static_cast<int>(tVal));
				break;
			case TSOSCCVChannel::ArgDataType::OscBool:
				tVal = static_cast<float>(static_cast<bool>(tVal));
				break;
			case TSOSCCVChannel::ArgDataType::OscFloat:
			default:
				break;
			}
		}
		return tVal;
	}
	// Single input
	void setOSCInValue(float oscVal) {
		numVals = 1;
		val = oscVal;
		translatedVal = getValOSC2CV();
		vals[0] = val;
		translatedVals[0] = translatedVal;
		return;
	}
	// Multiple input.
	void setOSCInValue(std::vector<float>& oscVals) {
		numVals = static_cast<int>(oscVals.size());
		for (int i = 0; i < numVals; i++)
		{
			vals[i] = oscVals[i];
			translatedVals[i] = getValOSC2CV(i);
		}
		val = oscVals[0];
		translatedVal = translatedVals[0];
		return;
	}
	// Multiple input.
	void setOSCInValue(float* oscVals, int size) 
	{
		numVals = size;
		for (int i = 0; i < size; i++)
		{
			vals[i] = oscVals[i];
			translatedVals[i] = getValOSC2CV(i);
		}
		val = oscVals[0];
		translatedVal = translatedVals[0];
		return;
	}	
	void addValToBuffer(float buffVal);

	// Sets the value from CV input.
	void setValue(float newVal) {
		val = newVal;
		if (convertVals)
			translatedVal = getValCV2OSC();
		else
			translatedVal = val;
		vals[0] = val;
		translatedVals[0] = translatedVal;
		if (storeHistory)
			addValToBuffer(newVal);
		return;
	}
	// Sets the value from CV input.
	void setValue(float newVal, int ix) {
		if (ix >= numVals || ix >= static_cast<int>(vals.size()))
		{
			numVals = ix + 1;
			int n = vals.size();
			for (int i = n; i < numVals; i++)
			{
				vals.push_back(VoltageUnused);
				translatedVals.push_back(VoltageUnused);
			}
		}
		vals[ix] = newVal;
		if (convertVals)
			translatedVals[ix] = getValCV2OSC(ix);
		else
			translatedVals[ix] = vals[ix];
		if (ix < 1)
		{
			val = vals[0];
			translatedVal = translatedVals[0];
			if (storeHistory)
				addValToBuffer(newVal);	
		}
		return;
	}
	
	void setPath(std::string path)
	{
		std::lock_guard<std::mutex> lock(mutPath);		
		if (path.length() > 0 && path.at(0) != '/') 
			this->path = "/" + path;
		else
			this->path = path;
		return;
	}
	std::string getPath() {
		std::lock_guard<std::mutex> lock(mutPath);
		return path;
	}

	//--------------------------------------------------------
	// serialize()
	// @returns : The channel json node.
	//--------------------------------------------------------
	virtual json_t* serialize();
	//--------------------------------------------------------
	// deserialize()
	// @rootJ : (IN) The channel json node.
	//--------------------------------------------------------
	virtual void deserialize(json_t* rootJ);

};
// Channel specifically for CV Input -> OSC.
// Extra stuff for knowing when to send output.
struct TSOSCCVInputChannel : TSOSCCVChannel {
	// The last value we SENT over OSC (for tracking changes).
	std::vector<float> lastVals;
	// The last translated value we SENT over OSC (for tracking changes).
	std::vector<float> lastTranslatedVals;
	
	// If trigger is not set up (input type channel), how much input change is needed to send a message out.
	float channelSensitivity = 0.05f;
	// If we should send. Working value for module.
	bool doSend = false;

	TSOSCCVInputChannel() : TSOSCCVChannel()
	{
		for (int i = 0; i < TROWA_OSCCV_VECTOR_MAX_SIZE; i++)
		{			
			lastVals.push_back(VoltageUnused);
			lastTranslatedVals.push_back(VoltageUnused);
		}
		return;
	}

	TSOSCCVInputChannel(int chNum, std::string path) : TSOSCCVInputChannel()
	{
		this->channelNum = chNum;
		this->path = path;
		this->initialize();
		return;
	}	
	void initialize() override {
		initLastVals();
		channelSensitivity = 0.05f;
		TSOSCCVChannel::initialize();
		doSend = false;
		return;
	} // end initialize()
	
	void initLastVals()
	{
		for (int i = 0; i < TROWA_OSCCV_VECTOR_MAX_SIZE; i++)
		{			
			lastVals[i] = VoltageUnused;
			lastTranslatedVals[i] = VoltageUnused;
		}
	}
	// Checks to see if vals have changed enough to send a value.
	bool valChanged()
	{
		bool sendVal = false;
		int i = 0;
		while (i < numVals && !sendVal)
		{
			if (convertVals)
			{
				sendVal = std::abs(translatedVals[i] - lastTranslatedVals[i]) > channelSensitivity;							
			}
			else
			{
				sendVal = std::abs(vals[i] - lastVals[i]) > channelSensitivity;							
			}
			i++;
		}
		return sendVal;
	}
	
	void storeLastValues()
	{
		for (int i = 0; i < TROWA_OSCCV_VECTOR_MAX_SIZE; i++)
		{
			if (i < numVals)
			{
				lastTranslatedVals[i] = translatedVals[i];
				lastVals[i] = vals[i];
			}
			else
			{
				lastTranslatedVals[i] = VoltageUnused;
				lastVals[i] = VoltageUnused;
			}
		}
	}

	//--------------------------------------------------------
	// serialize()
	// @returns : The channel json node.
	//--------------------------------------------------------
	json_t* serialize() override;
	//--------------------------------------------------------
	// deserialize()
	// @rootJ : (IN) The channel json node.
	//--------------------------------------------------------
	void deserialize(json_t* rootJ) override;
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Simple single message to/from OSC. 
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSOSCCVSimpleMessage 
{
	// Channel Number (1-N)
	int channelNum;	
	// [v1.0.2] Support for polyphonic (16-channel) cables
	//std::vector<float> rxVals;
	// std::vector is giving me grief on MacOS. 
	// Allocating new float* is giving grief too but seems to use less ram is faster
#if USE_STATIC_ARRAY
	/// TODO: If we do ever want to support > the max polyphonic channels (currently 16) like by just pushing the extra values to 
	///       one or more sister ports or something, then we will want to return to this to a larger size.
	// Our static array size. Let's limit it to the number channels a polyphonic cable can have.
	const int rxValsSize = TROWA_OSCCV_VECTOR_MAX_SIZE;
	// Use static array for the polyphonic 16-channels of data.
	float rxVals[TROWA_OSCCV_VECTOR_MAX_SIZE] = {0};
	//	float rxVals[TROWA_OSCCV_VAL_BUFFER_SIZE] = {0};	
#else
	// Dynamic array of values.
	float* rxVals = NULL; 
#endif	
	// How many elements we have in rxVals.
	int rxLength = 0;

	TSOSCCVSimpleMessage()
	{
		channelNum = -1;
		rxLength = 0;
		return;
	}
		
	TSOSCCVSimpleMessage(int chNum, std::vector<float>& vals)
	{
#if DEBUG_MAC_OS_POINTER
		DEBUG("*Construct* %d", this);
#endif		
		SetValues(chNum, vals);
		return;
	}

	TSOSCCVSimpleMessage(int chNum, float recvVal)
	{
		SetValues(chNum, recvVal);
		return;
	}
	
	~TSOSCCVSimpleMessage()
	{
#if DEBUG_MAC_OS_POINTER
		DEBUG("*DESTROY* %d", this);		
#endif
		//rxVals.clear();
#if !USE_STATIC_ARRAY		
		if (rxVals != NULL)
		{
			delete[] rxVals;
			rxVals = NULL;	
			rxLength = 0;			
		}
#endif		
		return;
	}
		
	TSOSCCVSimpleMessage(TSOSCCVSimpleMessage const &cpy)
	{
		// NOTES: Adding copy constructor didn't help on MAC OS (still get memory error).
		SetValues(cpy.channelNum, cpy.rxVals, cpy.rxLength);
		return;
	}
	TSOSCCVSimpleMessage & operator=(TSOSCCVSimpleMessage const &rhs)
	{
		if (this == &rhs)
		{
			return *this;
		}
		SetValues(rhs.channelNum, rhs.rxVals, rhs.rxLength);
		return *this;
	}
	
	void SetBuffer(int size)
	{
#if USE_STATIC_ARRAY
		if (size > rxValsSize)
			rxLength = rxValsSize; // only copy this many values
		else
			rxLength = size;
#else
		if (rxVals != NULL && rxLength < size)
		{
			// Resize our array. 
			// Delete current (throw away old values)
			delete[] rxVals;
			rxVals = NULL;
		}
		if (rxVals == NULL)
		{
			rxVals = new float[size];
		}
		rxLength = size;
#endif
		return;
	}
	
	void SetValues(int chNum, const float* vals, int size)
	{
		this->channelNum = chNum;
		SetBuffer(size);
		int numToCopy = (size < rxLength) ? size : rxLength;
		for (int i = 0; i < numToCopy; i++)
		{
			rxVals[i] = vals[i];
		}		
		return;
	}
	
	void SetValues(int chNum, float vals[TROWA_OSCCV_VECTOR_MAX_SIZE], int size)
	{
		this->channelNum = chNum;
		SetBuffer(size);
		int numToCopy = (size < rxLength) ? size : rxLength;
		for (int i = 0; i < numToCopy; i++)
		{
			rxVals[i] = vals[i];
		}		
		return;
	}	
	
	void SetValues(int chNum, const std::vector<float>& vals)
	{
		this->channelNum = chNum;
		int size = static_cast<int>(vals.size());
		SetBuffer(size);		
#if DEBUG_MAC_OS_POINTER
		DEBUG("Ch %d, Vals are size: %lu. RxLength now %d.", chNum, vals.size(), rxLength);
#endif
		int numToCopy = (size < rxLength) ? size : rxLength;
		for (int i = 0; i < numToCopy; i++)
		{
			rxVals[i] = vals[i];
		}		
		return;
	}
	void SetValues(int chNum, float val)
	{
		this->channelNum = chNum;
		SetBuffer(1);
		rxVals[0] = val;
		return;
	}	
};

//=== Expander ===
// Not needed anymore since I realize there is a 1 sample time delay...
enum TSOSCCVExpanderDirection 
{
	Unknown,
	Master,
	Input,
	Output
};
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Message to pass to/from expanders.-- NOT USED. Expander messaging has 1 sample delay.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSOSCCVExpanderMessage 
{
	int moduleId = 0;
	int numChannels = TROWA_OSCCV_DEFAULT_NUM_CHANNELS;
	TSOSCCVExpanderDirection direction;
	int expansionNum;
	std::vector<TSOSCCVChannel> expansionChannels;
};


#endif // endif !TSOSCCV_COMMON_HPP