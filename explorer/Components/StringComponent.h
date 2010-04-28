/*-----------------------------------------.---------------------------------.
| Filename: StringComponent.h              | String Component                |
| Author  : Francis Maes                   |                                 |
| Started : 15/06/2009 16:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_STRING_H_
# define EXPLORER_COMPONENTS_STRING_H_

# include "common.h"

namespace lbcpp
{

class StringComponent : public Viewport
{
public:
  StringComponent(ObjectPtr object)
  {
    setViewedComponent(label = new juce::Label("toto", object->toString()));
    label->setJustificationType(Justification::topLeft);
    label->setSize(5000, 5000); // bouh ! 
  }
    
  juce_UseDebuggingNewOperator

private:
  juce::Label* label;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_STRING_H_
