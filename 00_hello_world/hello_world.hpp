#pragma once

#include <iostream>

#ifdef WIN32
#define MAIN wmain
typedef wchar_t unicode_char;
#else
#define MAIN main
typedef char unicode_char;
#endif
