#include "d3dEngine.h"

/*
	Method intializes the Direct3D Devices used by the applciation.

*/
bool Engine::InitD3D(
	HWND hwnd,
	int width, int height,
	bool windowed,
	D3DDEVTYPE deviceType,
	IDirect3DDevice9** device)
{
	/*
		 Init D3D: 
	*/

	HRESULT hr = 0;

	// Step 1: Create the IDirect3D9 object.

	IDirect3D9* d3d9 = 0;
	d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d9)
	{
		::MessageBox(0, "Direct3DCreate9() - FAILED", 0, 0);
		return false;
	}

	// Step 2: Check for hardware vp.

	D3DCAPS9 caps;
	d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, deviceType, &caps);

	int vp = 0;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// Step 3: Fill out the D3DPRESENT_PARAMETERS structure.

	D3DPRESENT_PARAMETERS d3dpp;
	d3dpp.BackBufferWidth = width;
	d3dpp.BackBufferHeight = height;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hwnd;
	d3dpp.Windowed = windowed;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval = windowed ? 0 : D3DPRESENT_INTERVAL_IMMEDIATE;

	// Step 4: Create the device.

	hr = d3d9->CreateDevice(
		D3DADAPTER_DEFAULT, // primary adapter
		deviceType,         // device type
		hwnd,               // window associated with device
		vp,                 // vertex processing
		&d3dpp,             // present parameters
		device);            // return created device

	if (FAILED(hr))
	{
		// try again using a 16-bit depth buffer
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

		hr = d3d9->CreateDevice(
			D3DADAPTER_DEFAULT,
			deviceType,
			hwnd,
			vp,
			&d3dpp,
			device);

		if (FAILED(hr))
		{
			d3d9->Release(); // done with d3d9 object
			::MessageBox(0, "CreateDevice() - FAILED", 0, 0);
			return false;
		}
	}

	// Create Fireworks partical system
	D3DXVECTOR3 origin(20.0f, 80.0f, 20.0f);
	exp = new psys::Firework(&origin, 90);
	exp->init(*device, "baboon.bmp");



	Device = *device;

	d3d9->Release(); // done with d3d9 object

	return true;
}

/*
	Releases the Engines members.
*/
void Engine::Cleanup()
{
	d3d::Release<IDirect3DVertexBuffer9*>(VB);
	d3d::Release<IDirect3DTexture9*>(MirrorTex);
	d3d::Release<ID3DXMesh*>(Teapot);
	d3d::Release<ID3DXMesh*>(Sphere);
	d3d::Release<IDirect3DDevice9*>(Device);

	if (pTigerMeshMaterials_ != NULL)
		delete[] pTigerMeshMaterials_;

	if (pTigerMeshTextures_)
	{
		for (DWORD i = 0; i < dwTigerNumMaterials_; i++)
		{
			if (pTigerMeshTextures_[i])
				pTigerMeshTextures_[i]->Release();
		}
		delete[] pTigerMeshTextures_;
	}
	if (pTigerMesh_ != NULL)
		pTigerMesh_->Release();
}

/*
	Setups the Engines compents and data members.
*/
bool Engine::Setup()
{
	HRESULT r = 0;//return values
	D3DSURFACE_DESC desc;
	LPDIRECT3DSURFACE9 pSurface = 0;

	// intialize frame counter
	fc_ = FrameCounter(Device);
	fc_.InitTiming();

	// Create a surface the size of the screen area.
	r = Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pSurface);
	pSurface->GetDesc(&desc);
	Device->CreateOffscreenPlainSurface(desc.Width,
		desc.Height,
		D3DFMT_X8R8G8B8,
		D3DPOOL_SYSTEMMEM,
		&Baboon_,
		NULL);

	// load a texutre on to a surface.
	r = LoadBitmapToSurface(TEXT("gray.bmp"),
		&pSurface, Device);
	if (FAILED(r)) {
		Error::SetError(TEXT("could not load bitmap surface"));
	}

	// Strech the textures surface to the sized other surface
	r = D3DXLoadSurfaceFromSurface(Baboon_, NULL, NULL, pSurface, NULL, NULL, D3DX_FILTER_TRIANGLE, 0);
	if (FAILED(r))
		Error::SetError(TEXT("did not copy surface"));

	// release the other surface
	pSurface->Release();
	pSurface = 0;

	// Create the teapot.
	D3DXCreateTeapot(Device, &Teapot, 0);
	BYTE* v = 0;
	Teapot->LockVertexBuffer(0, (void**)&v);

	// Calculate the bounding sphere of teapot
	D3DXComputeBoundingSphere(
		(D3DXVECTOR3*)v,
		Teapot->GetNumVertices(),
		D3DXGetFVFVertexSize(Teapot->GetFVF()),
		&BSphere._center,
		&BSphere._radius);

	Teapot->UnlockVertexBuffer();

	// Build a sphere mesh that describes the teapot's bounding sphere.
	D3DXCreateSphere(Device, BSphere._radius, 20, 20, &Sphere, 0);


	// Initailize the tiger model
	InitTiger();

	// Build mirrored cube
	Device->CreateVertexBuffer(
		36 * sizeof(Vertex),
		0, // usage
		Vertex::FVF,
		D3DPOOL_MANAGED,
		&VB,
		0);

	Vertex* vert = 0;
	VB->Lock(0, 0, (void**)&vert, 0);

	// Mirror
	// Left Face
	vert[0] = Vertex(-2.5f, 0.0f, 5.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	vert[1] = Vertex(-2.5f, 5.0f, 5.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	vert[2] = Vertex(-2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	vert[3] = Vertex(-2.5f, 0.0f, 5.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	vert[4] = Vertex(-2.5f, 5.0f, -0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	vert[5] = Vertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	//Front Face
	vert[6] = Vertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	vert[7] = Vertex(-2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	vert[8] = Vertex(2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	vert[9] = Vertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	vert[10] = Vertex(2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	vert[11] = Vertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	//Right Face
	vert[12] = Vertex(2.5f, 5.0f, 5.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	vert[13] = Vertex(2.5f, 0.0f, 5.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	vert[14] = Vertex(2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	vert[15] = Vertex(2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	vert[16] = Vertex(2.5f, 0.0f, 5.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	vert[17] = Vertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	//Back Face
	vert[18] = Vertex(-2.5f, 5.0f, 5.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	vert[19] = Vertex(-2.5f, 0.0f, 5.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	vert[20] = Vertex(2.5f, 5.0f, 5.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	vert[21] = Vertex(2.5f, 5.0f, 5.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	vert[22] = Vertex(-2.5f, 0.0f, 5.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	vert[23] = Vertex(2.5f, 0.0f, 5.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	//Top Face
	vert[24] = Vertex(-2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	vert[25] = Vertex(-2.5f, 5.0f, 5.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	vert[26] = Vertex(2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	vert[27] = Vertex(-2.5f, 5.0f, 5.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	vert[28] = Vertex(2.5f, 5.0f, 5.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	vert[29] = Vertex(2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	//Bottem Face
	vert[30] = Vertex(-2.5f, 0.0f, 5.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	vert[31] = Vertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	vert[32] = Vertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	vert[33] = Vertex(2.5f, 0.0f, 5.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	vert[34] = Vertex(-2.5f, 0.0f, 5.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	vert[35] = Vertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	VB->Unlock();

	// Load Textures for the mirror.
	D3DXCreateTextureFromFile(Device, "gray.bmp", &MirrorTex);

	// Set filters of the objects.
	Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	// Lights.
	D3DXVECTOR3 lightDir(0.707f, -0.707f, 0.707f);
	D3DXCOLOR color(1.0f, 1.0f, 1.0f, 1.0f);
	D3DLIGHT9 light = d3d::InitDirectionalLight(&lightDir, &color);

	Device->SetLight(0, &light);
	Device->LightEnable(0, true);

	Device->SetRenderState(D3DRS_NORMALIZENORMALS, true);
	Device->SetRenderState(D3DRS_SPECULARENABLE, true);

	// Set Camera.
	TheCamera = Camera(Camera::LANDOBJECT);
	D3DXVECTOR3    pos(0.0f, 3.0f, -15.0f);
	TheCamera.setPosition(&pos);

	// Set projection matrix.
	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(
		&proj,
		D3DX_PI / 4.0f, // 45 - degree
		(float)desc.Width / (float)desc.Height,
		1.0f,
		1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);

	//D3DXMatrixTranslation(&World, 0, 0, 0.0f);
	TeapotPosition = D3DXVECTOR3(0.0f, 3.0f, -7.5f);
	D3DXMatrixTranslation(&World,
		TeapotPosition.x,
		TeapotPosition.y,
		TeapotPosition.z);
	BSphere._center = D3DXVECTOR3(TeapotPosition.x,
		TeapotPosition.y,
		TeapotPosition.z);

	return true;
}

/*
Initialized airplane model.
*/
int Engine::InitTiger() 
{
	LPD3DXBUFFER pD3DXMtrlBuffer;

	D3DXMatrixTranslation(&MatTiger_, 0.0f, 2.0f, 10.5f);

	// Load the mesh from the specified file
	if (FAILED(D3DXLoadMeshFromX(TEXT("airplane 2.x"), D3DXMESH_SYSTEMMEM,
		Device, NULL,
		&pD3DXMtrlBuffer, NULL, &dwTigerNumMaterials_,
		&pTigerMesh_)))
	{
		// If model is not in current folder, try parent folder
		if (FAILED(D3DXLoadMeshFromX(TEXT("..\\airplane 2.x"), D3DXMESH_SYSTEMMEM,
			Device, NULL,
			&pD3DXMtrlBuffer, NULL, &dwTigerNumMaterials_,
			&pTigerMesh_)))
		{
			MessageBox(NULL, TEXT("Could not find tiger.x"), TEXT("Meshes.exe"), MB_OK);
			return E_FAIL;
		}
	}

	// We need to extract the material properties and texture names from the 
	// pD3DXMtrlBuffer
	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	pTigerMeshMaterials_ = new D3DMATERIAL9[dwTigerNumMaterials_];
	pTigerMeshTextures_ = new LPDIRECT3DTEXTURE9[dwTigerNumMaterials_];

	for (DWORD i = 0; i<dwTigerNumMaterials_; i++)
	{
		// Copy the material
		pTigerMeshMaterials_[i] = d3dxMaterials[i].MatD3D;

		// Set the ambient color for the material (D3DX does not do this)
		pTigerMeshMaterials_[i].Ambient = pTigerMeshMaterials_[i].Diffuse;

		pTigerMeshTextures_[i] = NULL;
		if (d3dxMaterials[i].pTextureFilename != NULL &&
			lstrlen(d3dxMaterials[i].pTextureFilename) > 0)
		{
			// Create the texture
			if (FAILED(D3DXCreateTextureFromFile(Device,
				d3dxMaterials[i].pTextureFilename,
				&pTigerMeshTextures_[i])))
			{
				// If texture is not in current folder, try parent folder
				const TCHAR* strPrefix = TEXT("..\\");
				const int lenPrefix = lstrlen(strPrefix);
				TCHAR strTexture[MAX_PATH];
				lstrcpyn(strTexture, strPrefix, MAX_PATH);
				lstrcpyn(strTexture + lenPrefix, d3dxMaterials[i].pTextureFilename, MAX_PATH - lenPrefix);
				// If texture is not in current folder, try parent folder
				if (FAILED(D3DXCreateTextureFromFile(Device,
					strTexture,
					&pTigerMeshTextures_[i])))
				{
					MessageBox(NULL, TEXT("Could not find texture map"), TEXT("Meshes.exe"), MB_OK);
				}
			}
		}
	}

	pTigerMesh_->CloneMeshFVF(D3DXMESH_MANAGED, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, Device, &pTigerMesh_);
	if (FAILED(D3DXComputeNormals(pTigerMesh_, NULL))) 
	{
		return E_FAIL;
	}

	// Done with the material buffer
	pD3DXMtrlBuffer->Release();

	return S_OK;
}

/*
	Loads a bitmap file form specified path and loads it into a DircetX 9 surface
*/
int Engine::LoadBitmapToSurface(TCHAR* PathName, LPDIRECT3DSURFACE9* ppSurface, LPDIRECT3DDEVICE9 pDevice) 
{
	HRESULT r;
	HBITMAP hBitmap;
	BITMAP Bitmap;

	hBitmap = (HBITMAP)LoadImage(NULL, PathName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if (hBitmap == NULL) 
	{
		Error::SetError(TEXT("Unable to load bitmap"));
		return E_FAIL;
	}

	GetObject(hBitmap, sizeof(BITMAP), &Bitmap);
	DeleteObject(hBitmap);//we only needed it for the header info to create a D3D surface

						  //create surface for bitmap
	r = pDevice->CreateOffscreenPlainSurface(Bitmap.bmWidth, Bitmap.bmHeight, D3DFMT_X8R8G8B8, D3DPOOL_SCRATCH, ppSurface, NULL);

	if (FAILED(r)) 
	{
		Error::SetError(TEXT("Unable to create surface for bitmap load"));
		return E_FAIL;
	}
	//load bitmap onto surface
	r = D3DXLoadSurfaceFromFile(*ppSurface, NULL, NULL, PathName, NULL, D3DX_DEFAULT, 0, NULL);
	if (FAILED(r))
	{
		Error::SetError(TEXT("Unable to laod file to surface"));
		return E_FAIL;
	}

	return S_OK;
}

/*
	Function uses the Tigers matrix to translate the Tiger model, based on parameters.
*/
void Engine::TanslationTigerMatrix(float value, size_t direction)
{
	float x = MatTiger_._41, y = MatTiger_._42, z = MatTiger_._43;
	D3DXMATRIX trans;

	if (direction == 0) 
	{
		D3DXMatrixTranslation(&trans, value, 0, 0);
	}
	else if (direction == 1) 
	{
		D3DXMatrixTranslation(&trans, 0, value, 0);
	}
	else if (direction == 2) 
	{
		D3DXMatrixTranslation(&trans, 0, 0, value);
	}

	D3DXMatrixMultiply(&MatTiger_, &MatTiger_, &trans);

}

/*
	Function uses the Tigers matrix to rotates the Tiger model, based on parameters.
*/
void Engine::RotateTigerMatrix(float value, BOOL direction) 
{

	float x = MatTiger_._41, y = MatTiger_._42, z = MatTiger_._43;
	D3DXMATRIX trans, rotate, back;

	D3DXMatrixTranslation(&trans, -x, -y, -z);

	if (direction) 
	{
		D3DXMatrixRotationY(&rotate, value);
	}
	else 
	{
		D3DXMatrixRotationX(&rotate, value);
	}
	D3DXMatrixMultiply(&trans, &trans, &rotate);
	D3DXMatrixTranslation(&back, x, y, z);
	D3DXMatrixMultiply(&trans, &trans, &back);
	D3DXMatrixMultiply(&MatTiger_, &MatTiger_, &trans);
}

/*
	Mehod for rotating the teapot based matricies.
*/
void Engine::RotateTeapot(float value, BOOL direction)
{
	D3DXMATRIX trans, rotate, back;
	D3DXMatrixTranslation(&trans, -TeapotPosition.x, -TeapotPosition.y, -TeapotPosition.z);

	if (direction)
	{
		D3DXMatrixRotationY(&rotate, value);
	}
	else
	{
		D3DXMatrixRotationX(&rotate, value);
	}
	D3DXMatrixMultiply(&trans, &trans, &rotate);
	D3DXMatrixTranslation(&back, TeapotPosition.x, TeapotPosition.y, TeapotPosition.z);
	D3DXMatrixMultiply(&trans, &trans, &back);
	D3DXMatrixMultiply(&World, &World, &trans);
}

/*
	The game loop of the application prepares the scene for rendering and applies effects to.
*/
bool Engine::Display(float timeDelta)
{
	if (Device)
	{
		HRESULT r;
		LPDIRECT3DSURFACE9 pBackSurf = 0;

		fc_.FrameCount();

		if (::GetAsyncKeyState('W') & 0x8000f)
			TheCamera.walk(4.0f * timeDelta);

		if (::GetAsyncKeyState('S') & 0x8000f)
			TheCamera.walk(-4.0f * timeDelta);

		if (::GetAsyncKeyState('A') & 0x8000f)
			TheCamera.strafe(-4.0f * timeDelta);

		if (::GetAsyncKeyState('D') & 0x8000f)
			TheCamera.strafe(4.0f * timeDelta);

		if (::GetAsyncKeyState('R') & 0x8000f)
			TheCamera.fly(4.0f * timeDelta);

		if (::GetAsyncKeyState('F') & 0x8000f)
			TheCamera.fly(-4.0f * timeDelta);

		if (::GetAsyncKeyState(VK_UP) & 0x8000f)
			TheCamera.pitch(1.0f * timeDelta);

		if (::GetAsyncKeyState(VK_DOWN) & 0x8000f)
			TheCamera.pitch(-1.0f * timeDelta);

		if (::GetAsyncKeyState(VK_LEFT) & 0x8000f)
			TheCamera.yaw(-1.0f * timeDelta);

		if (::GetAsyncKeyState(VK_RIGHT) & 0x8000f)
			TheCamera.yaw(1.0f * timeDelta);

		if (::GetAsyncKeyState('N') & 0x8000f)
			TheCamera.roll(1.0f * timeDelta);

		if (::GetAsyncKeyState('M') & 0x8000f)
			TheCamera.roll(-1.0f * timeDelta);

		if (::GetAsyncKeyState(VK_NUMPAD8) & 0x8000f)
		{
			if (SelectModel == 0)
				TanslationTigerMatrix(1.0f * timeDelta, 1);

			else if (SelectModel == 1)
				TeapotPosition.y += 1.0f * timeDelta;
		}

		if (::GetAsyncKeyState(VK_NUMPAD2) & 0x8000f)
		{
			if (SelectModel == 0)
				TanslationTigerMatrix(-1.0f * timeDelta, 1);

			else if (SelectModel == 1)
				TeapotPosition.y -= 1.0f * timeDelta;
		}

		if (::GetAsyncKeyState(VK_NUMPAD4) & 0x8000f)
		{
			if (SelectModel == 0)
				TanslationTigerMatrix(1.0f * timeDelta, 0);

			else if (SelectModel == 1)
				TeapotPosition.x -= 1.0f * timeDelta;
		}

		if (::GetAsyncKeyState(VK_NUMPAD6) & 0x8000f)
		{
			if (SelectModel == 0)
				TanslationTigerMatrix(-1.0f * timeDelta, 0);

			else if (SelectModel == 1)
				TeapotPosition.x += 1.0f * timeDelta;
		}

		if (::GetAsyncKeyState(VK_NUMPAD9) & 0x8000f)
		{
			if (SelectModel == 0)
				TanslationTigerMatrix(1.0f * timeDelta, 2);

			else if (SelectModel == 1)
				TeapotPosition.z += 1.0f * timeDelta;
		}

		if (::GetAsyncKeyState(VK_NUMPAD3) & 0x8000f)
		{
			if (SelectModel == 0)
				TanslationTigerMatrix(-1.0f * timeDelta, 2);

			else if (SelectModel == 1)
				TeapotPosition.z -= 1.0f * timeDelta;
		}

		if (::GetAsyncKeyState('O') & 0x8000f)
			SelectModel = 0;

		if (::GetAsyncKeyState('2') & 0x8000f)
			SelectModel = 1;

		if (!pickMode) {
			GetCursorPos(&pos);

			if ((pos.x != currentPos.x) || (currentPos.y != pos.y))
			{
				TheCamera.yaw(((float)(pos.x - currentPos.x) / 2.0f) * timeDelta);
				TheCamera.pitch(((float)(pos.y - currentPos.y) / 2.0f) * timeDelta);

				SetCursorPos(Width / 2, Height / 2);
				GetCursorPos(&currentPos);
			}
		}

		if (::GetAsyncKeyState(VK_LBUTTON) & 0x8000f)
		{
			if (test)
			{
				GetCursorPos(&pos);
				TeapotPosition.x += (pos.x - currentPos.x) / 100.0f;
				TeapotPosition.y += (currentPos.y - pos.y) / 100.0f;
				currentPos = pos;

				D3DXMatrixTranslation(&World, TeapotPosition.x, TeapotPosition.y, TeapotPosition.z);
				BSphere._center = D3DXVECTOR3(TeapotPosition.x, TeapotPosition.y, TeapotPosition.z);
			}
		}

		// Update the view matrix representing the cameras 
		// new position/orientation.
		D3DXMATRIX V;
		TheCamera.getViewMatrix(&V);
		Device->SetTransform(D3DTS_VIEW, &V);

		exp->update(timeDelta);
		if (exp->isDead())
			exp->reset();

		//get pointer to backbuffer
		r = Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackSurf);
		if (FAILED(r)) 
		{
			Error::SetError(TEXT("Couldn't get backbuffer"));
		}

		// Draw the scene:
		Device->Clear(0, 0,
			D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
			0xff000000, 1.0f, 0L);

		r = Device->UpdateSurface(Baboon_, NULL, pBackSurf, NULL);
		if (FAILED(r)) 
		{
			Error::SetError(TEXT("did not copy surface"));
		}

		fc_.PrintFrameRate(50, 40, D3DCOLOR_XRGB(0, 255, 0));

		pBackSurf->Release();
		pBackSurf = 0;

		Device->BeginScene();

		RenderScene();

		int mirrors[6] = { 0, 6, 12, 18, 24, 30 };
		RenderMirror(mirrors, 6);

		Device->EndScene();
		Device->Present(0, 0, 0, 0);
	}
	return true;
}

/* 
	Draws the majority of the objects defined in the engine.
*/
void Engine::RenderScene()
{
	// draw teapot
	Device->SetMaterial(&TeapotMtrl);
	Device->SetTexture(0, 0);
	/*D3DXMATRIX W;
	D3DXMatrixTranslation(&W,
		TeapotPosition.x,
		TeapotPosition.y,
		TeapotPosition.z);*/
	//D3DXMatrixMultiply(&World, &World, &W);

	World._41 = TeapotPosition.x; World._42 = TeapotPosition.y; World._43 = TeapotPosition.z;

	Device->SetTransform(D3DTS_WORLD, &World);
	Teapot->DrawSubset(0);

	// Render the bounding sphere with alpha blending so we can see 
	// through it.
	/*Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	D3DMATERIAL9 blue = d3d::BLUE_MTRL;
	blue.Diffuse.a = 0.25f; // 25% opacity
	Device->SetMaterial(&blue);
	Sphere->DrawSubset(0);

	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);*/

	D3DXMATRIX I;
	D3DXMatrixIdentity(&I);
	Device->SetTransform(D3DTS_WORLD, &I);

	Device->SetStreamSource(0, VB, 0, sizeof(Vertex));
	Device->SetFVF(Vertex::FVF);

	// draw the mirror
	Device->SetMaterial(&MirrorMtrl);
	Device->SetTexture(0, MirrorTex);
	Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
	Device->DrawPrimitive(D3DPT_TRIANGLELIST, 6, 2);
	Device->DrawPrimitive(D3DPT_TRIANGLELIST, 12, 2);
	Device->DrawPrimitive(D3DPT_TRIANGLELIST, 18, 2);
	Device->DrawPrimitive(D3DPT_TRIANGLELIST, 24, 2);
	Device->DrawPrimitive(D3DPT_TRIANGLELIST, 30, 2);

	Device->SetTransform(D3DTS_WORLD, &MatTiger_);

	// Meshes are divided into subsets, one for each material. Render them in
	// a loop
	for (DWORD i = 0; i<dwTigerNumMaterials_; i++)
	{
		// Set the material and texture for this subset
		Device->SetMaterial(&pTigerMeshMaterials_[i]);
		Device->SetTexture(0, pTigerMeshTextures_[i]);

		// Draw the mesh subset
		pTigerMesh_->DrawSubset(i);
	}
	
	Device->SetTransform(D3DTS_WORLD, &I);
	exp->render();
}

/*
	Method renders the reflection of the scene in the mirrored cubed.
*/
void Engine::RenderMirror(int* mirrors, size_t size)
{

	/*	Draw Mirror quad to stencil buffer ONLY.  In this way
		only the stencil bits that correspond to the mirror will
		be on.  Therefore, the reflected teapot can only be rendered
		where the stencil bits are turned on, and thus on the mirror 
		only. */
	

	// Planes that the reflextion will be drawn on
	D3DXPLANE planes[6] = { { 1.0f, 0.0f, 0.0f, 2.5f },  // yz plane
							{ 0.0f, 0.0f, 1.0f, 0.0f },  // xy plane
							{ -1.0f, 0.0f, 0.0f, 2.5f }, // yz plane
							{ 0.0f, 0.0f, -1.0f, 5.0f }, // xy plane
							{ 0.0f, -1.0f, 0.0f, 5.0f }, // xz plane
							{ 0.0f, 1.0f, 0.0f, 0.0f } }; // xz plane

	Device->SetRenderState(D3DRS_STENCILENABLE, true);
	Device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	Device->SetRenderState(D3DRS_STENCILREF, 0x1);
	Device->SetRenderState(D3DRS_STENCILMASK, 0xffffffff);
	Device->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);
	Device->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
	Device->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
	Device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

	// disable writes to the depth and back buffers
	Device->SetRenderState(D3DRS_ZWRITEENABLE, false);
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
	Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	// draw the mirror to the stencil buffer
	Device->SetStreamSource(0, VB, 0, sizeof(Vertex));
	Device->SetFVF(Vertex::FVF);
	Device->SetMaterial(&MirrorMtrl);
	Device->SetTexture(0, MirrorTex);
	D3DXMATRIX I;
	D3DXMatrixIdentity(&I);
	Device->SetTransform(D3DTS_WORLD, &I);

	for (size_t i = 0; i < size; i++) {
		Device->SetRenderState(D3DRS_STENCILREF, i + 1);
		Device->DrawPrimitive(D3DPT_TRIANGLELIST, mirrors[i], 2);
	}

	// re-enable depth writes
	Device->SetRenderState(D3DRS_ZWRITEENABLE, true);

	// clear depth buffer and blend the reflected teapot with the mirror
	Device->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
	Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

	for (size_t i = 0; i < size; i++) {
		// only draw reflected teapot to the pixels where the mirror
		// was drawn to.
		Device->SetRenderState(D3DRS_STENCILREF, i + 1);
		Device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
		Device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);

		// position reflection
		D3DXMATRIX W, R;
		D3DXMatrixReflect(&R, &planes[i]);

		W = World * R;

		// Finally, draw the reflected teapot
		Device->SetTransform(D3DTS_WORLD, &W);
		Device->SetMaterial(&TeapotMtrl);
		Device->SetTexture(0, 0);

		Device->SetClipPlane(0, planes[i]);
		Device->SetRenderState(D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0);
		Teapot->DrawSubset(0);

		Device->SetRenderState(D3DRS_CLIPPLANEENABLE, false);

		// Reflect Tiger
		W = MatTiger_ * R;
		Device->SetTransform(D3DTS_WORLD, &W);

		Device->SetClipPlane(0, planes[i]);
		Device->SetRenderState(D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0);

		for (DWORD ii = 0; ii<dwTigerNumMaterials_; ii++)
		{
			// Set the material and texture for this subset
			Device->SetMaterial(&pTigerMeshMaterials_[ii]);
			Device->SetTexture(0, pTigerMeshTextures_[ii]);

			// Draw the mesh subset
			pTigerMesh_->DrawSubset(ii);
		}
		Device->SetRenderState(D3DRS_CLIPPLANEENABLE, false);

	}

	// Restore render states.
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	Device->SetRenderState(D3DRS_STENCILENABLE, false);
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

/*
	This is the message loof for the win32 window of the application.
	Calculates timeDelta and passes it in to the display loop.
*/
int Engine::EnterMsgLoop()
{
	MSG msg;
	::ZeroMemory(&msg, sizeof(MSG));

	static float lastTime = (float)timeGetTime();

	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			float currTime = (float)timeGetTime();
			float timeDelta = (currTime - lastTime)*0.001f;

			Display(timeDelta);

			lastTime = currTime;
		}
	}
	return msg.wParam;
}

#pragma once
