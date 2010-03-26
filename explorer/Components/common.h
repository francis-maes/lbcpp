/*-----------------------------------------.---------------------------------.
| Filename: common.h                       | Common include file for         |
| Author  : Francis Maes                   |      object components          |
| Started : 15/06/2009 18:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_COMMON_H_
# define EXPLORER_COMPONENTS_COMMON_H_

# include "../../juce/juce_amalgamated.h"
# include <lbcpp/lbcpp.h>

namespace lbcpp
{
  extern Component* createComponentForObject(ObjectPtr object, bool topLevelComponent = false);

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_COMMON_H_
