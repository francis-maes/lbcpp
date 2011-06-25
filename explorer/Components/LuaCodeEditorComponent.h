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
# include "../Explorer/ExplorerProject.h"

namespace lbcpp
{

class LuaCodeEditorComponent : public Component, public ComponentWithPreferedSize, public VariableSelector
{
public:
  LuaCodeEditorComponent(const File& luaFile) : name(luaFile.getFileName()), context(ExplorerProject::currentProject->workUnitContext), luaState(*context)
  {
    InputStream* istr = luaFile.createInputStream();
    if (istr)
    {
      String luaCode;
      while (!istr->isExhausted())
        luaCode += istr->readNextLine() + T("\n");
      delete istr;

      document.replaceAllContent(luaCode);
    }
    else
      context->errorCallback(T("Could not open file ") + luaFile.getFullPathName());

    addAndMakeVisible(codeEditor = new juce::CodeEditorComponent(document, &tokeniser));
  }

  virtual int getDefaultWidth() const
    {return juce::Desktop::getInstance().getMainMonitorArea().getWidth() / 3;}

  virtual void resized()
    {codeEditor->setBoundsRelative(0, 0, 1, 1);}

  void executeSelectedCode()
  {
    String code = codeEditor->getSelectedText().trim();
    if (code.isEmpty())
      code = document.getAllContent().trim();
    if (code.isEmpty())
      return;

    if (!trace)
    {
      trace = new ExecutionTrace(context->toString());
      sendSelectionChanged(trace, name);
    }

    luaState.execute(code, name);
  }
 
  virtual bool keyPressed(const juce::KeyPress& key)
  {
    if (key.getKeyCode() == juce::KeyPress::returnKey && key.getModifiers().isCommandDown())
    {
      executeSelectedCode();
      return true;
    }
    return false;
  }
 
  virtual juce::Component* createComponentForVariable(ExecutionContext& context, const Variable& variable, const String& name)
    {return userInterfaceManager().createExecutionTraceInteractiveTreeView(context, trace, this->context);}

protected:
  String name;
  ExecutionContextPtr context;  
  juce::CodeEditorComponent* codeEditor;
  juce::CodeDocument document;
  juce::LuaCodeTokeniser tokeniser;
  ExecutionTracePtr trace;
  LuaState luaState;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_COMPONENTS_LUA_CODE_EDITOR_H_
