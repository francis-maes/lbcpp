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

namespace lbcpp
{

class LuaCodeEditorComponent : public Component, public ComponentWithPreferedSize, public VariableSelector
{
public:
  LuaCodeEditorComponent(const File& luaFile)
    : luaFile(luaFile), name(luaFile.getFileName()), context(ExplorerProject::currentProject->workUnitContext), luaState(*context)
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

  virtual ~LuaCodeEditorComponent()
  {
    deleteAllChildren();
    document.replaceAllContent(String::empty);
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

    // set current working directory to root directory
    // FIXME: find a better way to fix lua directly to take our root directory
    ExplorerProject::getCurrentProject()->getRootDirectory().setAsCurrentWorkingDirectory(); 
    luaState.execute(code, name);
  }
 
  void saveFile()
  {
    if (luaFile.exists())
      luaFile.deleteFile();
    OutputStream* ostr = luaFile.createOutputStream();
    if (ostr)
    {
      *ostr << document.getAllContent();
      delete ostr;
    }
  }

  virtual bool keyPressed(const juce::KeyPress& key)
  {
    if (key.getKeyCode() == juce::KeyPress::returnKey && key.getModifiers().isCommandDown())
    {
      executeSelectedCode();
      return true;
    }
    if (juce::CharacterFunctions::toLowerCase((char)key.getKeyCode()) == 's' && key.getModifiers().isCommandDown())
    {
      saveFile();
      return true;
    }
    return false;
  }
 
  virtual juce::Component* createComponentForVariable(ExecutionContext& context, const Variable& variable, const String& name)
    {return userInterfaceManager().createExecutionTraceInteractiveTreeView(context, trace, this->context);}

protected:
  File luaFile;
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
