/*-----------------------------------------.---------------------------------.
| Filename: ObjectContainerNameListComp...h| A list of object' names         |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 13:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_OBJECT_CONTAINER_NAME_LIST_H_
# define EXPLORER_COMPONENTS_OBJECT_CONTAINER_NAME_LIST_H_

# include "common.h"

namespace lbcpp
{

class ObjectContainerNameListComponent : public juce::ListBox
{
public:
  ObjectContainerNameListComponent(ObjectContainerPtr container)
    : juce::ListBox(container->getName(), new Model(container)) {}
  
  virtual ~ObjectContainerNameListComponent()
    {delete getModel();}

  struct Model : public juce::ListBoxModel
  {
    Model(ObjectContainerPtr container) : container(container) {}

    virtual int getNumRows()
      {return (int)container->size();}

    virtual void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
    {
      if (rowIsSelected)
        g.fillAll(Colours::lightblue);

      g.setColour(Colours::black);
      Font f(height * 0.7f);
      f.setHorizontalScale(0.9f);
      g.setFont(f);

      ObjectPtr object = container->get(rowNumber);
      String name = object ? object->getName() : T("<null>");
      g.drawText(name, 4, 0, width - 6, height, Justification::centredLeft, true);
    }

    juce_UseDebuggingNewOperator

  private:
    ObjectContainerPtr container;
  };

  juce_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_OBJECT_CONTAINER_NAME_LIST_H_
