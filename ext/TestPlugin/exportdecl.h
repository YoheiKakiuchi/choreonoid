#ifndef CNOID_TESTPLUGIN_EXPORTDECL_H_INCLUDED
# define CNOID_TESTPLUGIN_EXPORTDECL_H_INCLUDED

# if defined _WIN32 || defined __CYGWIN__
#  define CNOID_TESTPLUGIN_DLLIMPORT __declspec(dllimport)
#  define CNOID_TESTPLUGIN_DLLEXPORT __declspec(dllexport)
#  define CNOID_TESTPLUGIN_DLLLOCAL
# else
#  if __GNUC__ >= 4
#   define CNOID_TESTPLUGIN_DLLIMPORT __attribute__ ((visibility("default")))
#   define CNOID_TESTPLUGIN_DLLEXPORT __attribute__ ((visibility("default")))
#   define CNOID_TESTPLUGIN_DLLLOCAL  __attribute__ ((visibility("hidden")))
#  else
#   define CNOID_TESTPLUGIN_DLLIMPORT
#   define CNOID_TESTPLUGIN_DLLEXPORT
#   define CNOID_TESTPLUGIN_DLLLOCAL
#  endif
# endif

# ifdef CNOID_TESTPLUGIN_STATIC
#  define CNOID_TESTPLUGIN_DLLAPI
#  define CNOID_TESTPLUGIN_LOCAL
# else
#  ifdef CnoidTestPlugin_EXPORTS
#   define CNOID_TESTPLUGIN_DLLAPI CNOID_TESTPLUGIN_DLLEXPORT
#  else
#   define CNOID_TESTPLUGIN_DLLAPI CNOID_TESTPLUGIN_DLLIMPORT
#  endif
#  define CNOID_TESTPLUGIN_LOCAL CNOID_TESTPLUGIN_DLLLOCAL
# endif

#endif

#ifdef CNOID_EXPORT
# undef CNOID_EXPORT
#endif
#define CNOID_EXPORT CNOID_TESTPLUGIN_DLLAPI
