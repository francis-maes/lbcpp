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
# include "../Utilities/PropertyListDisplayComponent.h"
# include <lbcpp/UserInterface/VariableSelector.h>

class ProcessConsoleComponent;

namespace lbcpp
{

class ProcessComponent : public Component
{
public:
  ProcessComponent(ProcessPtr process, ProcessConsoleSettingsPtr settings = ProcessConsoleSettingsPtr());
  virtual ~ProcessComponent();

  virtual void resized();

  void updateContent();

  juce_UseDebuggingNewOperator

protected:
  ProcessPtr process;

  PropertyListDisplayComponent* properties;

  Viewport* viewport;
  ProcessConsoleComponent* console;
  Component* consoleTools;
};

class ProcessManagerComponent : public SplittedLayout, public MenuBarModel, public juce::Timer, public VariableSelectorCallback
{
public:
  ProcessManagerComponent(ProcessManagerPtr processManager);

  virtual void selectionChangedCallback(VariableSelector* selector, const std::vector<Variable>& selectedVariables);

  // MenuBarModel
  virtual const StringArray getMenuBarNames();
  virtual const PopupMenu getMenuForIndex(int topLevelMenuIndex, const String& menuName);
  virtual void menuItemSelected(int menuItemID, int topLevelMenuIndex);

  // Timer
  virtual void timerCallback();

  juce_UseDebuggingNewOperator

private:
  ProcessManagerPtr processManager;

  void updateProcessLists();
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_PROCESS_MANAGER_COMPONENT_H_

