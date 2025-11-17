
#ifdef PUBLIC
#	undef PUBLIC
#endif

#ifdef PROTECTED
#	undef PROTECTED
#endif

#ifdef PRIVATE
#	undef PRIVATE
#endif

#ifdef EXPORT
   #undef EXPORT
#endif

#ifdef I
#	undef I
#endif

#ifdef INIT
#  undef INIT
#endif

#define PUBLIC extern
#define EXPORT extern __declspec( dllimport )
#define I( x )
#define INIT( x )
 

