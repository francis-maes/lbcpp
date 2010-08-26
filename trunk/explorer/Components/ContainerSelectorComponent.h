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
# include "../Utilities/VariableSelector.h"

namespace lbcpp
{

#ifdef JUCE_WIN32
# pragma warning(disable:4355)
#endif // JUCE_WIN32

class ContainerSelectorComponent : public juce::ListBox, public VariableSelector
{
public:
  ContainerSelectorComponent(ContainerPtr container)
    : juce::ListBox(container->getName(), new Model(this, container)), container(container)
    {setMultipleSelectionEnabled(true);}
  
  virtual ~ContainerSelectorComponent()
    {delete getModel();}

  void selectedRowsChanged()
  {
    std::vector<Variable> selectedVariables;
    selectedVariables.reserve(getNumSelectedRows());
    for (int i = 0; i < getNumSelectedRows(); ++i)
    {
      int rowNumber = getSelectedRow(i);
      Variable variable = container->getElement(rowNumber);
      if (variable)
        selectedVariables.push_back(variable);
    }
    sendSelectionChanged(selectedVariables);
  }

  struct Model : public juce::ListBoxModel
  {
    Model(ContainerSelectorComponent* owner, ContainerPtr container)
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

      Variable element = container->getElement(rowNumber);
      g.drawText(container->getElementName(rowNumber) + T(" = ") + element.getShortSummary(), 4, 0, width - 6, height, Justification::centredLeft, true);
    }

    virtual void selectedRowsChanged(int lastRowSelected)
      {owner->selectedRowsChanged();}

    juce_UseDebuggingNewOperator

  private:
    ContainerSelectorComponent* owner;
    ContainerPtr container;
  };

  juce_UseDebuggingNewOperator

  ContainerPtr getContainer() const
    {return container;}

private:
  ContainerPtr container;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_CONTAINER_SELECTOR_H_
