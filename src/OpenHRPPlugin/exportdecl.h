
#ifndef CNOID_OPENHRPPLUGIN_EXPORTDECL_H_INCLUDED
# define CNOID_OPENHRPPLUGIN_EXPORTDECL_H_INCLUDED

# if defined _WIN32 || defined __CYGWIN__
#  define CNOID_OPENHRPPLUGIN_DLLIMPORT __declspec(dllimport)
#  define CNOID_OPENHRPPLUGIN_DLLEXPORT __declspec(dllexport)
#  define CNOID_OPENHRPPLUGIN_DLLLOCAL
# else
#  if __GNUC__ >= 4
#   define CNOID_OPENHRPPLUGIN_DLLIMPORT __attribute__ ((visibility("default")))
#   define CNOID_OPENHRPPLUGIN_DLLEXPORT __attribute__ ((visibility("default")))
#   define CNOID_OPENHRPPLUGIN_DLLLOCAL  __attribute__ ((visibility("hidden")))
#  else
#   define CNOID_OPENHRPPLUGIN_DLLIMPORT
#   define CNOID_OPENHRPPLUGIN_DLLEXPORT
#   define CNOID_OPENHRPPLUGIN_DLLLOCAL
#  endif
# endif

# ifdef CNOID_OPENHRPPLUGIN_STATIC
#  define CNOID_OPENHRPPLUGIN_DLLAPI
#  define CNOID_OPENHRPPLUGIN_LOCAL
# else
#  ifdef CnoidOpenHRPPlugin_EXPORTS
#   define CNOID_OPENHRPPLUGIN_DLLAPI CNOID_OPENHRPPLUGIN_DLLEXPORT
#  else
#   define CNOID_OPENHRPPLUGIN_DLLAPI CNOID_OPENHRPPLUGIN_DLLIMPORT
#  endif
#  define CNOID_OPENHRPPLUGIN_LOCAL CNOID_OPENHRPPLUGIN_DLLLOCAL
# endif

#endif

#ifdef CNOID_EXPORT
# undef CNOID_EXPORT
#endif
#define CNOID_EXPORT CNOID_OPENHRPPLUGIN_DLLAPI
