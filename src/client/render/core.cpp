/*
Minetest
Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>
Copyright (C) 2017 numzero, Lobachevskiy Vitaliy <numzer0@yandex.ru>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "core.h"
#include "client/camera.h"
#include "client/client.h"
#include "client/clientmap.h"
#include "client/hud.h"
#include "client/minimap.h"
#include "client/shadows/dynamicshadowsrender.h"
#include "client/shader.h"


#if ENABLE_GLES
#ifdef _IRR_COMPILE_WITH_OGLES1_
#include <GLES/gl.h>
#else
#include <GLES2/gl2.h>
#endif
#else
#ifndef __APPLE__
#include <GL/gl.h>
#else
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#endif
#endif

video::ITexture* rt;
io::path ppVSFile;
io::path ppPSFile;

class QuadShaderCallBack : public video::IShaderConstantSetCallBack
{
public:

	virtual void OnSetConstants(video::IMaterialRendererServices* services,
			s32 userData)
	{
		core::dimension2d<u32> size = rt->getSize();

		// get texture size array
		f32 textureSize[] = 
		{
			(f32)size.Width, (f32)size.Height,
		};

		// set texture size to vertex shader
		services->setVertexShaderConstant("TextureSize", reinterpret_cast<f32*>(textureSize), 2);
		
		// set texture for an OpenGL driver
		s32 textureID = 0;
		services->setPixelShaderConstant("TextureSampler", &textureID, 1);
	}
};

class ScreenQuad : public IReferenceCounted
{
public:

	ScreenQuad(video::IVideoDriver* driver)
		: Driver(driver)
	{
		// --------------------------------> u
		// |[1](-1, 1)----------[2](1, 1)
		// | | ( 0, 0)		 / | (1, 0)
		// | |			   /   |
		// | |			 /	 |
		// | |		   /	   |
		// | |		 /		 |
		// | |	   /		   |
		// | |	 /			 |
		// | |   /			   |
		// | | /				 |
		// |[0](-1, -1)---------[3](1, -1)
		// |   ( 0,  1)			(1,  1)
		// V 
		// v

		/*
		A screen quad is composed of two adjacent triangles with 4 vertices.
		Vertex [0], [1] and [2] create the first triangle and Vertex [0], 
		[2] and [3] create the second one. To map texture on the quad, UV 
		coordinates are assigned to the vertices. The origin of UV coordinate 
		locates on the top-left corner. And the value of UVs range from 0 to 1.
		*/

		// define vertices array

		Vertices[0] = irr::video::S3DVertex(-1.0f, -1.0f, 0.0f, 1, 1, 0, irr::video::SColor(0,255,255,255), 0.0f, 1.0f);
		Vertices[1] = irr::video::S3DVertex(-1.0f,  1.0f, 0.0f, 1, 1, 0, irr::video::SColor(0,255,255,255), 0.0f, 0.0f);
		Vertices[2] = irr::video::S3DVertex( 1.0f,  1.0f, 0.0f, 1, 1, 0, irr::video::SColor(0,255,255,255), 1.0f, 0.0f);
		Vertices[3] = irr::video::S3DVertex( 1.0f, -1.0f, 0.0f, 1, 1, 0, irr::video::SColor(0,255,255,255), 1.0f, 1.0f);

		// define indices for triangles

		Indices[0] = 0;
		Indices[1] = 1;
		Indices[2] = 2;
		Indices[3] = 0;
		Indices[4] = 2;
		Indices[5] = 3;

		// turn off lighting as default
		Material.setFlag(video::EMF_LIGHTING, false);

		// set texture warp settings to clamp to edge pixel
		for (u32 i = 0; i < video::MATERIAL_MAX_TEXTURES; i++)
		{
			Material.TextureLayer[i].TextureWrapU = video::ETC_CLAMP_TO_EDGE;
			Material.TextureLayer[i].TextureWrapV = video::ETC_CLAMP_TO_EDGE;
		}
	}

	virtual ~ScreenQuad() {}

public:

	//! render the screen quad
	virtual void render()
	{
		// set the material of screen quad
		Driver->setMaterial(Material);

		// set matrices to fit the quad to full viewport
		Driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
		Driver->setTransform(video::ETS_VIEW, core::IdentityMatrix);
		Driver->setTransform(video::ETS_PROJECTION, core::IdentityMatrix);

		// draw screen quad 
		Driver->drawVertexPrimitiveList(Vertices, 4, Indices, 2);
	}

	//! sets a flag of material to a new value
	virtual void setMaterialFlag(video::E_MATERIAL_FLAG flag, bool newvalue)
	{
		Material.setFlag(flag, newvalue);
	}

	//! sets the texture of the specified layer in material to the new texture.
	void setMaterialTexture(u32 textureLayer, video::ITexture* texture)
	{
		Material.setTexture(textureLayer, texture);
	}

	//! sets the material type to a new material type.
	virtual void setMaterialType(video::E_MATERIAL_TYPE newType)
	{
		Material.MaterialType = newType;
	}

private:

	video::IVideoDriver *Driver;
	video::S3DVertex Vertices[4];
	u16 Indices[6];
	video::SMaterial Material;
};

ScreenQuad *screenQuad;

RenderingCore::RenderingCore(IrrlichtDevice *_device, Client *_client, Hud *_hud)
	: device(_device), driver(device->getVideoDriver()), smgr(device->getSceneManager()),
	guienv(device->getGUIEnvironment()), client(_client), camera(client->getCamera()),
	mapper(client->getMinimap()), hud(_hud),
	shadow_renderer(nullptr)
{
	screensize = driver->getScreenSize();
	virtual_size = screensize;
	screenQuad = new ScreenQuad(driver);

	if (g_settings->getBool("enable_shaders") &&
			g_settings->getBool("enable_dynamic_shadows")) {
		shadow_renderer = new ShadowRenderer(device, client);
	}
}

RenderingCore::~RenderingCore()
{
	clearTextures();
	delete shadow_renderer;
	delete screenQuad;
}

void RenderingCore::initialize()
{
	ppPSFile = getShaderPath("postprocessing_shader","pp_opengl.frag").c_str();
	ppVSFile = getShaderPath("postprocessing_shader","pp_opengl.vert").c_str();
	if (driver->queryFeature(video::EVDF_RENDER_TO_TARGET))
	{
		rt = driver->addRenderTargetTexture(screensize, "RTT1");
	}

	// turn off mip maps and bilinear filter since we do not want interpolated result
	screenQuad->setMaterialFlag(video::EMF_USE_MIP_MAPS, false);
	screenQuad->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);

	// set quad texture to RTT we just created
	screenQuad->setMaterialTexture(0, rt);

	// create materials

	video::IGPUProgrammingServices* gpu = driver->getGPUProgrammingServices();
	s32 ppMaterialType = 0;

	if (gpu)
	{
		// We write a QuadShaderCallBack class that implements OnSetConstants 
		// callback of IShaderConstantSetCallBack class at the beginning of 
		// this tutorial. We set shader constants in this callback.
	
		// create an instance of callback class

		QuadShaderCallBack* mc = new QuadShaderCallBack();

		// create material from post processing shaders

		ppMaterialType = gpu->addHighLevelShaderMaterialFromFiles(
			ppVSFile, "vertexMain", video::EVST_VS_1_1,
			ppPSFile, "pixelMain", video::EPST_PS_1_1, mc);

		mc->drop();
	}

	// set post processing material type to the quad
	screenQuad->setMaterialType((video::E_MATERIAL_TYPE)ppMaterialType);

	// have to be called late as the VMT is not ready in the constructor:
	initTextures();
	if (shadow_renderer)
		shadow_renderer->initialize();
}

void RenderingCore::updateScreenSize()
{
	virtual_size = screensize;
	if (driver->queryFeature(video::EVDF_RENDER_TO_TARGET))
	{
		rt = driver->addRenderTargetTexture(screensize, "RTT1");
	}

	// turn off mip maps and bilinear filter since we do not want interpolated result
	screenQuad->setMaterialFlag(video::EMF_USE_MIP_MAPS, false);
	screenQuad->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);

	// set quad texture to RTT we just created
	screenQuad->setMaterialTexture(0, rt);

	// create materials

	video::IGPUProgrammingServices* gpu = driver->getGPUProgrammingServices();
	s32 ppMaterialType = 0;

	if (gpu)
	{
		// We write a QuadShaderCallBack class that implements OnSetConstants 
		// callback of IShaderConstantSetCallBack class at the beginning of 
		// this tutorial. We set shader constants in this callback.
	
		// create an instance of callback class

		QuadShaderCallBack* mc = new QuadShaderCallBack();

		// create material from post processing shaders

		ppMaterialType = gpu->addHighLevelShaderMaterialFromFiles(
			ppVSFile, "vertexMain", video::EVST_VS_1_1,
			ppPSFile, "pixelMain", video::EPST_PS_1_1, mc);

		mc->drop();
	}

	// set post processing material type to the quad
	screenQuad->setMaterialType((video::E_MATERIAL_TYPE)ppMaterialType);
	clearTextures();
	initTextures();
}

void RenderingCore::draw(video::SColor _skycolor, bool _show_hud, bool _show_minimap,
		bool _draw_wield_tool, bool _draw_crosshair)
{
	v2u32 ss = driver->getScreenSize();
	if (screensize != ss) {
		screensize = ss;
		updateScreenSize();
	}
	skycolor = _skycolor;
	show_hud = _show_hud;
	show_minimap = _show_minimap;
	draw_wield_tool = _draw_wield_tool;
	draw_crosshair = _draw_crosshair;

	beforeDraw();
	drawAll();
}

void RenderingCore::draw3D()
{
	if (shadow_renderer) {
		// Shadow renderer will handle the draw stage
		shadow_renderer->setClearColor(skycolor);
		if (rt) {
			shadow_renderer->update(rt);
			// after rendering to RTT, we change render target back
			driver->setRenderTarget(0, false, false, video::SColor(255,0,0,0));
			// render screen quad to apply post processing
			screenQuad->render();
		} else {
			shadow_renderer->update();
		}
	} else {
		smgr->drawAll();
	}

	driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
	if (!show_hud)
		return;
	hud->drawBlockBounds();
	hud->drawSelectionMesh();
	if (draw_wield_tool)
		camera->drawWieldedTool();
}

void RenderingCore::drawHUD()
{
	if (show_hud) {
		if (draw_crosshair)
			hud->drawCrosshair();
	
		hud->drawHotbar(client->getEnv().getLocalPlayer()->getWieldIndex());
		hud->drawLuaElements(camera->getOffset());
		camera->drawNametags();
		if (mapper && show_minimap)
			mapper->drawMinimap();
	}
	guienv->drawAll();
}

void RenderingCore::drawPostFx()
{
	client->getEnv().getClientMap().renderPostFx(camera->getCameraMode());
}
