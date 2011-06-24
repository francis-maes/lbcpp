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
# include "../../juce/juce_CodeEditorComponent.h"
# include "../../juce/juce_LuaCodeTokeniser.h"

namespace lbcpp
{

class LuaCodeEditorComponent : public Component, public ComponentWithPreferedSize
{
public:
  LuaCodeEditorComponent(const File& luaFile)
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
      defaultExecutionContext().errorCallback(T("Could not open file ") + luaFile.getFullPathName());

    addAndMakeVisible(codeEditor = new juce::CodeEditorComponent(document, &tokeniser));
  }

  virtual int getDefaultWidth() const
    {return juce::Desktop::getInstance().getMainMonitorArea().getWidth() / 3;}

  virtual void resized()
    {codeEditor->setBoundsRelative(0, 0, 1, 1);}

protected:
  juce::CodeEditorComponent* codeEditor;
  juce::CodeDocument document;
  juce::LuaCodeTokeniser tokeniser;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_COMPONENTS_LUA_CODE_EDITOR_H_
