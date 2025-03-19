#pragma once
//git
#ifndef __GUICON_H__

#define __GUICON_H__

//#ifdef _DEBUG

void RedirectIOToConsole();

//#endif

#endif

#include <iostream>
#include <comdef.h>
#include <system_error>

#define CheckHR(x) if(x != S_OK){errorMSG(__FILE__, __LINE__, x);__debugbreak();}
#define breakDebug errorMSG(__FILE__, __LINE__, S_OK);

static void errorMSG(const char* file, int line, HRESULT error)
{
 	std::cout << "Error at : Line: " << line << " File " << file << std::endl;
	if (error != S_OK)
	{
		std::cout << "Error msg: " << std::system_category().message(error) << std::endl;
	}
}