#include "FrameCounter.h"

FrameCounter::~FrameCounter() {} //dtor

/*
	Counts the number of frames that are rendered per second
*/
void FrameCounter::FrameCount() {
	INT64 NewCount = 0;
	static INT64 LastCount = 0;
	INT64 Difference = 0;

	QueryPerformanceCounter((LARGE_INTEGER*)&NewCount);

	if (NewCount == 0)
		Error::SetError(TEXT("The system does not support high resolution timing"));

	frameCount_++;

	Difference = NewCount - LastCount;

	if (Difference >= frequency_) {
		frameRate_ = frameCount_;
		frameCount_ = 0;

		LastCount = NewCount;
	}
}

/*
	Get current system time/frequency, and initalize members for the counter.
*/
HRESULT FrameCounter::InitTiming() {

	D3DXFONT_DESC fontDesc;
	fontDesc.Height = 40;
	fontDesc.Width = 10;
	fontDesc.Weight = 0;
	fontDesc.MipLevels = 1;
	fontDesc.Italic = false;
	fontDesc.CharSet = DEFAULT_CHARSET;
	fontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;
	fontDesc.Quality = DEFAULT_QUALITY;
	fontDesc.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcscpy_s(fontDesc.FaceName, _T("Times New Roman"));
	D3DXCreateFontIndirect(pDevice_, &fontDesc, &font);

	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency_);

	if (frequency_ == 0) {
		Error::SetError(TEXT("The system does not support high resolution timing"));
		return E_FAIL;
	}

	frameCount_ = 0;
	frameRate_ = 0;

	return S_OK;
}

/*
	Function prints the frame rate on the screen using PrintString function
*/
void FrameCounter::PrintFrameRate(int x, int y, D3DCOLOR ColorKey) 
{
	char string[10];	// String to hold the frame rate

	ZeroMemory(&string, sizeof(string)); // Zero out the string

										 // Convert the frame rate to a string
	_itoa_s((int)frameRate_, string, 10);
	string[9] = '\0';

	// Draw the text of the sting to the screen
	RECT R = { x, y, 0, 0 };
	font->DrawText(0, string, -1, &R, DT_NOCLIP, ColorKey);
}