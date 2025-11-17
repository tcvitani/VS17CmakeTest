
#ifdef PUBLIC
#	undef PUBLIC
#endif

#ifdef PROTECTED
#	undef PROTECTED
#endif

#ifdef PRIVATE
#	undef PRIVATE
#endif

#ifdef I
#	undef I
#endif

#ifdef INIT
#	undef INIT
#endif

#ifdef LOC_DEF
#	define PROTECTED 
#	define I( x )	= {x}
#	define INIT( x )	= {x}
#else
#	define PROTECTED extern
#	define I( x )
#	define INIT( x )
#endif
 


