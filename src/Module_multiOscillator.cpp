#include "Module_multiOscillator.hpp"
#include "math.hpp"
#include "Widget_multiOscillator.hpp"
#include "TSParamQuantity.hpp"


// Model for trowa multiOscillator
Model* modelMultiOscillator = createModel<multiOscillator, multiOscillatorWidget>(/*slug*/ "multiWave");


const char* multiOscillator::WaveFormAbbr[WaveFormType::NUM_WAVEFORMS] = { "SIN", "TRI", "SAW", "SQR" };

// Conversion:
float multiOscillator::KnobVoltage2Frequency(float knobVal) {
#if TROWA_MOSC_FREQ_KNOB_NEEDS_CONVERSION
	return rescale(knobVal, TROWA_MOSC_F_KNOB_MIN_V, TROWA_MOSC_F_KNOB_MAX_V, MOSC_FREQ_MIN_HZ, MOSC_FREQ_MAX_HZ);
#else
	return knobVal;
#endif
}
float multiOscillator::Frequency2KnobVoltage(float frequency_Hz) {
#if TROWA_MOSC_FREQ_KNOB_NEEDS_CONVERSION
	return rescale(frequency_Hz, MOSC_FREQ_MIN_HZ, MOSC_FREQ_MAX_HZ, TROWA_MOSC_F_KNOB_MIN_V, TROWA_MOSC_F_KNOB_MAX_V);
#else
	return frequency_Hz;
#endif
}
float multiOscillator::KnobVoltage2PhaseShift(float knobVal) {
	return rescale(knobVal, TROWA_MOSC_KNOB_MIN_V, TROWA_MOSC_KNOB_MAX_V, MOSC_PHASE_SHIFT_MIN_DEG, MOSC_PHASE_SHIFT_MAX_DEG);
}
float multiOscillator::PhaseShift2KnobVoltage(float phi_deg) {
	return rescale(phi_deg, MOSC_PHASE_SHIFT_MIN_DEG, MOSC_PHASE_SHIFT_MAX_DEG, TROWA_MOSC_KNOB_MIN_V, TROWA_MOSC_KNOB_MAX_V);
}


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//::::::::::::: TS_Oscillator :::::::::::::::::::::::::::::
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-

//--------------------------------------------------------
// TS_Oscillator()
// @numOutWaveForms: (IN) The number of output waveforms we will have from this oscillator.
//--------------------------------------------------------
TS_Oscillator::TS_Oscillator(int numOutWaveForms)
{
	numOutputWaveForms = numOutWaveForms;
	if (numOutputWaveForms < 1)
		numOutputWaveForms = 1;
	for (int i = 0; i < numOutputWaveForms; i++)
	{		
		outputWaveforms.push_back(TS_OscillatorOutput());
		outputWaveforms[i].outputChannelNumber = i + 1;
	}
	initialize();
	return;
}
//--------------------------------------------------------
// initialize()
// Initialize (UI) values to default values.
//--------------------------------------------------------
void TS_Oscillator::initialize()
{
	ui_amplitude_V = MOSC_AMPLITUDE_DEFAULT_V;
	ui_frequency_Hz = MOSC_FREQ_DEFAULT_HZ;
	ui_phaseShift_deg = MOSC_PHASE_SHIFT_DEFAULT_DEG;
	ui_offset_V = MOSC_OFFSET_DEFAULT_V;

	for (int i = 0; i < static_cast<int>(outputWaveforms.size()); i++)
	{
		outputWaveforms[i].initialize();
	}

	return;
} // end initialize()
//--------------------------------------------------------
// serialize()
// @returns : The TS_Oscillator json node.
//--------------------------------------------------------
json_t* TS_Oscillator::serialize()
{
	json_t* rootJ = json_object();
	json_object_set_new(rootJ, "amplitude_V", json_real(ui_amplitude_V));
	json_object_set_new(rootJ, "frequency_Hz", json_real(ui_frequency_Hz));
	json_object_set_new(rootJ, "phaseShift_deg", json_real(ui_phaseShift_deg));
	json_object_set_new(rootJ, "offset_V", json_real(ui_offset_V));
	json_object_set_new(rootJ, "numWaveforms", json_integer(outputWaveforms.size()));
	json_t* waveformsJ = json_array();
	for (int i = 0; i < static_cast<int>(outputWaveforms.size()); i++)
	{
		json_array_append_new(waveformsJ, outputWaveforms[i].serialize());
	}
	json_object_set_new(rootJ, "waveforms", waveformsJ);

	return rootJ;	
} // end serialize()
//--------------------------------------------------------
// deserialize()
// @rootJ : (IN) The TS_Oscillator json node.
//--------------------------------------------------------
void TS_Oscillator::deserialize(json_t* rootJ)
{
	json_t* currJ = NULL;
	if (rootJ) {
		currJ = json_object_get(rootJ, "amplitude_V");
		if (currJ)
			ui_amplitude_V = json_number_value(currJ);
		currJ = json_object_get(rootJ, "frequency_Hz");
		if (currJ)
			ui_frequency_Hz = json_number_value(currJ);
		currJ = json_object_get(rootJ, "phaseShift_deg");
		if (currJ)
			ui_phaseShift_deg = json_number_value(currJ);
		currJ = json_object_get(rootJ, "offset_V");
		if (currJ)
			ui_offset_V = json_number_value(currJ);
		currJ = json_object_get(rootJ, "numWaveforms");
		if (currJ)
			numOutputWaveForms = json_integer_value(currJ);
		if (numOutputWaveForms > static_cast<int>(outputWaveforms.size()))
			numOutputWaveForms = static_cast<int>(outputWaveforms.size());
		json_t* waveformsJ = json_object_get(rootJ, "waveforms");
		for (int i = 0; i < numOutputWaveForms; i++)
		{
			currJ = json_array_get(waveformsJ, i);
			if (currJ)
			{
				outputWaveforms[i].deserialize(currJ);
			}
		}
	}
} // end deserialize()

//--------------------------------------------------------
// setPhaseShift_deg()
// @deg : (IN) The phase shift in degrees.
//--------------------------------------------------------
void TS_Oscillator::setPhaseShift_deg(float deg)
{
	phaseShift_deg = deg;
	phaseShift_norm = deg / 360.0f;
	return;
}

//--------------------------------------------------------
// calculatePhase()
// @dt : (IN) Time elapsed.
// @doSync : (IN) If sync / reset requested.
// @returns: True if shifted phase has reset (gone over 1)
//--------------------------------------------------------
bool TS_Oscillator::calculatePhase(float dt, bool doSync)
{
	bool waveReset = doSync;
	if (doSync)
	{
		phase = 0;
	}
	else
	{
		// VCO clamps this, not sure if we need to
		float dPhase = clamp(frequency_Hz * dt, 0.f, 0.5f);
		phase = eucMod(phase + dPhase, 1.0f);
	}
	float prevSPhi = shiftedPhase;
	shiftedPhase = eucMod(phase + phaseShift_norm, 1.0f);
	if (!waveReset)
		waveReset = prevSPhi > shiftedPhase;
	return waveReset;
}

//--------------------------------------------------------
// calcSin()
// Sine wave.
// Calculate with amplitude and offset.
// @phaseShift_n : (IN) Normalized phase shift (-1 to 1).
//--------------------------------------------------------
float TS_Oscillator::calcSin(float phaseShift_n)
{
	return amplitude_V * sinf(eucMod(1.0f + shiftedPhase + phaseShift_n, 1.0f) * 2.0f * NVG_PI) + offset_V;
} // end calcSin()
//--------------------------------------------------------
// calcRect()
// Rectangle wave.
// Calculate with amplitude and offset.
// @phaseShift_n : (IN) Normalized phase shift (-1 to 1).
// @pulseWidth_n : (IN) Normalized pulse width (0-1). Really should be like 0.01 to 0.99 or something.
//--------------------------------------------------------
float TS_Oscillator::calcRect(float phaseShift_n, float pulseWidth_n)
{
	float val = 0.0f;
	if (eucMod(1.0f + shiftedPhase + phaseShift_n, 1.0f) < pulseWidth_n)
		val = amplitude_V;
	else
		val = -amplitude_V;
	return val + offset_V;
} // end calcRect
//--------------------------------------------------------
// calcTri()
// Triange wave.
// Calculate with amplitude and offset.
// @phaseShift_n : (IN) Normalized phase shift (-1 to 1).
//--------------------------------------------------------
float TS_Oscillator::calcTri(float phaseShift_n)
{
	float p_n = eucMod(1.0f + shiftedPhase + phaseShift_n, 1.0f);
	float val = 0.0f;
	if (p_n < 0.25f)
		val = 4.0f * p_n; // 0 to 1 (positive slope)
	else if (p_n < 0.75f)
		val = 2.0f - 4.0f * p_n; // -1 to 0 (positive slope)
	else
		val = -4.0f + 4.f * p_n;
	return amplitude_V * val + offset_V;
} // end calcTri()
//--------------------------------------------------------
// calcSaw()
// Sawtooth wave.
// Calculate with amplitude and offset. (centered around 0)
// @phaseShift_n : (IN) Normalized phase shift (-1 to 1).
// @posRamp: (IN) True for positive ramp, false for negative ramp.
//--------------------------------------------------------
float TS_Oscillator::calcSaw(float phaseShift_n, bool posRamp)
{
	float p_n = eucMod(1.0f + shiftedPhase + phaseShift_n, 1.0f);
	float val = 0.0f;
	float a_v = 2 * amplitude_V;
	if (posRamp)
		val = -amplitude_V + a_v * p_n; // Going up /|/|/|
	else
		val = amplitude_V - a_v * p_n; // Going down \|\|\|
	return val + offset_V;
} // end calcSaw()

//--------------------------------------------------------
// calcSin()
// Calculates A*sin(wt + phi) + C.
// A = amplitude_V, w = frequency_radiansps, phi = phaseShift_radians, C = offset_V.
//--------------------------------------------------------
float TS_Oscillator::calcSin()
{
	return amplitude_V * sinf(shiftedPhase * 2.0f * NVG_PI) + offset_V;
}
//--------------------------------------------------------
// calcSquare()
// Pseudo calculates A*SIGN(sin(wt + phi)) + C.
// A = amplitude_V, w = frequency_radiansps, phi = phaseShift_radians, C = offset_V.
// SIGN() = +1 for positive, -1 for negative, 0 for 0.
//--------------------------------------------------------
float TS_Oscillator::calcSquare()
{
	float val = 0.0f;
	if (shiftedPhase < 0.5f)
		val = amplitude_V;
	else
		val = -amplitude_V;
	return val + offset_V;
}
//--------------------------------------------------------
// calcTri()
// Pseudo calculates A*SIGN(sin(wt + phi)) + C.
// A = amplitude_V, w = frequency_radiansps, phi = phaseShift_radians, C = offset_V.
// SIGN() = +1 for positive, -1 for negative, 0 for 0.
//--------------------------------------------------------
float TS_Oscillator::calcTri()
{
	float val = 0.0f;
	if (shiftedPhase < 0.25f)
		val = 4.0f * shiftedPhase; // 0 to 1 (positive slope)
	else if (shiftedPhase < 0.75f)
		val = 2.0f - 4.0f * shiftedPhase; // -1 to 0 (positive slope)
	else
		val = -4.0f + 4.f * shiftedPhase;
	return amplitude_V * val + offset_V;
}
//--------------------------------------------------------
// calcSaw()
// Sawtooth wave, positive ramp.
//--------------------------------------------------------
float TS_Oscillator::calcSaw()
{
	float val = 0.0f;
	if (shiftedPhase < 0.5f)
		val = 2.f * shiftedPhase;
	else
		val = -2.f + 2.f * shiftedPhase;
	return amplitude_V * val + offset_V;
}


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//::::::::::::: multiOscillator :::::::::::::::::::::::::::
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiOscillator()
// Create a module with numOscillators oscillators.
// @numOscillators: (IN) Number of oscillators
// @numOscillatorOutputs: (IN) Number of oscillators output signals (channels per oscillator).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
multiOscillator::multiOscillator(int numOscillators, int numOscillatorOutputs) : Module()
{
	config(NUM_PARAMS + numOscillators * (TS_Oscillator::OSCWF_NUM_PARAMS + numOscillatorOutputs * TS_OscillatorOutput::OUT_NUM_PARAMS),
		   NUM_INPUTS + numOscillators * (TS_Oscillator::OSCWF_NUM_INPUTS + numOscillatorOutputs * TS_OscillatorOutput::OUT_NUM_INPUTS),
		   NUM_OUTPUTS + numOscillators * (TS_Oscillator::OSCWF_NUM_OUTPUTS + numOscillatorOutputs * TS_OscillatorOutput::OUT_NUM_OUTPUTS),
		   NUM_LIGHTS + numOscillators * (TS_Oscillator::OSCWF_NUM_LIGHTS + numOscillatorOutputs * TS_OscillatorOutput::BaseLightIds::OUT_NUM_LIGHTS));
		   
	this->numberOscillators = numOscillators;
	this->oscillators = new TS_Oscillator[numberOscillators];
	this->numOscillatorOutputs = numOscillatorOutputs;
	this->isFirstRun = true;

	//--------------------------
	// * Configure Parameters *
	//--------------------------	
	// [v1.0] Parameters must be configured here now instead of in Widget objects...
	// Global: 
	//configParam(/*paramId*/ , /*minVal*/, /*maxVal*/, /*defVal*/, /*label*/, /*unit*/, /*displayBase*/, /*displayMultiplier*/, /*displayOffset*/)
	configParam( /*id*/ ParamIds::SYNC_PARAM, /*min*/ 0, /*max*/ 10, /*def*/ 0); // Currently not really used
	
	// Each oscillator:
	// // A_V: Amplitude (Volts). Should not be = 0 (pointless) and max +/-12 V.
	// OSCWF_AMPLITUDE_PARAM,
	// // f_Hz: Frequency (Hz)
	// OSCWF_FREQUENCY_PARAM,
	// // Phi_degrees: Phase Shift (degrees) [0-360]
	// OSCWF_PHASE_SHIFT_PARAM,
	// // y0_V : Offset (Volts). +/- 10 V?
	// OSCWF_OFFSET_PARAM,
	// // Sync/Restart waveform.
	// OSCWF_SYNC_PARAM,	
	const int numKnobVals = 5;
	// MinV, MaxV, DefV, DisplayBase, DisplayMult, DisplayOffset	
	float knobVals[][6] = {
		{ MOSC_AMPLITUDE_MIN_V, MOSC_AMPLITUDE_MAX_V, MOSC_AMPLITUDE_DEFAULT_V, /*displayBase*/ 0, /*displayMult*/ 1, /*displayOffset*/ 0 }, // Amplitude
		{ TROWA_MOSC_F_KNOB_MIN_V, TROWA_MOSC_F_KNOB_MAX_V, Frequency2KnobVoltage(MOSC_FREQ_DEFAULT_HZ), /*displayBase*/ 0, /*displayMult*/ 1, /*displayOffset*/ 0 }, // Frequency
		{ TROWA_MOSC_KNOB_MIN_V, TROWA_MOSC_KNOB_MAX_V, PhaseShift2KnobVoltage(MOSC_PHASE_SHIFT_DEFAULT_DEG),
			/*displayBase*/ 0, 
			/*displayMult*/ (MOSC_PHASE_SHIFT_MAX_DEG - MOSC_PHASE_SHIFT_MIN_DEG)/(TROWA_MOSC_KNOB_MAX_V - TROWA_MOSC_KNOB_MIN_V), // 36
			/*displayOffset*/ 0  }, // Phi
		{ MOSC_OFFSET_MIN_V, MOSC_OFFSET_MAX_V, MOSC_OFFSET_DEFAULT_V, /*displayBase*/ 0, /*displayMult*/ 1, /*displayOffset*/ 0 }, // Offset
		{ 0, 1, 0, /*displayBase*/ 0, /*displayMult*/ 1, /*displayOffset*/ 0} // Sync
	};
	const char* knobLabels[numKnobVals] = { "Amplitude", "Frequency", "Phase Shift", "Offset", "Sync" };
	const char* knobUnits[numKnobVals] = { "V", "Hz", "°", "V", "" };
		
	// Each oscillator output:
	// // What type of oscillator (SIN, SQU, TRI, SAW). [Voltage Range: +/- 10V]
	// OUT_OSC_TYPE_PARAM,
	// // Secondary parameter (i.e. Pulse width for Rectangle, Ramp slope sign for Saw).
	// OUT_AUX_PARAM,
	// // Phi_degrees: Phase Shift (degrees) [-360 to 360].
	// OUT_PHASE_SHIFT_PARAM,
	// // Mix of the raw with the AM signal. 1.0 is all AM; 0.0 is all raw.
	// OUT_AM_MIX_PARAM,
	// // Toggle (Digital or Ring Modulation)
	// OUT_AM_TYPE_PARAM,
	// // Number of params for an oscillator.
	// OUT_NUM_PARAMS	
	const int outputNumParamVals = 5;
	// MinV, MaxV, DefV, DisplayBase, DisplayMult, DisplayOffset
	// WaveFormType : Enumeration
	float outputParamVals[][6] = {
		{ 0, WaveFormType::NUM_WAVEFORMS -1, 0, /*displayBase*/ 0, /*displayMult*/ 1, /*displayOffset*/ 0 }, // Waveform Type TROWA_MOSC_KNOB_MIN_V, TROWA_MOSC_KNOB_MAX_V,  TROWA_MOSC_KNOB_MIN_V
		{ TROWA_MOSC_KNOB_AUX_MIN_V, TROWA_MOSC_KNOB_AUX_MAX_V, 0.5f*(TROWA_MOSC_KNOB_AUX_MIN_V+ TROWA_MOSC_KNOB_AUX_MAX_V), 
			/*displayBase*/ 0, /*displayMult*/ 100.f/TROWA_MOSC_KNOB_AUX_MAX_V, /*displayOffset*/ 0}, // Aux
		{ TROWA_MOSC_KNOB_MIN_V, TROWA_MOSC_KNOB_MAX_V, PhaseShift2KnobVoltage(MOSC_PHASE_SHIFT_DEFAULT_DEG),
			/*displayBase*/ 0, 
			/*displayMult*/ (MOSC_PHASE_SHIFT_MAX_DEG - MOSC_PHASE_SHIFT_MIN_DEG)/(TROWA_MOSC_KNOB_MAX_V - TROWA_MOSC_KNOB_MIN_V), // 36
			/*displayOffset*/ 0 }, // Phi
		{ TROWA_MOSC_MIX_MIN_V, TROWA_MOSC_MIX_MAX_V,  TROWA_MOSC_MIX_DEF_V, /*displayBase*/ 0, /*displayMult*/ 100.f, /*displayOffset*/ 0  }, // Mod Mix 
		{ 0, 1, 0, /*displayBase*/ 0, /*displayMult*/ 1, /*displayOffset*/ 0} // OUT_AM_TYPE_PARAM (button)
	};
	const char* outputParamLabels[outputNumParamVals] = { "Waveform", "Aux", "Phase Shift", "Mod Mix", "Dig/Ring" };
	const char* outputParamUnits[outputNumParamVals] = { "", " N/A", "°", "%", "" };
	
		
	int inputId = InputIds::OSC_INPUT_START;
	int outputId = OutputIds::OSC_OUTPUT_START;
	for (int i = 0; i < numOscillators; i++)
	{
		//--------------------------
		// * Initialize objects *
		//--------------------------
		oscillators[i] = TS_Oscillator(numOscillatorOutputs);
		//---------------------------
		// * Initialize parameters *
		//---------------------------		
		// Oscillator parameters:
		int baseParamId = multiOscillator::ParamIds::OSC_PARAM_START + i * (TS_Oscillator::BaseParamIds::OSCWF_NUM_PARAMS + numOscillatorOutputs * TS_OscillatorOutput::BaseParamIds::OUT_NUM_PARAMS);
		for (int paramId = 0; paramId < numKnobVals; paramId++)
		{
			//configParam(/*paramId*/ , /*minVal*/, /*maxVal*/, /*defVal*/, /*label*/, /*unit*/, /*displayBase*/, /*displayMultiplier*/, /*displayOffset*/)
			configParam( /*id*/ baseParamId + paramId, /*min*/ knobVals[paramId][0], /*max*/ knobVals[paramId][1], /*def*/ knobVals[paramId][2],
				/*label*/ knobLabels[paramId], /*unit*/ knobUnits[paramId], /*displayBase*/ knobVals[paramId][3], /*displayMult*/ knobVals[paramId][4], /*displayOffset*/ knobVals[paramId][5]);
		}
		// Oscillator Output parameters:
		for (int j = 0; j < numOscillatorOutputs; j++)
		{
			int bParamId = baseParamId + TS_Oscillator::BaseParamIds::OSCWF_NUM_PARAMS + j*TS_OscillatorOutput::BaseParamIds::OUT_NUM_PARAMS;
			// First one is waveform type
			configParam<TS_ParamQuantityEnum>(bParamId, /*min*/ outputParamVals[0][0], /*max*/ outputParamVals[0][1], /*def*/ outputParamVals[0][2], 
					/*label*/ outputParamLabels[0], /*unit*/ outputParamUnits[0], /*displayBase*/ outputParamVals[0][3], /*displayMult*/ outputParamVals[0][4], /*displayOffset*/ outputParamVals[0][5]);
			dynamic_cast<TS_ParamQuantityEnum*>(this->paramQuantities[bParamId])->valMult = 1;
			dynamic_cast<TS_ParamQuantityEnum*>(this->paramQuantities[bParamId])->snapEnabled = true;
			for (int w = 0; w < WaveFormType::NUM_WAVEFORMS; w++)
			{
				dynamic_cast<TS_ParamQuantityEnum*>(this->paramQuantities[bParamId])->addToEnumMap(w, WaveFormAbbr[w]);				
			}

			for (int paramId = 1; paramId < outputNumParamVals - 1; paramId++)
			{
				configParam( /*id*/ bParamId + paramId, /*min*/ outputParamVals[paramId][0], /*max*/ outputParamVals[paramId][1], /*def*/ outputParamVals[paramId][2],
					/*label*/ outputParamLabels[paramId], /*unit*/ outputParamUnits[paramId], /*displayBase*/ outputParamVals[paramId][3], /*displayMult*/ outputParamVals[paramId][4], /*displayOffset*/ outputParamVals[paramId][5]);
			}
			
			// Dig/Ring
			configButton(bParamId + outputNumParamVals - 1, "Dig/Ring");
		} // end loop through an oscillator's output
		
		//----------------------------------
		// * Initialize Inputs & Outputs *
		//----------------------------------
		// OSCILLATOR 1 ::::::::::::::::::::::::::::::::::::::::::::
		std::string prefix = "Osc[" + std::to_string(i + 1) + "] ";		
		//TS_Oscillator
		// // A_V: Amplitude (Volts). [Voltage Range: +/-12 V]
		// OSCWF_AMPLITUDE_INPUT,
		// // f_Hz: Frequency (Hz). [Voltage Range: +/- 10V]
		// OSCWF_FREQUENCY_INPUT,
		// // Phi_degrees: Phase Shift (degrees) [0-360].  [Voltage Range: +/- 10V]
		// OSCWF_PHASE_SHIFT_INPUT,
		// // y0_V : Offset (Volts).  [Voltage Range: +/- 10V]
		// OSCWF_OFFSET_INPUT,
		// // Sync/Restart waveform.
		// OSCWF_SYNC_INPUT,
		// // Frequency Modulator Input
		// OSCWF_FM_INPUT,
		configInput(inputId++, prefix + "Amplitude");	// 2
		configInput(inputId++, prefix + "Frequency");	// 3
		configInput(inputId++, prefix + "Phase Shift");	// 4
		configInput(inputId++, prefix + "Offset");		// 5
		configInput(inputId++, prefix + "Sync");		// 6
		configInput(inputId++, prefix + "Frequency Mod"); // 7
		
		// // Any time sync happens or oscillator is at 0 phase.
		// OSCWF_SYNC_OUTPUT,
		configOutput(outputId++, prefix + "Sync");	// 2
			
		// OUTPUT X1 ::::::::::::::::::::::::::::::::::::::::::::::::
		prefix = "Osc[" + std::to_string(i + 1) + "] X1 ";
		//TS_OscillatorOutput::BaseInputIds
		// // Oscillator type (SIN, SQU, TRI, SAW). [Voltage Range: +/- 5V].
		// OUT_OSC_TYPE_INPUT,
		// // Secondary input (i.e. Pulse width for Rectangle, Ramp slope sign for Saw).
		// OUT_AUX_INPUT,
		// // Phi_degrees: Phase Shift (degrees) [-360 to 360].  [Voltage Range: +/- 10V]
		// OUT_PHASE_SHIFT_INPUT,
		// // Amplitude modulation
		// OUT_AM_INPUT,		
		configInput(inputId++, prefix + "Waveform Type"); 	// 8
		configInput(inputId++, prefix + "Auxillary");		// 9
		configInput(inputId++, prefix + "Phase Shift");		// 10
		configInput(inputId++, prefix + "Amplitude Mod");	// 11
		
		// // Raw output.
		// OUT_RAW_SIGNAL,
		// // After amplitude modulation.
		// OUT_MULTIPLIED_SIGNAL,
		configOutput(outputId++, prefix + "Raw");	// 3
		configOutput(outputId++, prefix + "Mod");	// 4
		
		// OUTPUT Y2 ::::::::::::::::::::::::::::::::::::::::::::::::
		prefix = "Osc[" + std::to_string(i + 1) + "] Y2 ";
		//TS_OscillatorOutput::BaseInputIds
		// // Oscillator type (SIN, SQU, TRI, SAW). [Voltage Range: +/- 5V].
		// OUT_OSC_TYPE_INPUT,
		// // Secondary input (i.e. Pulse width for Rectangle, Ramp slope sign for Saw).
		// OUT_AUX_INPUT,
		// // Phi_degrees: Phase Shift (degrees) [-360 to 360].  [Voltage Range: +/- 10V]
		// OUT_PHASE_SHIFT_INPUT,
		// // Amplitude modulation
		// OUT_AM_INPUT,		
		configInput(inputId++, prefix + "Waveform Type");	// 12
		configInput(inputId++, prefix + "Auxillary");		// 13
		configInput(inputId++, prefix + "Phase Shift");		// 14
		configInput(inputId++, prefix + "Amplitude Mod");	// 15
		
		
		// // Raw output.
		// OUT_RAW_SIGNAL,
		// // After amplitude modulation.
		// OUT_MULTIPLIED_SIGNAL,
		configOutput(outputId++, prefix + "Raw");	// 5
		configOutput(outputId++, prefix + "Mod");	// 6
		
	} // end loop through oscillators
	
	//--------------------------
	// * Initialize objects *
	//--------------------------	
	initializeOscillators();	
	return;
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Clean up our ram.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
multiOscillator::~multiOscillator()
{
	isInitialized = false;
	if (oscillators != NULL)
		delete[] oscillators;
	oscillators = NULL;
	return;
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// reset(void)
// Initialize values.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void multiOscillator::onReset()
{
	for (int i = 0; i < numberOscillators; i++)
	{
		oscillators[i].initialize();
	}
	return;
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// dataToJson(void)
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
json_t *multiOscillator::dataToJson()
{
	json_t* rootJ = json_object();
	// version
	json_object_set_new(rootJ, "version", json_integer(TROWA_INTERNAL_VERSION_INT));
	json_object_set_new(rootJ, "numOsc", json_integer(numberOscillators));
	json_object_set_new(rootJ, "numOutputs", json_integer(numOscillatorOutputs));

	json_t* oscillatorsJ = json_array();
	for (int i = 0; i < numberOscillators; i++)
	{
		// Input
		json_array_append_new(oscillatorsJ, oscillators[i].serialize());
	}
	json_object_set_new(rootJ, "oscillators", oscillatorsJ);
	return rootJ;
} // end dataToJson()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// dataFromJson(void)
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void multiOscillator::dataFromJson(json_t *rootJ)
{
	json_t* currJ = NULL;
	int nOscillators = numberOscillators;
	currJ = json_object_get(rootJ, "numOsc");
	if (currJ)
	{
		nOscillators = json_integer_value(currJ);
		if (nOscillators > numberOscillators)
			nOscillators = numberOscillators;
	}
	currJ = json_object_get(rootJ, "numOutputs");
	if (currJ)
	{
		numOscillatorOutputs = json_integer_value(currJ);
	}

	json_t* oscillatorsJ = json_object_get(rootJ, "oscillators");
	for (int i = 0; i < nOscillators; i++)
	{
		currJ = json_array_get(oscillatorsJ, i);
		if (currJ)
		{
			oscillators[i].deserialize(currJ);
		}
	} // end loop through osccilators
	return;
} // end dataFromJson()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// process()
// (Formerly step(void)).
// Process the step.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void multiOscillator::process(const ProcessArgs &args)
{
	float dt = args.sampleTime;// engineGetSampleTime();
	bool setParameterConfigs = false;
	if (isFirstRun)
	{
		setParameterConfigs = true;
		isFirstRun = false;
	}
	
	// Get Oscillator CV and User Inputs
	for (int osc = 0; osc < numberOscillators; osc++)
	{
		bool sync = false;
		bool oscillatorReset = false; // If this oscillator is at 0 phase.
		TS_Oscillator* theOscillator = &(oscillators[osc]);
		int baseInputId = InputIds::OSC_INPUT_START + osc * (TS_Oscillator::BaseInputIds::OSCWF_NUM_INPUTS + numOscillatorOutputs * TS_OscillatorOutput::BaseInputIds::OUT_NUM_INPUTS);
		int baseParamId = ParamIds::OSC_PARAM_START + osc * (TS_Oscillator::BaseParamIds::OSCWF_NUM_PARAMS + numOscillatorOutputs * TS_OscillatorOutput::BaseParamIds::OUT_NUM_PARAMS);
		int baseOutputId = OutputIds::OSC_OUTPUT_START + osc * (TS_Oscillator::BaseOutputIds::OSCWF_NUM_OUTPUTS + numOscillatorOutputs * TS_OscillatorOutput::BaseOutputIds::OUT_NUM_OUTPUTS);
		int baseLightId = LightIds::OSC_LIGHT_START + osc * (TS_Oscillator::BaseLightIds::OSCWF_NUM_LIGHTS + numOscillatorOutputs * TS_OscillatorOutput::BaseLightIds::OUT_NUM_LIGHTS);

		//------------------------------
		// Sync this oscillator
		//------------------------------
		if (lights[baseLightId + TS_Oscillator::BaseLightIds::OSCWF_SYNC_LED].value > 0)
			lights[baseLightId + TS_Oscillator::BaseLightIds::OSCWF_SYNC_LED].value -= lights[baseLightId + TS_Oscillator::BaseLightIds::OSCWF_SYNC_LED].value / lightLambda * dt;
		else if (lights[baseLightId + TS_Oscillator::BaseLightIds::OSCWF_SYNC_LED].value < 0)
			lights[baseLightId + TS_Oscillator::BaseLightIds::OSCWF_SYNC_LED].value = 0;
		if (theOscillator->synchTrigger.process(params[baseParamId + TS_Oscillator::BaseParamIds::OSCWF_SYNC_PARAM].getValue() + inputs[baseInputId + TS_Oscillator::BaseInputIds::OSCWF_SYNC_INPUT].getVoltage()))
		{
			lights[baseLightId + TS_Oscillator::BaseLightIds::OSCWF_SYNC_LED].value = 1.0f;
			sync = true;
		} // end if

		//------------------------------
		// Values In (Add Input + Knob)
		//------------------------------
		// *> Amplitude (V):
		theOscillator->ui_amplitude_V = params[baseParamId + TS_Oscillator::BaseParamIds::OSCWF_AMPLITUDE_PARAM].getValue();
		float a = theOscillator->ui_amplitude_V;
		if (inputs[baseInputId + TS_Oscillator::BaseInputIds::OSCWF_AMPLITUDE_INPUT].isConnected())
			a += inputs[baseInputId + TS_Oscillator::BaseInputIds::OSCWF_AMPLITUDE_INPUT].getVoltage();
		theOscillator->amplitude_V = a;

		// *> Frequency (Hz):
		// User Knob:
#if TROWA_MOSC_FREQ_KNOB_NEEDS_CONVERSION
		theOscillator->ui_frequency_Hz = rescale(params[baseParamId + TS_Oscillator::BaseParamIds::OSCWF_FREQUENCY_PARAM].getValue(), TROWA_MOSC_F_KNOB_MIN_V, TROWA_MOSC_F_KNOB_MAX_V, MOSC_FREQ_MIN_HZ, MOSC_FREQ_MAX_HZ);
#else
		theOscillator->ui_frequency_Hz = params[baseParamId + TS_Oscillator::BaseParamIds::OSCWF_FREQUENCY_PARAM].getValue();
#endif
		float f = theOscillator->ui_frequency_Hz;
		// Add Input:
		if (inputs[baseInputId + TS_Oscillator::BaseInputIds::OSCWF_FREQUENCY_INPUT].isConnected()) {
			// CV frequency
			f = clamp(f + VoltageToFrequency(inputs[baseInputId + TS_Oscillator::BaseInputIds::OSCWF_FREQUENCY_INPUT].getVoltage()), MOSC_FREQ_MIN_HZ, MOSC_FREQ_MAX_HZ);
		}
		theOscillator->frequency_Hz = f; // Actual Frequency

		// *> Phase Shift (deg): 
		theOscillator->ui_phaseShift_deg = rescale(params[baseParamId + TS_Oscillator::BaseParamIds::OSCWF_PHASE_SHIFT_PARAM].getValue(), TROWA_MOSC_KNOB_MIN_V, TROWA_MOSC_KNOB_MAX_V, MOSC_PHASE_SHIFT_MIN_DEG, MOSC_PHASE_SHIFT_MAX_DEG);
		float phi = theOscillator->ui_phaseShift_deg;
		if (inputs[baseInputId + TS_Oscillator::BaseInputIds::OSCWF_PHASE_SHIFT_INPUT].isConnected()) {
			phi += rescale(inputs[baseInputId + TS_Oscillator::BaseInputIds::OSCWF_PHASE_SHIFT_INPUT].getVoltage(), TROWA_MOSC_INPUT_MIN_V, TROWA_MOSC_INPUT_MAX_V, MOSC_PHASE_SHIFT_MIN_DEG, MOSC_PHASE_SHIFT_MAX_DEG);
		}
		theOscillator->setPhaseShift_deg(phi);

		// *> Offset (V):
		theOscillator->ui_offset_V = params[baseParamId + TS_Oscillator::BaseParamIds::OSCWF_OFFSET_PARAM].getValue();
		float c = theOscillator->ui_offset_V;
		if (inputs[baseInputId + TS_Oscillator::BaseInputIds::OSCWF_OFFSET_INPUT].isConnected())
			c += inputs[baseInputId + TS_Oscillator::BaseInputIds::OSCWF_OFFSET_INPUT].getVoltage();
		theOscillator->offset_V = c;

		//------------------------------
		// Calculate phase
		//------------------------------
		oscillatorReset = theOscillator->calculatePhase(dt, sync);

		//------------------------------
		// Sync Output
		//------------------------------
		if (oscillatorReset)
		{
			theOscillator->synchPulse.trigger(1e-4); // 1e-3
		}
		outputs[baseOutputId + TS_Oscillator::BaseOutputIds::OSCWF_SYNC_OUTPUT].setVoltage((theOscillator->synchPulse.process(dt)) ? 10.0f : 0.0f);

		//------------------------------
		// Each output channel
		//------------------------------
		baseParamId += TS_Oscillator::BaseParamIds::OSCWF_NUM_PARAMS;
		baseInputId += TS_Oscillator::BaseInputIds::OSCWF_NUM_INPUTS;
		baseOutputId += TS_Oscillator::BaseOutputIds::OSCWF_NUM_OUTPUTS;
		baseLightId += TS_Oscillator::BaseLightIds::OSCWF_NUM_LIGHTS;
		for (int i = 0; i < theOscillator->numOutputWaveForms; i++)
		{
			//float type = clamp((int)rescale(params[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_OSC_TYPE_PARAM].getValue(), TROWA_MOSC_KNOB_MIN_V, TROWA_MOSC_KNOB_MAX_V, 0, WaveFormType::NUM_WAVEFORMS), 0, WaveFormType::NUM_WAVEFORMS - 1);
			// [v1.0]- No longer -10 to 10 V from knob. Now just 0 to NUM_WAVEFORMS
			WaveFormType lastType = theOscillator->outputWaveforms[i].waveFormType;
			float type = (int) params[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_OSC_TYPE_PARAM].getValue();
			theOscillator->outputWaveforms[i].ui_waveFormType = static_cast<WaveFormType>(type);
			if (inputs[baseInputId + TS_OscillatorOutput::BaseInputIds::OUT_OSC_TYPE_INPUT].isConnected()) {
				type = clamp((int)rescale(inputs[baseInputId + TS_OscillatorOutput::BaseInputIds::OUT_OSC_TYPE_INPUT].getVoltage(),
					TROWA_MOSC_TYPE_INPUT_MIN_V, TROWA_MOSC_TYPE_INPUT_MAX_V, 0, WaveFormType::NUM_WAVEFORMS), 0, WaveFormType::NUM_WAVEFORMS - 1);
			}
			theOscillator->outputWaveforms[i].waveFormType = static_cast<WaveFormType>(type);
			// Change the built-in param quantity labels for the 'Aux' based on waveform type:
			if (lastType != theOscillator->outputWaveforms[i].waveFormType || setParameterConfigs)
			{
				// [Rack v2] label is not member of ParamQuantity anymore... Is it 'name'?
				switch (theOscillator->outputWaveforms[i].waveFormType)
				{
					case WAVEFORM_SAW:					
						this->paramQuantities[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AUX_PARAM]->name = std::string("Slope");
						this->paramQuantities[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AUX_PARAM]->unit = std::string("");
						this->paramQuantities[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AUX_PARAM]->displayMultiplier = 2.f/TROWA_MOSC_KNOB_AUX_MAX_V;
						this->paramQuantities[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AUX_PARAM]->displayOffset = -1.0f; // -1 to 1
						break;
					case WAVEFORM_SQR:
						this->paramQuantities[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AUX_PARAM]->name = std::string("Pulse Width");
						this->paramQuantities[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AUX_PARAM]->unit = std::string("%");
						this->paramQuantities[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AUX_PARAM]->displayMultiplier = 100.f/TROWA_MOSC_KNOB_AUX_MAX_V;
						this->paramQuantities[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AUX_PARAM]->displayOffset = 0.0f; // 0 to 100
						break;
					case WAVEFORM_SIN:
					case WAVEFORM_TRI:
					default:
						// No AUX
						this->paramQuantities[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AUX_PARAM]->name = std::string("Aux");
						this->paramQuantities[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AUX_PARAM]->unit = std::string(" N/A");
						this->paramQuantities[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AUX_PARAM]->displayMultiplier = 100.f/TROWA_MOSC_KNOB_AUX_MAX_V;
						this->paramQuantities[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AUX_PARAM]->displayOffset = 0.0f; // 0 to 100, doesn't really matter			
						break;
				} // end switch	
			}

			// *> Phase shift for this output
			float phi = rescale(params[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_PHASE_SHIFT_PARAM].getValue(),
				TROWA_MOSC_KNOB_MIN_V, TROWA_MOSC_KNOB_MAX_V,
				MOSC_PHASE_SHIFT_MIN_DEG, MOSC_PHASE_SHIFT_MAX_DEG);
			theOscillator->outputWaveforms[i].ui_phaseShift_deg = phi;
			if (inputs[baseInputId + TS_OscillatorOutput::BaseInputIds::OUT_PHASE_SHIFT_INPUT].isConnected()) {
				phi += rescale(inputs[baseInputId + TS_OscillatorOutput::BaseInputIds::OUT_PHASE_SHIFT_INPUT].getVoltage(),
					TROWA_MOSC_INPUT_MIN_V, TROWA_MOSC_INPUT_MAX_V, MOSC_PHASE_SHIFT_MIN_DEG, MOSC_PHASE_SHIFT_MAX_DEG);
			}
			theOscillator->outputWaveforms[i].setPhaseShift_deg(phi);

			// *> Aux parameter: (Currently only rect/square and saw/ramp)
			float aux = 0.5f;
			if (inputs[baseInputId + TS_OscillatorOutput::BaseInputIds::OUT_AUX_INPUT].isConnected()) {
				aux = clamp(rescale(inputs[baseInputId + TS_OscillatorOutput::BaseInputIds::OUT_AUX_INPUT].getVoltage(), TROWA_MOSC_AUX_MIN_V, TROWA_MOSC_AUX_MAX_V, 0.f, 1.f), 0.f, 1.f);
			}
			else
			{
				aux = rescale(params[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AUX_PARAM].getValue(), TROWA_MOSC_KNOB_AUX_MIN_V, TROWA_MOSC_KNOB_AUX_MAX_V, 0.0f, 1.0f);
			}
			theOscillator->outputWaveforms[i].auxParam_norm = aux;

			// *> AM Type (digital = false, ring = true)
			if (theOscillator->outputWaveforms[i].amRingModulationTrigger.process(params[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AM_TYPE_PARAM].getValue()))
			{
				theOscillator->outputWaveforms[i].amRingModulation = !theOscillator->outputWaveforms[i].amRingModulation;
				//INFO("[Ch %d] AM Button Click id %d. Ring Mod = %d.", i + 1, baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AM_TYPE_PARAM, theOscillator->outputWaveforms[i].amRingModulation);
			}
			lights[baseLightId + TS_OscillatorOutput::BaseLightIds::OUT_AM_MODE_LED].value = (theOscillator->outputWaveforms[i].amRingModulation) ? 1.0f : 0.0f;

			//------------------------------------------
			// Calculate an output/outputs
			//------------------------------------------
			if (outputs[baseOutputId + TS_OscillatorOutput::BaseOutputIds::OUT_RAW_SIGNAL].isConnected() ||
				(outputs[baseOutputId + TS_OscillatorOutput::BaseOutputIds::OUT_MULTIPLIED_SIGNAL].isConnected() && inputs[baseInputId + TS_OscillatorOutput::BaseInputIds::OUT_AM_INPUT].isConnected()))
			{
				// Calculate the RAW output:
				float rawOutput = 0.0f;
				float phi_n = theOscillator->outputWaveforms[i].phaseShift_norm;
				switch (theOscillator->outputWaveforms[i].waveFormType)
				{
				case WaveFormType::WAVEFORM_SIN:
					rawOutput = theOscillator->calcSin(phi_n);
					break;
				case WaveFormType::WAVEFORM_TRI:
					rawOutput = theOscillator->calcTri(phi_n);
					break;
				case WaveFormType::WAVEFORM_SQR:
					rawOutput = theOscillator->calcRect(phi_n, theOscillator->outputWaveforms[i].auxParam_norm);
					break;
				case WaveFormType::WAVEFORM_SAW:
					rawOutput = theOscillator->calcSaw(phi_n, theOscillator->outputWaveforms[i].getRampSlope());
					break;
				default:
					break;
				}
				outputs[baseOutputId + TS_OscillatorOutput::BaseOutputIds::OUT_RAW_SIGNAL].setVoltage(rawOutput);

				// Calculate AM output:
				if (outputs[baseOutputId + TS_OscillatorOutput::BaseOutputIds::OUT_MULTIPLIED_SIGNAL].isConnected())
				{
					float modOutput = 0.f;
					float modulator = inputs[baseInputId + TS_OscillatorOutput::BaseInputIds::OUT_AM_INPUT].getVoltage();
					float modWeight = params[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AM_MIX_PARAM].getValue(); //rescale(params[baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AM_MIX_PARAM].getValue(), TROWA_MOSC_KNOB_MIN_V, TROWA_MOSC_KNOB_MAX_V, 0.f, 1.f)
					if (theOscillator->outputWaveforms[i].amRingModulation)
					{
						// Ring modulation
						modOutput = theOscillator->ringModulator.ringMod(modulator, rawOutput);
					}
					else
					{
						// Digital modulation (just mulitply)
						modOutput = modulator * rawOutput;
					}
					outputs[baseOutputId + TS_OscillatorOutput::BaseOutputIds::OUT_MULTIPLIED_SIGNAL].setVoltage(modWeight * modOutput + (1.0f - modWeight)*rawOutput);
				} // end calculate the multiplied signal
			} // end calculate the raw signal

			baseParamId += TS_OscillatorOutput::BaseParamIds::OUT_NUM_PARAMS;
			baseInputId += TS_OscillatorOutput::BaseInputIds::OUT_NUM_INPUTS;
			baseOutputId += TS_OscillatorOutput::BaseOutputIds::OUT_NUM_OUTPUTS;
			baseLightId += TS_OscillatorOutput::BaseLightIds::OUT_NUM_LIGHTS;
		} // end loop through output signals/channels
	} // end loop through oscillators
	return;
} // end process()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//::::::::::::: TS_OscillatorOutput :::::::::::::::::::::::::::::
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-

//--------------------------------------------------------
// initialize()
//--------------------------------------------------------
void TS_OscillatorOutput::initialize()
{
	phaseShift_deg = 0;
	phaseShift_norm = 0;
	amRingModulation = false;
	auxParam_norm = 0.5f;
	return;
}
//--------------------------------------------------------
// setPhaseShift_deg()
// @deg : (IN) The phase shift in degrees.
//--------------------------------------------------------
void TS_OscillatorOutput::setPhaseShift_deg(float deg)
{
	phaseShift_deg = deg;
	phaseShift_norm = deg / 360.0f;
	return;
}
//--------------------------------------------------------
// serialize()
// @returns : The TS_Oscillator json node.
//--------------------------------------------------------
json_t* TS_OscillatorOutput::serialize()
{
	json_t* rootJ = json_object();
	json_object_set_new(rootJ, "wavetype", json_integer(waveFormType));
	json_object_set_new(rootJ, "phaseShift_deg", json_real(ui_phaseShift_deg));
	json_object_set_new(rootJ, "auxParam_norm", json_real(auxParam_norm));
	json_object_set_new(rootJ, "amRingMod", json_integer(amRingModulation));
	return rootJ;
} // end serialize()
//--------------------------------------------------------
// deserialize()
// @rootJ : (IN) The TS_Oscillator json node.
//--------------------------------------------------------
void TS_OscillatorOutput::deserialize(json_t* rootJ)
{
	json_t* currJ = NULL;
	if (rootJ) {
		currJ = json_object_get(rootJ, "wavetype");
		if (currJ)
			waveFormType = static_cast<WaveFormType>( json_integer_value(currJ) );
		currJ = json_object_get(rootJ, "phaseShift_deg");
		if (currJ)
			ui_phaseShift_deg = json_number_value(currJ);	
		currJ = json_object_get(rootJ, "auxParam_norm");
		if (currJ)
			auxParam_norm = json_number_value(currJ);
		currJ = json_object_get(rootJ, "amRingMod");
		if (currJ)
			amRingModulation = json_integer_value(currJ) > 0;
	}
} // end deserialize()