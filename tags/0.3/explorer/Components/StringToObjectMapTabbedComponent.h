/*-----------------------------------------.---------------------------------.
| Filename: StringToObjectMapTabbedComponent.h| String -> Object Maps Tabs   |
| Author  : Francis Maes                   |                                 |
| Started : 27/03/2010 13:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_STRING_TO_OBJECT_MAP_TABBED_COMPONENT_H_
# define EXPLORER_COMPONENTS_STRING_TO_OBJECT_MAP_TABBED_COMPONENT_H_

# include "common.h"

namespace lbcpp
{

class StringToObjectMapTabbedComponent : public TabbedComponent
{
public:
  StringToObjectMapTabbedComponent(StringToObjectMapPtr map)
    : TabbedComponent(TabbedButtonBar::TabsAtBottom)
  {
    std::map<String, ObjectPtr> objects = map->getObjects();
    for (std::map<String, ObjectPtr>::const_iterator it = objects.begin(); it != objects.end(); ++it)
      addTab(it->first, Colours::white, lbcpp::createComponentForObject(it->second), true);
  }
    
  juce_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_STRING_TO_OBJECT_MAP_TABBED_COMPONENT_H_
