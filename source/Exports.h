#pragma once
#ifdef JDE_DTS_EXPORTS
	#ifdef _MSC_VER
		#define JDE_DTS_VISIBILITY __declspec( dllexport )
	#else
		#define JDE_DTS_VISIBILITY __attribute__((visibility("default")))
	#endif
#else 
	#ifdef _MSC_VER 
		#define JDE_DTS_VISIBILITY __declspec( dllimport )
		//#define _GLIBCXX_USE_NOEXCEPT noexcept
		#if NDEBUG
			#pragma comment(lib, "Jde.AI.Dts.lib")
		#else
			#pragma comment(lib, "Jde.AI.Dts.lib")
		#endif
	#else
		#define JDE_DTS_VISIBILITY
	#endif
#endif 
