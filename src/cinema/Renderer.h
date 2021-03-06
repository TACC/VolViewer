#pragma once

#include <ospray/ospray.h>
#include <vector>

#include "CinemaWindow.h"
#include "Camera.h"
#include "Lights.h"
#include "TransferFunction.h"
#include "ColorMap.h"
#include "Slices.h"
#include "Isos.h"
#include "RenderProperties.h"

#include "Volume.h"

using namespace std;

class Renderer
{
public:

	Renderer(int, int);
	~Renderer();

	CinemaWindow 		 *getWindow() 					{return window;}
	Camera 		 			 &getCamera() 					{return camera;}
	ColorMap	 			 &getColorMap() 				{return colorMap;}
	Lights 		 			 &getLights() 					{return lights;}
	Slices 		 			 &getSlices() 					{return slices;}
	Isos	 		 			 &getIsos() 						{return isos;}
	TransferFunction &getTransferFunction() {return transferFunction;}
	OSPRenderer 		 &getRenderer() 			  {return renderer;}
	Volume  				 *getVolume()						{return &volume;}
	RenderProperties &getRenderProperties()	{return renderProperties;}


	// Use this to load either a state file (with its data)
	// or a volume file
	void Load(std::string, bool with_data = true);

	// Use this to load a state file.   May or may not load data,
	// but will not munge camera state if it does
	void LoadState(std::string, bool with_data);

	// Use this to load a volume and set up a default camera
	void LoadVolume(std::string);

	// Save current state
	void SaveState(std::string);

	// Call this when you've set up shared volume data
	void CommitVolume();

	void Render(std::string fname);

private:

  // Use this to load a volume without changing other state (e.g. camera)
	void LoadDataFromFile(std::string);
	
	CinemaWindow 			*window;
	Camera	 				 	camera;
	Lights   				 	lights;
	TransferFunction 	transferFunction;
	Slices					 	slices;
	Isos							isos;
	ColorMap					colorMap;
	RenderProperties  renderProperties;

	OSPRenderer renderer;
	Volume volume;
};
