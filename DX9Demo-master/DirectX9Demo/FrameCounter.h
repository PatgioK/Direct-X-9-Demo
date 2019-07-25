#ifndef FRAMECOUNTER_H
#define FRAMECOUNTER_H

#include <d3d9.h>
#include <d3dx9.h>
#include "Error.h"

class FrameCounter {
public:

	FrameCounter() {} //default dtor

	FrameCounter(LPDIRECT3DDEVICE9 pDevice)
		: pDevice_(pDevice) {} // ctor

	~FrameCounter(); // dtor

	void FrameCount();

	HRESULT InitTiming();

	void PrintFrameRate(int x, int y, D3DCOLOR ColorKey);

private:
	INT64 frameCount_ = 0;					// counter for number of frames rendered per second
	INT64 frameRate_;						// frame rate displayed
	INT64 frequency_;						// frequency of the system
	LPDIRECT3DDEVICE9 pDevice_;				// Pointer to the D3D device
	ID3DXFont* font;
};

#endif