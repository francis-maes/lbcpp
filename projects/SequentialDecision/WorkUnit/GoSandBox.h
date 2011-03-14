/*-----------------------------------------.---------------------------------.
| Filename: GoSandBox.h                    | Go Sand Box                     |
| Author  : Francis Maes                   |                                 |
| Started : 13/03/2011 21:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_SAND_BOX_H_

# include "../Problem/GoProblem.h"
# include "../Problem/LoadSGFFileFunction.h"

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Data/Stream.h>

// ui
# include <lbcpp/UserInterface/ObjectEditor.h>
# include <lbcpp/UserInterface/ComponentWithPreferedSize.h>
# include <lbcpp/UserInterface/VariableSelector.h>
using namespace juce;

namespace lbcpp
{

class MatrixComponent : public Component, public ComponentWithPreferedSize, public VariableSelector
{
public:
  MatrixComponent(MatrixPtr matrix)
    : matrix(matrix), selectedRow(-1), selectedColumn(-1)
  {
  }
  
  int computePixelsPerEntry(int availableWidth, int availableHeight) const
  {
    int availableSize = juce::jmin(availableWidth, availableHeight);
    int n = juce::jmax((int)matrix->getNumRows(), (int)matrix->getNumColumns());
    return n ? juce::jmax(1, availableSize / (int)n) : 4;
  }
  
  virtual int getDefaultWidth() const
    {return 400;}

  virtual int getDefaultHeight() const
    {return 400;}

  virtual int getPreferedWidth(int availableWidth, int availableHeight) const
  {
    int pixelsPerEntry = computePixelsPerEntry(availableWidth, availableHeight);
    return juce::jmax(pixelsPerEntry * (int)matrix->getNumColumns(), availableWidth);
  }

  virtual int getPreferedHeight(int availableWidth, int availableHeight) const
  {
    int pixelsPerEntry = computePixelsPerEntry(availableWidth, availableHeight);
    return juce::jmax(pixelsPerEntry * (int)matrix->getNumRows(), availableHeight);
  }

  virtual void paint(Graphics& g)
  {
    int pixelsPerRow, pixelsPerColumn, x1, y1;
    getPaintCoordinates(pixelsPerRow, pixelsPerColumn, x1, y1);
  
    size_t numRows = matrix->getNumRows();
    size_t numColumns = matrix->getNumColumns();

    for (size_t row = 0; row < numRows; ++row)
      for (size_t column = 0; column < numColumns; ++column)
        paintEntry(g, x1 + column * pixelsPerColumn, y1 + row * pixelsPerRow, pixelsPerColumn, pixelsPerRow, matrix->getElement(row, column));
      
    if (selectedRow >= 0 && selectedColumn >= 0)
    {
      g.setColour(Colours::lightblue.withAlpha(0.7f));
      g.fillRect(x1 + (selectedColumn - 4) * pixelsPerColumn, y1 + selectedRow * pixelsPerRow, 9 * pixelsPerColumn - 1, pixelsPerRow - 1);
      g.fillRect(x1 + selectedColumn * pixelsPerColumn, y1 + (selectedRow - 4) * pixelsPerRow, pixelsPerColumn - 1, 9 * pixelsPerRow - 1);
    }

    g.setColour(Colours::black);
    g.drawRect(x1, y1, (int)numColumns * pixelsPerColumn, (int)numRows * pixelsPerRow);
  }

  virtual void mouseUp(const MouseEvent& e)
  {
    int pixelsPerRow, pixelsPerColumn, x1, y1;
    getPaintCoordinates(pixelsPerRow, pixelsPerColumn, x1, y1);
    jassert(pixelsPerRow && pixelsPerColumn);
    int x = (e.getMouseDownX() - x1) / pixelsPerColumn;
    int y = (e.getMouseDownY() - y1) / pixelsPerRow;
    if (x >= 0 && y >= 0 && x < (int)matrix->getNumColumns() && y < (int)matrix->getNumRows())
    {
      selectedRow = y;
      selectedColumn = x;
      sendSelectionChanged(matrix->getElement(selectedRow, selectedColumn),
        T("Element (") + String((int)selectedRow) + T(", ") + String((int)selectedColumn) + T(")"));
      repaint();
      return;
    }

    selectedRow = selectedColumn = -1;
    sendSelectionChanged(std::vector<Variable>(), String::empty);
    repaint();
  }
  
  // overidable
  virtual Colour selectColour(const Variable& element) const = 0;
  virtual void paintEntry(Graphics& g, int x1, int y1, int width, int height, const Variable& element)
  {
    if (matrix)
    {
      g.setColour(selectColour(element));
      g.fillRect(x1, y1, width, height);
    }
  }

private:
  MatrixPtr matrix;
  int selectedRow, selectedColumn;

  void getPaintCoordinates(int& pixelsPerRow, int& pixelsPerColumn, int& x1, int& y1) const
  {
    pixelsPerRow = pixelsPerColumn = computePixelsPerEntry(getWidth(), getHeight());
    x1 = (getWidth() - pixelsPerColumn * (int)matrix->getNumColumns()) / 2;
    y1 = (getHeight() - pixelsPerRow * (int)matrix->getNumRows()) / 2;
  }
};

class GoStateComponent : public MatrixComponent
{
public:
  GoStateComponent(GoStatePtr state, const String& name)
    : MatrixComponent(state->getBoard()) {}
 
  virtual Colour selectColour(const Variable& element) const
  {
    if (!element.exists())
      return Colours::lightgrey;
    const juce::Colour colours[] = {juce::Colours::beige, juce::Colours::black, juce::Colours::white, juce::Colours::grey};
    return colours[element.getInteger() % (sizeof (colours) / sizeof (juce::Colour))];
  }
};

//////////////////////////////////////////////////////////////////////////


class GoSandBox : public WorkUnit
{
public:
  GoSandBox() : maxCount(0), numFolds(7)
  {
  }

  ContainerPtr loadGames(ExecutionContext& context, const File& directory, size_t maxCount)
  {
    if (!gamesDirectory.isDirectory())
    {
      context.errorCallback(T("Invalid games directory"));
      return ContainerPtr();
    }

    return directoryFileStream(context, directory, T("*.sgf"))->load(maxCount, false)->apply(context, new LoadSGFFileFunction(), Container::parallelApply);
  }

  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr games = loadGames(context, gamesDirectory, maxCount);
    if (!games)
      return false;

    ContainerPtr trainingGames = games->invFold(0, numFolds);
    ContainerPtr testingGames = games->fold(0, numFolds);
    context.informationCallback(String((int)trainingGames->getNumElements()) + T(" training games, ") + String((int)testingGames->getNumElements()) + T(" testing games"));

    return true;
#if 0
    if (!fileToParse.existsAsFile())
    {
      context.errorCallback(T("File to parse does not exist"));
      return false;
    }

    XmlElementPtr xml = (new SGFFileParser(context, fileToParse))->next().dynamicCast<XmlElement>();
    if (!xml)
      return false;

    //context.resultCallback(T("XML"), xml);

    FunctionPtr convert = new ConvertSGFXmlToStateAndTrajectory();
    PairPtr stateAndTrajectory = convert->compute(context, xml).getObjectAndCast<Pair>();
    if (!stateAndTrajectory)
      return false;
  
    Variable initialState = stateAndTrajectory->getFirst();
    ContainerPtr trajectory = stateAndTrajectory->getSecond().getObjectAndCast<Container>();

    context.resultCallback(T("Initial State"), initialState);
    context.resultCallback(T("Trajectory"), trajectory);

    DecisionProblemPtr problem = new GoProblem(0);
    if (!problem->initialize(context))
      return false;

    Variable finalState = problem->computeFinalState(context, initialState, trajectory);
    context.resultCallback(T("Final State"), finalState);
    return true;
#endif // 0
  }

private:
  friend class GoSandBoxClass;

  File gamesDirectory;
  size_t maxCount;
  size_t numFolds;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_
