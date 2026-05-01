#pragma once
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "config.h"
#include <SDL.h>

#ifdef __cplusplus
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <queue>
#include <stack>
#include <sstream>
#include <array>
#include <set>
#include <optional>
#include <variant>
#include <thread>
#include <numeric>
#include <regex>
#include <fstream>

#if defined(__EMSCRIPTEN__)
#define HAVE_STD_FILESYSTEM 1
#define HAVE_LIBSDL2 1

// Global flag: audio is initially disabled until user gesture (click/touch).
// This prevents audio buffer overflow while AudioContext is suspended.
extern bool g_audioEnabled;
#endif

#ifdef HAVE_STD_FILESYSTEM
#include <filesystem>
namespace fs = std::filesystem;
#else
#include "filesystem.hpp"
namespace fs = ghc::filesystem;
#endif

#include <fmt/format.h>
#include <fmt/chrono.h>

#include "z80.h"
#include "Util.h"
#include "OSD.h"
#include "SAM.h"
#include "libspectrum.h"

#ifdef HAVE_LIBZ
#include "unzip.h"
#include "zlib.h"
#endif

#endif
