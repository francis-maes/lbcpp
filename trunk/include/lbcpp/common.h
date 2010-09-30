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

namespace lbcpp
{
#ifdef LBCPP_MULTI_THREAD
using juce::CriticalSection;
using juce::ScopedLock;
using juce::ReadWriteLock;
using juce::ScopedReadLock;
using juce::ScopedWriteLock;
#else
class CriticalSection {};
class ScopedLock
{
public:
  ScopedLock(const CriticalSection& cs) {}
};
#endif // LBCPP_MULTI_THREAD
}; /* namespace lbcpp */

#endif // !LBCPP_COMMON_H_

