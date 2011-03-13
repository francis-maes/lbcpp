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

class GoProblem : public SequentialDecisionProblem
{
public:
  GoProblem(double discount = 1.0) 
    : SequentialDecisionProblem(FunctionPtr(), // initial state sampler
                                new GoTransitionFunction(),
                                new GoRewardFunction(), discount) {}
};

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

  

    context.resultCallback(T("XML"), xml);
    return true;
  }

private:
  friend class GoSandBoxClass;

  File fileToParse;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_
