#ifndef CNOID_TESTLIB_EXPORTDECL_H_INCLUDED
# define CNOID_TESTLIB_EXPORTDECL_H_INCLUDED

# if defined _WIN32 || defined __CYGWIN__
#  define CNOID_TESTLIB_DLLIMPORT __declspec(dllimport)
#  define CNOID_TESTLIB_DLLEXPORT __declspec(dllexport)
#  define CNOID_TESTLIB_DLLLOCAL
# else
#  if __GNUC__ >= 4
#   define CNOID_TESTLIB_DLLIMPORT __attribute__ ((visibility("default")))
#   define CNOID_TESTLIB_DLLEXPORT __attribute__ ((visibility("default")))
#   define CNOID_TESTLIB_DLLLOCAL  __attribute__ ((visibility("hidden")))
#  else
#   define CNOID_TESTLIB_DLLIMPORT
#   define CNOID_TESTLIB_DLLEXPORT
#   define CNOID_TESTLIB_DLLLOCAL
#  endif
# endif

# ifdef CNOID_TESTLIB_STATIC
#  define CNOID_TESTLIB_DLLAPI
#  define CNOID_TESTLIB_LOCAL
# else
#  ifdef Test_EXPORTS
#   define CNOID_TESTLIB_DLLAPI CNOID_TESTLIB_DLLEXPORT
#  else
#   define CNOID_TESTLIB_DLLAPI CNOID_TESTLIB_DLLIMPORT
#  endif
#  define CNOID_TESTLIB_LOCAL CNOID_TESTLIB_DLLLOCAL
# endif

#endif

#ifdef CNOID_EXPORT
# undef CNOID_EXPORT
#endif
#define CNOID_EXPORT CNOID_TESTLIB_DLLAPI
