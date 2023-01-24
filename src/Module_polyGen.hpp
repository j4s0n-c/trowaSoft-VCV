#ifndef TS_POLYGEN_HPP
#define TS_POLYGEN_HPP

#include <rack.hpp>
using namespace rack;

#include "TSModuleBase.hpp"

// Model for trowaSoft polyGen
extern Model* modelPolyGen;

#ifndef PI
	#define PI 			3.14159265
#endif // ! PI
#ifndef PI_HALF
	#define PI_HALF 	(PI/2.0)
#endif // ! PI_HALF

#define TS_POLYGEN_VERTICES_MIN		3	// Min # vertices/sides in polygon
#define TS_POLYGEN_VERTICES_MAX		33 	// Max #vertices/sides in polygon. Wanted 32 but for a nice 3 sides/Volt, we can do 33 I guess
#define TS_POLYGEN_VERTICES_DEF		3 	// Default # vertices/sides in polygon
#define TS_POLYGEN_ANGLE_OFFSET_DEG_MIN		-180	// Initial rotation/angle offset min
#define TS_POLYGEN_ANGLE_OFFSET_DEG_MAX		 180	// Initial rotation/angle offset max
#define TS_POLYGEN_ANGLE_OFFSET_DEG_DEF		   0	// Initial rotation/angle offset default

#define TS_CV_INPUT_RANGE_MIN			-12.0f	// Min Rack states should be 'allowed' 
#define TS_CV_INPUT_RANGE_MAX			 12.0f	// Max Rack states should be 'allowed' 
#define TS_CV_INPUT_MIN_DEF				  0.0f	// Our CV Input min value (was -5V), now 0 so it will match MIDI controllers.
#define TS_CV_INPUT_MAX_DEF				 10.0f	// Our CV Input max value (was +5 V), now 10 so it will match MIDI controllers.
#define TS_CV_OUTPUT_MIN_DEF			-10.0f	// Our CV OUTPUT minimum
#define TS_CV_OUTPUT_MAX_DEF			 10.0f	// Our CV OUTPUT maximum

#define TS_POLYGEN_AMPL_MIN			-10.0f
#define TS_POLYGEN_AMPL_MAX			10.0f
#define TS_POLYGEN_AMPL_DEF			5.0f

#define TS_POLGEN_ROT_DEG_MIN		-720.0f  // Rotation min (degrees). Why did we make 2 rotations? Don't remember now...
#define TS_POLGEN_ROT_DEG_MAX		 720.0f	 // Rotation max (degrees)
#define TS_POLGEN_ROT_DEG_DEF		   0.0f  // Rotation def (degrees)

// Inner Radius ======================
#define TS_POLYGEN_INNER_RADIUS_MULT_MIN	-5.0f // -500%
#define TS_POLYGEN_INNER_RADIUS_MULT_MAX	 5.0f // +500%
#define TS_POLYGEN_INNER_RADIUS_MULT_DEF	 1.0f //  100%

#define TS_POLYGEN_INNER_OFFSET_DEG_MIN	 -5.0f
#define TS_POLYGEN_INNER_OFFSET_DEG_MAX	 5.0f
#define TS_POLYGEN_INNER_OFFSET_DEG_DEF	 0.0f

#define TS_POLYGEN_BUFF_SIZE			1024

#define SINFUNC(x)					std::sin(x)	
#define COSFUNC(x)					std::cos(x)

#define DEBUG_POLY		0

#define TS_POLYGEN_IRADIUS_REL_2_MID_POINT		1 // Inner radius multiplier is multiplied by 0:Outer Amplitude, 1:Mid Point of line between corners
#define TS_POLYGEN_MOD_ENABLED					0 // Add modulation items. Currently we don't add these, we ran out of panel space and decided not needed since users can technically do this with the outputs in another module. 
												  // If turned back on, we have to add controls and such for these inputs/parameters.
#define TS_POLYGEN_TRIGGER_SYNC_EARLY			0 // (1) Trigger sync 1 dt before next cycle or (0) wait until we are actually starting the next cycle.

// This was originally written way back in late v0.6 (converted to v1) / early v1, but j4s0n now wants it in the official (v2) 
// plugin. May have some issues now after v2 conversion but seems OK so far.

//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// polyGen
// Generate polygons (stand-alone).
// Very simple linear interpolation between vertices.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct polyGen : TSModuleBase {
	// User control parameters
	enum ParamIds {
		// Frequency (Hz) - 1 cycle = 1 shape, so (shapes/s)
		FREQ_PARAM,
		// Number of outer vertices. 'Inner' vertices will be mapped in between, but by default will be in-line with the outer vertices.
		NUM_VERTICES_PARAM,
		// Angle offset for shape / Initial rotation
		ANGLE_OFFSET_PARAM,
		// Radius of inner vertices relative to the outer radius. Default is 1 (no star).
		INNER_VERTICES_RADIUS_PARAM,
		// Angle offset of the inner vertices. Default is 0 degrees from 180/N (mid).
		INNER_VERTICES_ANGLE_PARAM,
		X_AMPLITUDE_PARAM,
		Y_AMPLITUDE_PARAM,
		X_OFFSET_PARAM,		
		Y_OFFSET_PARAM,		
		ROTATION_PARAM,
		// Center of Rotation X
		X_C_ROTATION_PARAM,
		// Center of Rotation Y
		Y_C_ROTATION_PARAM,		
		// Apply ABSOLUTE rotation or RELATIVE rotation (true/false)		
		ROTATION_ABS_PARAM,
		// Currently not used modulation parameters:
		// Multiply shape X values (X * A). (Amplitude A). Multiplier A (Ax + B * sin (f*t)).
		X_AMP_MOD_MULT_PARAM,
		// Multiply shape Y values (Y * A).  (Amplitude A). Multiplier A (Ax + B * sin (f*t)).
		Y_AMP_MOD_MULT_PARAM,
		// Modulation of shape X values (X*A + B) (Frequency multiplier of B). Offset Frequency f (Ax + B * sin (f*t)).
		X_OFFSET_MOD_FREQ_MULT_PARAM,
		// Modulation of shape Y values (Y*A + B) (Frequency multiplier of B). Offset Frequency f (Ax + B * sin (f*t)).
		Y_OFFSET_MOD_FREQ_MULT_PARAM,	
		// Modulation of shape X values (X*A + B) (Amplitude of B). Offset Amplitude B (Ax + B * sin (f*t)).
		X_OFFSET_MOD_AMPL_MULT_PARAM,
		// Modulation of shape Y values (X*A + B) (Amplitude of B). Offset Amplitude B (Ax + B * sin (f*t)).
		Y_OFFSET_MOD_AMPL_MULT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FREQ_INPUT,
		NUM_VERTICES_INPUT,
		ANGLE_OFFSET_INPUT,		
		INNER_VERTICES_RADIUS_INPUT,		
		INNER_VERTICES_ANGLE_INPUT,
		X_AMPLITUDE_INPUT,
		Y_AMPLITUDE_INPUT,
		X_OFFSET_INPUT,
		Y_OFFSET_INPUT,		
		ROTATION_INPUT,
		// Center of Rotation X
		X_C_ROTATION_INPUT,
		// Center of Rotation Y
		Y_C_ROTATION_INPUT,		
		// Sync in
		SYNC_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		// X output
		X_OUTPUT,
		// Y output
		Y_OUTPUT,
		// X modified output
		X_MOD_OUTPUT,
		// Y modified output
		Y_MOD_OUTPUT,	
		// Sync out
		SYNC_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		X_OFFSET_AFTER_LIGHT,
		Y_OFFSET_AFTER_LIGHT,
		ROTATION_ABS_LIGHT,
		NUM_LIGHTS
	};
	
	// Phase
	float phase = 0.0f;
	// Current Vertex
	int currVertexIx = 0;
	// Next Vertex
	int nextVertexIx = 1;
	// Main shape, number of sides/vertices
	uint8_t numVertices = TS_POLYGEN_VERTICES_DEF;
	float angleOffset_rad;
	float xAmpl;
	float yAmpl;
	float xOffset;
	float yOffset;
	// Pre offset (center of rotation) X
	float xCRot;
	// Pre offset (center of rotation) Y	
	float yCRot;
	// Rotation 
	bool rotationIsAbs = true;
	float rotation_rad;
	float rotation_deg;
	int lastRotationAbs = -1;
	
	//=== * Inner Vertices * ===
	float innerRadiusMult = 1.0f; 	// Multiplier for radius (relative to main shape)
	float innerAngleMult = 0.0f; 	// Multiplier for angle (relative to the mid-angle of main shape)
	float innerPhase = 0.0f; 		// Inner/2ndary phase.
	int innerSideIx = 0;			// Which side we are on (from inner/2ndary point). Either 0 (before inner vertex) or 1 (after inner vertex).
	bool useInnerVerts = false;

	/// TODO: Allow user to specify the ranges of input/output voltages? -5, 5 or 0, 10, etc.
	const int MIN = 0;
	const int MAX = 1;	
	float inputVoltageRange[2] = { TS_CV_INPUT_MIN_DEF, TS_CV_INPUT_MAX_DEF };
	float outputVoltageRange[2] = { TS_CV_OUTPUT_MIN_DEF, TS_CV_OUTPUT_MIN_DEF };
	
	
	dsp::SchmittTrigger syncInTrigger;
	dsp::PulseGenerator syncOutPulse;
#if TS_POLYGEN_TRIGGER_SYNC_EARLY
	// If we started a sync out last time.
	bool lastSampleSyncOut = false;
#endif
	
	
#if TS_POLYGEN_MOD_ENABLED	
	// Modulation removed....
	float modPhaseX = 0.0f; // Phase for modulated X
	float modPhaseY = 0.0f; // Phase for modulated Y
#endif	
	

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// polyGen()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	polyGen();
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// ~polyGen()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	~polyGen();

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// initialize(void)
	// Set default values for members.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void initialize();
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// process()
	// [Previously step(void)]
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void process(const ProcessArgs &args) override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// getVoltage()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	float getVoltage(float v, bool input);
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
};


#endif // !TS_POLYGEN_HPP