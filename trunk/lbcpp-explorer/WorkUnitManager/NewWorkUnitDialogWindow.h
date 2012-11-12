/*-----------------------------------------.---------------------------------.
| Filename: NewWorkUnitDialogWindow.h      | The dialog window to start a    |
| Author  : Francis Maes                   |  new work unit                  |
| Started : 02/12/2010 03:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXPLORER_WORK_UNIT_MANAGER_NEW_DIALOG_WINDOW_H_
# define LBCPP_EXPLORER_WORK_UNIT_MANAGER_NEW_DIALOG_WINDOW_H_

# include "../ExplorerProject.h"
# include "../Components/common.h"

namespace lbcpp
{

class NewWorkUnitContentComponent;

class NewWorkUnitDialogWindow : public juce::DocumentWindow
{
public:
  static bool run(ExecutionContext& context, RecentWorkUnitsConfigurationPtr recent, string& workUnitName, string& arguments);
  
  virtual void closeButtonPressed();

  //virtual void resized();

  juce_UseDebuggingNewOperator

private:
  NewWorkUnitDialogWindow(ExecutionContext& context, RecentWorkUnitsConfigurationPtr recent, string& workUnitName, string& arguments);
  
  ExecutionContext& context;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_WORK_UNIT_MANAGER_NEW_DIALOG_WINDOW_H_
