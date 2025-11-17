
#ifdef PUBLIC
#  undef PUBLIC
#endif

#ifdef PROTECTED
#  undef PROTECTED
#endif

#ifdef PRIVATE
#  undef PRIVATE
#endif

#ifdef EXPORT
#  undef EXPORT
#endif

#ifdef I
#  undef I
#endif

#ifdef INIT
#  undef INIT
#endif

#define PUBLIC
#define I( x )  = (x) 
#define INIT( x )  = (x) 
#define EXPORT __declspec( dllexport )