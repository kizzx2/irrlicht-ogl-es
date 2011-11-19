#include "testUtils.h"

using namespace irr;

namespace
{

bool testWithRenderTarget(video::E_DRIVER_TYPE driverType)
{
	// create device

	IrrlichtDevice *device = createDevice(driverType, core::dimension2d<u32>(160,120));

	if (device == 0)
		return true; // could not create selected driver.

	video::IVideoDriver* driver = device->getVideoDriver();

	if (!driver->queryFeature(video::EVDF_RENDER_TO_TARGET))
	{
		device->closeDevice();
		device->run();
		device->drop();
		return true;
	}
	logTestString("Testing driver %ls\n", driver->getName());

	video::ITexture* RenderTarget=driver->addRenderTargetTexture(core::dimension2d<u32>(64,64), "BASEMAP");

	video::ITexture *tex=driver->getTexture("../media/water.jpg");

	driver->beginScene(true, true, video::SColor(255,255,0,255));//Backbuffer background is pink

	//draw the 256x256 water image on the rendertarget:
	driver->setRenderTarget(RenderTarget,true,true,video::SColor(255,0,0,255));//Rendertarget background is blue
	driver->draw2DImage(tex, core::position2d<s32>(0,0), core::recti(0,0,32,32));
	driver->setRenderTarget(0, false);

	//draw the rendertarget on screen:
	//this should normally draw a 64x64 image containing a 32x32 image in the top left corner
	driver->draw2DImage(RenderTarget, core::position2d<s32>(0,0));
	driver->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-draw2DImageRTT.png");

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

// draws a complex (interlaced, paletted, alpha) png image
bool testWithPNG(video::E_DRIVER_TYPE driverType)
{
	// create device

	IrrlichtDevice *device = createDevice(driverType, core::dimension2d<u32>(160,120));

	if (device == 0)
		return true; // could not create selected driver.

	video::IVideoDriver* driver = device->getVideoDriver();

	logTestString("Testing driver %ls\n", driver->getName());

	video::ITexture *tex=driver->getTexture("media/RedbrushAlpha-0.25.png");

	driver->beginScene(true, true, video::SColor(255,40,40,255));//Backbuffer background is blue
	driver->draw2DImage(tex, core::recti(0,0,160,120), core::recti(0,0,256,256), 0, 0, true);
	driver->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-draw2DImagePNG.png", 98.f);

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

// draws an image and checks if the written example equals the original image
bool testExactPlacement(video::E_DRIVER_TYPE driverType)
{
	// create device

	IrrlichtDevice *device = createDevice(video::EDT_DIRECT3D9, core::dimension2d<u32>(160,120), 32);

	if (device == 0)
		return true; // could not create selected driver.

	video::IVideoDriver* driver = device->getVideoDriver();

	if (driver->getColorFormat() != video::ECF_A8R8G8B8 || !driver->queryFeature(video::EVDF_RENDER_TO_TARGET))
	{
		device->closeDevice();
		device->run();
		device->drop();
		return true;
	}
	logTestString("Testing driver %ls\n", driver->getName());

	video::ITexture* rt=driver->addRenderTargetTexture(core::dimension2d<u32>(32,32), "rt1");
	video::ITexture *tex=driver->getTexture("../media/fireball.bmp");

	driver->beginScene(true, true, video::SColor(255,40,40,255));//Backbuffer background is blue
	driver->setRenderTarget(rt);
	driver->draw2DImage(tex, core::recti(0,0,32,32), core::recti(0,0,64,64));
	driver->setRenderTarget(0);
	driver->endScene();

	video::IImage* img = driver->createImage(rt, core::vector2di(), rt->getSize());
	driver->writeImageToFile(img, "results/fireball.png");
	bool result = binaryCompareFiles("media/fireball.png", "results/fireball.png");

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

}

bool draw2DImage()
{
	bool result = true;
	TestWithAllDrivers(testWithRenderTarget);
	TestWithAllDrivers(testExactPlacement);
	TestWithAllHWDrivers(testWithPNG);
	return result;
}
