
#ifdef PUBLIC
#undef PUBLIC
#endif

#ifdef PROTECTED
#undef PROTECTED
#endif

#ifdef PRIVATE
#undef PRIVATE
#endif

#ifdef EXPORT
#undef EXPORT
#endif

#ifdef I
#undef I
#endif

#ifdef INIT
#undef INIT
#endif

#define PUBLIC
#define PROTECTED
#define PRIVATE static
#define EXPORT __declspec( dllexport )
