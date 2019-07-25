#ifndef ERROR_H
#define ERROR_H

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

class Error {
public:

	/*
	Disable ctors
	*/
	Error() = delete;

	Error(const Error&) = delete;

	Error& operator=(const Error&) = delete;

	static void SetError(TCHAR* szFormat, ...);

};
#endif
