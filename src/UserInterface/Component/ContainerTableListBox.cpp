/*-----------------------------------------.---------------------------------.
| Filename: ContainerTableListBox.cpp      | Table Component                 |
| Author  : Francis Maes                   |                                 |
| Started : 27/01/2011 18:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ContainerTableListBox.h"
using namespace lbcpp;

using juce::TableListBox;
using juce::TableListBoxModel;
using juce::TableHeaderComponent;
using juce::Graphics;
using juce::Justification;
using juce::Font;
using juce::MouseEvent;
using juce::Colour;
using juce::Colours;

class ContainerTableListBoxModel : public TableListBoxModel
{
public:
  ContainerTableListBoxModel(const ContainerPtr& container)
    : container(container)
  {
    computeAutoColumnWidths();
  }
     
  virtual int getNumRows()
    {return container->getNumElements();}

  virtual void paintRowBackground(Graphics& g, int row, int width, int height, bool selected)
  {
    g.setColour(selected ? Colours::lightskyblue : (row % 2 ? Colour(230, 240, 255) : Colour(220, 230, 245)));
    g.fillRect(0, 0, width, height);
  }
  
  virtual void paintCell(Graphics& g, int row, int column, int width, int height, bool selected)
  {
    if (row < (int)container->getNumElements())
    {
      g.setFont(10);
      g.setColour(Colours::black);
      g.drawFittedText(elementValueToString(row, column - 100),
        0, 0, width, height, Justification::centred, 1);
    }
  }
  
  virtual void cellClicked(int row, int column, const MouseEvent& e)
    {}

  virtual void cellDoubleClicked(int row, int column, const MouseEvent& e)
    {}
  
  virtual int getColumnAutoSizeWidth(int columnId)
  {
    columnId -= 100;
    return columnId >= 0 && columnId < (int)autoSizeWidths.size() ? autoSizeWidths[columnId] + 10 : 0;
  }

  virtual void sortOrderChanged(int columnId, const bool isForwards)
  {
    columnId -= 100;
    // FIXME !
    //jassert(columnId >= 0 && columnId < (int)table->getNumColumns()); 
    //setTable(table->sort((size_t)columnId, !isForwards));
  }

  juce_UseDebuggingNewOperator

protected:
  ContainerPtr container;

  std::vector<int> autoSizeWidths;
  
  void computeAutoColumnWidths()
  {
    Font font(10);

    autoSizeWidths.clear();

    TypePtr rowType = container->getElementsType();
    size_t numRows = container->getNumElements();
    size_t numColumns = rowType->getObjectNumVariables();

    for (size_t j = 0; j < numColumns; ++j)
    {
      String value = rowType->getObjectVariableName(j);
      autoSizeWidths.push_back(font.getStringWidth(value));
    }

    for (size_t i = 0; i < numRows; ++i)
    {
      ObjectPtr row = container->getElement(i).getObject();
      for (size_t j = 0; j < numColumns; ++j)
      {
        int w = font.getStringWidth(elementValueToString(row, j));
        if (w > autoSizeWidths[j])
          autoSizeWidths[j] = w;
      }
    }
  }

  String elementValueToString(size_t rowIndex, size_t columnIndex) const
    {return elementValueToString(container->getElement(rowIndex).getObject(), columnIndex);}

  String elementValueToString(const ObjectPtr& row, size_t columnIndex) const
  {
    Variable value = row->getVariable(columnIndex);
    if (!value.exists())
      return String::empty;
    return value.toShortString();
  }
};

/*
** ContainerTableListBox
*/
ContainerTableListBox::ContainerTableListBox(const ContainerPtr& container)
  : TableListBox("table", NULL)
{
  TypePtr rowType = container->getElementsType();

  TableHeaderComponent* hdr = getHeader();
  for (size_t i = 0; i < rowType->getObjectNumVariables(); ++i)
    hdr->addColumn(rowType->getObjectVariableName(i), i + 100, 100);

  setModel(new ContainerTableListBoxModel(container));
//    autoSizeAllColumns();
  setAutoSizeMenuOptionShown(true);
}

ContainerTableListBox::~ContainerTableListBox()
  {delete getModel();}
