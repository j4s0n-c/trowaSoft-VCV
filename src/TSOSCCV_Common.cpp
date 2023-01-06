#include "TSOSCCV_Common.hpp"

// Available options for send frequency (Hz).
// Add in some frequencies to match up with FPS.
const int TROWA_OSCCV_Send_Freq_Opts_Hz[TROWA_OSCCV_NUM_SEND_HZ_OPTS] = { 100, 120, 240, 300, 500, 1000 };

//--------------------------------------------------------
// addValToBuffer()
// Add a value to the buffer.
// @buffVal : (IN) The value to possibly add.
//--------------------------------------------------------
void TSOSCCVChannel::addValToBuffer(float buffVal)
{
	if (!storeHistory)
		return;
	float deltaTime = powf(2.0, -12.0);
	int frameCount = (int)ceilf(deltaTime * APP->engine->getSampleRate());
	// Add frame to buffer
	if (valBuffIx < TROWA_OSCCV_VAL_BUFFER_SIZE) {
		if (++frameIx > frameCount) {
			frameIx = 0;
			valBuffer[valBuffIx++] = buffVal;
		}
	}
	else {
		frameIx++;
		const float holdTime = 0.1;
		if (frameIx >= APP->engine->getSampleRate() * holdTime) {
			valBuffIx = 0;
			frameIx = 0;
		}
	}
	return;
} // end addValToBuffer()

//--------------------------------------------------------
// serialize()
// @returns : The channel json node.
//--------------------------------------------------------
json_t* TSOSCCVChannel::serialize()
{
	json_t* channelJ = json_object();
	json_object_set_new(channelJ, "chNum", json_integer(channelNum));	
	json_object_set_new(channelJ, "path", json_string(getPath().c_str()));
	json_object_set_new(channelJ, "dataType", json_integer(dataType));
	json_object_set_new(channelJ, "convertVals", json_integer(convertVals));
	json_object_set_new(channelJ, "clipVals", json_integer(clipVals));	
	json_object_set_new(channelJ, "minV", json_real(minVoltage));
	json_object_set_new(channelJ, "maxV", json_real(maxVoltage));
	json_object_set_new(channelJ, "minOSC", json_real(minOscVal));
	json_object_set_new(channelJ, "maxOSC", json_real(maxOscVal));
	return channelJ;
} // end serialize()
//--------------------------------------------------------
// deserialize()
// @rootJ : (IN) The channel json node.
//--------------------------------------------------------
void TSOSCCVChannel::deserialize(json_t* rootJ) {
	json_t* currJ = NULL;
	if (rootJ) {
		currJ = json_object_get(rootJ, "chNum");
		if (currJ)
			channelNum = json_integer_value(currJ);				
		currJ = json_object_get(rootJ, "path");
		if (currJ)
			setPath(json_string_value(currJ));		
		currJ = json_object_get(rootJ, "dataType");
		if (currJ)
			dataType = static_cast<TSOSCCVChannel::ArgDataType>(json_integer_value(currJ));
		currJ = json_object_get(rootJ, "convertVals");
		if (currJ)
			convertVals = static_cast<bool>(json_integer_value(currJ));
		currJ = json_object_get(rootJ, "clipVals");
		if (currJ)
			clipVals = static_cast<bool>(json_integer_value(currJ));		
		currJ = json_object_get(rootJ, "minV");
		if (currJ)
			minVoltage = json_number_value(currJ);
		currJ = json_object_get(rootJ, "maxV");
		if (currJ)
			maxVoltage = json_number_value(currJ);
		currJ = json_object_get(rootJ, "minOSC");
		if (currJ)
			minOscVal = json_number_value(currJ);
		currJ = json_object_get(rootJ, "maxOSC");
		if (currJ)
			maxOscVal = json_number_value(currJ);
	}
	return;
} // end deserialize()

//--------------------------------------------------------
// serialize()
// @returns : The channel json node.
//--------------------------------------------------------
json_t* TSOSCCVInputChannel::serialize()
{
	json_t* channelJ = this->TSOSCCVChannel::serialize();
	json_object_set_new(channelJ, "channelSensitivity", json_real(channelSensitivity));
	return channelJ;
} // end serialize()
//--------------------------------------------------------
// deserialize()
// @rootJ : (IN) The channel json node.
//--------------------------------------------------------
void TSOSCCVInputChannel::deserialize(json_t* rootJ) {
	if (rootJ) {
		this->TSOSCCVChannel::deserialize(rootJ);		
		json_t* currJ = NULL;
		currJ = json_object_get(rootJ, "channelSensitivity");
		if (currJ)
			channelSensitivity = json_number_value(currJ);
	}
	return;
} // end deserialize()