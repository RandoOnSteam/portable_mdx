#ifndef __STDBOOLWRAPPER_H__
#define __STDBOOLWRAPPER_H__
#if (defined(_MSC_VER) && _MSC_VER <= 1400) /* 2005 <= */ \
	|| defined(__MWERKS__)
	#ifndef __cplusplus
		#define bool int
		#define true 1
		#define false 0
	#endif
#else
	#include <stdbool.h>
#endif
#endif /* __STDBOOLWRAPPER_H__ */