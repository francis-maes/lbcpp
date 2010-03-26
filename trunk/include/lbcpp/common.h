/*-----------------------------------------.---------------------------------.
| Filename: common.h                       | Common include file             |
| Author  : Francis Maes                   |                                 |
| Started : 26/03/2010 16:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef LBCPP_COMMON_H_
# define LBCPP_COMMON_H_

/*
** Standard library
*/
# include <sstream>
# include <vector>
# include <set>
# include <typeinfo>
# include <cmath>

/*
** Juce
*/
# define DONT_SET_USING_JUCE_NAMESPACE
# include "../juce/juce_amalgamated.h"
using juce::String;
using juce::File;

#endif // !LBCPP_COMMON_H_

