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

#ifdef JUCE_WIN32
# pragma warning(disable:4355)
#endif // JUCE_WIN32

class ObjectContainerNameListComponent : public juce::ListBox
{
public:
  ObjectContainerNameListComponent(ObjectContainerPtr container)
    : juce::ListBox(container->getName(), new Model(this, container)), container(container) {}
  
  virtual ~ObjectContainerNameListComponent()
    {delete getModel();}

  virtual void objectSelectedCallback(size_t index, ObjectPtr object) {}

  void selectedRowsChanged()
  {
    int row = getSelectedRow();
    if (row >= 0)
      objectSelectedCallback((size_t)row, container->get(row));
  }


  struct Model : public juce::ListBoxModel
  {
    Model(ObjectContainerNameListComponent* owner, ObjectContainerPtr container) : owner(owner), container(container) {}

    virtual int getNumRows()
      {return (int)container->size();}

    virtual void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
    {
      if (rowIsSelected)
        g.fillAll(Colours::lightgrey);

      g.setColour(Colours::black);
      Font f(height * 0.7f);
      f.setHorizontalScale(0.9f);
      g.setFont(f);

      ObjectPtr object = container->get(rowNumber);
      String name = object ? object->getName() : T("<null>");
      g.drawText(name, 4, 0, width - 6, height, Justification::centredLeft, true);
    }

    virtual void selectedRowsChanged(int lastRowSelected)
      {owner->selectedRowsChanged();}

    juce_UseDebuggingNewOperator

  private:
    ObjectContainerNameListComponent* owner;
    ObjectContainerPtr container;
  };

  juce_UseDebuggingNewOperator

  ObjectContainerPtr getContainer() const
    {return container;}

private:
  ObjectContainerPtr container;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_OBJECT_CONTAINER_NAME_LIST_H_
