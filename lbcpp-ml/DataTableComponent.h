/*-----------------------------------------.---------------------------------.
| Filename: DataTableComponent.h           | Data Table Component            |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 16:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_DATA_TABLE_COMPONENT_H_
# define LBCPP_ML_DATA_TABLE_COMPONENT_H_

# include <lbcpp/UserInterface/VariableSelector.h>
# include <lbcpp/UserInterface/ComponentWithPreferedSize.h>
# include <lbcpp/Execution/ExecutionTrace.h>
# include <lbcpp-ml/DataTable.h>

namespace lbcpp
{

class DataTableComponent : public juce::TableListBox, public ComponentWithPreferedSize
{
public:
  DataTableComponent(const DataTablePtr& data, const String& name);
  ~DataTableComponent();
  
  virtual int getDefaultWidth() const
    {return 600;}

  virtual int getPreferedWidth(int availableWidth, int availableHeight) const;

  juce_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_DATA_TABLE_COMPONENT_H_
