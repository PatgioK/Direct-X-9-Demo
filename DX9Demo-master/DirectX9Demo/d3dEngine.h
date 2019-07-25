#ifndef D3DENGINE_H
#define D3DENGINE_H

#include <d3dx9.h>
#include <string>
#include "Camera.h"
#include "FrameCounter.h"
#include "pSystem.h"
#include "d3dUtility.h"

//
// Classes and Structures
//
struct Vertex
{
	Vertex() {}
	Vertex(float x, float y, float z,
		float nx, float ny, float nz,
		float u, float v)
	{
		_x = x;  _y = y;  _z = z;
		_nx = nx; _ny = ny; _nz = nz;
		_u = u;  _v = v;
	}
	float _x, _y, _z;
	float _nx, _ny, _nz;
	float _u, _v;

	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;;
};

class Engine
{
public:

	IDirect3DDevice9* Device = 0;
	IDirect3DVertexBuffer9* VB = 0;
	IDirect3DTexture9* MirrorTex = 0;
	ID3DXMesh* Teapot = 0;
	ID3DXMesh* Sphere = 0;
	ID3DXMesh* Teapot2 = 0;
	ID3DXMesh* Sphere2 = 0;
	FrameCounter fc_;
	LPDIRECT3DSURFACE9 Baboon_;
	d3d::BoundingSphere BSphere;
	d3d::BoundingSphere BSphere2;
	Camera TheCamera;
	D3DXMATRIX World;
	D3DXVECTOR3 TeapotPosition;
	D3DXVECTOR3 TeapotPosition2;
	D3DMATERIAL9 TeapotMtrl = d3d::GREEN_MTRL;
	D3DMATERIAL9 TeapotMtrl2 = d3d::RED_MTRL;
	LPD3DXMESH              pTigerMesh_ = NULL; // Our mesh object in sysmem
	D3DMATERIAL9*           pTigerMeshMaterials_ = NULL; // Materials for our mesh
	LPDIRECT3DTEXTURE9*     pTigerMeshTextures_ = NULL; // Textures for our mesh
	DWORD                   dwTigerNumMaterials_ = 0L;   // Number of mesh materials
	D3DXMATRIX				MatTiger_;
	D3DMATERIAL9 MirrorMtrl = d3d::WHITE_MTRL;
	psys::PSystem* exp;
	size_t SelectModel = 0;
	POINT currentPos, pos;
	bool test = false, pickMode = false;
	bool test2 = false;
	int Width = 1280;
	int Height = 720;
	bool ambient = false;

	//
	// Windows/Direct3D Initialization
	//
	bool InitD3D(
		HWND hwnd,				   // [in] Application handle.
		int width, int height,     // [in] Backbuffer dimensions.
		bool windowed,             // [in] Windowed (true)or full screen (false).
		D3DDEVTYPE deviceType,     // [in] HAL or REF
		IDirect3DDevice9** device);// [out]The created device.

	void Cleanup();

	bool Setup();


	int InitTiger();
	void TanslationTigerMatrix(float value, size_t direction);
	void RotateTigerMatrix(float value, BOOL direction);

	void RotateTeapot(float value, BOOL direction);


	int LoadBitmapToSurface(TCHAR* PathName, LPDIRECT3DSURFACE9* ppSurface, LPDIRECT3DDEVICE9 pDevice);
	void RenderMirror(int* mirrors, size_t size);
	void RenderScene();
	bool Display(float timeDelta);
	int EnterMsgLoop();
};

#endif // D3DENGINE_H