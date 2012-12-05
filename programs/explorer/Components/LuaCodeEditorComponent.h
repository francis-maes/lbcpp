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
# include <oil/UserInterface/ObjectComponent.h>
# include <oil/UserInterface/UserInterfaceManager.h>
# include <oil/Lua/Lua.h>
# include "../../juce/juce_CodeEditorComponent.h"
# include "../../juce/juce_LuaCodeTokeniser.h"
# include "../ExplorerProject.h"

class LuaCodeEditorComponent;
class LuaCodeEditorStatusBar;

namespace lbcpp
{

class LuaCodeEditor : public Component, public ComponentWithPreferedSize, public ObjectSelector
{
public:
  LuaCodeEditor(const juce::File& luaFile);
  virtual ~LuaCodeEditor();

  virtual int getDefaultWidth() const
    {return juce::Desktop::getInstance().getMainMonitorArea().getWidth() / 3;}

  virtual void resized();
  virtual bool keyPressed(const juce::KeyPress& key);
 
  virtual juce::Component* createComponentForObject(ExecutionContext& context, const ObjectPtr& object, const string& name)
    {return userInterfaceManager().createExecutionTraceInteractiveTreeView(context, trace, this->context);}

  void updateStatus();

protected:
  juce::File luaFile;
  string name;
  ExecutionContextPtr context;  
  juce::CodeEditorComponent* codeEditor;
  LuaCodeEditorStatusBar* statusBar;
  juce::CodeDocument document;
  juce::LuaCodeTokeniser tokeniser;
  ExecutionTracePtr trace;

  void executeCode(bool verbose);
  void saveFile();
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_COMPONENTS_LUA_CODE_EDITOR_H_
