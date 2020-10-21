
#ifdef _OS_WINDOWS
	#define _AP_BREAK __debugbreak()
#elif defined (__GNUC__)
	#define _AP_BREAK __builtin_trap()
#else
	#define _AP_BREAK signal(SIGTRAP)
#endif

#ifndef _AP_DISABLE_ASSERTS

	#define ap_assert(expr, ...) \
    if (!(expr)) { \
        log_critical("Assertion failed for expression '" #expr "'\nMessage:\n" __VA_ARGS__); _AP_BREAK; \
    }

#else

	#define ap_assert(expr, ...)

#endif

#define BIT1  1
#define BIT2  2
#define BIT3  4
#define BIT4  8
#define BIT5  16
#define BIT6  32
#define BIT7  64
#define BIT8  128
#define BIT9  256
#define BIT10 512
#define BIT11 1024
#define BIT12 2048
#define BIT13 4096
#define BIT14 8192
#define BIT15 16384
#define BIT16 32768
#define BIT17 65536
#define BIT18 131072
#define BIT19 262144
#define BIT20 524288
#define BIT21 1048576
#define BIT22 2097152
#define BIT23 4194304
#define BIT24 8388608
#define BIT25 16777216
#define BIT26 33554432
#define BIT27 67108864
#define BIT28 134217728
#define BIT29 268435456
#define BIT30 536870912
#define BIT31 1073741824
#define BIT32 2147483648

#define BIT(n) (1 << n)

#ifdef _MSC_VER
	#define _ap_force_inline __forceinline
#elif defined(__GNUC__) || defined (__MINGW32__) || defined(__MINGW64__)
	#define _ap_force_inline __attribute__((always_inline)) inline
#elif defined(__clang__)
	#define _ap_force_inline inline
#else
	#define _ap_force_inline inline
	#warning _ap_force_inline could not be defined for compiler. Performance impact is possible.    
#endif

#ifdef _MSVC_LANG
	#define _AP_FUNCTION __FUNCTION__
#elif defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
	#define _AP_FUNCTION __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
	#define _AP_FUNCTION __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__)
	#define _AP_FUNCTION __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
	#define _AP_FUNCTION __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
	#define _AP_FUNCTION __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
	#define _AP_FUNCTION __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
	#define _AP_FUNCTION __func__
#else
	#define _AP_FUNCTION "Function signature N/A"
	#warning could not define function signature macro for this compiler
#endif

#ifdef AP_SUPPORT_DX11
	#define Default_Api DirectX11
#elif defined(AP_SUPPORT_OPENGL45)
	#define Default_Api OpenGL45
#else
	#error This configuration is not supported on your system
#endif

#define NOMINMAX

#ifdef _OS_WINDOWS
	#ifdef AP_EXPORT
		#define AP_API __declspec(dllexport)
	#else
		#define AP_API __declspec(dllimport)
	#endif
#else
	#define AP_API 
#endif

#define stringify(x) #x

#ifdef _CONFIG_DEBUG
	#define debug_only(x) x
#else
	#define debug_only(x)
#endif

#define AP_NS_BEGIN(x) namespace x {

#define AP_NS_END(x) }

#ifdef _OS_WINDOWS
	#define MODULE_FILE_EXTENSION "dll"
#elif defined(_OS_LINUX)
	#define MODULE_FILE_EXTENSION "so"
#endif