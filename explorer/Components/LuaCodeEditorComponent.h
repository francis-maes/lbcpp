/*-----------------------------------------.---------------------------------.
| Filename: LuaCodeEditorComponent.h       | Lua Code Editor Component       |
| Author  : Francis Maes                   |                                 |
| Started : 25/06/2011 02:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXPLORER_COMPONENTS_LUA_CODE_EDITOR_H_
# define LBCPP_EXPLORER_COMPONENTS_LUA_CODE_EDITOR_H_

# include "common.h"

# include <lbcpp/UserInterface/VariableSelector.h>
# include <lbcpp/UserInterface/ObjectEditor.h>
# include <lbcpp/Lua/Lua.h>
# include "../../juce/juce_CodeEditorComponent.h"
# include "../../juce/juce_LuaCodeTokeniser.h"
# include "../explorer/ExplorerProject.h"

class LuaCodeEditorComponent;
class LuaCodeEditorStatusBar;

namespace lbcpp
{

class LuaCodeEditor : public Component, public ComponentWithPreferedSize, public VariableSelector
{
public:
  LuaCodeEditor(const File& luaFile);
  virtual ~LuaCodeEditor();

  virtual int getDefaultWidth() const
    {return juce::Desktop::getInstance().getMainMonitorArea().getWidth() / 3;}

  virtual void resized();
  virtual bool keyPressed(const juce::KeyPress& key);
 
  virtual juce::Component* createComponentForVariable(ExecutionContext& context, const Variable& variable, const String& name)
    {return userInterfaceManager().createExecutionTraceInteractiveTreeView(context, trace, this->context);}

  void updateStatus();

protected:
  File luaFile;
  String name;
  ExecutionContextPtr context;  
  juce::CodeEditorComponent* codeEditor;
  LuaCodeEditorStatusBar* statusBar;
  juce::CodeDocument document;
  juce::LuaCodeTokeniser tokeniser;
  ExecutionTracePtr trace;

  void executeCode();
  void saveFile();
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_COMPONENTS_LUA_CODE_EDITOR_H_
