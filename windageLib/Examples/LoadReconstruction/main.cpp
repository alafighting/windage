/* ========================================================================
 * PROJECT: windage Library
 * ========================================================================
 * This work is based on the original windage Library developed by
 *   Woonhyuk Baek
 *   Woontack Woo (wwoo@gist.ac.kr)
 *   U-VR Lab, GIST of Gwangju in Korea.
 *   http://windage.googlecode.com/
 *   http://uvr.gist.ac.kr/
 *
 * Copyright of the derived and new portions of this work
 *     (C) 2009 GIST U-VR Lab.
 *
 * This framework is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this framework; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * For further information please contact 
 *   Woonhyuk Baek
 *   <windage@live.com>
 *   GIST U-VR Lab.
 *   Department of Information and Communication
 *   Gwangju Institute of Science and Technology
 *   1, Oryong-dong, Buk-gu, Gwangju
 *   South Korea
 * ========================================================================
 ** @author   Woonhyuk Baek
 * ======================================================================== */

#include <iostream>
#include <omp.h>

#include <cv.h>
#include <highgui.h>
#include <irrlicht.h>

#include <windage.h>
#include "Coordinator/ARForOSG.h"
#include "../Common/IrrlichtRenderer.h"

const char* FILE_NAME = "data/reconstruction-2010-03-27_08_04_44/reconstruction.txt";

const int WIDTH = 640;
const int HEIGHT = (WIDTH * 3) / 4;
const int RENDERING_WIDTH = 640;
const int RENDERING_HEIGHT = (RENDERING_WIDTH * 3) / 4;

void main()
{
	std::vector<IplImage*> inputImage;
	std::vector<windage::ReconstructionPoint> reconstructionPoints;
	std::vector<std::string> filenameList;
	std::vector<IplImage*> imageList;
	std::vector<windage::Calibration*> calibrationList;

	int selectedCamera = -1;

	// load data
	std::cout << "load reconstruction datas" << std::endl;
	std::cout << std::endl;

	windage::Reconstruction::Loader* loader = new windage::Reconstruction::Loader();
	loader->AttatchCalibration(&calibrationList);
	loader->AttatchFilename(&filenameList);
	loader->AttatchReconstructionPoints(&reconstructionPoints);
	loader->DoLoad(FILE_NAME);

	for(unsigned int i=0; i<filenameList.size(); i++)
	{
		imageList.push_back(cvLoadImage(filenameList[i].c_str()));
	}

	std::cout << std::endl;
	std::cout << "complete load reconstruction datas" << std::endl;
	std::cout << "reconstruction data count : " << reconstructionPoints.size() << std::endl;
	std::cout << std::endl;

	// for rendering
	KeyEventReceiver receiver;
	irr::IrrlichtDevice* device = irr::createDevice( irr::video::EDT_DIRECT3D9, irr::core::dimension2d<irr::u32>(640, 480), 16, false, false, false, &receiver);
	if (!device) return;
	device->setWindowCaption(L"windage : Spatial Reconstruction");

	irr::video::IVideoDriver* driver = device->getVideoDriver();
	irr::scene::ISceneManager* smgr = device->getSceneManager();
	irr::scene::ICameraSceneNode* arCamera = smgr->addCameraSceneNode();
	irr::scene::ICameraSceneNode* mayaCamera = smgr->addCameraSceneNodeMaya();	

	SceneNode *renderingSceneNode = new SceneNode(smgr->getRootSceneNode(), smgr, 666);
	renderingSceneNode->SetCalibrationList(&calibrationList);
	renderingSceneNode->SetReconstructionPoints(&reconstructionPoints);
	renderingSceneNode->SetFileNameList(&filenameList);

	renderingSceneNode->Initialize();

	while(device->run())
	if (device->isWindowActive())
	{
		driver->beginScene(true, true, irr::video::SColor(0,100,100,100));
		smgr->drawAll();
		driver->endScene();

		if(receiver.IsKeyDown(irr::KEY_KEY_Q))
		{
			selectedCamera = -1;
			smgr->setActiveCamera(mayaCamera);

			renderingSceneNode->setSelectedCamera();
			renderingSceneNode->resetTransparent();
		}
		else if(receiver.IsKeyDown(irr::KEY_KEY_A))
		{
			selectedCamera++;
			if(selectedCamera >= (int)calibrationList.size())
				selectedCamera = 0;

			windage::Coordinator::ARForOSG coordinator;
			coordinator.Initialize(RENDERING_WIDTH, RENDERING_HEIGHT);
			coordinator.AttatchCameraParameter(calibrationList[selectedCamera]);
			coordinator.SetProjectionMatrix();
			coordinator.SetModelViewMatrix();

			windage::Matrix4 projection = coordinator.GetProjectionMatrix();
			windage::Matrix4 modelview = coordinator.GetModelViewMatrix();

			irr::core::matrix4 irrProj;
			irr::core::matrix4 irrModel;
			for(int i=0; i<16; i++)
			{
				irrProj[i] = projection.m1[i];
				irrModel[i] = modelview.m1[i];
			}

			smgr->setActiveCamera(arCamera);
			smgr->getActiveCamera()->setProjectionMatrix(irrProj);
			smgr->getActiveCamera()->setViewMatrixAffector(irrModel);

			renderingSceneNode->setSelectedCamera(selectedCamera);
			renderingSceneNode->setTransparent();
		}
		else if(receiver.IsKeyDown(irr::KEY_KEY_Z))
		{
			selectedCamera--;
			if(0 > selectedCamera)
				selectedCamera = (int)calibrationList.size() - 1;

			windage::Coordinator::ARForOSG coordinator;
			coordinator.Initialize(RENDERING_WIDTH, RENDERING_HEIGHT);
			coordinator.AttatchCameraParameter(calibrationList[selectedCamera]);
			coordinator.SetProjectionMatrix();
			coordinator.SetModelViewMatrix();

			windage::Matrix4 projection = coordinator.GetProjectionMatrix();
			windage::Matrix4 modelview = coordinator.GetModelViewMatrix();

			irr::core::matrix4 irrProj;
			irr::core::matrix4 irrModel;
			for(int i=0; i<16; i++)
			{
				irrProj[i] = projection.m1[i];
				irrModel[i] = modelview.m1[i];
			}

			smgr->setActiveCamera(arCamera);
			smgr->getActiveCamera()->setProjectionMatrix(irrProj);
			smgr->getActiveCamera()->setViewMatrixAffector(irrModel);

			renderingSceneNode->setSelectedCamera(selectedCamera);
			renderingSceneNode->setTransparent();
		}
	}

	device->drop();
}
