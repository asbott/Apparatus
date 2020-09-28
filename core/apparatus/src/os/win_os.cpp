#include "pch.h"

#ifdef _OS_WINDOWS

#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE

#define NOMINMAX

#include <Windows.h>

#include <filesystem>

#include "os.h"

namespace fs = std::filesystem;

namespace os {
	module_t load_module(path_str_t path) {
		fs::path p(path);
		std::wstring wpath = p.wstring();
		return (void*)LoadLibraryEx(wpath.c_str(), NULL, LOAD_IGNORE_CODE_AUTHZ_LEVEL);
	}
	void* load_module_function(module_t mod, str_t<> name_of_function) {
		HMODULE lib = (HMODULE)mod;
		return (void*)GetProcAddress(lib, name_of_function);
	}
	void free_module(module_t mod) {
		while (FreeLibrary((HMODULE)mod)) ;
	}
}

#endif