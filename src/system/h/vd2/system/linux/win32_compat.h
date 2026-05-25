#ifndef VD2_SYSTEM_LINUX_WIN32_COMPAT_H
#define VD2_SYSTEM_LINUX_WIN32_COMPAT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef _LINUX_PORT

typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef int32_t LONG;
typedef uint32_t ULONG;
typedef int32_t BOOL;
typedef int64_t LONGLONG;
typedef uint64_t ULONGLONG;

typedef char CHAR;
typedef int16_t SHORT;
typedef uint16_t USHORT;
typedef float FLOAT;
typedef double DOUBLE;

typedef int INT;
typedef unsigned int UINT;
typedef unsigned int* PUINT;

typedef void* LPVOID;
typedef const void* LPCVOID;
typedef void* PVOID;

typedef struct HWND__* HWND;
typedef void* HANDLE;
typedef void* HMODULE;
typedef void* HINSTANCE;
typedef void* HMENU;
typedef void* HICON;
typedef void* HDC;
typedef void* HBRUSH;
typedef void* HFONT;
typedef void* HBITMAP;
typedef void* HGDIOBJ;

#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)

typedef char* LPSTR;
typedef const char* LPCSTR;
typedef wchar_t* LPWSTR;
typedef const wchar_t* LPCWSTR;

#ifdef UNICODE
typedef LPWSTR LPTSTR;
typedef LPCWSTR LPCTSTR;
typedef wchar_t TCHAR;
#else
typedef LPSTR LPTSTR;
typedef LPCSTR LPCTSTR;
typedef char TCHAR;
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

#define WINAPI
#define CALLBACK
#define STDAPICALLTYPE
#define __cdecl
#define __stdcall
#define __fastcall
#define __declspec(x)

typedef struct _RECT { LONG left; LONG top; LONG right; LONG bottom; } RECT, *PRECT, *LPRECT;
typedef struct _POINT { LONG x; LONG y; } POINT, *PPOINT, *LPPOINT;
typedef struct _SIZE { LONG cx; LONG cy; } SIZE, *PSIZE, *LPSIZE;
typedef struct _FILETIME { DWORD dwLowDateTime; DWORD dwHighDateTime; } FILETIME, *PFILETIME, *LPFILETIME;

typedef struct _SYSTEMTIME {
  WORD wYear; WORD wMonth; WORD wDayOfWeek; WORD wDay;
  WORD wHour; WORD wMinute; WORD wSecond; WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

typedef union _ULARGE_INTEGER {
  struct { DWORD LowPart; DWORD HighPart; } DUMMYSTRUCTNAME;
  struct { DWORD LowPart; DWORD HighPart; } u;
  ULONGLONG QuadPart;
} ULARGE_INTEGER;

typedef union _LARGE_INTEGER {
  struct { DWORD LowPart; LONG HighPart; } DUMMYSTRUCTNAME;
  struct { DWORD LowPart; LONG HighPart; } u;
  LONGLONG QuadPart;
} LARGE_INTEGER;

#define S_OK ((int32_t)0L)
#define S_FALSE ((int32_t)1L)
#define E_FAIL ((int32_t)0x80004005L)
#define E_OUTOFMEMORY ((int32_t)0x8007000EL)
#define E_INVALIDARG ((int32_t)0x80070057L)
typedef int32_t HRESULT;

#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr) (((HRESULT)(hr)) < 0)

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#include <string.h>
#include <wchar.h>
#include <stdio.h>
#include <wctype.h>
#define _strdup strdup
#define _vsnprintf vsnprintf
#define _snprintf snprintf
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define _wcsicmp wcscasecmp
#define _wcsnicmp wcsncasecmp
#define _countof(array) (sizeof(array) / sizeof(array[0]))

#endif
#endif
