
#ifndef __CharacterSet_H__
#define __CharacterSet_H__

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <objidl.h>
#include "tp_stub.h"

#define tjs_string std::wstring

// Avoid symbol collision
#define TVPWideCharToUtf8String TVPWideCharToUtf8String_internal
#define TVPUtf8ToWideCharString TVPUtf8ToWideCharString_internal

extern bool TVPUtf8ToUtf16( tjs_string& out, const std::string& in );
extern bool TVPUtf16ToUtf8( std::string& out, const tjs_string& in );
extern tjs_int TVPWideCharToUtf8String(const tjs_char *in, char * out);
extern tjs_int TVPUtf8ToWideCharString(const char * in, tjs_char *out);


#endif
