/*-----------------------------------------.---------------------------------.
| Filename: LuaCodeEditorComponent.cpp     | Lua Code Editor Component       |
| Author  : Francis Maes                   |                                 |
| Started : 08/08/2011 13:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "LuaCodeEditorComponent.h"
using namespace lbcpp;

class LuaCodeEditorComponent : public juce::CodeEditorComponent
{
public:
  typedef juce::CodeDocument::Position Position;

  LuaCodeEditorComponent(LuaCodeEditor* owner, juce::CodeDocument& document, juce::CodeTokeniser* const codeTokeniser)
    : juce::CodeEditorComponent(document, codeTokeniser), owner(owner)
  {
    setTabSize(2, true);
  }

  virtual bool keyPressed(const juce::KeyPress& key)
  {
    bool hasSelectedText = (getSelectionEnd() != getSelectionStart());
    if (hasSelectedText && key.getKeyCode() == juce::KeyPress::tabKey)
      {indentSelection(!key.getModifiers().isShiftDown()); return true;}

    if (key.getModifiers().isCommandDown() && (key.getKeyCode() == 'G' || key.getKeyCode() == 'g'))
      {gotoLine(); return true;}

    bool res = juce::CodeEditorComponent::keyPressed(key);
    owner->updateStatus();
    return res;
  }

  virtual void mouseUp(const juce::MouseEvent& e)
  {
    juce::CodeEditorComponent::mouseUp(e);
    owner->updateStatus();
  }

protected:
  LuaCodeEditor* owner;

  // command+g
  void gotoLine()
  {
    String lineStr(getCaretPos().getLineNumber() + 1);
    AlertWindow alertWindow("Goto line", "Choose the line number", AlertWindow::QuestionIcon);
    alertWindow.addTextEditor("line", lineStr);
    alertWindow.addButton("Ok", 1, juce::KeyPress::returnKey);
    alertWindow.addButton("Cancel", 0, juce::KeyPress::escapeKey);
    
    if (alertWindow.runModalLoop() == 1)
    {
      lineStr = alertWindow.getTextEditorContents("line");
      if (lineStr.containsOnly(T("0123456789")))
      {
        int lineNumber = juce::jlimit(1, document.getNumLines(), lineStr.getIntValue());
        moveCaretTo(juce::CodeDocument::Position(&document, lineNumber - 1, 0), false);
        owner->updateStatus();
      }
      else
        AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Goto line", "Not a valid line number");
    }
  }

  // command+tab and command+shift+tab
  void indentSelection(bool addIndent)
  {
    Position selectionStart = getSelectionStart();
    Position selectionEnd = getSelectionEnd();
    Position caretPos = getCaretPos();

    document.newTransaction();
    for (int lineNumber = selectionStart.getLineNumber(); lineNumber <= selectionEnd.getLineNumber(); ++lineNumber)
    {
      String line = document.getLine(lineNumber);
      document.deleteSection(Position(&document, lineNumber, 0), Position(&document, lineNumber, line.length()));
      line = line.dropLastCharacters(1); // remove \n

      int n = 0;
      while (n < line.length() && line[n] == ' ')
        ++n;

      if (addIndent)
        line = (n % 2 ? " " : "  ") + line;
      else
      {
        if (n >= 2)
          line = line.substring(2);
        else if (n == 1)
          line = line.substring(1);
      }

      document.insertText(Position(&document, lineNumber, 0), line);
    }

 /*   moveCaretTo(selectionStart, false);
    moveCaretTo(selectionEnd, true);
    moveCaretTo(caretPos, false);*/
  }
};

class LuaCodeEditorStatusBar : public Component
{
public:
  juce::CodeDocument::Position currentPosition;

  virtual void paint(Graphics& g)
  {
    g.fillAll(Colour(240, 245, 250));
    String text = "Ln " + String(currentPosition.getLineNumber() + 1) + " Col " + String(currentPosition.getIndexInLine() + 1);
    g.setColour(Colours::black);
    g.drawText(text, 0, 0, getWidth(), getHeight(), Justification::centredRight, false);
  }
};

/*
** LuaCodeEditor
*/
LuaCodeEditor::LuaCodeEditor(const File& luaFile)
  : luaFile(luaFile), name(luaFile.getFileName()), context(ExplorerProject::currentProject->workUnitContext)
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

  addAndMakeVisible(codeEditor = new LuaCodeEditorComponent(this, document, &tokeniser));
  addAndMakeVisible(statusBar = new LuaCodeEditorStatusBar());
}

LuaCodeEditor::~LuaCodeEditor()
{
  deleteAllChildren();
  document.replaceAllContent(String::empty);
}

void LuaCodeEditor::resized()
{
  enum {statusBarHeight = 15};
  int w = getWidth();
  int h = getHeight();
  codeEditor->setBounds(0, 0, w, h - statusBarHeight);
  statusBar->setBounds(0, h - statusBarHeight, w, statusBarHeight);
}

bool LuaCodeEditor::keyPressed(const juce::KeyPress& key)
{
  if (key.getModifiers().isCommandDown())
  {
    if (key.getKeyCode() == juce::KeyPress::returnKey)
      {executeCode(key.getModifiers().isShiftDown()); return true;}

    char c = juce::CharacterFunctions::toLowerCase((char)key.getKeyCode());
    if (c == 's')
      {saveFile(); return true;}
    //if (c == 'g')
    //  {gotoLine(); return true;}
  }
  return false;
}

void LuaCodeEditor::updateStatus()
{
  if (codeEditor->getCaretPos() != statusBar->currentPosition)
  {
    statusBar->currentPosition = codeEditor->getCaretPos();
    statusBar->repaint();
  }
}

// command+enter
void LuaCodeEditor::executeCode(bool verbose)
{
  String code = codeEditor->getSelectedText();
  if (!code.containsNonWhitespaceChars())
    code = document.getAllContent();
  if (!code.containsNonWhitespaceChars())
    return;

  if (!trace)
  {
    trace = new ExecutionTrace(context->toString());
    sendSelectionChanged(trace, name);
  }

  // set current working directory to root directory
  // FIXME: find a better way to fix lua directly to take our root directory
  
  //ExplorerProject::getCurrentProject()->getRootDirectory().setAsCurrentWorkingDirectory(); 
  luaFile.getParentDirectory().setAsCurrentWorkingDirectory();

  ClassPtr workUnitClass = getType("ExecuteLuaString");
  if (workUnitClass)
  {
    WorkUnitPtr workUnit = WorkUnit::create(workUnitClass);
    workUnit->setVariable(0, code);
    workUnit->setVariable(1, name);
    workUnit->setVariable(2, verbose);
    context->pushWorkUnit(workUnit);
  }
}

// command+s
void LuaCodeEditor::saveFile()
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
