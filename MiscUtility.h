
#ifndef __MiscUtility_H__
#define __MiscUtility_H__

#include "tjsCommHead.h"
#include "CharacterSet.h"

#include <vector>

#define tjs_string std::wstring

extern bool TVPEncodeUTF8ToUTF16( tjs_string &output, const std::string &source );

#endif
