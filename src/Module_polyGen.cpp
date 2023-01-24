#include "Module_polyGen.hpp"
#include "trowaSoftUtilities.hpp"
#include "Widget_polyGen.hpp"
#include "trowaSoft.hpp"

#include <math.h> 



Model* modelPolyGen = createModel<polyGen, polyGenWidget>(/*slug*/ "polyGen");

// Tune to normal frequencies although looks like at higher frequencies.
#define TROWA_FREQ_KNOB_MIN			-5.f // -2 	
#define TROWA_FREQ_KNOB_MAX			 5.f // 9 
#define TROWA_FREQ_KNOB_DEF			0.f //
#define TROWA_FREQ_KNOB_MULT		dsp::FREQ_C4 // FREQ_C4 = 261.6256f; 2

#define TS_POLYGEN_VOLTAGE_MIN		TROWA_SEQ_PATTERN_MIN_V
#define TS_POLYGEN_VOLTAGE_MAX		TROWA_SEQ_PATTERN_MAX_V

#define TS_POLYGEN_SM_VOLT_MIN		-5
#define TS_POLYGEN_SM_VOLT_MAX		5

// Offset of raw polygon.
#define TS_POLYGEN_MOD_OFFSET_MIN		-10
#define TS_POLYGEN_MOD_OFFSET_MAX		10
#define TS_POLYGEN_MOD_OFFSET_DEF		0

// Mod: Amplitude / Multiplier of shape waveform (Ax + B*sin(f*t) = Range of A).
#define TS_POLYGEN_MOD_AMP_MIN		-1
#define TS_POLYGEN_MOD_AMP_MAX		1
#define TS_POLYGEN_MOD_AMP_DEF		1

// Mod: Offset Amplitude Range (Ax + B*sin(f*t) = Range of B).
#define TS_POLYGEN_MOD_OFFSET_AMPL_MIN	-5
#define TS_POLYGEN_MOD_OFFSET_AMPL_MAX	5
#define TS_POLYGEN_MOD_OFFSET_AMPL_DEF	0

// Mod: Offset Frequency Mult Range (Ax + B*sin(f*t) = Range of f).
#define TS_POLYGEN_MOD_OFFSET_FREQ_MIN	0
#define TS_POLYGEN_MOD_OFFSET_FREQ_MAX	1000
#define TS_POLYGEN_MOD_OFFSET_FREQ_DEF	0



//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// polyGen()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
polyGen::polyGen()
{
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);	
	
	// Initialize all our variables
	initialize();
	
	//---------------------------
	// User Params:
	//---------------------------	
	configParam(/*id*/ ParamIds::FREQ_PARAM, /*minVal*/ TROWA_FREQ_KNOB_MIN, /*maxVal*/ TROWA_FREQ_KNOB_MAX, 
		/*defVal*/ TROWA_FREQ_KNOB_DEF, 
		/*label*/ "Frequency", /*unit*/ " Hz", /*displayBase*/ 2, /*displayMultiplier*/ TROWA_FREQ_KNOB_MULT, /*displayOffset*/ 0);
		
	// Number of sides/vertices (main shape)
	ParamQuantity* pQty = dynamic_cast<ParamQuantity*>( configParam<ParamQuantity>(/*id*/ ParamIds::NUM_VERTICES_PARAM, 
		/*minVal*/ TS_POLYGEN_VERTICES_MIN, /*maxVal*/ TS_POLYGEN_VERTICES_MAX,
		/*defVal*/ TS_POLYGEN_VERTICES_DEF, 
		/*label*/ "# Sides", /*unit*/ "", /*displayBase*/ 0, /*displayMultiplier*/ 1, /*displayOffset*/ 0, 
		/*descr*/ "Number of sides for the main shape.") );	
	pQty->snapEnabled = true;

	configParam(/*id*/ ParamIds::ANGLE_OFFSET_PARAM, /*minVal*/ TS_POLYGEN_ANGLE_OFFSET_DEG_MIN, /*maxVal*/ TS_POLYGEN_ANGLE_OFFSET_DEG_MAX, 
		/*defVal*/ TS_POLYGEN_ANGLE_OFFSET_DEG_DEF, 
		/*label*/ "Angle Offset", /*unit*/ "º", /*displayBase*/ 0, /*displayMultiplier*/ 1, /*displayOffset*/ 0,
		/*descr*/ "Rotate the main shape before applying X- and Y-amplitudes/radii.");

	//=== * Inner Vertices * ===
	configParam(/*id*/ ParamIds::INNER_VERTICES_RADIUS_PARAM, /*minVal*/ TS_POLYGEN_INNER_RADIUS_MULT_MIN, /*maxVal*/ TS_POLYGEN_INNER_RADIUS_MULT_MAX, 
		/*defVal*/ TS_POLYGEN_INNER_RADIUS_MULT_DEF, 
		/*label*/ "Inner Radius Size", /*unit*/ "%", /*displayBase*/ 0, /*displayMultiplier*/ 100, /*displayOffset*/ 0,
		/*descr*/ "Radius of secondary vertices relative to midpoint of a main shape's side.");
	configParam(/*id*/ ParamIds::INNER_VERTICES_ANGLE_PARAM, /*minVal*/ TS_POLYGEN_INNER_OFFSET_DEG_MIN, /*maxVal*/ TS_POLYGEN_INNER_OFFSET_DEG_MAX, 
		/*defVal*/ TS_POLYGEN_INNER_OFFSET_DEG_DEF, 
		/*label*/ "Inner Radius Angle Offset", /*unit*/ "%", /*displayBase*/ 0, /*displayMultiplier*/ 100, /*displayOffset*/ 0,
		/*descr*/ "Offset angle of the secondary vertices relative to 1/2 of the main polygon vertex angle.");
		
	configParam(/*id*/ ParamIds::X_AMPLITUDE_PARAM, /*minVal*/ TS_POLYGEN_AMPL_MIN, /*maxVal*/ TS_POLYGEN_AMPL_MAX, 
		/*defVal*/ TS_POLYGEN_AMPL_DEF, 
		/*label*/ "X Radius/Amplitude", /*unit*/ " V", /*displayBase*/ 0, /*displayMultiplier*/ 1, /*displayOffset*/ 0,
		/*descr*/ "Main shape X-amplitude/radius.");
	configParam(/*id*/ ParamIds::Y_AMPLITUDE_PARAM, /*minVal*/ TS_POLYGEN_AMPL_MIN, /*maxVal*/ TS_POLYGEN_AMPL_MAX, 
		/*defVal*/ TS_POLYGEN_AMPL_DEF, 
		/*label*/ "Y Radius/Amplitude", /*unit*/ " V", /*displayBase*/ 0, /*displayMultiplier*/ 1, /*displayOffset*/ 0,
		/*descr*/ "Main shape Y-amplitude/radius.");
	configParam(/*id*/ ParamIds::X_OFFSET_PARAM, /*minVal*/ TS_POLYGEN_AMPL_MIN, /*maxVal*/ TS_POLYGEN_AMPL_MAX, 
		/*defVal*/ 0, 
		/*label*/ "X Offset", /*unit*/ " V", /*displayBase*/ 0, /*displayMultiplier*/ 1, /*displayOffset*/ 0);
	configParam(/*id*/ ParamIds::Y_OFFSET_PARAM, /*minVal*/ TS_POLYGEN_AMPL_MIN, /*maxVal*/ TS_POLYGEN_AMPL_MAX, 
		/*defVal*/ 0, 
		/*label*/ "Y Offset", /*unit*/ " V", /*displayBase*/ 0, /*displayMultiplier*/ 1, /*displayOffset*/ 0);
	configParam(/*id*/ ParamIds::ROTATION_PARAM, /*minVal*/ TS_POLGEN_ROT_DEG_MIN, /*maxVal*/ TS_POLGEN_ROT_DEG_MAX, 
		/*defVal*/ TS_POLGEN_ROT_DEG_DEF, 
		/*label*/ "Rotation", /*unit*/ "º", /*displayBase*/ 0, /*displayMultiplier*/ 1, /*displayOffset*/ 0);
	configParam(/*id*/ ParamIds::X_C_ROTATION_PARAM, /*minVal*/ TS_POLYGEN_AMPL_MIN, /*maxVal*/ TS_POLYGEN_AMPL_MAX, 
		/*defVal*/ 0.0f, 
		/*label*/ "X Center of Rotation", /*unit*/ " V", /*displayBase*/ 0, /*displayMultiplier*/ 1, /*displayOffset*/ 0);
	configParam(/*id*/ ParamIds::Y_C_ROTATION_PARAM, /*minVal*/ TS_POLYGEN_AMPL_MIN, /*maxVal*/ TS_POLYGEN_AMPL_MAX, 
		/*defVal*/ 0.0f, 
		/*label*/ "Y Center of Rotation", /*unit*/ " V", /*displayBase*/ 0, /*displayMultiplier*/ 1, /*displayOffset*/ 0);		
	// Renaming to 'Spin'
	//configParam(/*id*/ ParamIds::ROTATION_ABS_PARAM, /*minVal*/ 0, /*maxVal*/ 1, 
	//	/*defVal*/ 0, 
	//	/*label*/ "Spin", /*unit*/ "", /*displayBase*/ 0, /*displayMultiplier*/ 1, /*displayOffset*/ 0,
	//	/*descr*/ "Continually rotate (true/1) or not (false/0).");
	TSModuleBase::configSwitch(/*id*/ ParamIds::ROTATION_ABS_PARAM, /*minVal*/ 0, /*maxVal*/ 1, /*defVal*/ 0,
		/*label*/ "Spin", /*descr*/ "Continually rotate (true/1) or not (false/0).",
		/*labels*/ {"false", "true"});

#if TS_POLYGEN_MOD_ENABLED
	// Mod: Multiplier A (Ax + B * sin (f*t))
	configParam(/*id*/ ParamIds::X_AMP_MOD_MULT_PARAM, /*minVal*/ TS_POLYGEN_MOD_AMP_MIN, /*maxVal*/ TS_POLYGEN_MOD_AMP_MAX, 
		/*defVal*/ TS_POLYGEN_MOD_AMP_DEF, /*label*/ "X Mod (Multiply)", /*unit*/ "", /*displayBase*/ 0, /*displayMultiplier*/ 1, /*displayOffset*/ 0);
	configParam(/*id*/ ParamIds::Y_AMP_MOD_MULT_PARAM, /*minVal*/ TS_POLYGEN_MOD_AMP_MIN, /*maxVal*/ TS_POLYGEN_MOD_AMP_MAX, 
		/*defVal*/ TS_POLYGEN_MOD_AMP_DEF, /*label*/ "Y Mod (Multiply)", /*unit*/ "", /*displayBase*/ 0, /*displayMultiplier*/ 1, /*displayOffset*/ 0);

	// Mod: Offset Amplitude B (Ax + B * sin (f*t))
	configParam(/*id*/ ParamIds::X_OFFSET_MOD_AMPL_MULT_PARAM, /*minVal*/ TS_POLYGEN_MOD_OFFSET_AMPL_MIN, /*maxVal*/ TS_POLYGEN_MOD_OFFSET_AMPL_MAX, 
		/*defVal*/ TS_POLYGEN_MOD_OFFSET_AMPL_DEF, /*label*/ "X Mod Offset Ampl", /*unit*/ "", /*displayBase*/ 0, /*displayMultiplier*/ 1, /*displayOffset*/ 0);
	configParam(/*id*/ ParamIds::Y_OFFSET_MOD_AMPL_MULT_PARAM, /*minVal*/ TS_POLYGEN_MOD_OFFSET_AMPL_MIN, /*maxVal*/ TS_POLYGEN_MOD_OFFSET_AMPL_MAX, 
		/*defVal*/ TS_POLYGEN_MOD_OFFSET_AMPL_DEF, /*label*/ "Y Mod Offset Ampl", /*unit*/ "", /*displayBase*/ 0, /*displayMultiplier*/ 1, /*displayOffset*/ 0);
	
	// Mod: Offset Frequency f (Ax + B * sin (f*t))
	configParam(/*id*/ ParamIds::X_OFFSET_MOD_FREQ_MULT_PARAM, /*minVal*/ TS_POLYGEN_MOD_OFFSET_FREQ_MIN, /*maxVal*/ TS_POLYGEN_MOD_OFFSET_FREQ_MAX, 
		/*defVal*/ TS_POLYGEN_MOD_OFFSET_DEF, /*label*/ "X Mod Offset Freq Mult", /*unit*/ "", /*displayBase*/ 0, /*displayMultiplier*/ 1, /*displayOffset*/ 0);
	configParam(/*id*/ ParamIds::Y_OFFSET_MOD_FREQ_MULT_PARAM, /*minVal*/ TS_POLYGEN_MOD_OFFSET_FREQ_MIN, /*maxVal*/ TS_POLYGEN_MOD_OFFSET_FREQ_MAX, 
		/*defVal*/ TS_POLYGEN_MOD_OFFSET_FREQ_DEF, /*label*/ "Y Mod Offset Freq Mult", /*unit*/ "", /*displayBase*/ 0, /*displayMultiplier*/ 1, /*displayOffset*/ 0);
#endif

	//---------------------------
	// Input Labels:
	//---------------------------	
	//FREQ_INPUT,
	configInput(InputIds::FREQ_INPUT, "Frequency");
	//NUM_VERTICES_INPUT,
	configInput(InputIds::NUM_VERTICES_INPUT, "# Vertices/Sides");
	//ANGLE_OFFSET_INPUT,
	configInput(InputIds::ANGLE_OFFSET_INPUT, "Angle Offset");
	//INNER_VERTICES_RADIUS_INPUT,
	configInput(InputIds::INNER_VERTICES_RADIUS_INPUT, "Inner Radius");
	//INNER_VERTICES_ANGLE_INPUT,
	configInput(InputIds::INNER_VERTICES_ANGLE_INPUT, "Inner Vertices Angle");
	//X_AMPLITUDE_INPUT,
	configInput(InputIds::X_AMPLITUDE_INPUT, "X Amplitude");
	//Y_AMPLITUDE_INPUT,
	configInput(InputIds::Y_AMPLITUDE_INPUT, "Y Amplitude");
	//X_OFFSET_INPUT,
	configInput(InputIds::X_OFFSET_INPUT, "X Offset");
	//Y_OFFSET_INPUT,
	configInput(InputIds::Y_OFFSET_INPUT, "Y Offset");
	//ROTATION_INPUT,
	configInput(InputIds::ROTATION_INPUT, "Rotation");
	//// Center of Rotation X
	//X_C_ROTATION_INPUT,
	configInput(InputIds::X_C_ROTATION_INPUT, "X Center of Rotation");
	//// Center of Rotation Y
	//Y_C_ROTATION_INPUT,
	configInput(InputIds::Y_C_ROTATION_INPUT, "Y Center of Rotation");
	//// Sync in
	//SYNC_INPUT,
	configInput(InputIds::SYNC_INPUT, "Sync");

	//---------------------------
	// Output Labels:
	//---------------------------
	//// X output
	//X_OUTPUT,
	configOutput(OutputIds::X_OUTPUT, "X Output");
	//// Y output
	//Y_OUTPUT,
	configOutput(OutputIds::Y_OUTPUT, "Y Output");
	//// X modified output
	//X_MOD_OUTPUT,
	configOutput(OutputIds::X_MOD_OUTPUT, "X Modified Output");
	//// Y modified output
	//Y_MOD_OUTPUT,
	configOutput(OutputIds::Y_MOD_OUTPUT, "Y Modified Output");
	//// Sync out
	//SYNC_OUTPUT,
	configOutput(OutputIds::SYNC_OUTPUT, "Sync Output");

	return;
} // end polyGen()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// ~polyGen()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
polyGen::~polyGen()
{
	return;
} // end ~polyGen()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// initialize(void)
// Set default values for members.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void polyGen::initialize()
{
	phase = 0.0f;
	currVertexIx = 0;
	nextVertexIx = 1;
	// Inputs
	numVertices = TS_POLYGEN_VERTICES_DEF;
	angleOffset_rad = TS_POLYGEN_ANGLE_OFFSET_DEG_DEF * 2 * PI;
	xAmpl = TS_POLYGEN_AMPL_DEF;
	yAmpl = TS_POLYGEN_AMPL_DEF;
	//xOffsetIsAfterRot = false;
	//yOffsetIsAfterRot = false;	
	xOffset = 0;
	yOffset = 0;
	rotationIsAbs = true;	
	rotation_rad = 0;
	rotation_deg = 0;
	xCRot = 0;
	yCRot = 0;
	lastRotationAbs = -1;
	
	// Inner/2ndary Points:
	innerPhase = 0.0f;
	innerRadiusMult = TS_POLYGEN_INNER_RADIUS_MULT_DEF;
	innerAngleMult = TS_POLYGEN_INNER_OFFSET_DEG_DEF;

#if TS_POLYGEN_MOD_ENABLED	
	// Modulation removed, can just use another module for that.
	modPhaseX = 0.0f;
	modPhaseY = 0.0f;
#endif 		
	inputVoltageRange[MIN] = TS_CV_INPUT_MIN_DEF;
	inputVoltageRange[MAX] = TS_CV_INPUT_MAX_DEF;	
	outputVoltageRange[MIN] = TS_CV_OUTPUT_MIN_DEF;
	outputVoltageRange[MAX] = TS_CV_OUTPUT_MAX_DEF;	
	
	return;
} // end initialize()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// process()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void polyGen::process(const ProcessArgs &args)
{
	//====================================================================
	// Inputs (User Parameters and CV Inputs)
	//====================================================================
	
	//--------------------
	// Sync/Reset
	//--------------------
	bool syncIn = inputs[InputIds::SYNC_INPUT].isConnected() && syncInTrigger.process(inputs[InputIds::SYNC_INPUT].getVoltage());
	
	//--------------------
	//=== * Booleans * ===
	//--------------------
	rotationIsAbs = !(params[ROTATION_ABS_PARAM].getValue()  > 0);
	if (rotationIsAbs != lastRotationAbs)
	{
		if (rotationIsAbs)
		{
			paramQuantities[ROTATION_PARAM]->unit = "º";
			paramQuantities[ROTATION_PARAM]->name = "Rotation"; // v2: now name instead of label
			inputInfos[InputIds::ROTATION_INPUT]->name = "Rotation";
		}
		else
		{
			paramQuantities[ROTATION_PARAM]->unit = " º/s";
			paramQuantities[ROTATION_PARAM]->name = "Spin Rate"; // v2: now name instead of label
			inputInfos[InputIds::ROTATION_INPUT]->name = "Spin Rate";
		}
		lastRotationAbs = rotationIsAbs;
	}
	
	// Knobs that can go to 0 will be added to the CV input.
	// Knobs that can not go to 0 will be ignored if the CV input is active.

	//------------------------------------
	//=== * # Vertices (main polygon) *===
	//------------------------------------
	// Number of Sides/Vertices on Polygon. [CV --OR-- KNOB]
	if (inputs[NUM_VERTICES_INPUT].isConnected())
	{
		numVertices = clamp(static_cast<uint8_t>(rescale(inputs[NUM_VERTICES_INPUT].getVoltage(), inputVoltageRange[MIN], inputVoltageRange[MAX], TS_POLYGEN_VERTICES_MIN, TS_POLYGEN_VERTICES_MAX)), TS_POLYGEN_VERTICES_MIN, TS_POLYGEN_VERTICES_MAX);
	}
	else
	{
		numVertices = static_cast<uint8_t>( params[NUM_VERTICES_PARAM].getValue() );
	}

	// Angle Offset [CV + KNOB] 
	angleOffset_rad = params[ANGLE_OFFSET_PARAM].getValue();	
	if (inputs[ANGLE_OFFSET_INPUT].isConnected())
	{
		angleOffset_rad += rescale(getVoltage(inputs[ANGLE_OFFSET_INPUT].getVoltage(), true), inputVoltageRange[MIN], inputVoltageRange[MAX], TS_POLYGEN_ANGLE_OFFSET_DEG_MIN, TS_POLYGEN_ANGLE_OFFSET_DEG_MAX);
	}
	angleOffset_rad *= PI / 180.0f;
	
	
	//--------------------------
	//=== * Inner Vertices * ===
	//--------------------------
	innerRadiusMult = params[INNER_VERTICES_RADIUS_PARAM].getValue();	
	if (inputs[INNER_VERTICES_RADIUS_INPUT].isConnected())
	{
		// 1 Volt = 100%. 
		innerRadiusMult += rescale(inputs[INNER_VERTICES_RADIUS_INPUT].getVoltage(), inputVoltageRange[MIN], inputVoltageRange[MAX], TS_POLYGEN_INNER_RADIUS_MULT_MIN, TS_POLYGEN_INNER_RADIUS_MULT_MAX);
		/// TODO: clamp if just let run wild if user wants to?		
	}
	innerAngleMult = params[INNER_VERTICES_ANGLE_PARAM].getValue();	
	if (inputs[INNER_VERTICES_ANGLE_INPUT].isConnected())
	{
		// 1 Volt = 100%.
		innerAngleMult += rescale(inputs[INNER_VERTICES_ANGLE_INPUT].getVoltage(), inputVoltageRange[MIN], inputVoltageRange[MAX], TS_POLYGEN_INNER_OFFSET_DEG_MIN, TS_POLYGEN_INNER_OFFSET_DEG_MAX);
		/// TODO: clamp if just let run wild if user wants to?		
	}
	// See if we even have to worry about inner (2ndary) vertices (ignore if very close to 100%)
	const float threshold = 0.0005f;
	float radiusDiff = 1.0f - innerRadiusMult;
	useInnerVerts = radiusDiff < -threshold || radiusDiff > threshold;

	//---------------------------------------
	//=== * Amplitude, Offset for X & Y * ===
	//---------------------------------------
	// X_AMPLITUDE_PARAM,
	// Y_AMPLITUDE_PARAM,
	// X_OFFSET_PARAM,		
	// Y_OFFSET_PARAM,	
	// X_C_ROTATION_PARAM,
	// Y_C_ROTATION_PARAM
	const int numVoltageParams = 6;
	float* vPtrs[] = { &xAmpl, &yAmpl, &xOffset, &yOffset, &xCRot, &yCRot };
	int vParamIds[] = { ParamIds::X_AMPLITUDE_PARAM, ParamIds::Y_AMPLITUDE_PARAM, ParamIds::X_OFFSET_PARAM, ParamIds::Y_OFFSET_PARAM, ParamIds::X_C_ROTATION_PARAM, ParamIds::Y_C_ROTATION_PARAM };
	int vInputIds[] = { InputIds::X_AMPLITUDE_INPUT, InputIds::Y_AMPLITUDE_INPUT, InputIds::X_OFFSET_INPUT, InputIds::Y_OFFSET_INPUT, InputIds::X_C_ROTATION_INPUT, InputIds::Y_C_ROTATION_INPUT };
	for (int i = 0; i < numVoltageParams; i++)
	{
		int paramAmpId = vParamIds[i];
		int inputAmpId = vInputIds[i];
		if (inputs[inputAmpId].isConnected())
		{
			// Use voltage directly. Limit/clip based off Rack's abs limits (TS_CV_INPUT_RANGE_MIN, TS_CV_INPUT_RANGE_MAX)
			// Or make it just consistent (-10 to 10)
			(*(vPtrs[i])) = clamp(inputs[inputAmpId].getVoltage(), TS_POLYGEN_AMPL_MIN, TS_POLYGEN_AMPL_MAX);
		}
		else
		{
			(*(vPtrs[i])) = params[paramAmpId].getValue();
		}
	}
	
	//--------------------
	//=== * Rotation * ===
	//--------------------
	float rot_deg = params[ROTATION_PARAM].getValue();	
	if (inputs[ROTATION_INPUT].isConnected())
	{
		// 1 V per rotation
		rot_deg += clamp(inputs[ROTATION_INPUT].getVoltage(), TS_POLYGEN_AMPL_MIN, TS_POLYGEN_AMPL_MAX) * 360.0f;
	}
	if (rotationIsAbs)
	{
		rotation_deg = rot_deg;
	}
	else
	{
		// Rotations is N deg/second
		// So need to reduce by sample rate
		rotation_deg += rot_deg / args.sampleRate;
	}
	// Just make rotation simplier (-360 to 360)
	if (rotation_deg < -360 || rotation_deg > 360)
	{
		int n = static_cast<int>( std::abs(rotation_deg) / 360.0f + 0.5f );
		if (rotation_deg >= 0.0f)
			rotation_deg -= (n * 360);
		else
			rotation_deg += (n * 360);
	}
	rotation_rad = rotation_deg / 180.0f * PI;		
	
	
	//---------------------------
	//=== * Timing/Frequency *===
	//---------------------------
	// Main Clock:
	float vertexTime, nextVertexTime; // Normalized 0 to 1 time for vertices.
	
	// Add Knob + Input
	float input = params[FREQ_PARAM].getValue();
	if (inputs[FREQ_INPUT].isConnected())
	{
		input += inputs[FREQ_INPUT].getVoltage();
	}
	// clamp
	input = clamp(input, static_cast<float>(TROWA_FREQ_KNOB_MIN), static_cast<float>(TROWA_FREQ_KNOB_MAX));
	// Want to draw N polygons per second (so multiply by # vertices):
	float clockTime = powf(2.0, input) * TROWA_FREQ_KNOB_MULT * numVertices;	
	float dt = clockTime / args.sampleRate; // Real dt
	phase += dt; // Main vertex phase
	innerPhase += dt; // 2ndary/Inner vertex phase		

#if TS_POLYGEN_TRIGGER_SYNC_EARLY
	//============================
	// Sync Out - Trigger one sample early
	//============================
	// The problem with triggering early is that the dt can change (user can be modulating frequency and # sides).
	// We could track the total phase instead of the 'phase' for a side though so that part could be mitigated, but frequency change could not.
	// Even though dt can change next sample, we'll assume it's the same
	// Also if we got a sync signal and we didn't already trigger within the last pulse time, 
	// send a trigger (even though it won't be early).
	if ( (currVertexIx == (numVertices - 1) && phase + dt >= (1.0f - dt)) || (syncIn && !lastSampleSyncOut) )
	{
		syncOutPulse.trigger(TROWA_PULSE_WIDTH);
	}
#endif


#if TS_POLYGEN_MOD_ENABLED	
	// Modulation Clocks:
	float modMult = params[X_OFFSET_MOD_FREQ_MULT_PARAM].getValue(); 
	if (modMult < 0)
		modMult = -1.0f / modMult;		
	modPhaseX += dt * modMult;
	modMult = params[Y_OFFSET_MOD_FREQ_MULT_PARAM].getValue();
	if (modMult < 0)
		modMult = -1.0f / modMult;		
	modPhaseY += dt * modMult;
#endif	

	
	//============================
	// Check for Next Side/Vertex
	//============================
	bool newCorner = false;
	if (phase >= 1.0f || syncIn)
	{
		if (syncIn)
		{
			phase = 0.0f;
			currVertexIx = 0;
		}
		else
		{
			phase -= 1.0f; // (Soft) Reset main clock phase
			currVertexIx++;
		}
		newCorner = true;
		
		if (currVertexIx >= numVertices)
			currVertexIx = 0;

#if !TS_POLYGEN_TRIGGER_SYNC_EARLY
		//============================
		// Sync Out - Trigger at start of new cycle
		//============================
		if (currVertexIx < 1)
			syncOutPulse.trigger(TROWA_PULSE_WIDTH);
#endif


		innerPhase = 0; // (Hard) Reset inner/2ndary phase (for inner/2ndary vertices)
		innerSideIx = 0; // Reset the side we are on (for inner/2ndary vertices)				
#if TS_POLYGEN_MOD_ENABLED	
		// Modulation removed, can just use another module for that.		
		modPhaseX = 0; // (Hard) Reset our modulating phases.
		modPhaseY = 0; // (Hard) Reset our modulating phases.
#endif	
	}
	
#if TS_POLYGEN_MOD_ENABLED	
	if (modPhaseX >= 1.0f)
	{
		modPhaseX -= 1.0f;		
	}
	if (modPhaseY >= 1.0f)
	{
		modPhaseY -= 1.0f;		
	}
#endif
	
	// Which vertex we are on (outer/main)
	if (currVertexIx >= numVertices)
		currVertexIx = 0;
	nextVertexIx = currVertexIx + 1;
	if (nextVertexIx >= numVertices)
		nextVertexIx = 0;

	//=======================================
	// Calculate the 2 vertices we will use
	//=======================================
	// All this may be done more efficiently? But this is the simple way that I came up with and it works...
	
	// Time/Phase for sin
	vertexTime = static_cast<float>(currVertexIx) / static_cast<float>(numVertices);
	nextVertexTime = static_cast<float>(nextVertexIx) / static_cast<float>(numVertices);	
	
	float v1Time = vertexTime;
	float v2Time = nextVertexTime;
	Vec v1Ampl = Vec(xAmpl, yAmpl);
	Vec v2Ampl = Vec(xAmpl, yAmpl);
	float linearPhase = clamp(phase, 0.0f, 1.0f); // For interpolation
	Vec thisCorner, nextCorner;
	
	//---------------------
	// This Corner/Vertex
	//---------------------	
	thisCorner.x = v1Ampl.x * SINFUNC( 2 * PI * v1Time + angleOffset_rad);
	thisCorner.y = v1Ampl.y * COSFUNC( 2 * PI * v1Time + angleOffset_rad);

	//------------------------	
	// The Next Corner/Vertex
	//------------------------	
	nextCorner.x = v2Ampl.x * SINFUNC( 2 * PI * v2Time + angleOffset_rad);
	nextCorner.y = v2Ampl.y * COSFUNC( 2 * PI * v2Time + angleOffset_rad);	
	
	if (useInnerVerts)
	{
		// Inject inner/2ndary vertex
		float iTime = 0.5f * (1 + innerAngleMult);
				
		// Use our inner/2ndary phase to see where we are
		linearPhase = clamp(innerPhase, 0.0f, 1.0f);		
		
#if TS_POLYGEN_IRADIUS_REL_2_MID_POINT
		// Calculate the point on the line between the two corners
		float midX = thisCorner.x + (nextCorner.x - thisCorner.x) * 0.5f; 
		float midY = thisCorner.y + (nextCorner.y - thisCorner.y) * 0.5f;
		float ampl = std::sqrt(midX * midX + midY * midY) * innerRadiusMult;
#endif
		if (linearPhase < 0.5f)
		{
			// First Vertex then this middle inner one
			v2Time = vertexTime + iTime/numVertices;
#if TS_POLYGEN_IRADIUS_REL_2_MID_POINT
			v2Ampl.x = ampl * sgn(v2Ampl.x);
			v2Ampl.y = ampl * sgn(v2Ampl.y);
#else
			v2Ampl.x *= innerRadiusMult;
			v2Ampl.y *= innerRadiusMult;
#endif	
			linearPhase = linearPhase / iTime; // Rescale 0 to 1			
			nextCorner.x = v2Ampl.x * SINFUNC( 2 * PI * v2Time + angleOffset_rad);
			nextCorner.y = v2Ampl.y * COSFUNC( 2 * PI * v2Time + angleOffset_rad);				
		}
		else
		{
			// This middle inner one and then the 2nd vertex
			v1Time = vertexTime + iTime / numVertices;
#if TS_POLYGEN_IRADIUS_REL_2_MID_POINT			
			v1Ampl.x = ampl * sgn(v1Ampl.x); 			
			v1Ampl.y = ampl * sgn(v1Ampl.y);
#else
			v1Ampl.x *= innerRadiusMult;
			v1Ampl.y *= innerRadiusMult;	
#endif
			linearPhase = (linearPhase - 0.5f) / 0.5f;	// Rescale 0 to 1			
			thisCorner.x = v1Ampl.x * SINFUNC( 2 * PI * v1Time + angleOffset_rad);
			thisCorner.y = v1Ampl.y * COSFUNC( 2 * PI * v1Time + angleOffset_rad);			
		}		
	} // end if inner/2ndary vertices
			
	//===============================
	// Interpolate this step's value
	//===============================
	// Interpolate based on which point we are on this side
	float vx = thisCorner.x;
	float vy = thisCorner.y;	
	if (!newCorner)
	{
		// We don't have to interpolate if it is a new corner, otherwise simple linear interpolation
		float mult = clamp(linearPhase, 0.0f, 1.0f);
		vx += (nextCorner.x - thisCorner.x) * mult;
		vy += (nextCorner.y - thisCorner.y) * mult;
	}

	//===============================
	// Rotate the point
	//===============================
	float vxR, vyR;
	vxR = vx;
	vyR = vy;
	
	if (rotation_deg != 0 && rotation_deg != 360)
	{
		float sinrot = SINFUNC( rotation_rad );
		float cosrot = COSFUNC( rotation_rad );	
		
		// Translate to rotation center
		vx -= xCRot;
		vy -= yCRot;		
		
		// Rotate
		vxR = vx * cosrot - vy * sinrot;
		vyR = vx * sinrot + vy * cosrot;		
		
		// Translate back after rotation
		vxR += xCRot;
		vyR += yCRot;		
	}
	
	//================================
	// Post Rotation Offset
	//================================
	vxR += xOffset;
	vyR += yOffset;
	
#if TS_POLYGEN_MOD_ENABLED		
	//===============================	
	// Modulated outputs
	//===============================	
	float xMod = vxR;
	float yMod = vyR;
	float Ax, Ay;
	float Bx, By;
	float ph = modPhaseX; // Multiple of our shape's phase
	
	// Mod A*x + B*sin(f*t)	
	// X:
	Ax = params[X_AMP_MOD_MULT_PARAM].getValue();
	Bx = params[X_OFFSET_MOD_AMPL_MULT_PARAM].getValue() * SINFUNC(ph);	
	xMod = Ax * vxR + Bx;
	// Y:
	Ay = params[Y_AMP_MOD_MULT_PARAM].getValue();
	ph = modPhaseY;
	By = params[Y_OFFSET_MOD_AMPL_MULT_PARAM].getValue() * SINFUNC(ph);	
	yMod = Ay * vyR + By;

	// OUTPUTS:
	outputs[OutputIds::X_MOD_OUTPUT].setVoltage(xMod);
	outputs[OutputIds::Y_MOD_OUTPUT].setVoltage(yMod);
#endif

	//====================================================================	
	// Outputs
	//====================================================================
	outputs[OutputIds::X_OUTPUT].setVoltage( getVoltage(vxR, false) ); // Clip
	outputs[OutputIds::Y_OUTPUT].setVoltage( getVoltage(vyR, false) );
#if TS_POLYGEN_TRIGGER_SYNC_EARLY
	if (syncOutPulse.process(args.sampleTime))
	{
		lastSampleSyncOut = true; // Track whenever we have done this
		outputs[OutputIds::SYNC_OUTPUT].setVoltage(10.f);
	}
	else
	{
		lastSampleSyncOut = false; // Track whenever we have done this
		outputs[OutputIds::SYNC_OUTPUT].setVoltage(0.f);
	}
#else
	outputs[OutputIds::SYNC_OUTPUT].setVoltage((syncOutPulse.process(args.sampleTime)) ? 10.0f : 0.0f);
#endif
	
	//====================================================================
	// Lights
	//====================================================================	
	lights[ROTATION_ABS_LIGHT].value = !rotationIsAbs; // Now it is "Spin" is true.

	return;
} // end process()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// reset(void)
// Initialize values.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void polyGen::onReset() 
{
	initialize();
	return;
} // end onReset()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// getVoltage()
// Limits the voltage to the given range.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
float polyGen::getVoltage(float v, bool input)
{
	float* limits = (input) ? inputVoltageRange : outputVoltageRange;
	return clamp(v, limits[MIN], limits[MAX]);
}


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// dataToJson(void)
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
json_t* polyGen::dataToJson()
{
	json_t* rootJ = json_object();
	json_object_set_new(rootJ, "version", json_integer(TROWA_INTERNAL_VERSION_INT));
	
	// Don't really have to save this if controls save these values:
	json_object_set_new(rootJ, "numVertices", json_integer(numVertices));	
	json_object_set_new(rootJ, "innerRadiusMult", json_real(innerRadiusMult));	
	json_object_set_new(rootJ, "innerAngleMult", json_real(innerAngleMult));		
	json_object_set_new(rootJ, "xAmpl", json_real(xAmpl));
	json_object_set_new(rootJ, "yAmpl", json_real(yAmpl));
	json_object_set_new(rootJ, "xOffset", json_real(xOffset));
	json_object_set_new(rootJ, "yOffset", json_real(yOffset));	
	json_object_set_new(rootJ, "rotationIsAbs", json_integer(rotationIsAbs));			
	json_object_set_new(rootJ, "rotation_deg", json_real(rotation_deg));

	// Ranges would have to saved
	json_t* inRangeJ = json_array();
	json_t* outRangeJ = json_array();	
	for (int i = 0; i < 2; i++)
	{
		json_array_append_new(inRangeJ, json_real(inputVoltageRange[i]));
		json_array_append_new(outRangeJ, json_real(outputVoltageRange[i]));		
	}
	json_object_set_new(rootJ, "cvInRange_V", inRangeJ);
	json_object_set_new(rootJ, "cvOutRange_V", outRangeJ);	
	
	return rootJ;
} // end dataToJson()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// dataFromJson(void)
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void polyGen::dataFromJson(json_t *rootJ)
{
	if (rootJ) 
	{
		json_t* currJ = NULL;		
		json_t* inRangeJ = json_object_get(rootJ, "cvInRange_V");
		json_t* outRangeJ = json_object_get(rootJ, "cvOutRange_V");
		for (int i = 0; i < 2; i++)
		{
			currJ = json_array_get(inRangeJ, i);
			if (currJ)
				inputVoltageRange[i] = (float)json_number_value(currJ);
			currJ = json_array_get(outRangeJ, i);
			if (currJ)
				outputVoltageRange[i] = (float)json_number_value(currJ);
		}
		if (inputVoltageRange[MIN] >= inputVoltageRange[MAX])
		{
			// Needs adjustment
			if (inputVoltageRange[MAX] < TS_CV_INPUT_RANGE_MAX)
				inputVoltageRange[MAX] += 3.0f;
			else if (inputVoltageRange[MIN] > TS_CV_INPUT_RANGE_MIN)
				inputVoltageRange[MIN] -= 3.0f;
		}
		if (outputVoltageRange[MIN] >= outputVoltageRange[MAX])
		{
			// Needs adjustment
			if (outputVoltageRange[MAX] < TS_CV_INPUT_RANGE_MAX)
				outputVoltageRange[MAX] += 3.0f;
			else if (outputVoltageRange[MIN] > TS_CV_INPUT_RANGE_MIN)
				outputVoltageRange[MIN] -= 3.0f;
		}
		// Make sure at least within limits of Rack recommendations (PSU only -12 V to +12 V).
		for (int i = 0; i < 2; i++)
		{
			inputVoltageRange[i] = clamp(inputVoltageRange[i], static_cast<float>(TS_CV_INPUT_RANGE_MIN), static_cast<float>(TS_CV_INPUT_RANGE_MAX));
			outputVoltageRange[i] = clamp(outputVoltageRange[i], static_cast<float>(TS_CV_INPUT_RANGE_MIN), static_cast<float>(TS_CV_INPUT_RANGE_MAX));			
		}
		
		currJ = json_object_get(rootJ, "numVertices");
		if (currJ)
			numVertices = static_cast<uint8_t>( json_integer_value(currJ) );
		currJ = json_object_get(rootJ, "innerRadiusMult");
		if (currJ)
			innerRadiusMult = json_real_value(currJ);
		currJ = json_object_get(rootJ, "innerAngleMult");
		if (currJ)
			innerAngleMult = json_real_value(currJ);		
		currJ = json_object_get(rootJ, "xAmpl");
		if (currJ)
			xAmpl = json_real_value(currJ);
		currJ = json_object_get(rootJ, "yAmpl");
		if (currJ)
			yAmpl = json_real_value(currJ);
		currJ = json_object_get(rootJ, "xOffset");
		if (currJ)
			xOffset = json_real_value(currJ);
		currJ = json_object_get(rootJ, "yOffset");
		if (currJ)
			yOffset = json_real_value(currJ);		
		currJ = json_object_get(rootJ, "rotationIsAbs");
		if (currJ)
			rotationIsAbs = json_integer_value(currJ) > 0;
		currJ = json_object_get(rootJ, "rotation_deg");
		if (currJ)
			rotation_deg = json_real_value(currJ);

	} // end if rootJ
	return;
} // end dataFromJson()
