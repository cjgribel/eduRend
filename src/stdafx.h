#ifndef _STDAFX__H
#define _STDAFX__H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <D3D11.h>
#include <d3dCompiler.h>
#include <dinput.h>

#include <string>
#include <fstream>

#define SAFE_RELEASE(x) if( x ) { (x)->Release(); (x) = nullptr; }
#define SAFE_DELETE(x) if( x ) { delete(x); (x) = nullptr; }
#define SAFE_DELETE_ARRAY(x) if( x ) { delete[](x); (x) = nullptr; }
constexpr auto PI = (3.14159265358979323846f);

#define ASSERT(x) if(FAILED(x)) { __debugbreak(); }

#ifdef _DEBUG
#define SETNAME(object, name) object->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(name) - 1, name)
#else
#define SETNAME(object, name) (void)0
#endif

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

//////////////////////////////////////////////////////////////////////////
// to find memory leaks
//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define myMalloc(s)       _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define myCalloc(c, s)    _calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define myRealloc(p, s)   _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define myExpand(p, s)    _expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define myFree(p)         _free_dbg(p, _NORMAL_BLOCK)
#define myMemSize(p)      _msize_dbg(p, _NORMAL_BLOCK)
#define myNew new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define myDelete delete  // Set to dump leaks at the program exit.
#define myInitMemoryCheck() \
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)
#define myDumpMemoryLeaks() \
	_CrtDumpMemoryLeaks()
#else
#define myMalloc malloc
#define myCalloc calloc
#define myRealloc realloc
#define myExpand _expand
#define myFree free
#define myMemSize _msize
#define myNew new
#define myDelete delete
#define myInitMemoryCheck()
#define myDumpMemoryLeaks()
#endif 
//////////////////////////////////////////////////////////////////////////



#endif
