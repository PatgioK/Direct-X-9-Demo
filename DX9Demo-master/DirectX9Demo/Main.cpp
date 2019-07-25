#include "d3dEngine.h"

/*
	 Globals
*/

Engine				Eng;			// DirectX Engine, that contains all the 
									// Methods used for rendering the DirectX app.
IDirect3DDevice9*	Device = 0;		// Pointer to the DirectX 9 device.
d3d::Ray			ray;			// Ray that is used for Ray Picking objects.
D3DXMATRIX			view;			// View of the Ray in the world space.
D3DXMATRIX			viewInverse;	// Inverse of view matrix. 

/*
	Function Calculates the ray in the world space and returns it.
*/
d3d::Ray CalcPickingRay(int x, int y)
{
	float px = 0.0f;
	float py = 0.0f;

	D3DVIEWPORT9 vp;
	Device->GetViewport(&vp);

	D3DXMATRIX proj;
	Device->GetTransform(D3DTS_PROJECTION, &proj);

	px = (((2.0f*x) / vp.Width) - 1.0f) / proj(0, 0);
	py = (((-2.0f*y) / vp.Height) + 1.0f) / proj(1, 1);

	d3d::Ray ray;
	ray._origin = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	ray._direction = D3DXVECTOR3(px, py, 1.0f);

	return ray;
}

/*
	
*/
void TransformRay(d3d::Ray* ray, D3DXMATRIX* T)
{
	// transform the ray's origin, w = 1.
	D3DXVec3TransformCoord(
		&ray->_origin,
		&ray->_origin,
		T);

	// transform the ray's direction, w = 0.
	D3DXVec3TransformNormal(
		&ray->_direction,
		&ray->_direction,
		T);

	// normalize the direction
	D3DXVec3Normalize(&ray->_direction, &ray->_direction);
}

/*
	Function checks if the Ray intersects with the Bounding sphere.
*/
bool RaySphereIntTest(d3d::Ray* ray, d3d::BoundingSphere* sphere)
{
	D3DXVECTOR3 v = ray->_origin - sphere->_center;

	float b = 2.0f * D3DXVec3Dot(&ray->_direction, &v);
	float c = D3DXVec3Dot(&v, &v) - (sphere->_radius * sphere->_radius);

	// find the discriminant
	float discriminant = (b * b) - (4.0f * c);

	// test for imaginary number
	if (discriminant < 0.0f)
		return false;

	discriminant = sqrtf(discriminant);

	float s0 = (-b + discriminant) / 2.0f;
	float s1 = (-b - discriminant) / 2.0f;

	// if a solution is >= 0, then we intersected the sphere
	if (s0 >= 0.0f || s1 >= 0.0f)
		return true;

	return false;
}

/*
	 WndProc

	 Window Procedure for the DirectX applciation. 
*/
LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			::DestroyWindow(hwnd);
		}
		else if (wParam == 'P')							// Check if P key was pressed
		{
			if (Eng.pickMode) {						// If pick mode is enabled
				SetCursorPos(Eng.Width / 2,			// set Cursor posotion to the center of the scree
							 Eng.Height / 2);		// to prevent the camera from moving irratically during mode change
				GetCursorPos(&Eng.currentPos);		// Save postion of cursor to POINT structs
				Eng.pos = Eng.currentPos;
				Eng.pickMode = !Eng.pickMode;		// disable pick mode
			}
			else {
				Eng.currentPos = Eng.pos;			
				Eng.pickMode = !Eng.pickMode;		// enable pick mode
			}
		}
		else if (wParam == '7')
		{
			if (Eng.ambient) 
			{
				Device->SetRenderState(D3DRS_AMBIENT, NULL);
				Eng.ambient = !Eng.ambient;
			}
			else
			{
				Device->SetRenderState(D3DRS_AMBIENT, d3d::WHITE);
				Eng.ambient = !Eng.ambient;
			}
		}
		else if (wParam == '8')
		{
			D3DXVECTOR3 lightDir(0.707f, -0.707f, 0.707f);
			D3DXCOLOR color(1.0f, 1.0f, 1.0f, 1.0f);
			D3DLIGHT9 light = d3d::InitDirectionalLight(&lightDir, &color);
			Device->SetLight(0, &light);
			Device->LightEnable(0, true);
		}
		else if (wParam == '9')
		{
			D3DXVECTOR3 lightPos(0.0f, 15.0f, -10.0f);
			D3DXVECTOR3 lightDir(0.0f, -0.707f, 0.707f);
			D3DXCOLOR color(1.0f, 1.0f, 1.0f, 1.0f);
			D3DLIGHT9 light = d3d::InitSpotLight(&lightPos, &lightDir, &color);
			Device->SetLight(0, &light);
			Device->LightEnable(0, true);
		}
		else if (wParam == '0')
		{
			D3DXVECTOR3 lightPos(0.0f, 15.0f, -10.0f);
			D3DXCOLOR color(1.0f, 1.0f, 1.0f, 1.0f);
			D3DLIGHT9 light = d3d::InitPointLight(&lightPos, &color);
			Device->SetLight(0, &light);
			Device->LightEnable(0, true);
		}

		break;

	case WM_LBUTTONUP:
		if (Eng.test || Eng.test2) {								// if the Ray pick test passed
			Eng.test = false;						// reset test boolean
			Eng.test2 = false;

			// Translate the workd so that the Picked item will move
			D3DXMatrixTranslation(&Eng.World,		
								Eng.TeapotPosition.x,
								Eng.TeapotPosition.y,
								Eng.TeapotPosition.z);

			// Center the bonding sphere with the object
			Eng.BSphere._center = D3DXVECTOR3(Eng.TeapotPosition.x,
								Eng.TeapotPosition.y,
								Eng.TeapotPosition.z);
		}
		break;

	case WM_LBUTTONDOWN:

		// compute the ray in view space given the clicked screen point
		ray = CalcPickingRay(LOWORD(lParam), HIWORD(lParam));

		// transform the ray to world space
		Device->GetTransform(D3DTS_VIEW, &view);
		D3DXMatrixInverse(&viewInverse, 0, &view);
		TransformRay(&ray, &viewInverse);

		// test for a hit
		if (RaySphereIntTest(&ray, &Eng.BSphere))
		{
			//::MessageBox(0, "Hit!", "HIT", 0);
			Eng.test = true;				// Test passed
			Eng.currentPos = Eng.pos;
		}
		if (RaySphereIntTest(&ray, &Eng.BSphere2))
		{
			Eng.test2 = true;
			Eng.currentPos = Eng.pos;
		}

		break;

	case WM_MOUSEWHEEL: // Handles model roatition about the X axis
		short zDelta = (short)HIWORD(wParam); // wheel rotation 

		// Check the rotation of the mouse wheel to get direction and rotatate
		if (zDelta > 0)
		{
			if (Eng.SelectModel == 0)
			{
				Eng.RotateTigerMatrix(.3f, FALSE);
			}
			else if (Eng.SelectModel == 1)
			{
				Eng.RotateTeapot(.3f, FALSE);
			}
		}
		else
		{
			if (Eng.SelectModel == 0)
			{
				Eng.RotateTigerMatrix(-.3f, FALSE);
			}
			else if (Eng.SelectModel == 1)
			{
				Eng.RotateTeapot(-3.f, FALSE);
			}
		}

		break;
	}

	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

/*
	 WinMain

	 Main entry point of the program.
*/
int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{

	//
	// Create the main application window.
	//

	int Width = 1280;
	int Height = 720;
	Eng = Engine();
	WNDCLASS wc;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)d3d::WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = "Direct3D9App";

	if (!RegisterClass(&wc))
	{
		::MessageBox(0, "RegisterClass() - FAILED", 0, 0);
		return false;
	}

	HWND hwnd = 0;
	hwnd = ::CreateWindow("Direct3D9App", "Direct3D9App",
		WS_EX_TOPMOST,
		0, 0, Width, Height,
		0 /*parent hwnd*/, 0 /* menu */, hinstance, 0 /*extra*/);

	if (!hwnd)
	{
		::MessageBox(0, "CreateWindow() - FAILED", 0, 0);
		return false;
	}

	::ShowWindow(hwnd, SW_SHOW);
	::UpdateWindow(hwnd);

	if (!Eng.InitD3D(hwnd,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}

	if (!Eng.Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}
	SetCursorPos(Width / 2, Height / 2);
	GetCursorPos(&Eng.currentPos);
	GetCursorPos(&Eng.pos);

	Eng.EnterMsgLoop();

	Eng.Cleanup();

	Device->Release();

	return 0;
}