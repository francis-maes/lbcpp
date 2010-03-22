/*-----------------------------------------.---------------------------------.
| Filename: TableComponent.h               | Table Component                 |
| Author  : Francis Maes                   |                                 |
| Started : 15/06/2009 16:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_TABLE_H_
# define EXPLORER_COMPONENTS_TABLE_H_

# include "common.h"

namespace lbcpp
{

class TableFrameListBoxModel : public TableListBoxModel
{
public:
  TableFrameListBoxModel(TablePtr table)
     {setTable(table);}
     
  virtual int getNumRows()
    {return table->getNumRows();}

  virtual void paintRowBackground(Graphics& g, int row, int width, int height, bool selected)
  {
    g.setColour(selected ? Colours::lightskyblue : (row % 2 ? Colour(230, 240, 255) : Colour(220, 230, 245)));
    g.fillRect(0, 0, width, height);
  }
  
  virtual void paintCell(Graphics& g, int row, int column, int width, int height, bool selected)
  {
    if (row < (int)table->getNumRows())
    {
      g.setFont(10);
      g.setColour(Colours::black);
      g.drawFittedText(table->toString(row, column - 100).c_str(),
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

  virtual void sortOrderChanged(int newSortColumnId, const bool isForwards)
  {
/*    representation::TaskPtr sortTask = 
      representation::Frame::createFrameCast<representation::Task>("SortTableTask");
    representation::TaskEnvironmentPtr taskEnvironment = 
      representation::Frame::createFrameCast<representation::TaskEnvironment>("QuietTaskEnvironment");
    sortTask->setFrameAttribute(T("table"), table);
    sortTask->setStringAttribute(T("attribute"), table->getColumnAttribute(newSortColumnId - 100));
    sortTask->setBoolAttribute(T("ascending"), isForwards);
    sortTask->run(taskEnvironment);
    setTable(sortTask->getFrameAttributeCast<representation::Table>(T("result")));*/
  }

protected:
  TablePtr table;
  std::vector<int> autoSizeWidths;
  
  void setTable(TablePtr table)
  {
    this->table = table;
    Font font(10);

    autoSizeWidths.clear();
    TableHeaderPtr header = table->getHeader();
    for (size_t j = 0; j < header->getNumColumns(); ++j)
    {
      std::string value = header->getColumnName(j);
      autoSizeWidths.push_back(font.getStringWidth(value.c_str()));
    }

    for (size_t i = 0; i < table->getNumRows(); ++i)
      for (size_t j = 0; j < header->getNumColumns(); ++j)
      {
        int w = font.getStringWidth(table->toString(i, j).c_str());
        if (w > autoSizeWidths[j])
          autoSizeWidths[j] = w;
      }
  }
};

class TableComponent : public TableListBox
{
public:
  TableComponent(TablePtr table)
    : TableListBox(T("table"), NULL)
  {
    TableHeaderPtr header = table->getHeader();
    TableHeaderComponent* hdr = getHeader();
    for (size_t i = 0; i < header->getNumColumns(); ++i)
      hdr->addColumn(header->getColumnName(i).c_str(), i + 100, 100);
    setModel(new TableFrameListBoxModel(table));
//    autoSizeAllColumns();
    setAutoSizeMenuOptionShown(true);
  }

  ~TableComponent()
    {delete getModel();}

  juce_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_TABLE_H_
