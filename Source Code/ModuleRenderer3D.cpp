#include "Globals.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModuleWindow.h"

#include "OpenGL.h"

#include "ModuleImport.h"

#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment (lib, "Glew/libx86/glew32.lib") /* link Microsoft OpenGL lib   */

ModuleRenderer3D::ModuleRenderer3D(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

// Destructor
ModuleRenderer3D::~ModuleRenderer3D()
{}

// Called before render is available
bool ModuleRenderer3D::Init()
{
	LOG("Creating 3D Renderer context");
	bool ret = true;
	
	//Create context
	context = SDL_GL_CreateContext(App->window->window);
	if(context == NULL)
	{
		LOG("OpenGL context could not be created! SDL_Error: %s", SDL_GetError());
		ret = false;
	}
	
	GLenum error = glewInit();

	if (error != GL_NO_ERROR)
	{
		LOG("Error initializing glew library! %s", SDL_GetError());
		ret = false;
	}

	if(ret == true)
	{
		//Use Vsync
		if(VSYNC && SDL_GL_SetSwapInterval(1) < 0)
			LOG("Warning: Unable to set VSync! SDL Error: %s", SDL_GetError());

		//Initialize Projection Matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		//Check for error
		GLenum error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s", gluErrorString(error));
			ret = false;
		}

		//Initialize Modelview Matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		ProjectionMatrix = perspective(60.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.125f, 512.0f);
		glLoadMatrixf(&ProjectionMatrix);

		//Check for error
		error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s", gluErrorString(error));
			ret = false;
		}
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glClearDepth(1.0f);
		
		//Initialize clear color
		glClearColor(0.f, 0.f, 0.f, 1.f);

		//Check for error
		error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s", gluErrorString(error));
			ret = false;
		}
		
		// Blend for transparency
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		GLfloat LightModelAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);
		
		lights[0].ref = GL_LIGHT0;
		lights[0].ambient.Set(0.25f, 0.25f, 0.25f, 1.0f);
		lights[0].diffuse.Set(0.75f, 0.75f, 0.75f, 1.0f);
		lights[0].SetPos(0.0f, 0.0f, 2.5f);
		lights[0].Init();
		
		GLfloat MaterialAmbient[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MaterialAmbient);

		GLfloat MaterialDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialDiffuse);
		
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_TEXTURE_2D);

		lights[0].Active(true);

		glShadeModel(GL_SMOOTH);
	}

	// Projection matrix for
	OnResize(SCREEN_WIDTH, SCREEN_HEIGHT);

#pragma region Array_Cube

	float array_cube_vertices[] = 
	{
		//Front face
		0, 0, 0,
		0, 1, 0,
		1, 0, 0,

		1, 0, 0,
		0, 1, 0,
		1, 1, 0,

		//Right face
		0, 0, 0,
		0, 0, 1,
		0, 1, 1,

		0, 0, 0,
		0, 1, 1,
		0, 1, 0,

		//Back face
		0, 0, 1,
		1, 1, 1,
		0, 1, 1,

		0, 0, 1,
		1, 0, 1,
		1, 1, 1,

		//Left face
		1, 0, 0,
		1, 1, 0,
		1, 0, 1,

		1, 0, 1,
		1, 1, 0,
		1, 1, 1,

		//Upper face
		0, 1, 0,
		0, 1, 1,
		1, 1, 0,

		1, 1, 0,
		0, 1, 1,
		1, 1, 1,

		//Bottom face
		0, 0, 0,
		1, 0, 0,
		1, 0, 1,

		0, 0, 0,
		1, 0, 1,
		0, 0, 1
	};

	for (int i = 0; i < 108; i += 3)
	{
		array_cube_vertices[i] += 3;
	}
	glGenBuffers(1, (GLuint*)&array_cube_id);
	glBindBuffer(GL_ARRAY_BUFFER, array_cube_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 108, array_cube_vertices, GL_STATIC_DRAW);
#pragma endregion

#pragma region Array Index Cube

	cube_vertices[0] = 0;		cube_vertices[1] = 0;		cube_vertices[2] = 0;
	cube_vertices[3] = 0;		cube_vertices[4] = 0;		cube_vertices[5] = 1;
	cube_vertices[6] = 1;		cube_vertices[7] = 0;		cube_vertices[8] = 1;
	cube_vertices[9] = 1;		cube_vertices[10] = 0;		cube_vertices[11] = 0;
	cube_vertices[12] = 0;		cube_vertices[13] = 1;		cube_vertices[14] = 0;
	cube_vertices[15] = 0;		cube_vertices[16] = 1;		cube_vertices[17] = 1;
	cube_vertices[18] = 1;		cube_vertices[19] = 1;		cube_vertices[20] = 1;
	cube_vertices[21] = 1;		cube_vertices[22] = 1;		cube_vertices[23] = 0;

	for (int i = 0; i < 24; i += 3)
	{
		cube_vertices[i] -= 3;
	}

	glGenBuffers(1, (GLuint*)&index_cube_vertex_id);
	glBindBuffer(GL_ARRAY_BUFFER, index_cube_vertex_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, cube_vertices, GL_STATIC_DRAW);


	//Front face
	cube_indices[0] = 0;		cube_indices[1] = 4;		cube_indices[2] = 3;
	cube_indices[3] = 3;		cube_indices[4] = 4;		cube_indices[5] = 7;

	//Right face
	cube_indices[6] = 0;		cube_indices[7] = 1;		cube_indices[8] = 5;
	cube_indices[9] = 0;		cube_indices[10] = 5;		cube_indices[11] = 4;

	//Back face
	cube_indices[12] = 1;		cube_indices[13] = 2;		cube_indices[14] = 6;
	cube_indices[15] = 1;		cube_indices[16] = 6;		cube_indices[17] = 5;
	
	//Left face
	cube_indices[18] = 2;		cube_indices[19] = 3;		cube_indices[20] = 7;
	cube_indices[21] = 2;		cube_indices[22] = 7;		cube_indices[23] = 6;
	
	//Upper face
	cube_indices[24] = 7;		cube_indices[25] = 4;		cube_indices[26] = 5;
	cube_indices[27] = 7;		cube_indices[28] = 5;		cube_indices[29] = 6;
	
	//Bottom face
	cube_indices[30] = 0;		cube_indices[31] = 3;		cube_indices[32] = 2;
	cube_indices[33] = 0;		cube_indices[34] = 2;		cube_indices[35] = 1;

	glGenBuffers(1, (GLuint*)&index_cube_index_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_cube_index_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * 36, cube_indices, GL_STATIC_DRAW);
#pragma endregion




	return ret;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate(float dt)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(App->camera->GetViewMatrix());

	// light 0 on cam pos
	lights[0].SetPos(App->camera->Position.x, App->camera->Position.y, App->camera->Position.z);

	for(uint i = 0; i < MAX_LIGHTS; ++i)
		lights[i].Render();

#pragma region Direct-Mode Cube

	glBegin(GL_TRIANGLES);

	//Front face
	glColor4f(1, 0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 1, 0);
	glVertex3f(1, 0, 0);

	glVertex3f(1, 0, 0);
	glVertex3f(0, 1, 0);
	glVertex3f(1, 1, 0);
	
	//Right face
	glColor4f(1, 0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 1, 1);
	glVertex3f(0, 1, 0);

	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 1);
	glVertex3f(0, 1, 1);

	//Back face
	glColor4f(1, 0, 0, 1);
	glVertex3f(0, 0, 1);
	glVertex3f(1, 1, 1);
	glVertex3f(0, 1, 1);

	glVertex3f(0, 0, 1);
	glVertex3f(1, 0, 1);
	glVertex3f(1, 1, 1);

	//Left face
	glColor4f(1, 0, 0, 1);
	glVertex3f(1, 0, 0);
	glVertex3f(1, 1, 0);
	glVertex3f(1, 0, 1);

	glVertex3f(1, 1, 0);
	glVertex3f(1, 1, 1);
	glVertex3f(1, 0, 1);

	//Upper face
	glColor4f(1, 0, 1, 1);
	glVertex3f(0, 1, 0);
	glVertex3f(0, 1, 1);
	glVertex3f(1, 1, 0);

	glVertex3f(1, 1, 0);
	glVertex3f(0, 1, 1);
	glVertex3f(1, 1, 1);

	//Bottom face
	glColor4f(1, 0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(1, 0, 0);
	glVertex3f(1, 0, 1);

	glVertex3f(0, 0, 0);
	glVertex3f(1, 0, 1);
	glVertex3f(0, 0, 1);

	glColor4f(1.0, 1, 1, 1);
	glEnd();

#pragma endregion

#pragma region Array Cube
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, array_cube_id);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glColor4f(0.0, 1, 0, 1);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glColor4f(1, 1, 1, 1);

	glDisableClientState(GL_VERTEX_ARRAY);
#pragma endregion

#pragma region Array Index Cube

	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, index_cube_vertex_id);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_cube_index_id);

	glColor4f(0.0, 0, 1, 1);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);
	glColor4f(1, 1, 1, 1);

	glDisableClientState(GL_VERTEX_ARRAY);

#pragma endregion

#pragma region Robot

	if (!BuffersON)
	{
		Mesh* mesh = App->moduleImport->GetMesh();

		glGenBuffers(1, (GLuint*)&mesh->id_vertices);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->id_vertices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh->num_vertices * 3, mesh->vertices, GL_STATIC_DRAW);


		glGenBuffers(1, (GLuint*)&mesh->id_indices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->id_indices);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * mesh->num_indices, mesh->indices, GL_STATIC_DRAW);

		BuffersON = true;
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, App->moduleImport->robotMesh.id_vertices);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, App->moduleImport->robotMesh.id_indices);

	glDrawElements(GL_TRIANGLES, App->moduleImport->robotMesh.num_indices, GL_UNSIGNED_INT, NULL);

	glDisableClientState(GL_VERTEX_ARRAY);
#pragma endregion
	return UPDATE_CONTINUE;
}

// PostUpdate present buffer to screen
update_status ModuleRenderer3D::PostUpdate(float dt)
{
	SDL_GL_SwapWindow(App->window->window);
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleRenderer3D::CleanUp()
{
	LOG("Destroying 3D Renderer");

	SDL_GL_DeleteContext(context);

	return true;
}


void ModuleRenderer3D::OnResize(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	ProjectionMatrix = perspective(60.0f, (float)width / (float)height, 0.125f, 512.0f);
	glLoadMatrixf(&ProjectionMatrix);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
