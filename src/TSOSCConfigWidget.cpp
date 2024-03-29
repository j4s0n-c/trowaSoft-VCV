#include <rack.hpp>
using namespace rack;
#include "asset.hpp"
#include "componentlibrary.hpp"
#include "plugin.hpp"
#include "TSOSCConfigWidget.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSOSCCommon.hpp"
#include <string>
#include "trowaSoftComponents.hpp"
//#include "Module_oscCV.hpp"

#define START_Y   15

/// TODO: DISABLE WHEN NOT VISIBLE


void TSOSCClientItem::onAction(const event::Action &e) {
	parentButton->selectedOSCClient = this->oscClient;
	return;
}

TSOSCClientSelectBtn::TSOSCClientSelectBtn() {
	fontPath = asset::plugin(pluginInstance, TROWA_MONOSPACE_FONT); // Rack v2 store font path
	fontSize = 14.0f;
	backgroundColor = FORMS_DEFAULT_BG_COLOR;
	color = FORMS_DEFAULT_TEXT_COLOR;
	textOffset = Vec(5, 3);
	borderWidth = 1;
	borderColor = FORMS_DEFAULT_BORDER_COLOR;
	return;
}

// On button click, create drop down menu.
void TSOSCClientSelectBtn::onAction(const event::Action &e) {
	if (visible)
	{
		Menu *menu = createMenu();// gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y)).round();
		menu->box.size.x = box.size.x;
		for (unsigned int i = 0; i < OSCClient::NUM_OSC_CLIENTS; i++) {
			TSOSCClientItem *option = new TSOSCClientItem(this);
			option->oscClient = static_cast<OSCClient>(i);
			option->text = OSCClientStr[i];
			menu->addChild(option);
		}
	}
	return;
}

void TSOSCClientSelectBtn::step() {
	text = string::ellipsize(OSCClientStr[selectedOSCClient], 15);
}

void TSOSCClientSelectBtn::drawControl(const DrawArgs &args)
{
	// v2: Load font reference every draw call
	std::shared_ptr<Font> font = APP->window->loadFont(fontPath); // Rack v2 load font each time
	
	nvgScissor(args.vg, 0, 0, box.size.x, box.size.y);

	// Background
	nvgBeginPath(args.vg);
	nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 5.0);
	nvgFillColor(args.vg, backgroundColor);
	nvgFill(args.vg);

	// Border
	if (borderWidth > 0) {
		nvgStrokeWidth(args.vg, borderWidth);
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);
	}
	nvgResetScissor(args.vg);

	if (font->handle >= 0) {

		nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);	
		nvgScissor(args.vg, 0, 0, box.size.x - 5, box.size.y);

		nvgFillColor(args.vg, color);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 0.0);

		nvgFontSize(args.vg, fontSize);
		nvgText(args.vg, textOffset.x, textOffset.y, text.c_str(), NULL);

		nvgResetScissor(args.vg);

		bndUpDownArrow(args.vg, box.size.x - 10,  10, 5, color);
	}
	return;
}

void TSOSCClientSelectBtn::drawLayer(const DrawArgs &args, int layer) {
	if (visible)
	{
		if (layer == 1)
		{
			drawControl(args);
		}
		//this->Widget::drawLayer(args, layer);
	}
	return;
}
void TSOSCClientSelectBtn::draw(const DrawArgs &args) {
	if (visible)
	{
		drawControl(args);
		
		//this->Widget::draw(args);
	}
	return;
}



TSOSCConfigWidget::TSOSCConfigWidget(Module* mod, int btnSaveId, int btnAutoReconnectId, OSCClient selectedClient) : TSOSCConfigWidget(mod, btnSaveId, btnAutoReconnectId, selectedClient, "", 1000, 1001)
{
	return;
}
TSOSCConfigWidget::TSOSCConfigWidget(Module* mod, int btnSaveId, int btnAutoReconnectId, std::string ipAddress, uint16_t txPort, uint16_t rxPort, 
	bool showClient, OSCClient selectedClient, bool showNamespace, std::string oscNamespace)
{
	fontPath = asset::plugin(pluginInstance, TROWA_LABEL_FONT); // Rack v2 store font path
	this->module = mod;
	
	this->showClientSelect = showClient;
	this->showNamespace = showNamespace;

	statusMsg2 = "";

	this->box.size = Vec(400, 50);
	int height = 20;
	visible = true;
	int x, y;
	int dx = 4;
	int i = 0;
	x = 6;
	y = START_Y;
	tbIpAddress = new TSTextField(TSTextField::TextType::IpAddress, 15);
	tbIpAddress->box.size = Vec(105, height); //115
	tbIpAddress->box.pos = Vec(x, y);
	tbIpAddress->visible = visible;
	tbIpAddress->text = ipAddress;
	tbIpAddress->placeholder = "127.0.0.1";
	tbIpAddress->id = i;
	addChild(tbIpAddress);
	textBoxes[i++] = tbIpAddress;

	x += tbIpAddress->box.size.x + dx;
	tbTxPort = new TSTextField(TSTextField::TextType::DigitsOnly, 5);
	tbTxPort->box.size = Vec(50, height);
	tbTxPort->box.pos = Vec(x, y);
	tbTxPort->visible = visible;
	tbTxPort->text = std::to_string(txPort);
	tbTxPort->id = i;
	addChild(tbTxPort);
	textBoxes[i++] = tbTxPort;

	x += tbTxPort->box.size.x + dx;
	tbRxPort = new TSTextField(TSTextField::TextType::DigitsOnly, 5);
	tbRxPort->box.size = Vec(50, height);
	tbRxPort->box.pos = Vec(x, y);
	tbRxPort->visible = visible;
	tbRxPort->text = std::to_string(rxPort);
	tbRxPort->id = i;
	addChild(tbRxPort);
	textBoxes[i++] = tbRxPort;

	x += tbRxPort->box.size.x + dx;
	// OSC Client Type:
	// (since touchOSC needs special handling, Lemur probably does too)
	btnClientSelect = new TSOSCClientSelectBtn();
	btnClientSelect->selectedOSCClient = selectedClient;
	btnClientSelect->box.size = Vec(78, height);
	btnClientSelect->box.pos = Vec(x, y);
	btnClientSelect->visible = this->showClientSelect && visible;
	addChild(btnClientSelect);

	if (this->showClientSelect)
		x += btnClientSelect->box.size.x + dx + 3;

	// Namespace
	tbNamespace = new TSTextField(TSTextField::TextType::Any, 20);
	tbNamespace->box.size = Vec(78, height);
	tbNamespace->box.pos = Vec(x, y);
	tbNamespace->visible = this->showNamespace && visible;
	tbNamespace->text = oscNamespace;
	tbNamespace->id = i;
	tbNamespace->placeholder = "namespace";
	addChild(tbNamespace);
	if (this->showNamespace)
	{
		textBoxes[i++] = tbNamespace;
		xNamespace = x; // Save this so we can label it
		x += tbNamespace->box.size.x + dx + 3;
	}

	// Button Enable/Disable
	Vec btnSize = Vec(36, height);
	this->btnSave = dynamic_cast<TS_PadBtn*>( createParam<TS_PadBtn>(Vec(x, y), module, btnSaveId) );
	// this->btnSave->module = module;
	// this->btnSave->paramId = btnSaveId;
	this->btnSave->box.size = btnSize;
	//this->btnSave->box.pos = Vec(x, y);
	addChild(btnSave);

	// Checkbox for Auto-reconnect:
	this->ckAutoReconnect = new TS_ScreenCheckBox(Vec(50, 12), module, btnAutoReconnectId, "Auto Con");//, 0.f, 1.f, 0.f);
	this->ckAutoReconnect->box.pos = Vec(x + 4, y - 13);
	this->ckAutoReconnect->checkBoxWidth = 10;
	this->ckAutoReconnect->checkBoxHeight = 10;
	this->ckAutoReconnect->fontSize = 9;
	this->ckAutoReconnect->borderWidth = 0;
	this->ckAutoReconnect->padding = 1;
	this->ckAutoReconnect->color = textColor;
	addChild(ckAutoReconnect);
		
	numTextFields = i;
	for (i = 0; i < numTextFields; i++)
	{
		int prevIx = (i > 0) ? i - 1 : numTextFields - 1;
		int nextIx = (i < numTextFields - 1) ? i + 1 : 0;
		textBoxes[i]->nextField = textBoxes[nextIx];
		textBoxes[i]->prevField = textBoxes[prevIx];
	}
	return;
}

TSOSCConfigWidget::TSOSCConfigWidget(Module* mod, int btnSaveId, int btnAutoReconnectId, OSCClient selectedClient, std::string ipAddress, uint16_t txPort, uint16_t rxPort)
	: TSOSCConfigWidget(mod, btnSaveId, btnAutoReconnectId, ipAddress, txPort, rxPort, true, selectedClient, false, std::string(""))
{
	return;
}

void TSOSCConfigWidget::step() {
	// Check for enable/disable auto-reconnect
	if (module && visible)
	{		
		if (ckAutoReconnect->getParamQuantity() && autoReconnectTrigger.process(module->params[ckAutoReconnect->getParamQuantity()->paramId].getValue())) {
			ckAutoReconnect->checked = !ckAutoReconnect->checked;
		}
	}
	Widget::step();
	return;
}

// Callback for tabbing between our text boxes.
void TSOSCConfigWidget::onTabField(int id)
{
	int focusIx = (id + 1) % 3;
	textBoxes[focusIx]->requestFocus();
	return;
}

// Callback for shift-tabbing between our text boxes.
// This doesn't work since in C++ I can't figure out how to get a damn pointer to a member function...
void TSOSCConfigWidget::onShiftTabField(int id)
{
	int focusIx = id - 1;
	if (focusIx < 0)
		focusIx = 2;
	textBoxes[focusIx]->requestFocus();
	return;
}

void TSOSCConfigWidget::drawLayer(const DrawArgs &args, int layer) {
	if (this->visible)
	{
		// v2: Load font reference every draw call	
		std::shared_ptr<Font> font = APP->window->loadFont(fontPath); // Rack v2 load font each time	
		if (layer == 1)
		{
			nvgFontSize(args.vg, fontSize);
			nvgFontFaceId(args.vg, font->handle);

			// Screen:
			nvgBeginPath(args.vg);
			nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
			nvgFillColor(args.vg, backgroundColor);
			nvgFill(args.vg);
			nvgStrokeWidth(args.vg, 1.0);
			nvgStrokeColor(args.vg, borderColor);
			nvgStroke(args.vg);

			// Draw labels
			nvgFillColor(args.vg, textColor);
			nvgFontSize(args.vg, fontSize);
			nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);
			float y = START_Y - 1;
			float x;
			const char* labels[] = { "OSC IP Address", "Out Port", "In Port" };
			for (int i = 0; i < TSOSC_NUM_TXTFIELDS; i++)
			{
				x = textBoxes[i]->box.pos.x + 2;
				nvgText(args.vg, x, y, labels[i], NULL);
			}
			if (xNamespace > -1)
			{
				nvgText(args.vg, xNamespace + 1, y, "Namespace", NULL);
			}

			// Current status:
			x = box.size.x - 8;
			nvgTextAlign(args.vg, NVG_ALIGN_RIGHT | NVG_ALIGN_BOTTOM);
			nvgFillColor(args.vg, statusColor);
			nvgText(args.vg, x, y, statusMsg.c_str(), NULL);
			// Status 2
			y += textBoxes[0]->box.size.y + 2;
			if (!statusMsg2.empty())
			{
				nvgTextAlign(args.vg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
				nvgText(args.vg, x, y, statusMsg2.c_str(), NULL);
			}

			// Draw Messages:
			x = textBoxes[0]->box.pos.x + 2;
			nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
			if (!errorMsg.empty())
			{
				nvgFillColor(args.vg, errorColor);		
				nvgText(args.vg, x, y, errorMsg.c_str(), NULL);
			}
			else if (!successMsg.empty())
			{
				nvgFillColor(args.vg, successColor);
				nvgText(args.vg, x, y, successMsg.c_str(), NULL);
			}
			else
			{
				nvgFillColor(args.vg, textColor);
				nvgText(args.vg, x, y, "Open Sound Control Configuration", NULL);
			}

			// Draw children:
			this->OpaqueWidget::draw(args);	
			
			// Quick and dirty -- Draw labels on buttons:
			nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
			y = btnSave->box.pos.y + btnSave->box.size.y / 2.0 + 1;
			// Save:
			x = btnSave->box.pos.x + btnSave->box.size.x / 2.0 + 7;
			if (btnActionEnable)
			{
				nvgFillColor(args.vg, TSColors::COLOR_TS_GREEN);
				nvgText(args.vg, x, y, "ENABLE", NULL);
			}
			else
			{
				nvgFillColor(args.vg, TSColors::COLOR_TS_ORANGE);
				nvgText(args.vg, x, y, "DISABLE", NULL);
			}
		} // end if layer		
		this->OpaqueWidget::drawLayer(args, 1);
	}
	
	return;
}
void onTabField(int id)
{
	return;
}
void onShiftTabField(int id)
{
	return;
}
