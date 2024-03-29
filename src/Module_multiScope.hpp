#ifndef MODULE_MULTISCOPE_HPP
#define MODULE_MULTISCOPE_HPP

#include "Features.hpp"

#if USE_NEW_SCOPE

#include <string.h>
#include <stdint.h>
#include "trowaSoft.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSColors.hpp"
#include "TSScopeBase.hpp"

// For now, the color picker is disabled in Apple because it will crash Rack & Eat your soul...
// Linux & Windows seem fine though
// Now it seems the scope just crashes Rack on OSX no matter what.
// [2019-07-25] v1.0 Mac doesn't crash with new scope now. Yay!
#define ENABLE_BG_COLOR_PICKER			1 //(!(__APPLE__))

#define TROWA_SCOPE_NUM_WAVEFORMS	3

// Laying out controls
#define TROWA_SCOPE_CONTROL_START_X			52  // 47
#define TROWA_SCOPE_CONTROL_START_Y			40  // 94
#define TROWA_SCOPE_CONTROL_DX					35 // 35
#define TROWA_SCOPE_CONTROL_DY					30 // 26
#define TROWA_SCOPE_CONTROL_SHAPE_SPACING		16 // 8


// Labels for each waveform / shape
#define TROWA_SCOPE_SHAPE_FORMAT_STRING		"Shape %d"

// multiScope model.
extern Model *modelMultiScope;


//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiScope
// Scope module that draws multiple waveforms.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct multiScope : Module {
	// Should probably make this linearly growing in case of course, we add more freaken waveforms...
	// TODO: Refactor to make variable num waveforms.
	enum ParamIds {
		COLOR_PARAM,
		EXTERNAL_PARAM = COLOR_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		ROTATION_PARAM = EXTERNAL_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		ROTATION_MODE_PARAM = ROTATION_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		TIME_PARAM = ROTATION_MODE_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		TRIG_PARAM = TIME_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		X_POS_PARAM = TRIG_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		X_SCALE_PARAM = X_POS_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		Y_POS_PARAM = X_SCALE_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		Y_SCALE_PARAM = Y_POS_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		LINK_XY_SCALE_PARAM = Y_SCALE_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,	   // Force Scale X = Scale Y.
		OPACITY_PARAM = LINK_XY_SCALE_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,	   // Alpha channel
		LISSAJOUS_PARAM = OPACITY_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,		  
		THICKNESS_PARAM = LISSAJOUS_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		FILL_ON_PARAM = THICKNESS_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		FILL_COLOR_PARAM = FILL_ON_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		FILL_OPACITY_PARAM = FILL_COLOR_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		EFFECT_PARAM = FILL_OPACITY_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		// Single Inputs:
		// Info display
		INFO_DISPLAY_TOGGLE_PARAM = EFFECT_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		// BG Color Display
		BGCOLOR_DISPLAY_PARAM = INFO_DISPLAY_TOGGLE_PARAM + 1,
		BGCOLOR_HUE_PARAM = BGCOLOR_DISPLAY_PARAM + 1,
		BGCOLOR_SAT_PARAM = BGCOLOR_HUE_PARAM + 1,
		BGCOLOR_VAL_PARAM = BGCOLOR_SAT_PARAM + 1,		
		NUM_PARAMS = BGCOLOR_VAL_PARAM + 1
	};
	enum InputIds {
		COLOR_INPUT,
		ROTATION_INPUT = COLOR_INPUT+TROWA_SCOPE_NUM_WAVEFORMS,
		TIME_INPUT = ROTATION_INPUT+TROWA_SCOPE_NUM_WAVEFORMS,
		X_INPUT = TIME_INPUT+TROWA_SCOPE_NUM_WAVEFORMS,
		Y_INPUT = X_INPUT+TROWA_SCOPE_NUM_WAVEFORMS,
		OPACITY_INPUT = Y_INPUT + TROWA_SCOPE_NUM_WAVEFORMS, // Opacity/Alpha channel
		PEN_ON_INPUT = OPACITY_INPUT + TROWA_SCOPE_NUM_WAVEFORMS, // Turn on/off drawing lines in between points.
		THICKNESS_INPUT = PEN_ON_INPUT + TROWA_SCOPE_NUM_WAVEFORMS,
		FILL_COLOR_INPUT = THICKNESS_INPUT + TROWA_SCOPE_NUM_WAVEFORMS,
		FILL_OPACITY_INPUT = FILL_COLOR_INPUT + TROWA_SCOPE_NUM_WAVEFORMS,
		NUM_INPUTS = FILL_OPACITY_INPUT + TROWA_SCOPE_NUM_WAVEFORMS
	};

	enum LightIds {
		COLOR_LED,
		ROT_LED = COLOR_LED+TROWA_SCOPE_NUM_WAVEFORMS,
		LINK_XY_SCALE_LED = ROT_LED + TROWA_SCOPE_NUM_WAVEFORMS,
		LISSAJOUS_LED = LINK_XY_SCALE_LED + TROWA_SCOPE_NUM_WAVEFORMS,
		HIGHLIGHT_LED = LISSAJOUS_LED + TROWA_SCOPE_NUM_WAVEFORMS,
		FILL_ON_LED = HIGHLIGHT_LED + TROWA_SCOPE_NUM_WAVEFORMS,
		INFO_DISPLAY_TOGGLE_LED = FILL_ON_LED + TROWA_SCOPE_NUM_WAVEFORMS,
		BGCOLOR_DISPLAY_LED = INFO_DISPLAY_TOGGLE_LED + 1,
		NUM_LEDS = BGCOLOR_DISPLAY_LED + 1
	};

	enum OutputIds {
		NUM_OUTPUTS
	};

	bool initialized = false;
	bool firstLoad = true;

	dsp::SchmittTrigger infoDisplayOnTrigger;
	bool negativeImage = false;
	// Background color for our scope/plotting area
	NVGcolor plotBackgroundColor;
	dsp::SchmittTrigger plotBackgroundDisplayOnTrigger;
	bool showColorPicker = false;
	// Current Color Picker Color (ptr to which color we are editing)
	NVGcolor* editColorPointer;

	// Information about what we are plotting. In future may become dynamically allocated.
	TSWaveform* waveForms[TROWA_SCOPE_NUM_WAVEFORMS];

	// Widget Values =====
	// Widget width
	float widgetWidth = 0;
	bool widgetShowDisplay = false; // Default off now.
	static const int widgetWidthDefault = 720;

	multiScope();
	~multiScope();
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// process()
	// [Previously step(void)]
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void process(const ProcessArgs &args) override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// dataToJson(void)
	// Save to json.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	json_t *dataToJson() override {
		// Really should just serialize the TSWaveForm object.
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "version", json_integer(TROWA_INTERNAL_VERSION_INT));
		json_t *huesJ = json_array();
		json_t *fillHuesJ = json_array();
		json_t *fillOnJ = json_array();
		json_t *linkXYScalesJ = json_array();
		json_t* lissajousJ = json_array();
		json_t *rotModeJ = json_array();
		json_t* gEffectsIxJ = json_array();
		json_t* waveColorJ = json_array();
		json_t* waveFillColorJ = json_array();
		//json_t* waveDoFillJ = json_array();
		for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
		{
			json_array_append_new(huesJ, json_real(waveForms[wIx]->waveHue));
			json_array_append_new(linkXYScalesJ, json_integer((int)waveForms[wIx]->linkXYScales));
			json_array_append_new(lissajousJ, json_integer((int)waveForms[wIx]->lissajous));
			json_array_append_new(rotModeJ, json_integer(waveForms[wIx]->rotMode));

			json_array_append_new(fillHuesJ, json_real(waveForms[wIx]->fillHue));
			json_array_append_new(fillOnJ, json_integer(waveForms[wIx]->doFill));
			json_array_append_new(gEffectsIxJ, json_integer((int)waveForms[wIx]->gEffectIx));

			// itemJ = json_integer((int)waveForms[wIx]->doFill);
			// json_array_append_new(waveDoFillJ, itemJ);

			json_t* colorArr = json_array();
			json_t* fillColorArr = json_array();
			for (int i = 0; i < 3; i++)
			{
				json_array_append_new(colorArr, json_real(waveForms[wIx]->waveColor.rgba[i]));
				json_array_append_new(fillColorArr, json_real(waveForms[wIx]->fillColor.rgba[i]));
			}
			json_array_append_new(waveColorJ, colorArr);
			json_array_append_new(waveFillColorJ, fillColorArr);
		}
		json_object_set_new(rootJ, "hues", huesJ);
		json_object_set_new(rootJ, "fillHues", fillHuesJ);
		json_object_set_new(rootJ, "fillOn", fillOnJ);
		json_object_set_new(rootJ, "linkXYScales", linkXYScalesJ);
		json_object_set_new(rootJ, "lissajous", lissajousJ);
		json_object_set_new(rootJ, "rotMode", rotModeJ);
		json_object_set_new(rootJ, "gEffectsIx", gEffectsIxJ);
		json_object_set_new(rootJ, "waveColor", waveColorJ);
		json_object_set_new(rootJ, "waveFillColor", waveFillColorJ);
		//json_object_set_new(rootJ, "waveDoFill", waveDoFillJ);

		// Background color:
		json_t* bgColorJ = json_array();
		for (int i = 0; i < 3; i++)
		{
			json_array_append_new(bgColorJ, json_real(plotBackgroundColor.rgba[i]));
		}
		json_object_set_new(rootJ, "bgColor", bgColorJ);
//DEBUG("background color saved to json (rgb): %0.2f, %0.2f, %0.2f", plotBackgroundColor.r, plotBackgroundColor.g, plotBackgroundColor.b);
		
		// Widget values (v1 - Widgets do not get json saves
		json_t* widgetJ = json_object();
		json_object_set_new(widgetJ, "width", json_real(widgetWidth));
		json_object_set_new(widgetJ, "showDisplay", json_integer(widgetShowDisplay));
		json_object_set_new(rootJ, "widget", widgetJ);
		return rootJ;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// dataFromJson(void)
	// Load settings.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void dataFromJson(json_t *rootJ) override {		
		json_t *huesJ = json_object_get(rootJ, "hues");
		json_t *rotModeJ = json_object_get(rootJ, "rotMode");
		json_t *linkXYScalesJ = json_object_get(rootJ, "linkXYScales");
		json_t* lissajousJ = json_object_get(rootJ, "lissajous");
		json_t* gEffectsIxJ = json_object_get(rootJ, "gEffectsIx");
		json_t* fillHuesJ = json_object_get(rootJ, "fillHues");
		json_t* fillOnJ = json_object_get(rootJ, "fillOn");
		json_t* waveColorJ = json_object_get(rootJ, "waveColor");
		json_t* waveFillColorJ = json_object_get(rootJ, "waveFillColor");
		// json_t* waveDoFillJ = json_object_get(rootJ, "waveDoFill");

		for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
		{
			json_t* itemJ = json_array_get(huesJ, wIx);
			if (itemJ)
				waveForms[wIx]->setHue((float)json_number_value(itemJ));
			itemJ = NULL;
			itemJ = json_array_get(fillHuesJ, wIx);
			if (itemJ)
				waveForms[wIx]->setFillHue((float)json_number_value(itemJ));
			itemJ = NULL;
			itemJ = json_array_get(fillOnJ, wIx);
			if (itemJ)
				waveForms[wIx]->doFill = (bool)json_integer_value(itemJ);
			itemJ = NULL;

			itemJ = json_array_get(rotModeJ, wIx);
			if (itemJ)
				waveForms[wIx]->rotMode = json_integer_value(itemJ);
			itemJ = NULL;
			itemJ = json_array_get(linkXYScalesJ, wIx);
			if (itemJ)
				waveForms[wIx]->linkXYScales = (bool)json_integer_value(itemJ);
			itemJ = NULL;
			itemJ = json_array_get(lissajousJ, wIx);
			if (itemJ)
				waveForms[wIx]->lissajous = (bool)json_integer_value(itemJ);
			else
				waveForms[wIx]->lissajous = true;
			itemJ = NULL;
			itemJ = json_array_get(gEffectsIxJ, wIx);
			if (itemJ)
				waveForms[wIx]->gEffectIx = (int)clamp((int)json_integer_value(itemJ), 0, TROWA_SCOPE_NUM_EFFECTS - 1);
			itemJ = NULL;

			// itemJ = json_array_get(waveDoFillJ, wIx);
			// if (itemJ)
				// waveForms[wIx]->doFill = (bool)json_integer_value(itemJ);
			// itemJ = NULL;

			json_t* colorArrJ = json_array_get(waveColorJ, wIx);
			json_t* fillColorArrJ = json_array_get(waveFillColorJ, wIx);
			for (int i = 0; i < 3; i++)
			{
				itemJ = json_array_get(colorArrJ, i);
				if (itemJ)
					waveForms[wIx]->waveColor.rgba[i] = (float)json_number_value(itemJ);
				itemJ = NULL;

				itemJ = json_array_get(fillColorArrJ, i);
				if (itemJ)
					waveForms[wIx]->fillColor.rgba[i] = (float)json_number_value(itemJ);
				itemJ = NULL;
			}
			waveForms[wIx]->waveColor.a = 1.0;
			waveForms[wIx]->fillColor.a = 1.0;
		}

		// Background color:
		json_t* bgColorJ = json_object_get(rootJ, "bgColor");
		for (int i = 0; i < 3; i++)
		{
			json_t* itemJ = json_array_get(bgColorJ, i);
			if (itemJ)
				plotBackgroundColor.rgba[i] = (float)(json_number_value(itemJ));
		}
		//INFO("BG COLOR loaded from json (rgb): %0.2f, %0.2f, %0.2f", plotBackgroundColor.r, plotBackgroundColor.g, plotBackgroundColor.b);
		plotBackgroundColor.a = 1.0f;
		
		// Widget values (v1 - Widgets do not get json saves)
		json_t* widgetJ = json_object_get(rootJ, "widget");
		json_t* itemJ = json_object_get(widgetJ, "width");
		widgetWidth = (float)(json_real_value(itemJ));
		itemJ = json_object_get(widgetJ, "showDisplay");
		widgetShowDisplay = (bool)(json_integer_value(itemJ));		
		firstLoad = true;
		return;
	}

	void onReset()  override {
		for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
		{
			waveForms[wIx]->doFill = false;
			waveForms[wIx]->linkXYScales = false; // Added
			waveForms[wIx]->rotMode = false; // Added
			waveForms[wIx]->lissajous = true;
		}
	}
};


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSScopeDisplay
// A top digital display for trowaSoft scope.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSScopeDisplay : TransparentWidget {
	multiScope *module;
	std::shared_ptr<Font> font;
	std::shared_ptr<Font> labelFont;
	int fontSize;
	char messageStr[TROWA_DISP_MSG_SIZE];
	bool visible = true;
	int originalWidth = 240;

	TSScopeDisplay() {
		visible = true;
		fontSize = 11; // Was 12
		for (int i = 0; i < TROWA_DISP_MSG_SIZE; i++)
			messageStr[i] = '\0';
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (visible)
		{
			if (layer == 1)
			{
				bool isPreview = module == NULL; // May have NULL module? Make sure we don't just eat it.

				font = APP->window->loadFont(asset::plugin(pluginInstance, TROWA_DIGITAL_FONT));  	// Rack v2 load font each time
				labelFont = APP->window->loadFont(asset::plugin(pluginInstance, TROWA_LABEL_FONT)); // Rack v2 load font each time

				nvgSave(args.vg);
				Rect b = Rect(Vec(0, 0), box.size);
				nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

				// Default Font:
				nvgFontSize(args.vg, fontSize);
				nvgTextLetterSpacing(args.vg, 1);

				// Background Colors:
				NVGcolor backgroundColor = nvgRGBA(0x20, 0x20, 0x20, 0x80);
				NVGcolor borderColor = nvgRGBA(0x10, 0x10, 0x10, 0x80);

				// Screen:
				nvgBeginPath(args.vg);
				nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
				nvgFillColor(args.vg, backgroundColor);
				nvgFill(args.vg);
				nvgStrokeWidth(args.vg, 1.0);
				nvgStrokeColor(args.vg, borderColor);
				nvgStroke(args.vg);

				NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);

				////////////// Labels /////////////////
				const int yStart = 10;
				int xStart = 10;
				const int dxRotation = 3; // 10
				const int dxEffect = 12;
				int y = yStart;
				int x = xStart;
				int dx = 59; //41 // 37; // 35
				int dy = 16; //14

				//nvgFontSize(args.vg, fontSize); // Small font
				nvgFontFaceId(args.vg, labelFont->handle);
				nvgFillColor(args.vg, textColor);
				// Row Labels 
				nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
				y = yStart + 5;
				for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
				{
					NVGcolor currColor = (isPreview) ? TSColors::COLOR_RED  : module->waveForms[wIx]->waveColor;
					nvgFillColor(args.vg, currColor);
					sprintf(messageStr, "S%d", wIx + 1);
					nvgText(args.vg, 5, y, messageStr, NULL);
					y += dy;
				}
				
				// Column Labels:		
				nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);
				nvgFillColor(args.vg, textColor);

				// Column 1 (Labels)
				y = yStart;
				xStart = 35; // 40
				x = xStart + dx / 2.0;
				nvgText(args.vg, x, y, "X Offset", NULL);

				x += dx;
				nvgText(args.vg, x, y, "X Scale", NULL);

				x += dx;
				nvgText(args.vg, x, y, "Y Offset", NULL);

				x += dx;
				nvgText(args.vg, x, y, "Y Scale", NULL);

				// Rotation (wider)
				x += dx + dxRotation;
				nvgText(args.vg, x, y, "Rotate", NULL);

				// Effect (wider)
				x += (dx + dxEffect)/2.0;
				nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
				nvgText(args.vg, x, y, "Effect", NULL);

				// Values:
				y = yStart + 5;
				NVGcolor absRotColor = TROWA_SCOPE_ABS_ROT_ON_COLOR;
				absRotColor.a = 0.50;
				for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
				{
					// NVGcolor currColor = module->waveForms[wIx]->waveColor;
					nvgTextAlign(args.vg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
					float val = 0.0f;

					// X Offset
					x = xStart + dx / 2.0;
					val = (isPreview) ? 0.0f  : module->params[multiScope::X_POS_PARAM + wIx].getValue();
					sprintf(messageStr, TROWA_SCOPE_ROUND_FORMAT, val);
					nvgText(args.vg, x, y, messageStr, NULL);

					// X Gain
					x += dx;
					val = (isPreview) ? 1.0f  : module->params[multiScope::X_SCALE_PARAM + wIx].getValue();
					sprintf(messageStr, TROWA_SCOPE_ROUND_FORMAT, val);
					nvgText(args.vg, x, y, messageStr, NULL);

					// Y Offset
					x += dx;
					val = (isPreview) ? 0.0f  : module->params[multiScope::Y_POS_PARAM + wIx].getValue();
					sprintf(messageStr, TROWA_SCOPE_ROUND_FORMAT, val);
					nvgText(args.vg, x, y, messageStr, NULL);

					// Y Gain
					x += dx;
					val = (isPreview) ? 1.0f  : module->params[multiScope::Y_SCALE_PARAM + wIx].getValue();
					sprintf(messageStr, TROWA_SCOPE_ROUND_FORMAT, val);
					nvgText(args.vg, x, y, messageStr, NULL);

					// Rotation
					x += dx + dxRotation;
					float v = 0.0;
					if (!isPreview && module->waveForms[wIx]->rotMode)
					{
						// Absolute
						v = (isPreview) ? 0.0  : module->waveForms[wIx]->rotAbsValue;
						// Background:
						nvgBeginPath(args.vg);
						nvgRoundedRect(args.vg, x - dx + 2, y - 2, dx + dxRotation - 2, fontSize + 2, 2);
						nvgFillColor(args.vg, absRotColor);
						nvgFill(args.vg);

						// Text will be black for this
						nvgFillColor(args.vg, TSColors::COLOR_BLACK);
						sprintf(messageStr, "%.1f", v * 180.0 / NVG_PI);
					}
					else
					{
						// Differential
						v = (isPreview) ? 0.0  : module->waveForms[wIx]->rotDiffValue;
						sprintf(messageStr, "%+.1f", v * 180.0 / NVG_PI);
					}
					nvgText(args.vg, x, y, messageStr, NULL);

					// Effect
					x += (dx + dxEffect)/2.0;
					nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
					nvgFillColor(args.vg, textColor);
					int gIx = (isPreview) ? 0  : module->waveForms[wIx]->gEffectIx;
					nvgText(args.vg, x, y, SCOPE_GLOBAL_EFFECTS[gIx]->label, NULL);

					// Advance y to next 
					y += dy;
				} // end loop through wave forms

				nvgResetScissor(args.vg);
				nvgRestore(args.vg);

			} // end if layer == 1			
			this->Widget::drawLayer(args, layer); // Draw children (if we have any)
		}
		return;
	}
}; // end TSScopeDisplay


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiScopeDisplay
// Draws a waveform.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct multiScopeDisplay : TransparentWidget {
	multiScope *module;
	int frame = 0;
	float rot = 0;
	std::shared_ptr<Font> font;
	int wIx = 0; // Waveform index
	
	multiScopeDisplay() {
		//spoutInitSpout();
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// drawWaveform()
	// @args.vg : (IN) NVGcontext
	// @valX: (IN) Pointer to x values.
	// @valY: (IN) Pointer to y values.
	// @rotRate: (IN) Rotation rate in radians
	// @lineThickness: (IN) Line thickness
	// @compositeOp: (IN) Some global effect if any
	// @flipX: (IN) Flip along x (at x=0)
	// @flipY: (IN) Flip along y
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void drawWaveform(const DrawArgs &args, float *valX, float *valY, bool* penOn,
		float rotRate, float lineThickness, NVGcolor lineColor,
		bool doFill, NVGcolor fillColor,
		NVGcompositeOperation compositeOp, bool flipX, bool flipY);
		
		
	void drawLayer(const DrawArgs& args, int layer) override;

	// void drawBackground(const DrawArgs& args) override;


	// void draw(const DrawArgs &args) override {
		// if (module == NULL || !module->initialized)
			// return;
		// float gainX = ((int)(module->params[multiScope::X_SCALE_PARAM + wIx].getValue() * TROWA_SCOPE_ROUND_VALUE)) / (float)(TROWA_SCOPE_ROUND_VALUE);
		// float gainY = ((int)(module->params[multiScope::Y_SCALE_PARAM + wIx].getValue() * TROWA_SCOPE_ROUND_VALUE)) / (float)(TROWA_SCOPE_ROUND_VALUE);
		// float offsetX = ((int)(module->params[multiScope::X_POS_PARAM + wIx].getValue() * TROWA_SCOPE_ROUND_VALUE)) / (float)(TROWA_SCOPE_ROUND_VALUE);
		// float offsetY = ((int)(module->params[multiScope::Y_POS_PARAM + wIx].getValue() * TROWA_SCOPE_ROUND_VALUE)) / (float)(TROWA_SCOPE_ROUND_VALUE);

		// TSWaveform* waveForm = module->waveForms[wIx];
		// float valuesX[BUFFER_SIZE];
		// float valuesY[BUFFER_SIZE];
		// bool penOn[BUFFER_SIZE];
		// float multX = gainX / 10.0;
		// float multY = gainY / 10.0;
		// for (int i = 0; i < BUFFER_SIZE; i++) {
			// int j = i;
			// // Lock display to buffer if buffer update deltaTime <= 2^-11
			// if (waveForm->lissajous)
				// j = (i + waveForm->bufferIndex) % BUFFER_SIZE;
			// valuesX[i] = (waveForm->bufferX[j] + offsetX) * multX;
			// valuesY[i] = (waveForm->bufferY[j] + offsetY) * multY;
			// penOn[i] = waveForm->bufferPenOn[j];
		// }

		// // Draw waveforms
		// // 1. Line Color:
		// NVGcolor waveColor = waveForm->waveColor;
		// waveColor.a = waveForm->waveOpacity;
		// if (waveForm->negativeImage)
			// waveColor = ColorInvertToNegative(waveColor);
		// nvgStrokeColor(args.vg, waveColor); // Color has already been calculated by main module
		// // 2. Fill color:
		// NVGcolor fillColor = waveForm->fillColor;
		// if (waveForm->doFill)
		// {
			// fillColor.a = waveForm->fillOpacity;
			// nvgFillColor(args.vg, fillColor);
		// }
		// // 3. Rotation
		// float rotRate = 0;
		// if (waveForm->rotMode)
		// {
			// // Absolute position:
			// rot = waveForm->rotAbsValue;
		// }
		// else
		// {
			// // Differential rotation
			// rotRate = waveForm->rotDiffValue;
		// }
		// if (waveForm->lissajous) {
			// // X x Y
			// if (module->inputs[multiScope::X_INPUT + wIx].isConnected() || module->inputs[multiScope::Y_INPUT + wIx].isConnected()) {
				// drawWaveform(args, valuesX, valuesY, penOn, rotRate, waveForm->lineThickness, waveColor, waveForm->doFill, fillColor, SCOPE_GLOBAL_EFFECTS[module->waveForms[wIx]->gEffectIx]->compositeOperation, false, false);
			// }
		// }
		// else {
			// // Y
			// if (module->inputs[multiScope::Y_INPUT + wIx].isConnected()) {
				// drawWaveform(args, valuesY, NULL, penOn, rotRate, waveForm->lineThickness, waveColor, waveForm->doFill, fillColor, SCOPE_GLOBAL_EFFECTS[module->waveForms[wIx]->gEffectIx]->compositeOperation, false, false);
			// }
			// // X
			// if (module->inputs[multiScope::X_INPUT + wIx].isConnected()) {
				// drawWaveform(args, valuesX, NULL, penOn, rotRate, waveForm->lineThickness, waveColor, waveForm->doFill, fillColor, SCOPE_GLOBAL_EFFECTS[module->waveForms[wIx]->gEffectIx]->compositeOperation, false, false);
			// }
		// }
		// return;
	// } // end draw()
}; // end multiScopeDisplay

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSScopeLabelArea
// Draw labels on our scope.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSScopeLabelArea : TransparentWidget {
	multiScope *module;
	std::shared_ptr<Font> font;
	int fontSize;
	char messageStr[TROWA_DISP_MSG_SIZE];

	TSScopeLabelArea() {
		fontSize = 10;
		for (int i = 0; i < TROWA_DISP_MSG_SIZE; i++)
			messageStr[i] = '\0';
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// drawColorGrandientArc()
	// @cx : (IN) x-coordinate for center of arc.
	// @cy : (IN) y-coordinate for center of arc.
	// @radius: (IN) radius of arc.
	// @thickness: (IN) Border thickness of arc.
	// @start_radians : (IN) Arc start angle [radians].
	// @end_radians : (IN) Arc end angle [radians].
	// @startHue: (IN) Starting hue.
	// @endHue: (IN) Ending hue.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void drawColorGradientArc(const DrawArgs &args, float cx, float cy, float radius, float thickness, float start_radians, float end_radians, float startHue, float endHue)
	{
		NVGcolor startColor = HueToColorGradient(startHue);
		NVGcolor endColor = HueToColorGradient(endHue);
		nvgBeginPath(args.vg);
		nvgArc(args.vg, /*cx*/ cx, /*cy*/ cy, radius - thickness/2.0,
			/*a0*/ start_radians, /*a1*/ end_radians, /*dir*/ NVG_CW);

		// Creates and returns a linear gradient. Parameters (sx,sy)-(ex,ey) specify the start and end coordinates
		// of the linear gradient, icol specifies the start color and ocol the end color.
		// The gradient is transformed by the current transform when it is passed to nvgFillPaint() or nvgStrokePaint().
		//float circum = radius * (end_radians - start_radians);
		float sx = cx + radius * cos(start_radians);
		float sy = cy + radius * sin(start_radians);
		float ex = cx + radius * cos(end_radians) + 1;
		float ey = cy + radius * sin(end_radians) + 1;
		NVGpaint paint = nvgLinearGradient(args.vg, sx, sy, ex, ey, startColor, endColor);
		nvgStrokeWidth(args.vg, thickness);
		nvgStrokePaint(args.vg, paint);
		nvgStroke(args.vg);
		return;
	} // end drawColorGradientArc()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// drawColorGrandientArc()
	// @cx : (IN) x-coordinate for center of arc.
	// @cy : (IN) y-coordinate for center of arc.
	// @radius: (IN) radius of arc.
	// @thickness: (IN) Border thickness of arc.
	// @start_radians : (IN) Arc start angle [radians].
	// @end_radians : (IN) Arc end angle [radians].
	// @startHue: (IN) Starting hue.
	// @endHue: (IN) Ending hue.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void drawColorGradientArc(const DrawArgs &args, float cx, float cy, float radius, float thickness)
	{
		const float startAngle = 0.67*NVG_PI;
		const float endAngle = 2.33*NVG_PI;
		const int numStops = 20;
		float dH = 1.0 / numStops;
		float dA = (endAngle - startAngle) / numStops;
		float hue = 0;
		float start_radians = startAngle;
		
		while (hue < 1.0)
		{
			drawColorGradientArc(args, cx, cy, radius, thickness, 
				start_radians, start_radians + dA*1.2, 
				hue, hue + dH);
			start_radians += dA;
			hue += dH;
		}
		return;
	} // end drawColorGradientArc()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(const DrawArgs &args) override {
		font = APP->window->loadFont(asset::plugin(pluginInstance, TROWA_LABEL_FONT)); // Rack v2 load font each time

		// Default Font:
		nvgFontSize(args.vg, fontSize);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 1);

		NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
		nvgFillColor(args.vg, textColor);
		nvgFontSize(args.vg, fontSize);

		const int xStart = 17; // 23
		const int yStart = 6; // 10
		int knobOffset = 5;
		int dx = TROWA_SCOPE_CONTROL_DX; // 35
		int dy = TROWA_SCOPE_CONTROL_DY; // 26
		int shapeSpacingY = TROWA_SCOPE_CONTROL_SHAPE_SPACING; // 8 An extra amount between shapes
		int x, y;
		int shapeDy = dy * 3 + knobOffset + shapeSpacingY + 3;

		x = xStart;
		y = yStart;
		for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
		{
			nvgFontSize(args.vg, fontSize);
			x = xStart - 5;
			int waveY = y;
			// Shape Label:		
			nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
			// BG Box
			nvgBeginPath(args.vg);
			nvgRect(args.vg, x - 3, y - 2, TROWA_SCOPE_CONTROL_START_X - x + 7, fontSize + 3); //TROWA_SCOPE_CONTROL_START_X - x + 3
			nvgFillColor(args.vg, textColor);
			nvgFill(args.vg);
			nvgFillColor(args.vg, TSColors::COLOR_BLACK);
			sprintf(messageStr, TROWA_SCOPE_SHAPE_FORMAT_STRING, (wIx + 1));
			nvgText(args.vg, x, y, messageStr, NULL);
			nvgFillColor(args.vg, textColor);

			// Line down lhs:
			nvgBeginPath(args.vg);
			//nvgRect(args.vg, x - 3, y - 2, TROWA_SCOPE_CONTROL_START_X - x + 3, fontSize + 3);
			nvgMoveTo(args.vg, /*x*/ x - 3, /*y*/ y - 2);
			nvgLineTo(args.vg, /*x*/ x - 3, /*y*/ y + shapeDy - 10);
			nvgStrokeWidth(args.vg, 1.0);
			nvgStrokeColor(args.vg, textColor);
			nvgStroke(args.vg);

			// Row Labels:
			x = TROWA_SCOPE_CONTROL_START_X - 5;
			y += fontSize + 8;
			nvgTextAlign(args.vg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
			nvgText(args.vg, x, y, "IN", NULL);
			y += dy;
			nvgText(args.vg, x, y, "OFF", NULL);
			y += dy;
			nvgText(args.vg, x, y, "SCL", NULL);

			const char* colLabels[] = { "X", "Y", "C", "A", "FC", "FA", "R", "T", "TH" };
			nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
			x = TROWA_SCOPE_CONTROL_START_X + 15;
			y = waveY;
			for (int i = 0; i < 9; i++)
			{
				nvgText(args.vg, x, y, colLabels[i], NULL);
				x += dx;
			}

			int x1 = TROWA_SCOPE_CONTROL_START_X + 15 + 0.5*dx;
			int y1 = waveY + dy * 2.3 + 1;
			nvgFontSize(args.vg, fontSize*0.8);
			nvgText(args.vg, x1, y1, "LNK", NULL);

			x1 += 2*dx;
			int y2 = y1 + 9;
			nvgText(args.vg, x1 - 8, y2, "BLANK", NULL);
			y2 += fontSize * 0.8 + 0.5;
			nvgFontSize(args.vg, fontSize*1.05);
			nvgText(args.vg, x1 - 8, y2, "<= 0", NULL);
			// Line to port
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, /*x*/ x1 + 4, /*y*/ y2 + 4); // Near Text
			nvgLineTo(args.vg, /*x*/ TROWA_SCOPE_CONTROL_START_X + 15 + 3*dx, /*y*/ y2 - 5); // Near port
			nvgStrokeWidth(args.vg, 1.0);
			nvgStrokeColor(args.vg, textColor);
			nvgStroke(args.vg);			

			nvgFontSize(args.vg, fontSize*0.8);
			x1 += 1.5*dx; // Fill Color
			nvgText(args.vg, x1+1, y1+10, "FILL", NULL);
			nvgText(args.vg, x1 + 1, y1 + 10 + fontSize * 0.8, "ON", NULL);

			x1 += 2*dx;
			nvgText(args.vg, x1, y1, "ABS", NULL);

			x1 += dx;
			nvgText(args.vg, x1, y1, "X*Y", NULL);

			x1 += dx / 3.0 + 2;
			nvgText(args.vg, x1, y1 + 20, "EFFECT", NULL);


			// Color Knob gradient:
			drawColorGradientArc(args, 
				/*cx*/ TROWA_SCOPE_CONTROL_START_X + 15 + 2*dx,
				/*cy*/ y + dy + 23 + TROWA_SCOPE_COLOR_KNOB_Y_OFFSET, 
				/*radius*/ 14, 
				/*thickness*/ 4.0);

			// Fill Color Knob gradient:
			drawColorGradientArc(args,
				/*cx*/ TROWA_SCOPE_CONTROL_START_X + 15 + 4 * dx,
				/*cy*/ y + dy + 23 + TROWA_SCOPE_COLOR_KNOB_Y_OFFSET,
				/*radius*/ 14,
				/*thickness*/ 4.0);


			y += shapeDy - 3;
		} // end loop through shapes/waveforms
		return;
	} // end draw()
}; // end TSScopeLabelArea

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSScopeSideBarArea
// Draw labels on the RHS bar of our scope.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSScopeSideBarLabelArea : TransparentWidget {
	//multiScope *module;
	std::shared_ptr<Font> font;
	int fontSize;
	TSScopeSideBarLabelArea() {
		fontSize = 10;
	}
	TSScopeSideBarLabelArea(Vec bsize) : TSScopeSideBarLabelArea() {
		this->box.size = bsize;
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(const DrawArgs &args) override {
		font = APP->window->loadFont(asset::plugin(pluginInstance, TROWA_LABEL_FONT)); // Rack v2 load font each time
		nvgSave(args.vg);
		//nvgTranslate(args.vg, -box.size.y / 2.0, -box.size.x / 2.0);
		nvgRotate(args.vg, NVG_PI*0.5);

		// Default Font:
		nvgFontSize(args.vg, fontSize);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 1);

		NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
		nvgFillColor(args.vg, textColor);
		nvgFontSize(args.vg, fontSize);
		nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

		float x, y;
		x = 34;// box.size.x / 2.0;
		y = -box.size.x / 2.0; // 0;// 32;
		nvgText(args.vg, x, y, "INFO", NULL); // Info display btn toggle

#if ENABLE_BG_COLOR_PICKER
		x += 45;
		nvgText(args.vg, x, y, "BG COLOR", NULL); // Info display btn toggle
#endif

		nvgRestore(args.vg);
		return;
	} // end draw()
}; // end TSScopeSideBarLabelArea


#endif // end if use new scope


#endif // end if not defined
