/*-----------------------------------------.---------------------------------.
| Filename: GoSandBox.h                    | Go Sand Box                     |
| Author  : Francis Maes                   |                                 |
| Started : 13/03/2011 21:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_GO_SAND_BOX_H_

# include "../Core/SearchTree.h"
# include "../Core/SearchTreeEvaluator.h"
# include "../Core/SearchFunction.h"
# include "../Core/SearchPolicy.h"

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Data/Stream.h>


# include <lbcpp/UserInterface/ObjectEditor.h>
# include <lbcpp/UserInterface/ComponentWithPreferedSize.h>
# include <lbcpp/UserInterface/VariableSelector.h>
using namespace juce;


namespace lbcpp
{

class Matrix : public Container
{
public:
  Matrix(ClassPtr thisClass)
    : Container(thisClass) {}
  Matrix() {}
  
  virtual size_t getNumRows() const = 0;
  virtual size_t getNumColumns() const = 0;

  virtual size_t getNumElements() const
    {return getNumRows() * getNumColumns();}

  virtual Variable getElement(size_t row, size_t column) const = 0;
  virtual void setElement(size_t row, size_t column, const Variable& value) = 0;

protected:
  size_t makeIndex(size_t row, size_t column) const
    {return row * getNumColumns() + column;}
};

typedef ReferenceCountedObjectPtr<Matrix> MatrixPtr;

template<class ElementsType>
class BuiltinTypeMatrix : public Matrix
{
public:
  BuiltinTypeMatrix(TypePtr elementsType, size_t numRows, size_t numColumns, const ElementsType& initialValue)
    : elementsType(elementsType), elements(numRows * numColumns, initialValue), numRows(numRows), numColumns(numColumns) {}
  BuiltinTypeMatrix() : numRows(0), numColumns(0) {}

  virtual size_t getNumRows() const
    {return numRows;}

  virtual size_t getNumColumns() const
    {return numColumns;}

  virtual TypePtr getElementsType() const
    {return elementsType;}

  virtual Variable getElement(size_t index) const
    {jassert(index < elements.size()); return Variable(elements[index], elementsType);}

  virtual Variable getElement(size_t row, size_t column) const
    {return getElement(makeIndex(row, column));}

  void setElement(size_t row, size_t column, const ElementsType& value)
    {setElement(makeIndex(row, column), value);}

  void setElement(size_t index, const ElementsType& value)
    {jassert(index < elements.size()); elements[index] = value;}
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    BuiltinTypeMatrix<ElementsType>& target = *t.staticCast< BuiltinTypeMatrix<ElementsType> >();
    target.elementsType = elementsType;
    target.elements = elements;
    target.numRows = numRows;
    target.numColumns = numColumns;
  }

protected:
  TypePtr elementsType;
  std::vector<ElementsType> elements;
  size_t numRows, numColumns;
};

class ShortEnumerationMatrix : public BuiltinTypeMatrix<char>
{
public:
  ShortEnumerationMatrix(EnumerationPtr enumeration, size_t numRows, size_t numColumns, char defaultValue)
    : BuiltinTypeMatrix(enumeration, numRows, numColumns, defaultValue)
    {jassert(enumeration->getNumElements() < 255);}

  ShortEnumerationMatrix() {}

  virtual void setElement(size_t row, size_t column, const Variable& value)
    {BuiltinTypeMatrix<char>::setElement(row, column, value.getInteger());}

  virtual void setElement(size_t index, const Variable& value)
    {BuiltinTypeMatrix<char>::setElement(index, value.getInteger());}
};

//////////////////////////////////////////////////////////////////////////

extern EnumerationPtr playerEnumeration;

enum Player
{
  noPlayers = 0,
  blackPlayer,
  whitePlayer,
  bothPlayers
};

class GoBoard : public ShortEnumerationMatrix
{
public:
  GoBoard(size_t size)
    : ShortEnumerationMatrix(playerEnumeration, size, size, 0) {}
  GoBoard() {}

  void set(size_t row, size_t column, Player player)
    {elements[makeIndex(row, column)] = (char)player;}

  Player get(size_t row, size_t column) const
    {return (Player)elements[makeIndex(row, column)];}
};

typedef ReferenceCountedObjectPtr<GoBoard> GoBoardPtr;

extern ClassPtr goStateClass;

class GoState : public Object
{
public:
  GoState(size_t time, GoBoardPtr board)
    : Object(goStateClass), time(time), board(board) {}
  GoState() : time(0) {}

  size_t getTime() const
    {return time;}

  const GoBoardPtr& getBoard() const
    {return board;}

protected:
  friend class GoStateClass;

  size_t time;
  GoBoardPtr board;
};

typedef ReferenceCountedObjectPtr<GoState> GoStatePtr;

class GoStateSampler : public SimpleUnaryFunction
{
public:
  GoStateSampler(size_t size = 19)
    : SimpleUnaryFunction(randomGeneratorClass, goStateClass), minSize(size), maxSize(size + 1) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const RandomGeneratorPtr& random = input.getObjectAndCast<RandomGenerator>();
    return new GoState(0, new GoBoard(random->sampleSize(minSize, maxSize)));
  }

protected:
  friend class GoStateSamplerClass;

  size_t minSize;
  size_t maxSize;
};


// state: GoState, action: Pair<PositiveInteger, PositiveInteger>
class GoTransitionFunction : public SimpleBinaryFunction
{
public:
  GoTransitionFunction()
    : SimpleBinaryFunction(goStateClass, pairClass(positiveIntegerType, positiveIntegerType), goStateClass) {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const GoStatePtr& state = inputs[0].getObjectAndCast<GoState>();
    const PairPtr& action = inputs[1].getObjectAndCast<Pair>();
    size_t row = (size_t)action->getFirst().getInteger();
    size_t column = (size_t)action->getSecond().getInteger();

    size_t time = state->getTime();

    GoBoardPtr newBoard = state->getBoard()->cloneAndCast<GoBoard>();
    newBoard->set(row, column, (time % 2) == 0 ? blackPlayer : whitePlayer);
    
    // todo: ...

    return new GoState(time + 1, newBoard);
  }
};

class GoRewardFunction : public SimpleBinaryFunction
{
public:
  GoRewardFunction()
    : SimpleBinaryFunction(goStateClass, pairClass(positiveIntegerType, positiveIntegerType), doubleType) {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const GoStatePtr& state = inputs[0].getObjectAndCast<GoState>();
    const PairPtr& action = inputs[1].getObjectAndCast<Pair>();
    // FIXME
    return 0.0;
  }
};

class GoProblem : public DecisionProblem
{
public:
  GoProblem(size_t size = 19)
    : DecisionProblem(new GoStateSampler(size), new GoTransitionFunction(), new GoRewardFunction(), 1.0) {}
};

typedef ReferenceCountedObjectPtr<GoProblem> GoProblemPtr;

//////////////////////////////////////////////////////////////////////////

class SGFFileParser : public TextParser
{
public:
  SGFFileParser(ExecutionContext& context, const File& file)
    : TextParser(context, file) {}
  SGFFileParser() {}
    
  virtual TypePtr getElementsType() const
    {return xmlElementClass;}

  virtual void parseBegin()
  {
    stack.clear();
    isParsingAttributeValue = false;
  }

  virtual bool parseLine(const String& line)
  {
    for (int i = 0; i < line.length(); ++i)
    {
      if (line[i] == '(')
      {
        XmlElementPtr node = new XmlElement(T("node"));
        if (!res)
          res = node;
        stack.push_back(node);
      }
      else if (line[i] == ')')
      {
        if (stack.empty())
        {
          context.errorCallback(T("Missing parenthesis"));
          return false;
        }
        stack.pop_back();
      }
      else if (line[i] == ';')
      {
        if (stack.empty())
        {
          context.errorCallback(T("Empty stack, missing parenthesis"));
          return false;
        }
        XmlElementPtr res = stack.back();
        res->addChildElement(currentElement = new XmlElement(T("node")));
      }
      else
      {
        if (juce::CharacterFunctions::isWhitespace(line[i]) && 
            (!currentElement || 
              (isParsingAttributeValue && currentAttributeValue.isEmpty()) ||
              (!isParsingAttributeValue && currentAttributeName.isEmpty())))
          continue;

        if (!currentElement)
        {
          context.errorCallback(T("No current element, missing parenthesis or semicolon"));
          return false;
        }
        if (!isParsingAttributeValue)
        {
          if (line[i] == '[')
            isParsingAttributeValue = true;
          else
            currentAttributeName += line[i];
        }
        else
        {
          if (line[i] == ']')
          {
            currentElement->setAttribute(currentAttributeName, currentAttributeValue);
            currentAttributeName = currentAttributeValue = String::empty;
            isParsingAttributeValue = false;
          }
          else
            currentAttributeValue += line[i];
        }
      }
    }
    return true;
  }

  virtual bool parseEnd()
  {
    if (!res)
      return false;
    setResult(res);
    return true;
  }

private:
  XmlElementPtr res;
  std::vector<XmlElementPtr> stack;
  XmlElementPtr currentElement;
  String currentAttributeName;
  bool isParsingAttributeValue;
  String currentAttributeValue;
};

class ConvertSGFXmlToStateAndTrajectory : public SimpleUnaryFunction
{
public:
  ConvertSGFXmlToStateAndTrajectory()
    : SimpleUnaryFunction(xmlElementClass, pairClass(goStateClass, objectVectorClass(pairClass(positiveIntegerType, positiveIntegerType)))) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const XmlElementPtr& xml = input.getObjectAndCast<XmlElement>();
    size_t numXmlElements = xml->getNumChildElements();
    if (!numXmlElements)
    {
      context.errorCallback(T("Empty xml"));
      return Variable::missingValue(outputType);

    }

    XmlElementPtr firstChild = xml->getChildElement(0);
    size_t size = firstChild->getIntAttribute(T("SZ"), 0);
    if (!size)
    {
      context.errorCallback(T("No size attribute"));
      return Variable::missingValue(outputType);
    }

    ClassPtr actionType = pairClass(positiveIntegerType, positiveIntegerType);
    ObjectVectorPtr trajectory = new ObjectVector(actionType, numXmlElements - 1);
    bool isBlackTurn = true;
    for (size_t i = 1; i < numXmlElements; ++i)
    {
      String attribute = (isBlackTurn ? T("B") : T("W"));
      String value = xml->getChildElement(i)->getStringAttribute(attribute);
      if (value.isEmpty())
      {
        context.errorCallback(String(T("Could not find ")) + (isBlackTurn ? T("black") : T("white")) + T(" stone at step ") + String((int)i));
        return Variable::missingValue(outputType);
      }
      if (value.length() != 2)
      {
        context.errorCallback(T("Invalid move: ") + value);
        return Variable::missingValue(outputType);
      }
      size_t column = letterToIndex(context, value[0]);
      size_t row = letterToIndex(context, value[1]);
      if (row == (size_t)-1 || column == (size_t)-1)
        return Variable::missingValue(outputType);

      trajectory->set(i - 1, new Pair(actionType, row, column));
      isBlackTurn = !isBlackTurn;
    }

    return Variable(new Pair(outputType, new GoState(0, new GoBoard(size)), trajectory), outputType);
  }

private:
  static size_t letterToIndex(ExecutionContext& context, const juce::tchar letter)
  {
    if (letter >= 'a' && letter <= 'z')
      return letter - 'a';
    else if (letter >= 'A' && letter <= 'Z')
      return letter - 'A' + 26;
    else
    {
      context.errorCallback(T("Could not parse position ") + letter);
      return (size_t)-1;
    }
  }
};

//////////////////////////////////////////////////////////////////////////

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
  GoSandBox()
  {
  }

  virtual Variable run(ExecutionContext& context)
  {
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
  }

private:
  friend class GoSandBoxClass;

  File fileToParse;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_
