/*-----------------------------------------.---------------------------------.
| Filename: ContainerTableListBox.h        | Table Component                 |
| Author  : Francis Maes                   |                                 |
| Started : 27/01/2011 18:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_COMPONENTS_CONTAINER_TABLE_LIST_BOX_H_
# define LBCPP_USER_INTERFACE_COMPONENTS_CONTAINER_TABLE_LIST_BOX_H_

# include <lbcpp/UserInterface/SimpleTreeViewItem.h>
# include <lbcpp/UserInterface/VariableSelector.h>
# include <lbcpp/UserInterface/ComponentWithPreferedSize.h>
# include <lbcpp/Execution/ExecutionTrace.h>

namespace lbcpp
{

class ContainerTableListBox : public juce::TableListBox
{
public:
  ContainerTableListBox(const ContainerPtr& container);
  ~ContainerTableListBox();

  juce_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENTS_CONTAINER_TABLE_LIST_BOX_H_
