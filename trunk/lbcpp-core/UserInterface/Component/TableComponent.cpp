/*-----------------------------------------.---------------------------------.
| Filename: TableComponent.cpp             | Table Component                 |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 16:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include "TableComponent.h"
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

class TableComponentListBoxModel : public TableListBoxModel
{
public:
  TableComponentListBoxModel(const TablePtr& table) : table(table)
    {computeAutoColumnWidths();}
     
  virtual int getNumRows()
    {return table->getNumRows();}

  virtual void paintRowBackground(Graphics& g, int row, int width, int height, bool selected)
  {
    g.setColour(selected ? Colours::lightskyblue : (row % 2 ? Colour(230, 240, 255) : Colour(220, 230, 245)));
    g.fillRect(0, 0, width, height);
  }
  
  virtual void paintCell(Graphics& g, int row, int column, int width, int height, bool selected)
  {
    if (order.size())
      row = (int)order[row];
    if (row < (int)table->getNumRows())
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
    table->makeOrder((size_t)columnId, isForwards, order);
  }

  juce_UseDebuggingNewOperator

protected:
  TablePtr table;

  std::vector<int> autoSizeWidths;
  std::vector<size_t> order;
  
  void computeAutoColumnWidths()
  {
    Font font(10);

    autoSizeWidths.clear();

    size_t numRows = table->getNumRows();
    size_t numColumns = table->getNumColumns();

    for (size_t j = 0; j < numColumns; ++j)
    {
      String value = table->getDescription(j);
      autoSizeWidths.push_back(font.getStringWidth(value));
    }

    for (size_t i = 0; i < numRows; ++i)
    {
      for (size_t j = 0; j < numColumns; ++j)
      {
        int w = font.getStringWidth(elementValueToString(i, j));
        if (w > autoSizeWidths[j])
          autoSizeWidths[j] = w;
      }
    }
  }

  String elementValueToString(size_t rowIndex, size_t columnIndex) const
  {
    ObjectPtr value = table->getElement(rowIndex, columnIndex);
    return value ? value->toShortString() : String::empty;
  }
};

/*
** TableComponent
*/
TableComponent::TableComponent(const TablePtr& table, const String& name)
  : TableListBox(name, NULL)
{
  TableHeaderComponent* hdr = getHeader();
  for (size_t i = 0; i < table->getNumColumns(); ++i)
    hdr->addColumn(table->getDescription(i), i + 100, 100);

  setModel(new TableComponentListBoxModel(table));
  autoSizeAllColumns();
  setAutoSizeMenuOptionShown(true);
}

TableComponent::~TableComponent()
  {delete getModel();}

int TableComponent::getPreferedWidth(int availableWidth, int availableHeight) const
  {return getViewport()->getViewedComponent()->getWidth();}
