#include <rack.hpp>
using namespace rack;
#include "Widget_multiOscillator.hpp"
#include "Module_multiOscillator.hpp"
//#include "Widget_oscCV.hpp" // Just for the channel colors 
/// TODO: Move channel colors into a common file.
#include "math.hpp"
#include "trowaSoftComponents.hpp"

#define OSC_WIDGET_HEIGHT		115
#define OSC_WIDGET_WIDTH		(49 * RACK_GRID_WIDTH)

// Mix % (0-100) to knob voltage (0-1V).
static float AMMix2KnobVoltage(float mix) {
	return mix / 100.f;
}
// Knob voltage (0-1V) to Mix % (0-100).
static float KnobVoltageAMMix(float knobVal) {
	return knobVal * 100.f;
}

static float AUX2KnobVoltage(float aux) {
	return aux / 10.f;
}
static float KnobVoltage2AUX(float knobVal) {
	return knobVal * 10.f;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiOscillator()
// @thisModule : (IN) Pointer to the multiOscillator module.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
multiOscillatorWidget::multiOscillatorWidget(multiOscillator* thisModule) : TSSModuleWidgetBase(thisModule, true)
{
	// 26 : Sequencer width, so a little wider...
	// was 30, now 43
	box.size = Vec(48 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	bool isPreview = this->module == NULL; // If this is null, then this isn't a real module instance but a 'Preview'?	
	if (!isPreview && thisModule == NULL)
	{
		thisModule = dynamic_cast<multiOscillator*>(this->module);
	}
	this->numberOscillators = (isPreview) ? TROWA_MOSC_DEFAULT_NUM_OSCILLATORS : thisModule->numberOscillators;
	this->numberOutputOscillators = (isPreview) ? TROWA_MOSC_DEFAULT_NUM_OSC_OUTPUTS : thisModule->numOscillatorOutputs;

	//////////////////////////////////////////////
	// Background
	//////////////////////////////////////////////	
	{
		SvgPanel *panel = new SvgPanel();
		panel->box.size = box.size;
		panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/multiOscillator.svg")));
		addChild(panel);
	}

	if (isPreview)
	{
		// Make fake oscillators
		oscillators = new TS_Oscillator[numberOscillators];
		for (int i = 0; i < numberOscillators; i++)
		{
			oscillators[i] = TS_Oscillator(numberOutputOscillators);
		}
	}
	else
	{
		oscillators = thisModule->oscillators;
	}

	//////////////////////////////////////
	// Oscillators
	//////////////////////////////////////
	int y = 15;
	int x = 0;
	TSParamTextField* firstTb = NULL;
	TSParamTextField* lastTb = NULL;
	for (int i = 0; i < numberOscillators; i++)
	{
		TSSingleOscillatorWidget* oscillatorWidget = new TSSingleOscillatorWidget(this, &(oscillators[i]), i + 1);
		oscillatorWidget->box.pos = Vec(x, y);
		oscillatorWidget->box.size.x = box.size.x;
		addChild(oscillatorWidget);
		y += OSC_WIDGET_HEIGHT;

		// Text box tabbing:
		if (numberOscillators > 1)
		{
			// For Prev-Next
			if (firstTb == NULL)
				firstTb = oscillatorWidget->tbAllParamValues[0];
			TSParamTextField* thisfirst = oscillatorWidget->tbAllParamValues[0];
			if (lastTb != NULL)
			{
				lastTb->nextField = thisfirst;
				thisfirst->prevField = lastTb;
			}
			lastTb = oscillatorWidget->tbAllParamValues[static_cast<int>(oscillatorWidget->tbAllParamValues.size()) - 1];
		}
	} // end for
	if (numberOscillators > 1)
	{
		// Text box tabbing:
		if (firstTb != NULL && lastTb != NULL)
		{
			firstTb->prevField = lastTb;
			lastTb->nextField = firstTb;
		}
	}

	// Screws:
	addChild(createWidget<ScrewBlack>(Vec(0, 0)));
	addChild(createWidget<ScrewBlack>(Vec(box.size.x - 15, 0)));
	addChild(createWidget<ScrewBlack>(Vec(0, box.size.y - 15)));
	addChild(createWidget<ScrewBlack>(Vec(box.size.x - 15, box.size.y - 15)));
	
	if (thisModule != NULL)
	{
		thisModule->isInitialized = true;
	}
	return;
} //end multiOscillatorWidget()

multiOscillatorWidget::~multiOscillatorWidget()
{
	if (module == NULL)
	{
		// We make some empty/fake/default oscilators
		delete[] oscillators;
	}
	oscillators = NULL;
	channelWidgets.clear();
	return;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// step()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void multiOscillatorWidget::step()
{
	if (module == NULL)
		return;
	TSSModuleWidgetBase::step();
	return;
} // end multiOscillator::step()



//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// draw()
// A single oscillator info.
// @args : (IN) Draw args and NVGcontext to draw on
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSingleOscillatorDisplay::draw(/*in*/ const DrawArgs &args) {
	if (showBackground)
	{
		// Background Colors:
		NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
		NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
		// Screen:
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
		nvgFillColor(args.vg, backgroundColor);
		nvgFill(args.vg);
		nvgStrokeWidth(args.vg, 1.0);
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);
	}

	if (!showDisplay)
		return;

	NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
	nvgFillColor(args.vg, textColor);
	nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
	float largeFontSize = fontSize * 1.15f;
	float largeSpacing = 1.5;
	float spacing = 2.5;

	const int padding = 5;
	float dx = (box.size.x - padding * 2) / numTextBoxes;
	float x = padding + dx / 2.f;
	float y = padding * 2;
	//float y2 = box.size.y - fontSize - 4;
	float y2 = box.size.y - fontSize - 8;
	for (int i = 0; i < numTextBoxes; i++)
	{
		nvgFillColor(args.vg, textColor); // Value is white
		std::string label = std::string(labels[i]);
		if (!this->textBoxes[i]->visible) // If the text box is visible, then don't draw the values.
		{
			// Large Font:
			nvgFontSize(args.vg, largeFontSize);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, largeSpacing);
			// Show value
			float val = (this->textBoxes[i]->text.length() > 0) ? std::stof(this->textBoxes[i]->text, NULL) : 0.0f;
			if (val >= 1000 && label.compare("FREQ (Hz)") == 0)
			{
				if (val >= 1000)
				{
					val /= 1000;
					label = std::string("FREQ (kHz)");
				}
			}
			sprintf(messageStr, this->textBoxes[i]->formatString, val);
			nvgText(args.vg, x, y, messageStr, NULL);
		}

		// Label Font:
		nvgFillColor(args.vg, parentWidget->oscillatorColor);
		nvgFontSize(args.vg, fontSize); // Small font
		nvgFontFaceId(args.vg, labelFont->handle);
		nvgTextLetterSpacing(args.vg, spacing);
		nvgText(args.vg, x, y2, label.c_str(), NULL);
		if (i == phaseShiftIx)
		{
			// Make a damn degree symbol (our font doesn't have one apparently)
			float txtBounds[4];
			float nextX = nvgTextBounds(args.vg, 0, 0, "PHASE (", NULL, txtBounds);
			nvgFontSize(args.vg, fontSize * 0.6f);
			nvgText(args.vg, x + nextX / 2.f - 1.25f, y2, "o", NULL); // make a tiny o
		}
		x += dx;
	} // end loop
	return;
} // end draw()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSingleOscillatorWidget()
// @parentWidget: (IN) Parent MODULE widget.
// @osc : (IN) Pointer to the oscillator this widget represents.
// @num : (IN) The oscillator number.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
TSSingleOscillatorWidget::TSSingleOscillatorWidget(multiOscillatorWidget* parentWidget, TS_Oscillator* osc, int num)
{
	box.size = Vec(OSC_WIDGET_WIDTH, OSC_WIDGET_HEIGHT);
	multiOscillator* thisModule = dynamic_cast<multiOscillator*>(parentWidget->module);
	this->oscillator = osc;
	//this->parentWidget = parentWidget;
	this->oscillatorNumber = num;	
	int oscIx = oscillatorNumber - 1;
	bool plugLightsEnabled = parentWidget->plugLightsEnabled;
	
	int numOscillatorOutputs = osc->numOutputWaveForms;
	this->baseParamId = multiOscillator::ParamIds::OSC_PARAM_START + oscIx * (TS_Oscillator::BaseParamIds::OSCWF_NUM_PARAMS + numOscillatorOutputs * TS_OscillatorOutput::BaseParamIds::OUT_NUM_PARAMS);
	this->baseInputId = multiOscillator::InputIds::OSC_INPUT_START + oscIx * (TS_Oscillator::BaseInputIds::OSCWF_NUM_INPUTS + numOscillatorOutputs * TS_OscillatorOutput::BaseInputIds::OUT_NUM_INPUTS);
	this->baseOutputId = multiOscillator::OutputIds::OSC_OUTPUT_START + oscIx * (TS_Oscillator::BaseOutputIds::OSCWF_NUM_OUTPUTS + numOscillatorOutputs * TS_OscillatorOutput::BaseOutputIds::OUT_NUM_OUTPUTS);
	this->baseLightId = multiOscillator::LightIds::OSC_LIGHT_START + oscIx * (TS_Oscillator::BaseLightIds::OSCWF_NUM_LIGHTS + numOscillatorOutputs * TS_OscillatorOutput::BaseLightIds::OUT_NUM_LIGHTS);

	// int colorIx = (oscIx * (1 + numOscillatorOutputs)) % TROWA_OSCCV_NUM_COLORS;
	// oscillatorColor = oscCVWidget::CHANNEL_COLORS[colorIx];
	int colorIx = (oscIx * (numOscillatorOutputs)) % TSColors::NUM_CHANNEL_COLORS;// TROWA_OSCCV_NUM_COLORS;
	oscillatorColor = nvgRGB(0xee, 0xee, 0xee);

	//-------------------------
	// Display - In center
	//-------------------------
	this->oscillatorDisplay = new TSSingleOscillatorDisplay();
	this->oscillatorDisplay->box.size = Vec(365, 45);
	this->oscillatorDisplay->box.pos = Vec(screenStartX, 5);
	this->oscillatorDisplay->module = thisModule;
	this->oscillatorDisplay->parentWidget = this;
	addChild(oscillatorDisplay);

	//-------------------------
	// User controls (params) & Inputs:
	// (Left)
	//-------------------------
	NVGcolor thisColor = TSColors::COLOR_WHITE;
	int xStart = 15;
	int yStart = 5;
	int dx = 60;
	const int numKnobVals = 4;
	// float knobVals[][3] = {
		// { MOSC_AMPLITUDE_MIN_V, MOSC_AMPLITUDE_MAX_V, MOSC_AMPLITUDE_DEFAULT_V }, // Amplitude
		// { TROWA_MOSC_F_KNOB_MIN_V, TROWA_MOSC_F_KNOB_MAX_V, multiOscillator::Frequency2KnobVoltage(MOSC_FREQ_DEFAULT_HZ) }, // Frequency
		// { TROWA_MOSC_KNOB_MIN_V, TROWA_MOSC_KNOB_MAX_V, multiOscillator::PhaseShift2KnobVoltage(MOSC_PHASE_SHIFT_DEFAULT_DEG) }, // Phi
		// { MOSC_OFFSET_MIN_V, MOSC_OFFSET_MAX_V, MOSC_OFFSET_DEFAULT_V } // Offset
	// };
	bool isNormalKnob[4] = { true, false, true, true };
	const char* formatStrs[numKnobVals] = { "%0.3f", "%0.3f", "%0.3f", "%0.3f" };
	Knob* knobPtr = NULL;
	TS_Port* port = NULL;
	NVGcolor mainColor = TSColors::COLOR_TS_GRAY;
	Vec tbSize = Vec(65, 22);
	int x = xStart;
	int y = yStart;
	int tbDy = 5;
	for (int paramId = 0; paramId < numKnobVals; paramId++)
	{
		// Input CV Ports
		port = TS_createInput<TS_Port>(Vec(x - 5, y+5), thisModule, baseInputId + paramId, !plugLightsEnabled, thisColor);
		addChild(port);
		parentWidget->inputs.push_back(port);

		// Input Knobs
		int xKnob = x + 30;
		int yKnob = y + 10;
		if (isNormalKnob[paramId]) {
			// Knob (normal)
			knobPtr = dynamic_cast<TS_TinyBlackKnob*>(createParam<TS_TinyBlackKnob>(Vec(xKnob, yKnob), thisModule, /*id*/ baseParamId + paramId));
		}
		else {
			// Encoder
			TS_20_BlackEncoder* tmp = dynamic_cast<TS_20_BlackEncoder*>(createParam<TS_20_BlackEncoder>(Vec(xKnob, yKnob), thisModule, /*id*/ baseParamId + paramId));	
			knobPtr = tmp;
			tmp->setRotationAmount(20.f);
DEBUG("Added Encoder Knob - Param Id %d", baseParamId + paramId);			
		}
		addChild(knobPtr);
		parentWidget->params.push_back(knobPtr);

		// Input Text Field
		TSParamTextField* knobTextBox = new TSParamTextField(TSTextField::TextType::RealNumberOnly, /*maxLength*/ 30, /*control*/ knobPtr, /*formatStr*/ formatStrs[paramId]);
		knobTextBox->box.pos = Vec(screenStartX + 15 + paramId*88.75, y + tbDy);
		knobTextBox->box.size = tbSize;
		knobTextBox->borderColor = mainColor;
		knobTextBox->visible = false;
		knobTextBox->tabNextHiddenAction = TSTextField::TabFieldHiddenAction::ShowHiddenTabToField;
		knobTextBox->autoHideMode = TSParamTextField::AutoHideMode::AutoHideOnDefocus;
		addChild(knobTextBox);
		tbParamValues.push_back(knobTextBox);
		oscillatorDisplay->textBoxes[paramId] = knobTextBox;
		// Collect all textboxes
		tbAllParamValues.push_back(knobTextBox);

		x += dx;
	} // end for
	// Set the translators for the text boxes:
	int baseTextBoxIx = 0;
#if TROWA_MOSC_FREQ_KNOB_NEEDS_CONVERSION
	tbParamValues[baseTextBoxIx + TS_Oscillator::BaseParamIds::OSCWF_FREQUENCY_PARAM]->knob2TextVal = multiOscillator::KnobVoltage2Frequency;
	tbParamValues[baseTextBoxIx + TS_Oscillator::BaseParamIds::OSCWF_FREQUENCY_PARAM]->text2KnobVal = multiOscillator::Frequency2KnobVoltage;
#endif
	tbParamValues[baseTextBoxIx + TS_Oscillator::BaseParamIds::OSCWF_PHASE_SHIFT_PARAM]->knob2TextVal = multiOscillator::KnobVoltage2PhaseShift;
	tbParamValues[baseTextBoxIx + TS_Oscillator::BaseParamIds::OSCWF_PHASE_SHIFT_PARAM]->text2KnobVal = multiOscillator::PhaseShift2KnobVoltage;
	oscillatorDisplay->showDisplay = true;

	//------------------------------------------
	// Right : Outputs and Sync
	//------------------------------------------
	// Sync In and Out
	dx = 30;
	x = screenStartX + screenWidth + outPortOffsetX;
	y = yStart + 5;
	port = TS_createInput<TS_Port>(Vec(x, y), thisModule, baseInputId + TS_Oscillator::BaseInputIds::OSCWF_SYNC_INPUT, !plugLightsEnabled, thisColor);
	addChild(port);
	parentWidget->inputs.push_back(port);
	// Output:
	x += dx;
	port = TS_createOutput<TS_Port>(Vec(x, y), thisModule, baseOutputId + TS_Oscillator::BaseOutputIds::OSCWF_SYNC_OUTPUT, !plugLightsEnabled, thisColor);
	addChild(port);
	parentWidget->outputs.push_back(port);

	//-----------------------------------------
	// Channels
	// (below)
	//-----------------------------------------
	xStart = 15;	
	x = xStart;
	y = 35;
	int bParamId = baseParamId + TS_Oscillator::BaseParamIds::OSCWF_NUM_PARAMS;
	int bInputId = baseInputId + TS_Oscillator::BaseInputIds::OSCWF_NUM_INPUTS;
	int bOutputId = baseOutputId + TS_Oscillator::BaseOutputIds::OSCWF_NUM_OUTPUTS;
	int bLightId = baseLightId + TS_Oscillator::BaseLightIds::OSCWF_NUM_LIGHTS;

	for (int ch = 0; ch < numOscillatorOutputs; ch++)
	{		
		TSOscillatorChannelWidget* chWidget = new TSOscillatorChannelWidget(parentWidget, this, 
			Vec(0, y), ch + 1, oscillatorColor,
			bInputId, bParamId, bOutputId, bLightId, &( oscillator->outputWaveforms[ch] ));
		addChild(chWidget);
		channelWidgets.push_back(chWidget);

		for (int t = 0; t < static_cast<int>(chWidget->tbParamValues.size()); t++)
		{
			tbAllParamValues.push_back(chWidget->tbParamValues[t]);
		}

		y += 35;
		bParamId += TS_OscillatorOutput::BaseParamIds::OUT_NUM_PARAMS;
		bInputId += TS_OscillatorOutput::BaseInputIds::OUT_NUM_INPUTS;
		bOutputId += TS_OscillatorOutput::BaseOutputIds::OUT_NUM_OUTPUTS;
		bLightId += TS_OscillatorOutput::BaseLightIds::OUT_NUM_LIGHTS;
		colorIx = (colorIx + 1) % TSColors::NUM_CHANNEL_COLORS; //TROWA_OSCCV_NUM_COLORS;
	}

	//--------------------------------------------
	// Tabbing for text boxes
	//--------------------------------------------
	for (int t = 0; t < static_cast<int>(tbAllParamValues.size()); t++)
	{
		if (t > 0)
			tbAllParamValues[t]->prevField = tbAllParamValues[t - 1];
		else
			tbAllParamValues[t]->prevField = tbAllParamValues[static_cast<int>(tbAllParamValues.size()) - 1];
		if (t < static_cast<int>(tbAllParamValues.size()) - 1)
			tbAllParamValues[t]->nextField = tbAllParamValues[t + 1];
		else
			tbAllParamValues[t]->nextField = tbAllParamValues[0];
	}
	return;
} // end TSSingleOscillatorWidget()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// draw()
// @args : (IN) Draw args and NVGcontext to draw on
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSingleOscillatorWidget::draw(/*in*/ const DrawArgs &args)
{
	// Draw screen background
	// Background Colors:
	NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
	NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
	// Screen:
	nvgBeginPath(args.vg);
	nvgRoundedRect(args.vg, screenStartX, screenStartY, screenWidth, box.size.y - screenStartY, 5.0);
	nvgFillColor(args.vg, backgroundColor);
	nvgFill(args.vg);
	nvgStrokeWidth(args.vg, 1.0);
	nvgStrokeColor(args.vg, borderColor);
	nvgStroke(args.vg);

	this->Widget::draw(args);
	return;
} // end TSSingleOscillatorWidget::draw()


/**
Called when a mouse button is pressed over this widget
0 for left, 1 for right, 2 for middle.
Return `this` to accept the event.
Return NULL to reject the event and pass it to the widget behind this one.
*/
//void TSOscillatorChannelDisplayWidget::onMouseDown(const event::MouseDown &e) {
void TSOscillatorChannelDisplayWidget::onButton(const event::Button &e)  {	
	if (parentWidget == NULL || parentWidget->parentWidget->oscillatorDisplay->module == NULL)
		return;
	if (showDisplay) {		
		//		if (e.button == 0 && e.pos.y >= yTbStart && e.pos.y < yTbEnd)
		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT
			&& e.pos.y >= yTbStart && e.pos.y < yTbEnd)
		{
			// Left click, check position, find which text box this would go to.
			int txtBoxIx = -1;
			const int padding = 5;
			float dx = (box.size.x - padding * 2) / numFields;
			float x1 = padding;
			int i = 0;
			while (i < numFields && txtBoxIx < 0)
			{
				float x2 = x1 + dx;
				if (e.pos.x >= x1 && e.pos.x < x2 && hasTextBox[i]) {
					if (i == 1)
					{
						// For now, only show textbox on P.Width
						if (parentWidget->oscillatorOutput->waveFormType == WaveFormType::WAVEFORM_SQR)
							txtBoxIx = i;
					}
					else
					{
						txtBoxIx = i;
					}
				}
				x1 += dx;
				i++;
			}
			if (txtBoxIx > -1 && !textBoxes[txtBoxIx]->visible)
			{
//DEBUG("TSOscillatorChannelDisplayWidget - Showing text box %d.", txtBoxIx);				
				// Show the text box:
				textBoxes[txtBoxIx]->visible = true;
				//e.target = textBoxes[txtBoxIx];
				//e.consumed = true;
				e.consume(textBoxes[txtBoxIx]);	
				textBoxes[txtBoxIx]->requestFocus();	
			}
		} // end if left click
	} // end if visible
	return;
} // end TSOscillatorChannelDisplayWidget::onMouseDown()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// draw()
// A single oscillator info.
// @args : (IN) Draw args and NVGcontext to draw on
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscillatorChannelDisplayWidget::draw(/*in*/ const DrawArgs &args)
{
	if (showBackground)
	{
		// Background Colors:
		NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
		NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
		// Screen:
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
		nvgFillColor(args.vg, backgroundColor);
		nvgFill(args.vg);
		nvgStrokeWidth(args.vg, 1.0);
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);
	}
	if (!showDisplay)
		return;

	NVGcolor textColor = parentWidget->channelColor;// nvgRGB(0xee, 0xee, 0xee);
	NVGcolor numColor = nvgRGB(0xee, 0xee, 0xee);
	
	nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
	float largeFontSize = fontSize * 1.15f;
	float largeSpacing = 1.5;
	float spacing = 2.5;

	const int padding = 5;
	float dx = (box.size.x - padding * 2) / numFields;
	float x = padding + dx / 2.f;
	float y = padding * 2;
	float y2 = box.size.y - fontSize - 8;

	try 
	{
		for (int i = 0; i < numFields; i++)
		{
			std::string label = std::string(labels[i]);

			//---- VALUE -----
			// Large Font:
			nvgFillColor(args.vg, numColor);
			nvgFontSize(args.vg, largeFontSize);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, largeSpacing);
			if (hasTextBox[i] && (i != 1)) 
			{
				if (!this->textBoxes[i]->visible) // If the text box is visible, then don't draw the values.
				{
					// Show value
					float val = (this->textBoxes[i]->text.length() > 0) ? std::stof(this->textBoxes[i]->text, NULL) : 0.0f;
					sprintf(messageStr, this->textBoxes[i]->formatString, val);
					nvgText(args.vg, x, y, messageStr, NULL);					
				}
			}
			else
			{
				WaveFormType wType = (parentWidget->oscillatorOutput == NULL) ? WaveFormType::WAVEFORM_SIN : parentWidget->oscillatorOutput->waveFormType;
				switch (i)
				{
				case 0:
					// Waveform
					nvgText(args.vg, x, y, multiOscillator::WaveFormAbbr[wType], NULL);
					break;
				case 1:
					// AUX
					switch (wType)
					{
					case WaveFormType::WAVEFORM_SQR:
						label = std::string("P WIDTH");
						sprintf(messageStr, "%5.2f", parentWidget->oscillatorOutput->auxParam_norm*100);
						nvgText(args.vg, x, y, messageStr, NULL);
						break;
					case WaveFormType::WAVEFORM_SAW:
						label = std::string("SLOPE");
						if (parentWidget->oscillatorOutput->getRampSlope())
							nvgText(args.vg, x, y, "POS(+)", NULL);
						else
							nvgText(args.vg, x, y, "NEG(-)", NULL);
						break;
					case WaveFormType::WAVEFORM_SIN:
					case WaveFormType::WAVEFORM_TRI:
						nvgText(args.vg, x, y, "N/A", NULL);
						break;
					default:
						break;
					}
					break;
				}
			}

			//---- Label ----
			if (i == amodIx)
			{
				if (parentWidget->oscillatorOutput->amRingModulation)
					label += " (RNG)";
				else
					label += " (DIG)";
			}
			// Label Font:
			nvgFillColor(args.vg, textColor);
			nvgFontSize(args.vg, fontSize); // Small font
			nvgFontFaceId(args.vg, labelFont->handle);
			nvgTextLetterSpacing(args.vg, spacing);
			nvgText(args.vg, x, y2, label.c_str(), NULL);
			switch (i)
			{
			case phaseShiftIx:
			{
				// Make a damn degree symbol (our font doesn't have one apparently)
				float txtBounds[4];
				float nextX = nvgTextBounds(args.vg, 0, 0, "PHASE (", NULL, txtBounds);
				nvgFontSize(args.vg, fontSize * 0.6f);
				nvgText(args.vg, x + nextX / 2.f - 1.25f, y2, "o", NULL); // make a tiny o
			}
				break;
			default:
				break;
			} // end switch
			x += dx;
		} // end loop
	}
	catch (std::exception &ex)
	{
		WARN("Error in channel display: %s.", ex.what());
	}
	return;
} // end TSOscillatorChannelDisplayWidget::draw()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSOscillatorChannelWidget()
// Widget for all ports and parameters for a channel output.
// @bInputId : (IN) Base input id for channel.
// @bParamId : (IN) Base param id for channel.
// @bOutputId : (IN) Base output id for channel.
// @bLightId : (IN) Base light id for channel.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
TSOscillatorChannelWidget::TSOscillatorChannelWidget(multiOscillatorWidget* parentModuleWidget, TSSingleOscillatorWidget* parentOscWidget,
	Vec location,
	int chNumber, NVGcolor chColor, int bInputId, int bParamId, int bOutputId, int bLightId, TS_OscillatorOutput* oscOutput)
{
	labelFont = APP->window->loadFont(asset::plugin(pluginInstance, TROWA_LABEL_FONT));
	box.size = Vec(OSC_WIDGET_WIDTH, 50);
	box.pos = location;
	this->parentWidget = parentOscWidget;
	//this->parentModuleWidget = parentModuleWidget;
	multiOscillator* thisModule = NULL;
	if (parentModuleWidget->module != NULL)
		thisModule = dynamic_cast<multiOscillator*>(parentModuleWidget->module);
	baseInputId = bInputId;
	baseParamId = bParamId;
	baseOutputId = bOutputId;
	baseLightId = bLightId;
	oscillatorOutput = oscOutput;
	channelNumber = chNumber;
	channelColor = chColor;
	bool plugLightsEnabled = parentModuleWidget->plugLightsEnabled;

	//-------------------------
	// Display - In center
	//-------------------------
	TSOscillatorChannelDisplayWidget* display = new TSOscillatorChannelDisplayWidget();
	display->box.size = Vec(365, 45);
	display->box.pos = Vec(parentWidget->screenStartX, 5);
	//display->module = thisModule;
	//display->parentModuleWidget = parentModuleWidget;
	display->parentWidget = this;
	addChild(display);

	//-------------------------
	// User controls (params) & Inputs:
	// (Left)
	//-------------------------
	int xStart = 15;
	int yStart = 5;
	int dx = 60;
	const int numKnobVals = 4;
	// float knobVals[][3] = {
		// { TROWA_MOSC_KNOB_MIN_V, TROWA_MOSC_KNOB_MAX_V,  TROWA_MOSC_KNOB_MIN_V }, // Waveform Type
		// { TROWA_MOSC_KNOB_AUX_MIN_V, TROWA_MOSC_KNOB_AUX_MAX_V, 0.5f*(TROWA_MOSC_KNOB_AUX_MIN_V+ TROWA_MOSC_KNOB_AUX_MAX_V) }, // Aux
		// { TROWA_MOSC_KNOB_MIN_V, TROWA_MOSC_KNOB_MAX_V, multiOscillator::PhaseShift2KnobVoltage(MOSC_PHASE_SHIFT_DEFAULT_DEG) }, // Phi
		// { TROWA_MOSC_MIX_MIN_V, TROWA_MOSC_MIX_MAX_V,  TROWA_MOSC_MIX_DEF_V } // Mod Mix
	// };
	bool hasTextBox[] = { false, true, true, true };
	const char* formatStrs[numKnobVals] = { "%0.0f", "%0.2f", "%0.2f", "%0.2f" };
	TS_TinyBlackKnob* knobPtr = NULL;
	TS_Port* port = NULL;
	NVGcolor mainColor = TSColors::COLOR_TS_GRAY;
	Vec tbSize = Vec(65, 22);
	int x = xStart;
	int y = yStart;
	int tbDy = 5;
	for (int paramId = 0; paramId < numKnobVals; paramId++)
	{
		// Input CV Ports
		port = TS_createInput<TS_Port>(Vec(x - 5, y + 5), thisModule, baseInputId + paramId, !plugLightsEnabled, channelColor);
		addChild(port);
		parentModuleWidget->inputs.push_back(port);

		// Input Knobs
		int xKnob = x + 30;
		int yKnob = y + 10;
		//knobPtr = dynamic_cast<TS_TinyBlackKnob*>(createParam<TS_TinyBlackKnob>(Vec(xKnob, yKnob), thisModule, /*id*/ baseParamId + paramId, /*min*/ knobVals[paramId][0], /*max*/ knobVals[paramId][1], /*def*/ knobVals[paramId][2]));
		knobPtr = dynamic_cast<TS_TinyBlackKnob*>(createParam<TS_TinyBlackKnob>(Vec(xKnob, yKnob), thisModule, /*id*/ baseParamId + paramId));		
		addChild(knobPtr);
		parentModuleWidget->params.push_back(knobPtr);

		if (hasTextBox[paramId])
		{
			// Input Text Field
			TSParamTextField* knobTextBox = new TSParamTextField(TSTextField::TextType::RealNumberOnly, /*maxLength*/ 30,
				/*control*/ knobPtr, /*formatStr*/ formatStrs[paramId]);
			
			knobTextBox->box.pos = Vec(parentOscWidget->screenStartX + 15 + paramId * 88.75, y + tbDy);
			knobTextBox->box.size = tbSize;
			knobTextBox->borderColor = mainColor;
			knobTextBox->visible = false;
			knobTextBox->tabNextHiddenAction = TSTextField::TabFieldHiddenAction::ShowHiddenTabToField;
			knobTextBox->autoHideMode = TSParamTextField::AutoHideMode::AutoHideOnDefocus;
			addChild(knobTextBox);
			tbParamValues.push_back(knobTextBox);
			display->textBoxes[paramId] = knobTextBox;

			// Translators:
			switch (paramId)
			{
			case TS_OscillatorOutput::BaseParamIds::OUT_AUX_PARAM:
				knobTextBox->knob2TextVal = KnobVoltage2AUX;
				knobTextBox->text2KnobVal = AUX2KnobVoltage;
				break;
			case TS_OscillatorOutput::BaseParamIds::OUT_PHASE_SHIFT_PARAM:
				knobTextBox->knob2TextVal = multiOscillator::KnobVoltage2PhaseShift;
				knobTextBox->text2KnobVal = multiOscillator::PhaseShift2KnobVoltage;
				break;
			case TS_OscillatorOutput::BaseParamIds::OUT_AM_MIX_PARAM:
				knobTextBox->knob2TextVal = KnobVoltageAMMix;
				knobTextBox->text2KnobVal = AMMix2KnobVoltage;
				break;
			default:
				break;
			}
		}

		x += dx;
	} // end for
	display->showDisplay = true;
	//------------------------------------------
	// Button for RNG or DIG
	//------------------------------------------
	const int size = 13;
	Vec ledBtnSize = Vec(size, size); // LED button size
	//Vec ledSize = Vec(size-2, size-2); // LED size
	x += -7;
	//LEDButton* btn = dynamic_cast<LEDButton*>(createParam<LEDButton>(Vec(x, y + 11), thisModule, baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AM_TYPE_PARAM, 0, 1, 0));
	TS_LEDButton* btn = dynamic_cast<TS_LEDButton*>(createParam<TS_LEDButton>(Vec(x, y + 11), thisModule, baseParamId + TS_OscillatorOutput::BaseParamIds::OUT_AM_TYPE_PARAM));	
	//btn->box.size = ledBtnSize;
	btn->setSize(ledBtnSize);
	addChild(btn);
	parentModuleWidget->params.push_back(btn);
	ColorValueLight* light = TS_createColorValueLight<ColorValueLight>(Vec(x + 2.5, y + 13.5), thisModule,
		baseLightId + TS_OscillatorOutput::BaseLightIds::OUT_AM_MODE_LED, ledBtnSize, TSColors::COLOR_WHITE);
	addChild(light);


	//------------------------------------------
	// Right : Outputs
	//------------------------------------------
	// * RAW * signal
	x = parentOscWidget->screenStartX + parentOscWidget->screenWidth + parentOscWidget->outPortOffsetX;
	y = yStart + 5;
	//INFO(" >> Raw Signal id %d", baseOutputId + TS_OscillatorOutput::BaseOutputIds::OUT_RAW_SIGNAL);
	port = TS_createOutput<TS_Port>(Vec(x, y), thisModule, baseOutputId + TS_OscillatorOutput::BaseOutputIds::OUT_RAW_SIGNAL, !plugLightsEnabled, chColor);
	addChild(port);
	parentModuleWidget->outputs.push_back(port);
	// * Multiplied * signal
	x += 30;
	//INFO(" >> Mod Signal id %d", baseOutputId + TS_OscillatorOutput::BaseOutputIds::OUT_MULTIPLIED_SIGNAL);
	port = TS_createOutput<TS_Port>(Vec(x, y), thisModule, baseOutputId + TS_OscillatorOutput::BaseOutputIds::OUT_MULTIPLIED_SIGNAL, !plugLightsEnabled, chColor);
	addChild(port);
	parentModuleWidget->outputs.push_back(port);
	return;
} // end TSOscilatorChannelWidget()


