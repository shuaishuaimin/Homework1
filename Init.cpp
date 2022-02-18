#include "stdafx.h"
#include "d3dInitForWindows.h"
#include "DrawMatrix.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.

// ??
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		DrawMatrix theApp(hInstance);
		if (!theApp.Initialize())
			return 0;

		return theApp.Run();
	}
	catch (DxException& e)
	{
		{
			MessageBox(nullptr, e.ToString().c_str(), L"Init failed!", MB_OK);
			return 0;
		}
	}

}