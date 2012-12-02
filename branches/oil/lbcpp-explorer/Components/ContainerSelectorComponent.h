/*-----------------------------------------.---------------------------------.
| Filename: ContainerSelectorComponent.h   | A selector for containers       |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 13:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_CONTAINER_SELECTOR_H_
# define EXPLORER_COMPONENTS_CONTAINER_SELECTOR_H_

# include "common.h"
# include <lbcpp/UserInterface/ObjectComponent.h>

namespace lbcpp
{

#ifdef JUCE_WIN32
# pragma warning(disable:4355)
#endif // JUCE_WIN32

class ContainerSelectorComponent : public juce::ListBox, public ObjectSelector
{
public:
  ContainerSelectorComponent(VectorPtr container)
    : juce::ListBox(container->toShortString(), new Model(this, container)), container(container)
    {setMultipleSelectionEnabled(true);}
  
  virtual ~ContainerSelectorComponent()
    {delete getModel();}

  void selectedRowsChanged()
  {
    std::vector<ObjectPtr> selectedObjects;
    selectedObjects.reserve(getNumSelectedRows());
    string selectionName;
    for (int i = 0; i < getNumSelectedRows(); ++i)
    {
      int rowNumber = getSelectedRow(i);
      ObjectPtr element = container->getElement(rowNumber);
      if (element)
      {
        selectedObjects.push_back(element);
        if (selectionName.isNotEmpty())
          selectionName += T(", ");
        selectionName += string(i);
      }
    }
    sendSelectionChanged(selectedObjects, selectionName);
  }

  struct Model : public juce::ListBoxModel
  {
    Model(ContainerSelectorComponent* owner, VectorPtr container)
      : owner(owner), container(container) {}

    virtual int getNumRows()
      {return (int)container->getNumElements();}

    virtual void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
    {
      if (rowIsSelected)
        g.fillAll(Colours::lightgrey);

      g.setColour(Colours::black);
      Font f(height * 0.7f);
      f.setHorizontalScale(0.9f);
      g.setFont(f);

      ObjectPtr element = container->getElement(rowNumber);
      g.drawText(string(rowNumber) + T(" = ") + element->toShortString(), 4, 0, width - 6, height, Justification::centredLeft, true);
    }

    virtual void selectedRowsChanged(int lastRowSelected)
      {owner->selectedRowsChanged();}

    juce_UseDebuggingNewOperator

  private:
    ContainerSelectorComponent* owner;
    VectorPtr container;
  };

  juce_UseDebuggingNewOperator

  VectorPtr getContainer() const
    {return container;}

private:
  VectorPtr container;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_CONTAINER_SELECTOR_H_
