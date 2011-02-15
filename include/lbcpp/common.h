/*-----------------------------------------.---------------------------------.
| Filename: common.h                       | Common include file             |
| Author  : Francis Maes                   |                                 |
| Started : 26/03/2010 16:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef LBCPP_COMMON_H_
# define LBCPP_COMMON_H_

# define LBCPP_MULTI_THREAD

# ifdef JUCE_WIN32
#  if _MSC_VER>=1600
#   define LBCPP_ENABLE_CPP0X_RVALUES
#  endif
# else 
#  ifdef __GXX_EXPERIMENTAL_CXX0X__
#   define LBCPP_ENABLE_CPP0X_RVALUES
#  endif
# endif

//#define LBCPP_DEBUG_REFCOUNT_ATOMIC_OPERATIONS
//#define LBCPP_DEBUG_OBJECT_ALLOCATION

/*
** Standard library
*/
# include <iostream>
# include <sstream>
# include <vector>
# include <set>
# include <map>
# include <typeinfo>
# include <cmath>

# ifndef M_PI
#  define M_PI       3.14159265358979323846
# endif // M_PI

# define M_2_TIMES_PI (2.0 * M_PI)

/*
** Juce
*/
# define DONT_SET_USING_JUCE_NAMESPACE
# include "../juce/juce_amalgamated.h"
using juce::String;
using juce::StringArray;
using juce::File;
using juce::InputStream;
using juce::OutputStream;
using juce::Time;
using juce::XmlElement;
using juce::Logger;
using juce::ZipFile;

namespace lbcpp
{
#ifdef LBCPP_MULTI_THREAD
using juce::CriticalSection;
using juce::ScopedLock;
using juce::ReadWriteLock;
using juce::ScopedReadLock;
using juce::ScopedWriteLock;
using juce::Thread;
#else
class CriticalSection {};
class ScopedLock
{
public:
  ScopedLock(const CriticalSection& cs) {}
};
#endif // !LBCPP_MULTI_THREAD

#ifdef LBCPP_NETWORKING
using juce::InterprocessConnection;
using juce::InterprocessConnectionServer;
#endif // !LBCPP_NETWORKING

#ifdef JUCE_DEBUG

extern void* debugMalloc(const int size, const char* file, const int line);
extern void* debugCalloc(const int size, const char* file, const int line);
extern void* debugRealloc(void* const block, const int size, const char* file, const int line);
extern void debugFree(void* const block);

# define lbcpp_UseDebuggingNewOperator \
  static void* operator new (size_t sz)           { void* const p = lbcpp::debugMalloc ((int) sz,  __FILE__, __LINE__); return (p != 0) ? p : ::operator new (sz); } \
  static void* operator new (size_t sz, void* p)  { return ::operator new (sz, p); } \
  static void operator delete (void* p)           { lbcpp::debugFree (p); }

#else 
# define lbcpp_UseDebuggingNewOperator 
#endif // JUCE_DEBUG

}; /* namespace lbcpp */

#endif // !LBCPP_COMMON_H_

