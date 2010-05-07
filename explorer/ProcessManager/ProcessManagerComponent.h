/*-----------------------------------------.---------------------------------.
| Filename: ProcessManagerComponent.h      | Process Manager UI Component    |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 13:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXPLORER_PROCESS_MANAGER_COMPONENT_H_
# define LBCPP_EXPLORER_PROCESS_MANAGER_COMPONENT_H_

# include "ProcessManager.h"
# include "../Components/common.h"
# include "../Utilities/SplittedLayout.h"

namespace lbcpp
{

class ProcessManagerComponent : public SplittedLayout, public MenuBarModel, public juce::Timer
{
public:
  ProcessManagerComponent(ProcessManagerPtr processManager);

  virtual void processSelectedCallback(ProcessPtr process);

  // MenuBarModel
  virtual const StringArray getMenuBarNames();
  virtual const PopupMenu getMenuForIndex(int topLevelMenuIndex, const String& menuName);
  virtual void menuItemSelected(int menuItemID, int topLevelMenuIndex);

  // Timer
  virtual void timerCallback();

  // Component
  virtual void resized();

  juce_UseDebuggingNewOperator

private:
  ProcessManagerPtr processManager;

  void updateProcessLists();

  Viewport* getViewport() const
    {return (Viewport* )second;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_PROCESS_MANAGER_COMPONENT_H_

