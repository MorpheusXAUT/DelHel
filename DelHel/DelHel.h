// DelHel.h : main header file for the DelHel DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CDelHelApp
// See DelHel.cpp for the implementation of this class
//

class CDelHelApp : public CWinApp
{
public:
	CDelHelApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
