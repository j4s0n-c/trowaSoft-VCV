#ifndef TROWASOFT_MODULE_TSSEQUENCERWIDGETBASE_HPP
#define TROWASOFT_MODULE_TSSEQUENCERWIDGETBASE_HPP
#include <exception>
#include <rack.hpp>
using namespace rack;

#include "TSSequencerModuleBase.hpp"
#include "TSSModuleWidgetBase.hpp"
#include "TSOSCConfigWidget.hpp"
#include "TSColors.hpp"

struct TSSeqDisplay;
struct TSSequencerModuleBase;
struct TSSeqLabelArea;
struct TSSeqPatternSeqConfigWidget;


#define  colorRGBEqual(colorA, colorB) 		(colorA.r == colorB.r && colorA.g == colorB.g && colorA.b == colorB.b)
#define  colorRGBNotEqual(colorA, colorB) 		(colorA.r != colorB.r || colorA.g != colorB.g || colorA.b != colorB.b)

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSequencerWidgetBase
// Sequencer Widget Base Class (adds common UI controls).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSSequencerWidgetBase : TSSModuleWidgetBase {
	// Top digital display for sequencer.
	TSSeqDisplay *display;
	// The labels.
	TSSeqLabelArea* labelArea;
	// OSC configuration widget.
	TSOSCConfigWidget* oscConfigurationScreen;
	// Pattern sequencer config widget.
	TSSeqPatternSeqConfigWidget* pattSeqConfigurationScreen = NULL;
	// Numer of steps this should have (for when we get a NULL module).
	int maxSteps = 16;
	// Number of rows in step matrix.
	int numRows = 4;
	// Number of columns in step matrix.
	int numCols = 4;
	
	// References to our pad lights / knob lights. Derived class should create this.
	ColorValueLight*** padLightPtrs = NULL; 
	// Light for paste button
	TS_LightString* pasteLight;
	// Light for copy pattern button
	ColorValueLight* copyPatternLight;
	// Light for copy channel button
	ColorValueLight* copyChannelLight;
	// Keep track of the last step matrix color so we can detect changes.
	NVGcolor lastStepMatrixColor = TSColors::COLOR_BLACK;	
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSSequencerWidgetBase() - Base constructor.
	// Instantiate a trowaSoft Sequencer widget. v0.60 must have module as param.
	// @seqModule : (IN) Pointer to the sequencer module.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSSequencerWidgetBase(TSSequencerModuleBase* seqModule);	
	~TSSequencerWidgetBase()
	{
		if (padLightPtrs != NULL)
		{
			for (int r = 0; r < numRows; r++)
			{
				if (padLightPtrs[r])
					delete[] padLightPtrs[r];
			}
			delete[] padLightPtrs;	padLightPtrs = NULL;
		}
		return;
	}
	// Step
	void step() override;
	// Add base controls.
	void addBaseControls() { addBaseControls(false); }
	// Add base controls.
	void addBaseControls(bool addGridLines);
	// Create context menu with common adds to sequencers.
	/** 
	Override to add context menu entries to your subclass.
	It is recommended to add a blank ui::MenuEntry first for spacing.
	*/
	virtual void appendContextMenu(ui::Menu *menu) override;	
};

//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSeqLabelArea
// Draw labels on our sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct TSSeqLabelArea : TransparentWidget {
	TSSequencerModuleBase *module;
	std::shared_ptr<Font> font;
	int fontSize;
	bool drawGridLines = false;
	bool allowPatternSequencing = false;
	char messageStr[TROWA_DISP_MSG_SIZE];
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSSeqLabelArea()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSSeqLabelArea() 
	{
		
		fontSize = 13;
		for (int i = 0; i < TROWA_DISP_MSG_SIZE; i++)
			messageStr[i] = '\0';
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// @args.vg : (IN) NVGcontext to draw on
	// Draw labels on a trowaSoft sequencer.
	// (Common across trigSeq, voltSeq, multiSeq).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(const DrawArgs &args) override;
}; // end struct TSSeqLabelArea


//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSeqPatternSeqConfigWidget
// Pattern sequencing configuration.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct TSSeqPatternSeqConfigWidget : OpaqueWidget
{
	std::shared_ptr<Font> font;
	std::shared_ptr<Font> labelFont;
	int fontSize;
	char messageStr[TROWA_DISP_MSG_SIZE]; // tmp buffer for our strings.
	NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
	NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
	NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
	TSSequencerModuleBase* seqModule = NULL;	
	TS_ScreenCheckBox* ckEnabled = NULL;
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSSeqPatternSeqConfigWidget()
	// Pattern configuration.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	TSSeqPatternSeqConfigWidget(TSSequencerModuleBase* module);

	void setVisible(bool vis)
	{
		visible = vis;
		for (Widget* child : children)
		{
			child->visible = vis;
		}
		return;
	}
	// //-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// // draw()
	// // @args.vg : (IN) NVGcontext to draw on
	// //-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// void draw(const DrawArgs &args) override;	
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Draw the light layer (v2). For light emission when dark.
	// @layer : Layer #.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void drawLayer(const DrawArgs& args, int layer) override;	
	
	void onHover(const event::Hover& e) override {
		if (!visible)
			return;
		OpaqueWidget::onHover(e);
		return;
	}
	void onButton(const event::Button& e) override {
		if (!visible)
			return;
		OpaqueWidget::onButton(e);
		return;
	}
	void onHoverKey(const event::HoverKey& e) override {
		if (!visible)
			return;
		OpaqueWidget::onHoverKey(e);
		return;
	}
	void onHoverText(const event::HoverText& e) override {
		if (!visible)
			return;
		OpaqueWidget::onHoverText(e);
		return;
	}
	void onHoverScroll(const event::HoverScroll& e) override {
		if (!visible)
			return;
		OpaqueWidget::onHoverScroll(e);
		return;
	}
	void onDragHover(const event::DragHover& e) override {
		if (!visible)
			return;
		OpaqueWidget::onDragHover(e);
		return;
	}
	void onPathDrop(const event::PathDrop& e) override {
		if (!visible)
			return;
		OpaqueWidget::onPathDrop(e);
		return;
	}	

};

//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSeqDisplay
// A top digital display for trowaSoft sequencers.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct TSSeqDisplay : TransparentWidget {
	
	// Background Colors:
	NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
	NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);	
	TSSequencerModuleBase *module;
	std::shared_ptr<Font> font;
	std::shared_ptr<Font> labelFont;
	int fontSize;
	char messageStr[TROWA_DISP_MSG_SIZE]; // tmp buffer for our strings.
	bool showDisplay = true;
	
	enum SeqViewType : uint8_t 
	{
		// Display values of knobs
		NormalView,
		// Display currently edit step value (for multiSeq)
		EditStepView
	};
	
	SeqViewType currentView = SeqViewType::NormalView;
	ValueSequencerMode* sequencerValueModePtr = NULL;
	
	
	int lastStepEditShownParamId = -1;
	float lastStepEditShownValue = -20.0f;
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSSeqDisplay(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSSeqDisplay() {
		fontSize = 12;
		for (int i = 0; i < TROWA_DISP_MSG_SIZE; i++)
			messageStr[i] = '\0';
		showDisplay = true;
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Draw the normal default overview.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void drawNormalView(const DrawArgs &args);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Draw the edit step view.
	// @currEditStep : Step number (1 - N).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void drawEditStepView(const DrawArgs &args, int currEditStep);	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Draw the light layer (v2). For light emission when dark.
	// @layer : Layer #.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void drawLayer(const DrawArgs& args, int layer) override;
	
	// //-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// // draw()
	// // @args.vg : (IN) NVGcontext to draw on
	// //-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// void draw(/*in*/ const DrawArgs &args) override 
	// {

		// // Screen:
		// nvgBeginPath(args.vg);
		// nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
		// nvgFillColor(args.vg, backgroundColor); 
		// nvgFill(args.vg);
		// nvgStrokeWidth(args.vg, 1.0);
		// nvgStrokeColor(args.vg, borderColor);
		// nvgStroke(args.vg);

		// if (!showDisplay)
			// return;
		
		// switch (currentView)
		// {
			// case SeqViewType::NormalView:			
				// drawNormalView(args);
				// lastStepEditShownParamId = -1;
				// lastStepEditShownValue = -20.0f;
				// break;
			// case SeqViewType::EditStepView:
				// drawEditStepView(args, module->currentStepBeingEditedIx + 1);
				// break;
		// }
		// return;
	// } // end draw()
}; // end struct TSSeqDisplay

#endif // end !TROWASOFT_MODULE_TSSEQUENCERWIDGETBASE_HPP