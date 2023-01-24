#ifndef WIDGET_POLYGEN_HPP
#define WIDGET_POLYGEN_HPP

#include "TSSModuleWidgetBase.hpp"
#include "Module_polyGen.hpp"

#include <rack.hpp>
using namespace rack;

#define TS_POLYGEN_PREVIEW_SIZE 	(TS_POLYGEN_VERTICES_MAX*2)

//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// polyGenWidget
// Polygon generator module widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct polyGenWidget : TSSModuleWidgetBase 
{	
	bool plugLightsEnabled = true;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// polyGenWidget()
	// @polyGenModule : (IN) Pointer to the polyGen module.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	polyGenWidget(polyGen* polyGenModule) ;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// ~polyGenWidget()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	~polyGenWidget()
	{		
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// addInputControlGroup()
	// Add group of user controls + cv ports.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void addInputControlGroup(int numInputs, int inputIds[], int paramIds[], int boolIds[], int lightIds[], bool verticalLayout, Vec startLoc);
};

//===============================================================================
// polyGenShapePreviewWidget
//===============================================================================
struct polyGenShapePreviewWidget : TransparentWidget
{
	polyGen* module = NULL;
	NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
	NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);	
	NVGcolor lineColor = nvgRGB(0xCC, 0xCC, 0xCC);	
	NVGcolor secondColor = nvgRGBA(0xCC, 0x00, 0x00, 0x99);
	float lineWidth = 2.0f;
	float padding = 8.0f;
	// Basic shape buffer.
	Vec rawBuffer[TS_POLYGEN_PREVIEW_SIZE];
	// With rotation buffer.
	Vec rotatedBuffer[TS_POLYGEN_PREVIEW_SIZE];
	
#if DEBUG_POLY
	int DEBUG_COUNT = 0;
#endif	
	
	polyGenShapePreviewWidget(polyGen* polyGenModule)
	{
		this->module = polyGenModule;
		return;
	}
	polyGenShapePreviewWidget(polyGen* polyGenModule, Vec size)
	{
		this->module = polyGenModule;
		this->box.size = size;
		return;
	}
	void drawShape(const DrawArgs &args, Vec* buffer, int bufferLen, Vec offset, float mult, float lWidth, NVGcolor lColor);
	void drawBackground(const DrawArgs& args);
	// Normal draw
	void draw(/*in*/ const DrawArgs &args) override;
	// Draw on the light layer 
	void drawLayer(/*in*/ const DrawArgs& args, /*in*/ int layer) override;
};



#endif // !WIDGET_POLYGEN_HPP