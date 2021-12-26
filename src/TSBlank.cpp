#include "trowaSoft.hpp"
#include <rack.hpp>
using namespace rack;
#include "TSColors.hpp"
#include "trowaSoftComponents.hpp"

#define BLANK_MIN_WIDTH 			(1 * RACK_GRID_WIDTH)
#define	BLANK_TWO_SCREW_WIDTH		(4 * RACK_GRID_WIDTH)
#define BLANK_DEFAULT_WIDTH			(3 * RACK_GRID_WIDTH)
#define BUFFER_SIZE					512

struct ModuleResizeHandle : Widget {
	bool right = false;
	float dragX;
	Rect originalBox;
	int minWidth = BLANK_MIN_WIDTH;
	ModuleResizeHandle() {
		box.size = Vec(RACK_GRID_WIDTH * 1, RACK_GRID_HEIGHT);
	}
	void onButton(const event::Button &e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			e.consume(this);
		}
	}
	void onDragStart(const event::DragStart &e) override {
		// [Rack v2] mousePos no longer accessible. Now accessor getMousePos().
		dragX = APP->scene->rack->getMousePos().x; //APP->scene->rack->mousePos.x;//gRackWidget->lastMousePos.x;
		ModuleWidget *m = getAncestorOfType<ModuleWidget>();
		originalBox = m->box;
	}
	void onDragMove(const event::DragMove &e) override {
		ModuleWidget *m = getAncestorOfType<ModuleWidget>();

		// [Rack v2] mousePos no longer accessible. Now accessor getMousePos().
		float newDragX = APP->scene->rack->getMousePos().x; //APP->scene->rack->mousePos.x;//gRackWidget->lastMousePos.x;
		//float newDragX = gRackWidget->lastMousePos.x;
		//float newDragX = APP->scene->rack->mousePos.x;		
		float deltaX = newDragX - dragX;

		Rect newBox = originalBox;
		if (right) {
			newBox.size.x += deltaX;
			newBox.size.x = fmaxf(newBox.size.x, minWidth);
			newBox.size.x = roundf(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
		}
		else {
			newBox.size.x -= deltaX;
			newBox.size.x = fmaxf(newBox.size.x, minWidth);
			newBox.size.x = roundf(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
			newBox.pos.x = originalBox.pos.x + originalBox.size.x - newBox.size.x;
		}
		//gRackWidget->requestModuleBox(m, newBox);
		// Set box and test whether it's valid
		m->box = newBox;
		if (!APP->scene->rack->requestModulePos(m, newBox.pos)) {
			m->box = originalBox;
		}		
	}
};

struct TSBlankModule : Module {
	int panelWidth = BLANK_DEFAULT_WIDTH;
	
	TSBlankModule()
	{
		config(0, 0, 0, 0);
	}
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// dataToJson(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	json_t *dataToJson() override
	{
		json_t* rootJ = json_object();
		// version
		json_object_set_new(rootJ, "version", json_integer(TROWA_INTERNAL_VERSION_INT));
		json_object_set_new(rootJ, "width", json_integer(panelWidth));
		return rootJ;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// dataFromJson(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void dataFromJson(json_t *rootJ) override
	{
		json_t* currJ = NULL;
		currJ = json_object_get(rootJ, "width");
		if (currJ)
		{
			panelWidth = json_integer_value(currJ);
		}
	}
};

struct TSBlankWidget : ModuleWidget {
	TS_SvgPanel *panel;
	Widget *topRightScrew;
	Widget *bottomRightScrew;
	Widget *rightHandle;
	Widget *topLeftScrew;
	Widget *bottomLeftScrew;
	const int rhsScrewPos = -15;
	const int bottomPosY = 365;

	float bufferX[BUFFER_SIZE] = { 0 };
	float bufferY[BUFFER_SIZE] = { 0 };
	int bufferIx = 0;
	float speed = 0.5f;
	float dt = 0.f;
	
	bool loaded = false;

	TSBlankWidget(TSBlankModule *module) : ModuleWidget() 
	{	
		box.size = Vec(BLANK_DEFAULT_WIDTH, RACK_GRID_HEIGHT);
		if (module)
		{
			box.size.x = module->panelWidth;
			setModule(module);
		}
		//////////////////////////////////////////////
		// Background
		//////////////////////////////////////////////	
		{
			panel = new TS_SvgPanel(0,0,0,0);
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/blank.svg")));
			panel->box.size = Vec(BLANK_MIN_WIDTH, box.size.y);
			panel->box.pos = Vec(0,0);		
			addChild(panel);
		}

		ModuleResizeHandle *leftHandle = new ModuleResizeHandle();
		ModuleResizeHandle *rightHandle = new ModuleResizeHandle();
		rightHandle->right = true;
		this->rightHandle = rightHandle;
		addChild(leftHandle);
		addChild(rightHandle);
		
		topLeftScrew = createWidget<ScrewBlack>(Vec(0, 0));
		addChild(topLeftScrew);
		bottomLeftScrew = createWidget<ScrewBlack>(Vec(0, bottomPosY));
		addChild(bottomLeftScrew);
		topRightScrew = createWidget<ScrewBlack>(Vec(box.size.x + rhsScrewPos, 0));
		bottomRightScrew = createWidget<ScrewBlack>(Vec(box.size.x + rhsScrewPos, bottomPosY));
		addChild(topRightScrew);
		addChild(bottomRightScrew);
	}

	// void calcSpiro()
	// {
		// dt += engineGetSampleRate();
		// if (dt >= speed)
		// {
			// // Calc next item

		// }

	// }
	void draw(const DrawArgs &args) override 
	{
		nvgSave(args.vg);

		/// TODO: Don't draw every time, move to buffer.
		NVGcolor bgColor = TSColors::COLOR_TS_BACKGROUND; //#646464		
		NVGcolor barColor = TSColors::COLOR_BLACK;		

		// Background:		
		Rect b = Rect(Vec(0, 0), box.size);
		nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
		// Screen:
		nvgBeginPath(args.vg);
		nvgRect(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
		nvgFillColor(args.vg, bgColor);
		nvgFill(args.vg);		
		// nvgStrokeWidth(args.vg, 1.0);
		// nvgStrokeColor(args.vg, borderColor);
		// nvgStroke(args.vg);
		nvgResetScissor(args.vg);
		
		
		// Top bar
		int x = 0;
		int y = 0;
		int w = box.size.x;
		float h = topLeftScrew->box.size.y + 1;
		b = Rect(Vec(x, y), Vec(w, h));
		nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
		nvgBeginPath(args.vg);
		nvgRect(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
		nvgFillColor(args.vg, barColor);
		nvgFill(args.vg);
		nvgResetScissor(args.vg);
		
		
		// Bottom Bar
		y = box.size.y - h + 1;
		b = Rect(Vec(x, y), Vec(w, h - 1));
		nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
		nvgBeginPath(args.vg);
		nvgRect(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
		nvgFillColor(args.vg, barColor);
		nvgFill(args.vg);
		nvgResetScissor(args.vg);
		
		nvgScissor(args.vg, 0, 0, box.size.x, box.size.y);				
		this->Widget::draw(args);
		nvgResetScissor(args.vg);
		
		// Top & left have some kind of dinky border
		NVGcolor borderColor = nvgRGBAf(0.5, 0.5, 0.5, 0.5);
		b = Rect(Vec(0, 0), box.size);
		nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);		
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0.5, 0.5, box.size.x - 1.0, box.size.y - 1.0);
		nvgStrokeColor(args.vg, borderColor);
		nvgStrokeWidth(args.vg, 1);
		nvgStroke(args.vg);
		nvgResetScissor(args.vg);
		
		nvgRestore(args.vg);		
		return;
	}
	
	void step() override {
		if (module && !loaded && dynamic_cast<TSBlankModule*>(module)->panelWidth != box.size.x)
		{
			box.size.x = dynamic_cast<TSBlankModule*>(module)->panelWidth;
			loaded = true;
		}
		//panel->box.size = box.size;
		
		topRightScrew->box.pos.x = box.size.x + rhsScrewPos;
		bottomRightScrew->box.pos.x = box.size.x + rhsScrewPos;
		if (box.size.x < BLANK_TWO_SCREW_WIDTH) {
			topRightScrew->visible = bottomRightScrew->visible = false;
			if (box.size.x == 3 * RACK_GRID_WIDTH)
			{
				// Move left screw to center:
				topLeftScrew->box.pos.x = RACK_GRID_WIDTH;
				bottomLeftScrew->box.pos.x = RACK_GRID_WIDTH;				
			}
            else if(box.size.x == RACK_GRID_WIDTH)
            {
                topLeftScrew->box.pos.x = 0;
                bottomLeftScrew->box.pos.x = 0;
            }
		}
		else {
			topRightScrew->visible = bottomRightScrew->visible = true;
			topLeftScrew->box.pos.x = 0;
			bottomLeftScrew->box.pos.x = 0;			
		}
		rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;
		if (module) {
			dynamic_cast<TSBlankModule*>(module)->panelWidth = box.size.x;
		}
		ModuleWidget::step();
	}
};

Model* modelBlank = createModel<TSBlankModule, TSBlankWidget>(/*slug*/ "tsBlank");

