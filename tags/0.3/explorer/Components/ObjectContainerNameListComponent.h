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
# include "../Utilities/ObjectSelector.h"

namespace lbcpp
{

#ifdef JUCE_WIN32
# pragma warning(disable:4355)
#endif // JUCE_WIN32

class ObjectContainerNameListComponent : public juce::ListBox, public ObjectSelector
{
public:
  ObjectContainerNameListComponent(ObjectContainerPtr container)
    : juce::ListBox(container->getName(), new Model(this, container)), container(container)
    {setMultipleSelectionEnabled(true);}
  
  virtual ~ObjectContainerNameListComponent()
    {delete getModel();}

  void selectedRowsChanged()
  {
    std::vector<ObjectPtr> selectedObjects;
    selectedObjects.reserve(getNumSelectedRows());
    for (int i = 0; i < getNumSelectedRows(); ++i)
    {
      int rowNumber = getSelectedRow(i);
      ObjectPtr object = container->get(rowNumber);
      if (object)
        selectedObjects.push_back(object);
    }
    sendSelectionChanged(selectedObjects);
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
