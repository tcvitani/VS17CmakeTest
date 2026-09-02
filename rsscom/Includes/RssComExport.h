#pragma once

#ifdef _WIN32
	#ifdef RSSCOM_LIB_EXPORT
	#define RSSCOM_EXPORTS   __declspec(dllexport)
	#else
	#define RSSCOM_EXPORTS   __declspec(dllimport)
	#endif
#else
	#define RSSCOM_EXPORTS
#endif