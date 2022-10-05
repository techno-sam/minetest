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
#include "voxel.h"
#include "client/mapblock_mesh.h"
#include "settings.h"
#include "mapnode.h"
#include "nodedef.h"
#include "client/content_mapblock.h"
#include "client/meshgen/collector.h"
#include "client/shader.h"
#include "profiler.h"
 
#include <iostream>
#include <cmath>
#include "client/content_cao.h"
#include "mapblock.h"
#include "mapsector.h"
//Above
#include "plain.h"
#include "secondstage.h"
#include "client/camera.h"
#include "client/client.h"
#include "client/clientmap.h"
#include "client/hud.h"
#include "client/minimap.h"
#include "client/shadows/dynamicshadowsrender.h"

/// Draw3D pipeline step
void Draw3D::drawTracersAndESP()
{
	bool draw_esp = g_settings->getBool("enable_esp");
	bool draw_tracers = g_settings->getBool("enable_tracers");
	bool draw_node_esp = g_settings->getBool("enable_node_esp");
	bool draw_node_tracers = g_settings->getBool("enable_node_tracers");
	//errorstream << "drawTracersAndESP" << std::endl;
	//bool draw_chunk_bounds = g_settings->getBool("enable_chunk_bounds");
	//bool draw_sector_bounds = g_settings->getBool("enable_sector_bounds");
	ClientEnvironment &env = client->getEnv();
	Camera *camera = client->getCamera();
	
	v3f camera_offset = intToFloat(camera->getOffset(), BS);
	
	v3f eye_pos = (camera->getPosition() + camera->getDirection() - camera_offset);
 	
 	video::SMaterial material, oldmaterial;
 	oldmaterial = driver->getMaterial2D();
	material.setFlag(video::EMF_LIGHTING, false);
	material.setFlag(video::EMF_BILINEAR_FILTER, false);
	material.setFlag(video::EMF_ZBUFFER, false);
	material.setFlag(video::EMF_ZWRITE_ENABLE, false);
	driver->setMaterial(material);


	LocalPlayer *player = env.getLocalPlayer();

 	//draw tracers and esp
 	if (draw_esp || draw_tracers) {
 		//errorstream << "draw_esp || draw_tracers is true" << std::endl;
		auto allObjects = env.getAllActiveObjects();

		for (auto &it : allObjects) {
			//errorstream << "Iterating through an object" << std::endl;
			ClientActiveObject *cao = it.second;
			if (cao->isLocalPlayer() || cao->getParent())
				continue;
			auto draw_color = video::SColor(255, 255, 255, 255);
			if (cao->isPlayer()) {
				draw_color = video::SColor(255, 255, 0, 255);
			}
			/*if (cao->isLocalPlayer()) {
				draw_color = video::SColor(255, 0, 255, 255);
			}*/
			GenericCAO *obj = dynamic_cast<GenericCAO *>(cao);
			if (! obj)
				continue;
			aabb3f box;
			if (! obj->getSelectionBox(&box))
				continue;
			v3f pos = obj->getPosition() - camera_offset;
			box.MinEdge += pos;
			box.MaxEdge += pos;
			if (draw_esp and (cao->isPlayer()==true or g_settings->getBool("player_only_esp")==false))
				driver->draw3DBox(box, draw_color);
			if (draw_tracers and (cao->isPlayer()==true or g_settings->getBool("player_only_tracers")==false))
				driver->draw3DLine(eye_pos, box.getCenter(), draw_color);
		}
		//draw tracers to localplayer
		if (g_settings->getBool("freecam")) {
			auto draw_color = video::SColor(255,0,255,255);
			GenericCAO *obj = dynamic_cast<GenericCAO *>(player);
			aabb3f box;
			v3f pos = player->getLegitPosition() - camera_offset;
			box.MinEdge += pos;
			box.MaxEdge += pos;
			if (draw_esp)
				driver->draw3DBox(box, draw_color);
			if (draw_tracers)
				driver->draw3DLine(eye_pos, box.getCenter(), draw_color);
		}
	}
	if (draw_node_esp || draw_node_tracers) {
		Map &map = env.getMap();
		std::map<v2s16, MapSector*> *sectors = map.getSectorsPtr();
		
		for (auto &sector_it : *sectors) {
			MapSector *sector = sector_it.second;
			MapBlockVect blocks;
			sector->getBlocks(blocks);
			for (MapBlock *block : blocks) {
				if (! block->mesh)
					continue;
				for (v3s16 p : block->mesh->esp_nodes) {
					v3f pos = intToFloat(p, BS) - camera_offset;
					MapNode node = map.getNode(p);
					std::vector<aabb3f> boxes;
					node.getSelectionBoxes(client->getNodeDefManager(), &boxes, node.getNeighbors(p, &map));
					video::SColor color = client->getNodeDefManager()->get(node).minimap_color;
				
					for (aabb3f box : boxes) {
						box.MinEdge += pos;
						box.MaxEdge += pos;
						if (draw_node_esp)
							driver->draw3DBox(box, color);
						if (draw_node_tracers)
							driver->draw3DLine(eye_pos, box.getCenter(), color);
					}
				}
			}
		}

	}
	
	driver->setMaterial(oldmaterial);
}

void Draw3D::run(PipelineContext &context)
{
	if (m_target)
		m_target->activate(context);

	context.device->getSceneManager()->drawAll();
	context.device->getVideoDriver()->setTransform(video::ETS_WORLD, core::IdentityMatrix);
	if (!context.show_hud)
		return;
	context.hud->drawBlockBounds();
	context.hud->drawSelectionMesh();
	if (!shadow_renderer) {//for some reason, freezes computer when shadows and HUD nodes are enabled simultaneously
	// HUD nodes (can't be drawn in drawHUD, because the transform is all wrong)
	//preparation
	ClientEnvironment &env = client->getEnv();
	Camera *camera = client->getCamera();
	
	v3f camera_offset = intToFloat(camera->getOffset(), BS);
	
	v3f eye_pos = (camera->getPosition() + camera->getDirection() - camera_offset);
 	
 	video::SMaterial material, oldmaterial;
 	oldmaterial = driver->getMaterial2D();
	/*material.setFlag(video::EMF_LIGHTING, false);
	material.setFlag(video::EMF_BILINEAR_FILTER, false);
	material.setFlag(video::EMF_ZBUFFER, false);
	material.setFlag(video::EMF_ZWRITE_ENABLE, false);
	driver->setMaterial(material);*/

	LocalPlayer *player = env.getLocalPlayer();

	const NodeDefManager *ndefmgr = client->getNodeDefManager();

	auto *m_shdrsrc = client->getShaderSource();

	//voxelmanip prep
	auto m_cache_enable_shaders = g_settings->getBool("enable_shaders");
	auto m_cache_smooth_lighting = g_settings->getBool("smooth_lighting");
	auto m_cache_enable_vbo = g_settings->getBool("enable_vbo");
	MeshMakeData mesh_data = MeshMakeData(client,m_cache_enable_shaders);
	mesh_data.m_blockpos = {0,0,0};
	//VoxelManipulator data_manip = mesh_data.m_vmanip;
	// actual logic
	for (size_t i = 0; i != player->maxHudId(); i++) {
		HudElement *e = player->getHud(i);
		if (e) {
			if (e->type == HUD_ELEM_NODE) {
				v3f pos = (e->world_pos*10) - camera_offset;
				auto align = 0;//5;
				pos.X = pos.X + align;
				pos.Y = pos.Y + align;
				pos.Z = pos.Z + align;
				v3s16 pos_int = floatToInt(pos, BS);
				content_t content_id = ndefmgr->getId(e->text2);
				MapNode node = MapNode(content_id,0,e->number);
				mesh_data.m_vmanip.setNode(pos_int,node);
				//errorstream << "Setting node with id " << content_id << ", and name " << e->text2 << ", at pos: (" << pos_int.X << ", " << pos_int.Y << ", " << pos_int.Z <<")" << std::endl;
			}
		}
	}
	MeshCollector mesh_collector;
	MapblockMeshGenerator mesh_gen = MapblockMeshGenerator(&mesh_data,&mesh_collector,mesh_data.m_client->getSceneManager()->getMeshManipulator());
	mesh_gen.generateClient();

	/*
		Convert MeshCollector to SMesh
	*/
	scene::IMesh *mclient_mesh[MAX_TILE_LAYERS];
	for (auto &m : mclient_mesh)
		m = new scene::SMesh();

	for (int layer = 0; layer < MAX_TILE_LAYERS; layer++) {
		for(u32 i = 0; i < mesh_collector.prebuffers[layer].size(); i++)
		{
			PreMeshBuffer &p = mesh_collector.prebuffers[layer][i];

			applyTileColor(p);

			// Create material
			video::SMaterial material;
			material.setFlag(video::EMF_LIGHTING, false);
			material.setFlag(video::EMF_BACK_FACE_CULLING, true);
			material.setFlag(video::EMF_BILINEAR_FILTER, false);
			material.setFlag(video::EMF_FOG_ENABLE, true);
			material.setTexture(0, p.layer.texture);

			if (m_cache_enable_shaders) {
				//errorstream << "shader id: " << p.layer.client_shader_id << std::endl;
				material.MaterialType = m_shdrsrc->getShaderInfo(
						p.layer.client_shader_id).material;
				p.layer.applyMaterialOptionsWithShaders(material);
				if (p.layer.normal_texture)
					material.setTexture(1, p.layer.normal_texture);
				material.setTexture(2, p.layer.flags_texture);
			} else {
				p.layer.applyMaterialOptions(material);
			}

			scene::SMesh *clientmesh = (scene::SMesh *)mclient_mesh[layer];

			scene::SMeshBuffer *buf = new scene::SMeshBuffer();
			buf->Material = material;
			buf->append(&p.vertices[0], p.vertices.size(),
				&p.indices[0], p.indices.size());
			clientmesh->addMeshBuffer(buf);
			buf->drop();
		}

		if (mclient_mesh[layer]) {
			// Use VBO for mesh (this just would set this for ever buffer)
			if (m_cache_enable_vbo)
				mclient_mesh[layer]->setHardwareMappingHint(scene::EHM_STATIC);
		}
	}

	//create client_drawbufs
	v3s16 block_pos = {0,0,0};
	MeshBufListList client_drawbufs;
	for (int layer = 0; layer < MAX_TILE_LAYERS; layer++) {
		scene::IMesh *mesh = mclient_mesh[layer];
		assert(mesh);
		u32 c = mesh->getMeshBufferCount();
		//errorstream << "MeshBufferCount: " << c <<std::endl;
		for (u32 i = 0; i < c; i++) {
			scene::IMeshBuffer *buf = mesh->getMeshBuffer(i);

			video::SMaterial& material = buf->getMaterial();
			video::IMaterialRenderer* rnd =
				driver->getMaterialRenderer(material.MaterialType);
			bool c_transparent = (rnd && rnd->isTransparent());
			bool c_is_transparent_pass = false;//c_transparent;
			if (true) {//c_transparent == c_is_transparent_pass) {
				if (buf->getVertexCount() == 0)
					errorstream << "Block [" //<< analyze_block(block)
						<< "] contains an empty meshbuf" << std::endl;

				/*material.setFlag(video::EMF_TRILINEAR_FILTER,
					m_cache_trilinear_filter);
				material.setFlag(video::EMF_BILINEAR_FILTER,
					m_cache_bilinear_filter);
				material.setFlag(video::EMF_ANISOTROPIC_FILTER,
					m_cache_anistropic_filter);
				material.setFlag(video::EMF_WIREFRAME,
					m_control.show_wireframe);*/

				client_drawbufs.add(buf, block_pos, layer);
			}
		}
	}
	TimeTaker draw("Drawing mesh buffers for ghost nodes");

	core::matrix4 m; // Model matrix
	v3f offset = camera_offset;
	// Render all layers in order
	for (auto &lists : client_drawbufs.lists) {
		for (MeshBufList &list : lists) {
			// Check and abort if the machine is swapping a lot
			if (draw.getTimerTime() > 2000) {
				infostream << "ClientMap::renderMapGhost(): Rendering took >2s, " <<
						"returning." << std::endl;
				return;
			}
			driver->setMaterial(list.m);

			//drawcall_count += list.bufs.size();
			for (auto &pair : list.bufs) {
				scene::IMeshBuffer *buf = pair.second;

				v3f block_wpos = intToFloat(pair.first * MAP_BLOCKSIZE, BS);
				m.setTranslation(block_wpos - offset);
				//errorstream << "block_wpos: " << PP(block_wpos) << ", translation: " << PP(block_wpos - offset) << ", vertex count: " << buf->getVertexCount() << std::endl;

				driver->setTransform(video::ETS_WORLD, m);
				driver->drawMeshBuffer(buf);
				//vertex_count += buf->getVertexCount();
			}
		}
	}
	std::string prefix = "renderGhost(ALL): ";
	g_profiler->avg(prefix + "draw ghost meshes [ms]", draw.stop(true));

	//cleanup
	driver->setMaterial(oldmaterial);
	
	for (scene::IMesh *m : mclient_mesh) {
		if (m_cache_enable_vbo) {
			for (u32 i = 0; i < m->getMeshBufferCount(); i++) {
				scene::IMeshBuffer *buf = m->getMeshBuffer(i);
				driver->removeHardwareBuffer(buf);
			}
		}
		m->drop();
	}
	
	// End HUD nodes
	}
	drawTracersAndESP();
	if (context.draw_wield_tool)
		context.client->getCamera()->drawWieldedTool();
}

void DrawHUD::run(PipelineContext &context)
{
	if (context.show_hud) {
		if (context.shadow_renderer)
			context.shadow_renderer->drawDebug();

		if (context.draw_crosshair)
			context.hud->drawCrosshair();

		context.hud->drawHotbar(context.client->getEnv().getLocalPlayer()->getWieldIndex());
		context.hud->drawLuaElements(context.client->getCamera()->getOffset());
		context.client->getCamera()->drawNametags();
		auto mapper = context.client->getMinimap();
		if (mapper && context.show_minimap)
			mapper->drawMinimap();
	}
	context.device->getGUIEnvironment()->drawAll();
}


void MapPostFxStep::setRenderTarget(RenderTarget * _target)
{
	target = _target;
}

void MapPostFxStep::run(PipelineContext &context)
{
	if (target)
		target->activate(context);

	context.client->getEnv().getClientMap().renderPostFx(context.client->getCamera()->getCameraMode());
}

void RenderShadowMapStep::run(PipelineContext &context)
{
	// This is necessary to render shadows for animations correctly
	context.device->getSceneManager()->getRootSceneNode()->OnAnimate(context.device->getTimer()->getTime());
	context.shadow_renderer->update();
}

// class UpscaleStep

void UpscaleStep::run(PipelineContext &context)
{
	video::ITexture *lowres = m_source->getTexture(0);
	m_target->activate(context);
	context.device->getVideoDriver()->draw2DImage(lowres,
			core::rect<s32>(0, 0, context.target_size.X, context.target_size.Y),
			core::rect<s32>(0, 0, lowres->getSize().Width, lowres->getSize().Height));
}

std::unique_ptr<RenderStep> create3DStage(Client *client, v2f scale)
{
	RenderStep *step = new Draw3D();
	if (g_settings->getBool("enable_shaders")) {
		RenderPipeline *pipeline = new RenderPipeline();
		pipeline->addStep(pipeline->own(std::unique_ptr<RenderStep>(step)));

		auto effect = addPostProcessing(pipeline, step, scale, client);
		effect->setRenderTarget(pipeline->getOutput());
		step = pipeline;
	}
	return std::unique_ptr<RenderStep>(step);
}

static v2f getDownscaleFactor()
{
	u16 undersampling = MYMAX(g_settings->getU16("undersampling"), 1);
	return v2f(1.0f / undersampling);
}

RenderStep* addUpscaling(RenderPipeline *pipeline, RenderStep *previousStep, v2f downscale_factor)
{
	const int TEXTURE_UPSCALE = 0;

	if (downscale_factor.X == 1.0f && downscale_factor.Y == 1.0f)
		return previousStep;

	// Initialize buffer
	TextureBuffer *buffer = pipeline->createOwned<TextureBuffer>();
	buffer->setTexture(TEXTURE_UPSCALE, downscale_factor, "upscale", video::ECF_A8R8G8B8);

	// Attach previous step to the buffer
	TextureBufferOutput *buffer_output = pipeline->createOwned<TextureBufferOutput>(buffer, TEXTURE_UPSCALE);
	previousStep->setRenderTarget(buffer_output);

	// Add upscaling step
	RenderStep *upscale = pipeline->createOwned<UpscaleStep>();
	upscale->setRenderSource(buffer);
	pipeline->addStep(upscale);

	return upscale;
}

void populatePlainPipeline(RenderPipeline *pipeline, Client *client)
{
	auto downscale_factor = getDownscaleFactor();
	auto step3D = pipeline->own(create3DStage(client, downscale_factor));
	pipeline->addStep(step3D);
	pipeline->addStep<MapPostFxStep>();

	step3D = addUpscaling(pipeline, step3D, downscale_factor);

	step3D->setRenderTarget(pipeline->createOwned<ScreenTarget>());

	pipeline->addStep<DrawHUD>();
}
