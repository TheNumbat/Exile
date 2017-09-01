
#include <stdio.h>

#if 1
	#define DLL_IMPORT extern "C" __declspec(dllimport)
	#define NEAR
	#define FAR
	#define CONST const
	#define CALLBACK __stdcall
	#define WINAPI   __stdcall

	typedef int WINBOOL;
	typedef WINBOOL BOOL;
	typedef long LONG;
	typedef unsigned short WORD;
	typedef unsigned long DWORD;
	typedef unsigned int UINT;
	typedef void *HANDLE;
	typedef const char* LPCSTR;
	typedef void* LPVOID;
	typedef WORD ATOM;

	typedef UINT WPARAM;
	typedef LONG LPARAM;
	typedef LONG LRESULT;

	typedef HANDLE HINSTANCE;
	typedef HANDLE HICON;
	typedef HICON HCURSOR;
	typedef HANDLE HWND;
	typedef HANDLE HMENU;
	typedef HANDLE HBRUSH;
	typedef HINSTANCE HMODULE;

	#define CS_VREDRAW 		0x0001
	#define CS_HREDRAW 		0x0002
	#define SW_SHOW             5

	#define WS_OVERLAPPED	    0
	#define CW_USEDEFAULT       ((int)0x80000000)

	typedef LRESULT (CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

	typedef struct tagWNDCLASSEXA {
	    UINT        cbSize;
	    /* Win 3.x */
	    UINT        style;
	    WNDPROC     lpfnWndProc;
	    int         cbClsExtra;
	    int         cbWndExtra;
	    HINSTANCE   hInstance;
	    HICON       hIcon;
	    HCURSOR     hCursor;
	    HBRUSH      hbrBackground;
	    LPCSTR      lpszMenuName;
	    LPCSTR      lpszClassName;
	    /* Win 4.0 */
	    HICON       hIconSm;
	} WNDCLASSEXA, *PWNDCLASSEXA, NEAR *NPWNDCLASSEXA, FAR *LPWNDCLASSEXA;

	DLL_IMPORT ATOM WINAPI RegisterClassExA(CONST WNDCLASSEXA *);
	DLL_IMPORT HWND WINAPI CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
	DLL_IMPORT LRESULT CALLBACK DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	DLL_IMPORT HMODULE WINAPI GetModuleHandleA(LPCSTR lpModuleName);
	DLL_IMPORT BOOL WINAPI ShowWindow(HWND hWnd, int nCmdShow);
	DLL_IMPORT DWORD WINAPI GetLastError(void);
#else
	#include <windows.h>
#endif


LRESULT CALLBACK window_proc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam) {
	return DefWindowProcA(handle, msg, wParam, lParam);
}

int main() {

	const char* title = "test";
	int width = 1280, height = 720;

	HINSTANCE instance = GetModuleHandleA(NULL);

	if(instance == NULL) {
		printf("Instance failed: %d", GetLastError());
		return 1;
	}

	WNDCLASSEXA window_class = {
		sizeof(WNDCLASSEXA),
		CS_HREDRAW | CS_VREDRAW,
		window_proc,
		0, 0,
		instance,
		0, 0, 0, 0,
		title, 0
	};

	if(RegisterClassExA(&window_class) == 0) {
		printf("Class failed: %d", GetLastError());
		return 1;
	}

	HWND handle = CreateWindowExA(0, title, title, WS_OVERLAPPED,
			           			  CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, instance, 0);

	if(!handle) {
		printf("Window failed: %d", GetLastError());
		return 1;
	}

	ShowWindow(handle, SW_SHOW);

	printf("Success!");
	return 0;
}
