/*-----------------------------------------.---------------------------------.
| Filename: TableComponent.h               | Table Component                 |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 16:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_COMPONENT_DATA_TABLE_H_
# define LBCPP_USER_INTERFACE_COMPONENT_DATA_TABLE_H_

# include <oil/UserInterface/ObjectComponent.h>
# include <oil/Execution/ExecutionTrace.h>
# include <oil/Core/Table.h>

namespace lbcpp
{

class TableComponent : public juce::TableListBox, public ComponentWithPreferedSize
{
public:
  TableComponent(const TablePtr& table, const string& name);
  ~TableComponent();
  
  virtual int getDefaultWidth() const
    {return 600;}

  virtual int getPreferedWidth(int availableWidth, int availableHeight) const;

  juce_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENT_DATA_TABLE_H_
