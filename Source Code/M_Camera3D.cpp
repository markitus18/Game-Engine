#include "Engine.h"
#include "M_Camera3D.h"
#include "M_Editor.h"
#include "M_Input.h"
#include "M_Window.h"
#include "C_Camera.h"
#include "M_Renderer3D.h"
#include "GameObject.h"
#include "Config.h"
#include "OpenGL.h"
#include "Quadtree.h"
#include "M_Scene.h"
#include "Vec2.h"
#include "Time.h"

#include <vector>
#include <map>



M_Camera3D::M_Camera3D(bool start_enabled) : Module("Camera", start_enabled)
{
	camera = new C_Camera(nullptr);

	camera->frustum.SetViewPlaneDistances(0.1f, 2000.0f);
	camera->frustum.SetPerspective(1.0f, 1.0f);

	camera->frustum.SetPos(float3(10, 100, 0));
	camera->frustum.SetFront(float3::unitZ);
	camera->frustum.SetUp(float3::unitY);
	Look(float3(0, 0, 0));
	camera->update_projection = true;
}

M_Camera3D::~M_Camera3D()
{
	RELEASE(camera);
}

// -----------------------------------------------------------------
bool M_Camera3D::Init(Config& config)
{
	Engine->renderer3D->camera = camera;

	return true;
}

// -----------------------------------------------------------------
bool M_Camera3D::Start()
{
	bool ret = true;

	return ret;
}

// -----------------------------------------------------------------
bool M_Camera3D::CleanUp()
{
	LOG("Cleaning camera");

	Engine->renderer3D->camera = nullptr;
	return true;
}

// -----------------------------------------------------------------
update_status M_Camera3D::Update()
{
	if (Engine->moduleEditor->UsingKeyboard() == false)
		Move_Keyboard(Time::deltaTime);

	if (Engine->moduleEditor->UsingMouse() == false)
	{
		if (Engine->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
		{
			//OnClick();
		}
		//Move_Mouse();
	}

	return UPDATE_CONTINUE;
}

// -----------------------------------------------------------------
float3 M_Camera3D::GetPosition() const
{
	return camera->frustum.Pos();
}

float3 M_Camera3D::GetReference() const
{
	return reference;
}

// -----------------------------------------------------------------
void M_Camera3D::Look(const float3& position)
{
	camera->Look(position);
	reference = position;
}

// -----------------------------------------------------------------
void M_Camera3D::CenterOn(const float3& position, float distance)
{
	float3 v = camera->frustum.Front().Neg();
	camera->frustum.SetPos(position + (v * distance));
	reference = position;
}

void M_Camera3D::SetNewTarget(const float3& new_target)
{
	float distance = reference.Distance(new_target);
	CenterOn(new_target, distance);
}

void M_Camera3D::Match(const C_Camera* camera)
{
	this->camera->Match(camera);
	reference = this->camera->frustum.Pos() + this->camera->frustum.Front() * 40;
}

void M_Camera3D::SetPosition(float3 position)
{
	float3 difference = position - camera->frustum.Pos();
	camera->frustum.SetPos(position);
	reference += difference;
}

// -----------------------------------------------------------------
C_Camera * M_Camera3D::GetCamera() const
{
	return camera;
}

void M_Camera3D::SaveScene(Config& root) const
{
	float3 position = camera->frustum.Pos();
	Config_Array pos = root.SetArray("Position");
	pos.AddFloat3(position);

	Config_Array ref = root.SetArray("Reference");
	ref.AddFloat3(reference);
}

void M_Camera3D::LoadScene(Config& root, bool tmp)
{
	if (tmp == false)
	{
		camera->frustum.SetPos(root.GetArray("Position").GetFloat3(0));
		reference = root.GetArray("Reference").GetFloat3(0);
		Look(reference);
		camera->update_projection = true;
	}
}

void M_Camera3D::Move_Keyboard(float dt)
{
}

void M_Camera3D::Move_Mouse(float motion_x, float motion_y)
{
	// Mouse wheel for zoom
	int wheel = Engine->input->GetMouseZ();
	if (wheel != 0)
		Zoom(wheel);
}

void M_Camera3D::Pan(float motion_x, float motion_y)
{
	float distance = reference.Distance(camera->frustum.Pos());
	float3 Y_add = camera->frustum.Up() * motion_y * (distance / 1800);
	float3 X_add = camera->frustum.WorldRight() * -motion_x * (distance / 1800);

	reference += X_add;
	reference += Y_add;

	camera->frustum.SetPos(camera->frustum.Pos() + X_add + Y_add);
}

void M_Camera3D::DrawRay()
{
	glColor4f(1, 0, 0, 1);

	//Between-planes right
	GLfloat pointA[3] = { lastRay.a.x, lastRay.a.y, lastRay.a.z };
	GLfloat pointB[3] = { lastRay.b.x, lastRay.b.y, lastRay.b.z };

	glBegin(GL_LINES);
	glVertex3fv(pointA);
	glVertex3fv(pointB);
	glEnd();

	glColor4f(1, 1, 1, 1);
}

// -----------------------------------------------------------------
void M_Camera3D::Orbit(float dx, float dy)
{
	//Procedure: create a vector from camera position to reference position
	//Rotate that vector according to our mouse motion
	//Move the camera to where that vector ended up

	//TODO: this causes issues when rotating when frustrum Z = world +/- Y
	float3 vector = camera->frustum.Pos() - reference;

	Quat quat_y(camera->frustum.Up(), dx * 0.003);
	Quat quat_x(camera->frustum.WorldRight(), dy * 0.003);

	vector = quat_x.Transform(vector);
	vector = quat_y.Transform(vector);

	camera->frustum.SetPos(vector + reference);
	Look(reference);
}

// -----------------------------------------------------------------
void M_Camera3D::Zoom(float zoom)
{
	float distance = reference.Distance(camera->frustum.Pos());
	vec newPos = camera->frustum.Pos() + camera->frustum.Front() * zoom * distance * 0.05f;
	camera->frustum.SetPos(newPos);
}

void M_Camera3D::OnClick(const Vec2& mousePos)
{
	float mouseNormX = mousePos.x / (float)Engine->window->windowSize.x;
	//TODO: quick fix, mouse click is inverting Y
	float mouseNormY = mousePos.y / (float)Engine->window->windowSize.y;

	//Normalizing mouse position in range of -1 / 1 // -1, -1 being at the bottom left corner
	mouseNormX = (mouseNormX - 0.5) / 0.5;
	mouseNormY = (mouseNormY - 0.5) / 0.5;

	lastRay = Engine->renderer3D->camera->frustum.UnProjectLineSegment(mouseNormX, mouseNormY);

 	Engine->scene->OnClickSelection(lastRay);
}
