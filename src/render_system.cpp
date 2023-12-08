// internal
#include "render_system.hpp"
#include <SDL.h>
#include <iostream>
#include "tiny_ecs_registry.hpp"

void RenderSystem::drawTexturedMesh(Entity entity,
									const mat3& view_matrix,
									const mat3& projection)
{
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	mat3 transform = createModelMatrix(entity);	// Model matrix

	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer

	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED || render_request.used_effect == EFFECT_ASSET_ID::SPRITESHEET)
	{

		// Skip rendering spaceship home screen if player is at world 
		if (render_request.used_texture == TEXTURE_ASSET_ID::SPACESHIP_HOME || 
			render_request.used_texture == TEXTURE_ASSET_ID::BLACK_BLOCK || 
			render_request.used_texture == TEXTURE_ASSET_ID::SPACESHIP_HOME_HEALTH || 
			render_request.used_texture == TEXTURE_ASSET_ID::SPACESHIP_HOME_AMMO || 
			render_request.used_texture == TEXTURE_ASSET_ID::SPACESHIP_HOME_FOOD || 
			render_request.used_texture == TEXTURE_ASSET_ID::STORAGE_FRAME
		) {
			if (!registry.players.components[0].is_home) {
				return;
			}
		}

		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();
		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
		
		if (registry.animations.has(entity)) {
			// set the frame for shader
			Animation& animation = registry.animations.get(entity);


			GLint frame_uloc = glGetUniformLocation(program, "spriteFrame");
			glUniform2f(frame_uloc, animation.framex, animation.framey);



			// Set the frame dimensions for the player
			GLint frameDimensions_uloc = glGetUniformLocation(program, "frameDimensions");
			glUniform2f(frameDimensions_uloc, animation.frame_dimension_w, animation.frame_dimension_h);
			gl_has_errors();
		}


	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::SALMON || render_request.used_effect == EFFECT_ASSET_ID::PEBBLE)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)sizeof(vec3));
		gl_has_errors();
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec4 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec4(1);
	glUniform4fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform);
	GLuint view_loc = glGetUniformLocation(currProgram, "view");
	glUniformMatrix3fv(view_loc, 1, GL_FALSE, (float*)&view_matrix);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();

}

// refernece on instanced rendering: https://learnopengl.com/Advanced-OpenGL/Instancing
// refernece on instanced rendering: https://ogldev.org/www/tutorial33/tutorial33.html
// Basically first group the entities needed for the particle effect, compute the transform for each particle, setup the buffer for transforms and pass it to shader.
// The way I did is by storing mat3 as 3 vector, each of them is a attribute in the vertex buffer.
//  This way by calling 
// drawElementsInstance, we reused the same geometry and do instanced rendering by drawing each instance with a different transform.
void RenderSystem::drawParticles(Entity entity,const mat3& view_matrix,const mat3& projection)
{
	

	assert(registry.instancedRenderRequests.has(entity));
	const InstancedRenderRequest& render_request = registry.instancedRenderRequests.get(entity);

	// CREATE MODEL MATRIX FOR ALL ENTITIES
	std::vector<mat3> modelMatrixes;

	auto& particle_entities = render_request.entities;

	for (int i = 0; i < particle_entities.size(); i++) {
		if (registry.motions.has(particle_entities[i])) {
			mat3 transform = createModelMatrix(particle_entities[i]);	
			modelMatrixes.push_back(transform);
		}
		
	}

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();


	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	// USING SPRITE WITH A QUAD
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTUREPARTICLE)
	{

		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(TexturedVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void*)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();
		assert(registry.instancedRenderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)render_request.used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();

		// else if (render_request.used_texture == TEXTURE_ASSET_ID::PLAYER_PARTICLE) {
		// 	GLint isPlayer_uloc = glGetUniformLocation(program, "isPlayer");
		// }

	}
	// USING CIRCLE GEOMETRY 
	else if (render_request.used_effect == EFFECT_ASSET_ID::PARTICLE)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)sizeof(vec3));
		gl_has_errors();
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// create a buffer to store model matrixes for all particles
	GLuint matrixBufferID;
	GLuint Matrix_location = glGetAttribLocation(program, "modelMatrix");
	
	glGenBuffers(1, &matrixBufferID);

	// tell gpu to operate on this buffer
	glBindBuffer(GL_ARRAY_BUFFER, matrixBufferID);

	// loading the data
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(modelMatrixes[0]) * modelMatrixes.size(), modelMatrixes.data(), GL_DYNAMIC_DRAW);

	// telling gpu how to read the data in the buffer, since attribute cant be bigger than vec4, we use a loop to enable and configure
	// 3 consecutive vertex attributes. 5th parameter is the stride distance between each vertex and 
	// 6th is the offset to get to the vertex attribute
	for (unsigned int i = 0; i < 3; i++) {
		glEnableVertexAttribArray(Matrix_location + i);
		glVertexAttribPointer(Matrix_location + i, 3, GL_FLOAT, GL_FALSE,
			sizeof(glm::mat3), (const GLvoid*)(sizeof(GLfloat) * i * 3));
		glVertexAttribDivisor(Matrix_location + i, 1);
	}
	gl_has_errors();



	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec4 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec4(1);
	glUniform4fv(color_uloc, 1, (float*)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	//GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	//glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&modelMatrixes);
	GLuint view_loc = glGetUniformLocation(currProgram, "view");
	glUniformMatrix3fv(view_loc, 1, GL_FALSE, (float*)&view_matrix);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer and number of instance depending on number of particles
	glDrawElementsInstanced(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr, particle_entities.size());
	gl_has_errors();

	// Free up the buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

// draw the intermediate texture to the screen

void RenderSystem::drawToScreen()
{
	
	// Setting shaders
	// get the texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::FOG]);
	gl_has_errors();
	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER assoZZciates
																	 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();
	const GLuint fog_program = effects[(GLuint)EFFECT_ASSET_ID::FOG];
	float scaled_fow_radius = fow_radius / 18.0f;

	// set fog of war radius uniform
	GLuint fowRadius_uloc = glGetUniformLocation(fog_program, "scaled_down_fowRadius");
	glUniform1fv(fowRadius_uloc,1, (float*) &scaled_fow_radius);

	// set fow darken factor uniforms
	GLuint fow_Darken_factor_uloc = glGetUniformLocation(fog_program, "fow_darken_factor");
	glUniform1fv(fow_Darken_factor_uloc, 1, (float*)&fow_darken_factor);
	 
	// set enableFow uniforms
	GLuint enable_fow_uloc = glGetUniformLocation(fog_program, "enableFow");
	glUniform1iv(enable_fow_uloc, 1, (int*)&enableFow);

	// Aspect ratio uniform
	GLuint aspect_ratio_uloc = glGetUniformLocation(fog_program, "aspect_ratio");
	glUniform2iv(aspect_ratio_uloc, 1, (int*)&aspect_ratio);

	gl_has_errors();
	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(fog_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();
	
	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer
	gl_has_errors();
}

/// Totally not a blatant copy of the template code.
void RenderSystem::drawTerrain(const mat3& view_2D, const mat3& projection_2D)
{
	GLuint program = effects[(GLuint)EFFECT_ASSET_ID::TERRAIN];
	glUseProgram(program);

	const GLuint vbo = vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::TERRAIN];
	const GLuint ibo = index_buffers[(GLuint)GEOMETRY_BUFFER_ID::TERRAIN];

	// Load vertex and index buffers from GPU memory from GPU "address" vbo and ibo
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Get the position of each attributes and uniforms inside the shader
	// The asserts are to make sure they exist within the shader files.
	GLint viewMatrix_uloc = glGetUniformLocation(program, "viewMatrix");
	assert(viewMatrix_uloc >= 0);
	GLint projectionMatrix_uloc = glGetUniformLocation(program, "projectionMatrix");
	assert(projectionMatrix_uloc >= 0);
	/*
	GLint uv_offsets_uloc = glGetUniformLocation(program, "uv_offsets");
	assert(uv_offsets_uloc >= 0);
	GLint texel_offsets_uloc = glGetUniformLocation(program, "texel_offsets");
	assert(texel_offsets_uloc >= 0);
	*/
	GLint in_position_loc = glGetAttribLocation(program, "in_position");
	assert(in_position_loc >= 0);
	GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
	assert(in_texcoord_loc >= 0);
	GLint in_tex_i_loc = glGetAttribLocation(program, "in_tex_i");
	assert(in_tex_i_loc >= 0);
	/*
	GLint in_flags_loc = glGetAttribLocation(program, "in_flags");
	assert(in_flags_loc >= 0);
	GLint in_fvalue_loc = glGetAttribLocation(program, "in_fvalue");
	assert(in_fvalue_loc >= 0);
	*/

	// Calculates the offsets of each field within BatchedVertex
	// Thank you to: https://www.cs.ubc.ca/~rhodin/2023_2024_CPSC_427/resources/minimal_mesh_rendering.cpp
	const auto SIZE_OF_EACH_VERTEX = sizeof(BatchedVertex);
	const void* POSITION_OFFSET = reinterpret_cast<void*>(offsetof(BatchedVertex, position));
	const void* UV_OFFSET = reinterpret_cast<void*>(offsetof(BatchedVertex, texCoords));
	const void* TEX_INDEX_OFFSET = reinterpret_cast<void*>(offsetof(BatchedVertex, texIndex));
	const void* FLAGS_OFFSET = reinterpret_cast<void*>(offsetof(BatchedVertex, flags));
	const void* FRAME_VALUE_OFFSET = reinterpret_cast<void*>(offsetof(BatchedVertex, frameValue));

	// Vertex position
	glEnableVertexAttribArray(in_position_loc);
	gl_has_errors();
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, SIZE_OF_EACH_VERTEX, POSITION_OFFSET);
	gl_has_errors();

	// Texture uv
	glEnableVertexAttribArray(in_texcoord_loc);
	gl_has_errors();
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, SIZE_OF_EACH_VERTEX, UV_OFFSET);
	gl_has_errors();

	// Texture index used for the 2D texture array
	glEnableVertexAttribArray(in_tex_i_loc);
	gl_has_errors();
	glVertexAttribIPointer(in_tex_i_loc, 1, GL_UNSIGNED_SHORT, SIZE_OF_EACH_VERTEX, TEX_INDEX_OFFSET);
	gl_has_errors();

	/*
	glEnableVertexAttribArray(in_flags_loc);
	gl_has_errors();
	glVertexAttribPointer(in_flags_loc, 1, GL_UNSIGNED_BYTE, GL_FALSE, SIZE_OF_EACH_VERTEX, FLAGS_OFFSET);
	gl_has_errors();

	glEnableVertexAttribArray(in_fvalue_loc);
	gl_has_errors();
	glVertexAttribPointer(in_fvalue_loc, 1, GL_UNSIGNED_BYTE, GL_FALSE, SIZE_OF_EACH_VERTEX, FRAME_VALUE_OFFSET);
	gl_has_errors();
	*/

	//const vec3 color = vec3(0, 1, 0);

	// Camera and projectiin matrices
	glUniformMatrix3fv(viewMatrix_uloc, 1, GL_FALSE, (float*)&view_2D);
	gl_has_errors();
	glUniformMatrix3fv(projectionMatrix_uloc, 1, GL_FALSE, (float*)&projection_2D);
	gl_has_errors();
	/*
	glUniform2fv(uv_offsets_uloc, 1, (float*)&terrain_sheet_uv);
	gl_has_errors();
	glUniform2fv(texel_offsets_uloc, 1, (float*)&terrain_texel_offset);
	gl_has_errors();
	glUniform3fv(color_uloc, 1, (float*)&color);
	gl_has_errors();
	*/

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint32_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();
	size /= sizeof(uint32_t);

	// Draw!
	// Recall that we're using GL_UNSIGNED_INT because the index buffer are int32_t's
	glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, nullptr);
	gl_has_errors();

	// Free up the buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	glClearColor(0, 0, 0, 1.0);
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
	// and alpha blending, one would have to sort
	// sprites back to front
	gl_has_errors();

	Entity main_camera = registry.get_main_camera();
	//	Generate view matrix. This converts world space coordinates into camera-relative coords from its POV.
	//	The reason why we inverse:
	//		A regular TRS matrix from the given Motion component converts local (Entity) coordinates to
	//		the world space. 
	//		
	//		What we want here is the opposite: turning world space values into "camera" or "view" space.
	//		We can achieve this by inversing the local -> world matrix (regular Transform matrix!)
	//		This will give us a world -> local matrix.
	mat3 view_2D = inverse(createModelMatrix(main_camera));

	// Generate projection matrix. This maps camera-relative coords to pixel/window coordinates.
	mat3 projection_2D = createScaledProjectionMatrix();

	std::vector<Entity> layer_1_entities;
	std::vector<Entity> layer_2_entities;
	std::vector<Entity> layer_3_entities;
	std::vector<Entity> layer_4_entities;
	std::vector<Entity> layer_5_entities;
	Entity player_entity = registry.players.entities[0];

	for (Entity entity : registry.renderRequests.entities)
	{
		if (!registry.motions.has(entity))
			continue;
		// Note, its not very efficient to access elements indirectly via the entity
		// albeit iterating through all Sprites in sequence. A good point to optimize
		// re-sort them in different queue of render request based on layer they belong to
		if (registry.renderRequests.get(entity).layer_id == RENDER_LAYER_ID::LAYER_1) {

			// if entity is item or mob
			if (registry.items.has(entity) || (registry.mobs.has(entity)) || registry.renderRequests.get(entity).used_texture == TEXTURE_ASSET_ID::RED_BLOCK) {

				// put in draw array if distance to player is close enough
				if ((distance(registry.motions.get(player_entity).position, registry.motions.get(entity).position) < fow_radius) || enableFow == 0) {
					layer_1_entities.push_back(entity);
				}
			}
			else {
				// draw everything else
				layer_1_entities.push_back(entity);
			}
		}
		else if (registry.renderRequests.get(entity).layer_id == RENDER_LAYER_ID::LAYER_2) {
			layer_2_entities.push_back(entity);
		}
		else if (registry.renderRequests.get(entity).layer_id == RENDER_LAYER_ID::LAYER_3) {
			layer_3_entities.push_back(entity);
		}
		else if (registry.renderRequests.get(entity).layer_id == RENDER_LAYER_ID::LAYER_4) {
			layer_4_entities.push_back(entity);
		} 
		else if (registry.renderRequests.get(entity).layer_id == RENDER_LAYER_ID::LAYER_5) {
			layer_5_entities.push_back(entity);
		}
		else {
			assert(registry.renderRequests.get(entity).layer_id != RENDER_LAYER_ID::LAYER_COUNT && "entity render request with incorrect layer ID (LAYER_COUNT)");
		}
	}

	// Only two layer are supported for now
	std::vector<Entity> instanced_layer_1_entities;
	std::vector<Entity> instanced_layer_2_entities;

	for (int i = 0; i < registry.instancedRenderRequests.size(); i++) {
		if (registry.instancedRenderRequests.components[i].layer_id == RENDER_LAYER_ID::LAYER_1) {
			instanced_layer_1_entities.push_back(registry.instancedRenderRequests.entities[i]);
		}
		else {
			instanced_layer_2_entities.push_back(registry.instancedRenderRequests.entities[i]);

		}
	}
	


	// Draw all textured meshes that have a position and size component in corresponded order to achieve layering. Could be optimize later

	// Render terrain first
	drawTerrain(view_2D, projection_2D);

	

	for (Entity entity : layer_1_entities) {
		drawTexturedMesh(entity, view_2D, projection_2D);
	}

	// DRAW all particle effects
	for (Entity entity : instanced_layer_1_entities) {
		drawParticles(entity, view_2D, projection_2D);
	}
	

	for (Entity entity : layer_2_entities) {
		drawTexturedMesh(entity, view_2D, projection_2D);
	}

	// DRAW all particle effects
	for (Entity entity : instanced_layer_2_entities) {
		drawParticles(entity, view_2D, projection_2D);
	}

	// added guard to turn off while in debug mode 
	if (debugging.in_debug_mode == false) {
		for (Entity entity : layer_3_entities) {

			drawTexturedMesh(entity, view_2D, projection_2D);
		}
	}
	
	// Truely render to the screen. Multipass rendering with fog program
	drawToScreen();

	// Re-enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Draw for UI elements
	
	if (!debugging.hide_ui) {
		for (Entity entity : layer_4_entities) {
			drawTexturedMesh(entity, view_2D, projection_2D);
		}
	}

	// TODO: Process text rendering here and ONLY here.
	// If you render text before drawToScreen(), then fog of war will dim it.
	// If you render text before the layer_4_entities for loop, then UI will write over it
	for (Entity entity : registry.texts.entities) {
		Text& text = registry.texts.get(entity);
		Motion& motion = registry.motions.get(entity);

		renderText(text.str, motion.position.x, motion.position.y, text.scale, text.color, projection_2D, view_2D);
	}

	// For tutorial dialogs 
	for (Entity entity : layer_5_entities) {
		drawTexturedMesh(entity, view_2D, projection_2D);
	}

	// Do not touch anything past this point.
	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

// Set up and teardown of this function is identical to draw()
void RenderSystem::drawStartScreens() {
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	glClearColor(0, 0, 0, 1.0);
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front

	// Generate projection matrix. This is unscaled (does not take into account tile width).
	mat3 projection_2D = createUnscaledProjectionMatrix();

	// Only track layer 1, 2, and 3 entities. There should not be any other layer active at this stage.
	std::vector<Entity> layer_1_entities;
	std::vector<Entity> layer_2_entities;
	std::vector<Entity> layer_3_entities;
	for (Entity entity : registry.renderRequests.entities) {
		if (registry.renderRequests.get(entity).layer_id == RENDER_LAYER_ID::LAYER_1) {
			layer_1_entities.push_back(entity);
		} else if (registry.renderRequests.get(entity).layer_id == RENDER_LAYER_ID::LAYER_2) {
			layer_2_entities.push_back(entity);
		} else if (registry.renderRequests.get(entity).layer_id == RENDER_LAYER_ID::LAYER_3) {
			layer_3_entities.push_back(entity);
		}
	}

	// Render terrain first
	Entity main_camera = registry.get_main_camera();
	mat3 view_2D = inverse(createModelMatrix(main_camera));
	drawTerrain(view_2D, createScaledProjectionMatrix());

	// Truely render to the screen. Multipass rendering with fog program
	drawToScreen();

	// Re-enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Draw the meshes
	for (Entity entity : layer_1_entities) {
		// pass identity matrix for view_matrix since main camera not exist
		drawTexturedMesh(entity, mat3(1.f), projection_2D);
	}
	for (Entity entity : layer_2_entities) {
		// pass identity matrix for view_matrix since main camera not exist
		drawTexturedMesh(entity, mat3(1.f), projection_2D);
	}
	for (Entity entity : layer_3_entities) {
		// pass identity matrix for view_matrix since main camera not exist
		drawTexturedMesh(entity, mat3(1.f), projection_2D);
	}

	for (Entity entity : registry.texts.entities) {
		Text& text = registry.texts.get(entity);
		Motion& motion = registry.motions.get(entity);

		renderText(text.str, motion.position.x, motion.position.y, text.scale, text.color, projection_2D, mat3(1.f));
	}

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

/// <summary>
/// Generates a TRS matrix from an entity. Entity must have a Motion component.
/// </summary>
/// <param name="entity">Entity with a motion component to generate from</param>
/// <returns>A TRS Matrix that converts from local space to the parent space (usually world).</returns>
mat3 RenderSystem::createModelMatrix(Entity entity)
{
	Motion& motion = registry.motions.get(entity);

	Transform modelMatrix;
	modelMatrix.translate(motion.position);
	modelMatrix.rotate(motion.angle);
	modelMatrix.scale(motion.scale);

	// Parent space -> local/model space? Inverse this TRS matrix!
	return modelMatrix.mat;
}

/// <summary>
/// Generates an orthogonal projection matrix. 
/// </summary>
/// <returns>An orthogonal projection matrix relative to the window size</returns>
mat3 RenderSystem::createScaledProjectionMatrix()
{
	// Othogonal projection matrix.
	// Same code as the template since it scales with aspect ratio.
	int x = window_resolution.x;
	int y = window_resolution.y;

	// We must scale how each tile is affected by the resolution because:
	//	1. We don't want the player to see more of the map if they play on a larger resolution
	const double s = static_cast<double>(x) / target_resolution.x;
	const float tile_size_px_scaled = tile_size_px * s;

	float left = -x / 2.f; // Modified these values so the middle of the screen is now 0,0
	float top = -y / 2.f;
	
	float right = x / 2.f;
	float bottom = y /2.f;

	float sx = 2.f / (right - left) * tile_size_px_scaled;	// We finally scale the world space -> screen space tile size mappings
	float sy = 2.f / (top - bottom) * tile_size_px_scaled;
	float tx = -(right + left) / (right - left) * tile_size_px_scaled;
	float ty = -(top + bottom) / (top - bottom) * tile_size_px_scaled;

	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

mat3 RenderSystem::createUnscaledProjectionMatrix()
{
	// Code taken from salmon template
	float left = 0.f;
	float top = 0.f;

	gl_has_errors();
	float right = (float) window_resolution.x;
	float bottom = (float) window_resolution.y;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

void RenderSystem::changeTerrainData(Entity cell, unsigned int i, TerrainCell& data, uint8_t frameValue)
{
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::TERRAIN]);
	gl_has_errors();
	std::vector<BatchedVertex> vertices;
	mat3 transform = createModelMatrix(cell);
	uint8_t vertex_flags = directional_terrain.count(data.terrain_type) ? DIRECTIONAL : 0;
	makeQuadVertices(transform, (uint16_t)data.terrain_type, vertices, vertex_flags, frameValue);

	int x = i * sizeof(BatchedVertex) * 4;
	int y = sizeof(BatchedVertex) * 4;
	glBufferSubData(GL_ARRAY_BUFFER, i * sizeof(BatchedVertex) * 4, sizeof(BatchedVertex) * 4, vertices.data());
	gl_has_errors();
}

void RenderSystem::empty_terrain_buffer()
{
	// Tell GPU to deallocate those buffers
	glDeleteBuffers(1, &vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::TERRAIN]);
	glDeleteBuffers(1, &index_buffers[(GLuint)GEOMETRY_BUFFER_ID::TERRAIN]);

	// Tell GPU to reallocate those buffers
	glGenBuffers(1, &vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::TERRAIN]);
	glGenBuffers(1, &index_buffers[(GLuint)GEOMETRY_BUFFER_ID::TERRAIN]);
}

/// <summary>
/// Renders a string into the world
/// </summary>
/// <param name="text">The string to print</param>
/// <param name="x">The world x position where the text starts (left side)</param>
/// <param name="y">The world y position where the text starts (writes on top of y)</param>
/// <param name="scale">Scale of text relative to world/tile size</param>
/// <param name="color">Text colour</param>
/// <param name="projection_matrix">The projection matrix</param>
/// <param name="view_matrix">The camera view matrix</param>
void RenderSystem::renderText(
	std::string text, 
	float x, 
	float y, 
	float scale, 
	glm::vec3 color, 
	const mat3& projection_matrix, 
	const mat3& view_matrix) {
	// Switch to text vao
	glBindVertexArray(text_vao);
	// Load the vertex buffer allocated for "TEXT"
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint) GEOMETRY_BUFFER_ID::TEXT]);

	// Load the text shader
	const GLuint program = effects[(GLuint)EFFECT_ASSET_ID::TEXT];
	glUseProgram(program);
	gl_has_errors();

	// Pass a colour to a uniform "textColor". We expect it to stay constant for this render pass.
    glUniform3f(glGetUniformLocation(program, "textColor"), color.x, color.y, color.z);
	gl_has_errors();

	// Pass the projection matrix.
	GLuint projection_loc = glGetUniformLocation(program, "projectionMatrix");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*) &projection_matrix);
	gl_has_errors();

	// Pass the view matrix
	GLuint view_matrix_loc = glGetUniformLocation(program, "viewMatrix");
	glUniformMatrix3fv(view_matrix_loc, 1, GL_FALSE, (float*)&view_matrix);

	gl_has_errors();
	// We're telling OpenGL that we want to pass a texture to the shaders in the future
    glActiveTexture(GL_TEXTURE0);
	gl_has_errors();

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = characters[*c];

        float xpos = x + ch.bearing.x * scale / tile_size_px;	// Scaling relative to tile size to make it easier to think about
        float ypos = y + (ch.size.y - ch.bearing.y) * scale / tile_size_px; // recall up is negative, down is positive

        float w = ch.size.x * scale / tile_size_px;
        float h = -ch.size.y * scale / tile_size_px;	// recall up is negative, down is positive
        // update VBO for each character
        float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
                       
        };
        // Tell OpenGL to load in the glyph/texture associated with this character.
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
		gl_has_errors();

		// We replace the current vertex buffer with the new positions of the next character
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), (float*)vertices); 
		gl_has_errors();
        
		// We tell OpenGL to render 6 vertices. It knows how to read the vertex buffer because we said up front how to 
		// read the vertex buffer for the text VAO. 
		// VAOs allow us to let OpenGL know ahead of time what to expect when draw is called.
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.advance >> 6) * scale / tile_size_px; // bitshift by 6 to get value in pixels (2^6 = 64)
		gl_has_errors();
    }
	glBindBuffer(GL_ARRAY_BUFFER, 0);	// Release the vertex buffer, not really important
	glBindVertexArray(global_vao);	// Return to regular vao
}
