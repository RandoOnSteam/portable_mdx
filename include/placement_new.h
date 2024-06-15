#if defined(__cplusplus) && !defined(NOTHROW)
	#if defined(__GNUC__) || defined(__clang__)
		#include <new>
	#else
		#include <new.h>
	#endif
		template <class DATATYPE, class ARG> void placement_new(
			DATATYPE* d, ARG& t) { new (d) DATATYPE(t); }
		template <class DATATYPE> void placement_new_def(
			DATATYPE* d) { new (d) DATATYPE(); }
#endif