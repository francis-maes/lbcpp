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

class StringComponent : public Component
{
public:
  StringComponent(ObjectPtr object)
  {
    addAndMakeVisible(label = new juce::Label("toto", object->toString()));
    label->setJustificationType(Justification::topLeft);
  }
    
  virtual ~StringComponent()
    {deleteAllChildren();}
  
  virtual void resized()
    {label->setBoundsRelative(0, 0, 1, 1);}
  
  juce_UseDebuggingNewOperator

private:
  juce::Label* label;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_STRING_H_
