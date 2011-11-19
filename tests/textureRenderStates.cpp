// Copyright (C) 2008-2011 Christian Stehno, Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;

static bool manyTextures(video::E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device = createDevice(driverType, dimension2d<u32>(160, 120), 32);
	if (!device)
		return true; // Treat a failure to create a driver as benign; this saves a lot of #ifdefs

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager * smgr = device->getSceneManager();

	logTestString("Testing driver %ls\n", driver->getName());

	(void)smgr->addCameraSceneNode();

	scene::SMeshBufferLightMap* mesh = new scene::SMeshBufferLightMap;

	mesh->Vertices.reallocate(4);
	mesh->Indices.reallocate(6);

	mesh->Vertices.push_back(video::S3DVertex2TCoords(-50,50,80,irr::video::SColor(255,100,100,100),0,1,0,1));
	mesh->Vertices.push_back(video::S3DVertex2TCoords(50,50,80,irr::video::SColor(255,100,100,100),1,1,1,1));
	mesh->Vertices.push_back(video::S3DVertex2TCoords(50,-50,80,irr::video::SColor(255,100,100,100),1,0,1,0));
	mesh->Vertices.push_back(video::S3DVertex2TCoords(-50,-50,80,irr::video::SColor(255,100,100,100),0,0,0,0));

	mesh->Indices.push_back(0);
	mesh->Indices.push_back(1);
	mesh->Indices.push_back(2);
	mesh->Indices.push_back(2);
	mesh->Indices.push_back(3);
	mesh->Indices.push_back(0);

	video::SMaterial& mat = mesh->getMaterial();
	mat.Lighting = false;
	mat.setTexture(0,driver->getTexture("../media/fire.bmp"));
	mat.setTexture(1,driver->getTexture("../media/fire.bmp"));
	mat.setTexture(2,driver->getTexture("../media/fire.bmp"));
	mat.setTexture(3,driver->getTexture("../media/fire.bmp"));

	mesh->setDirty();

	driver->beginScene(true, true, video::SColor(255,100,101,140));
	// set camera
	smgr->drawAll();
	// draw meshbuffer
	driver->setMaterial(mat);
	driver->drawMeshBuffer(mesh); 
	driver->endScene();
	mesh->drop();

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-multiTexture.png", 99.31f);

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

//! Tests interleaved loading and rendering of textures
/** The test loads a texture, renders it using draw2dimage, loads another
	texture and renders the first one again. Due to the texture cache this
	can lead to rendering of the second texture in second place. */
static bool renderAndLoad(video::E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device = createDevice( driverType, dimension2d<u32>(160, 120), 32);
	if (!device)
		return true; // Treat a failure to create a driver as benign; this saves a lot of #ifdefs

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager * smgr = device->getSceneManager();

	logTestString("Testing driver %ls\n", driver->getName());

	video::ITexture* tex1 = driver->getTexture("../media/wall.bmp");

	(void)smgr->addCameraSceneNode();

	driver->beginScene(true, true, video::SColor(255,100,101,140));
	driver->draw2DImage(tex1, position2di(0,0));
	driver->endScene();

	driver->getTexture("../media/tools.png");

	driver->beginScene(true, true, video::SColor(255,100,101,140));
	driver->draw2DImage(tex1, position2di(0,0));
	driver->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-textureRenderStates.png", 99.85f);

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}


// This test would cause a crash if it does not work
// in 1.5.1 and 1.6 an illegal access in the OpenGL driver caused this
static bool renderAndRemove(video::E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device = createDevice( driverType, dimension2d<u32>(160, 120), 32);
	if (!device)
		return true; // Treat a failure to create a driver as benign; this saves a lot of #ifdefs

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager * smgr = device->getSceneManager();

	logTestString("Testing driver %ls\n", driver->getName());

	driver->beginScene (true, true, video::SColor(255, 0, 255, 0));
	smgr->drawAll();
	driver->endScene();

	smgr->addCameraSceneNode();
	video::ITexture* texture = driver->getTexture ("media/tools.png");
	scene::ISceneNode * img = smgr->addCubeSceneNode();
	img->setMaterialTexture(0, texture);

	driver->beginScene (true, true, video::SColor (255, 0, 255, 0));
	smgr->drawAll();
	driver->endScene();

	smgr->clear();	// Remove anything that used the texture
	driver->removeTexture(texture);

	driver->beginScene(true, true, video::SColor(255,100,101,140));
	smgr->drawAll();
	driver->endScene();

	smgr->addCameraSceneNode();
	texture = driver->getTexture("media/tools.png");
	img = smgr->addCubeSceneNode();
	img->setMaterialTexture(0, texture);

	driver->beginScene (true, true, irr::video::SColor (255, 0, 255, 0));
	smgr->drawAll();
	driver->endScene();

	device->closeDevice();
	device->run();
	device->drop();

	return true;
}


static bool testTextureMatrixInMixedScenes(video::E_DRIVER_TYPE driverType)
{
	irr::IrrlichtDevice* device = createDevice(driverType, dimension2du(160,120));
	if (!device)
		return true;

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* sceneManager = device->getSceneManager();
	gui::IGUIEnvironment* gui = device->getGUIEnvironment();

	if (!driver->queryFeature(video::EVDF_TEXTURE_MATRIX))
	{
		device->closeDevice();
		device->run();
		device->drop();
		return true;
	}

	logTestString("Testing driver %ls\n", driver->getName());

	scene::ICameraSceneNode* camera = sceneManager->addCameraSceneNode();
	camera->setPosition(vector3df(0,10,0));

	gui::IGUIStaticText* stext = gui->addStaticText(L" ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789 ",
			rect<s32>(5,100,150,125),false,false,0,false);
	stext->setBackgroundColor(video::SColor(255,128,0,0));
	stext->setOverrideColor(video::SColor(255,255,255,255));

	gui->addEditBox(L"Test edit box", rect<s32>(5,50,150,75));

	gui->addCheckBox(true, rect<s32>(5, 20, 150, 45),0,-1,L"Test CheckBox");

	video::SMaterial mat;
	mat.MaterialType = video::EMT_SOLID;
	mat.setFlag(video::EMF_LIGHTING, false);
	// problem doesn't occur if the scale defaults: (1.f,1.f).
	mat.getTextureMatrix(0).setTextureScale(2.0,2.0);

	scene::IAnimatedMesh* pmesh = sceneManager->addHillPlaneMesh("testMesh",dimension2d<f32>(50,50),dimension2d<u32>(6,6),&mat);
	sceneManager->addAnimatedMeshSceneNode(pmesh);

	driver->beginScene(true, true, video::SColor(255,100,101,140));
	sceneManager->drawAll();
	gui->drawAll();
	driver->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-textureMatrixInMixedScenes.png", 99.34f);

	device->closeDevice();
	device->run();
	device->drop();

	return result;
} 

// animated texture matrix test.
static bool textureMatrix(video::E_DRIVER_TYPE driverType)
{
	irr::IrrlichtDevice* device = createDevice(driverType, dimension2du(160,120));
	if (!device)
		return true;

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* sceneManager = device->getSceneManager();

	if (!driver->queryFeature(video::EVDF_TEXTURE_MATRIX))
	{
		device->closeDevice();
		device->run();
		device->drop();
		return true;
	}

	logTestString("Testing driver %ls\n", driver->getName());

	scene::ICameraSceneNode* camera = sceneManager->addCameraSceneNode();
	camera->setPosition(vector3df(0,0,-10));

	// set up plane mesh to face the camera
	scene::IMesh* mesh = sceneManager->getGeometryCreator()->createPlaneMesh(dimension2df(150,150), core::dimension2du(1,1),0,core::dimension2df(1,1));
	scene::IMeshSceneNode* node = sceneManager->addMeshSceneNode(mesh, 0, -1, vector3df(0, 0, 150), vector3df(-90, 0, 0));
	if (mesh)
		mesh->drop();

	// set the hillplane material texture & save a reference
	// to the texture matrix.
	video::SMaterial& material = node->getMaterial(0);
	video::ITexture* tex = driver->getTexture("../media/water.jpg");

	material.setTexture(0,tex);
	material.setFlag(video::EMF_LIGHTING,false);
	matrix4& textureMatrix = material.TextureLayer[0].getTextureMatrix();

	const vector2df rcenter, scale(1.f, 1.f);
	vector2df trans;

	trans.X += 0.0005f;
	textureMatrix.buildTextureTransform(0.f, rcenter, trans, scale);

	driver->beginScene(true, true, video::SColor(255,100,101,140));
	sceneManager->drawAll();
	driver->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-textureMatrix.png");

	trans.X += 0.45f;
	textureMatrix.buildTextureTransform(0.f, rcenter, trans, scale);

	driver->beginScene(true, true, video::SColor(255,100,101,140));
	sceneManager->drawAll();
	driver->endScene();

	result &= takeScreenshotAndCompareAgainstReference(driver, "-textureMatrix2.png");

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

bool textureRenderStates(void)
{
	bool result = true;

	TestWithAllDrivers(renderAndLoad);
	TestWithAllDrivers(renderAndRemove);
	TestWithAllDrivers(testTextureMatrixInMixedScenes);
	TestWithAllDrivers(manyTextures);
	TestWithAllDrivers(textureMatrix);

	return result;
}
