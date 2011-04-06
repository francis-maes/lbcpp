/*-----------------------------------------.---------------------------------.
| Filename: LoadSGFFileFunction.h          | Smart Game Format parser        |
| Author  : Francis Maes                   |                                 |
| Started : 14/03/2011 17:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_PROBLEM_LOAD_SGF_FILE_FUNCTION_H_
# define LBCPP_SEQUENTIAL_DECISION_PROBLEM_LOAD_SGF_FILE_FUNCTION_H_

# include "GoProblem.h"
# include <lbcpp/Data/Stream.h>

namespace lbcpp
{

class SGFFileParser : public TextParser
{
public:
  SGFFileParser(ExecutionContext& context, const File& file)
    : TextParser(context, file), fileName(file.getFileName()) {}
  SGFFileParser() {}
    
  virtual TypePtr getElementsType() const
    {return xmlElementClass;}

  virtual void parseBegin()
  {
    stack.clear();
    isParsingAttributeValue = false;
    res = new XmlElement(T("games"));
    stack.push_back(res);
  }

  virtual bool parseLine(const String& line)
  {
    for (int i = 0; i < line.length(); ++i)
    {
      // skip white spaces
      if (juce::CharacterFunctions::isWhitespace(line[i]) && 
          (!currentElement || 
            (isParsingAttributeValue && currentAttributeValue.isEmpty()) ||
            (!isParsingAttributeValue && currentAttributeName.isEmpty())))
        continue;
 
      // parse '(', ')', ';'
      if (!isParsingAttributeValue)
      {
        if (line[i] == '(')
        {
          XmlElementPtr node = new XmlElement(T("node"));
          if (stack.size() == 1)
            res->addChildElement(node);
          stack.push_back(node);
          continue;
        }

        if (line[i] == ')')
        {
          if (stack.empty())
          {
            context.errorCallback(fileName, T("Missing parenthesis"));
            return false;
          }
          stack.pop_back();
          continue;
        }

        if (line[i] == ';')
        {
          if (stack.empty())
          {
            context.errorCallback(fileName, T("Empty stack, missing parenthesis"));
            return false;
          }
          XmlElementPtr res = stack.back();
          res->addChildElement(currentElement = new XmlElement(T("node")));
          continue;
        }
      }

      if (!currentElement)
      {
        context.errorCallback(fileName, T("No current element, missing parenthesis or semicolon"));
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
    return true;
  }

  virtual bool parseEnd()
  {
    if (!res)
      return false;
    if (stack.size() != 1)
    {
      context.errorCallback(T("parseEnd ") + fileName, T("Missing closing parenthesis"));
      return false;
    }
    if (res->getNumChildElements() == 1)
      res = res->getChildElement(0); // single game
    if (fileName.isNotEmpty())
      res->setAttribute(T("filename"), fileName);
    setResult(res);
    return true;
  }

private:
  String fileName;
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
    : SimpleUnaryFunction(xmlElementClass, pairClass(goStateClass, objectVectorClass(positiveIntegerPairClass))) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const XmlElementPtr& xml = input.getObjectAndCast<XmlElement>();
    String fileName = xml->getStringAttribute(T("filename"));
    size_t numXmlElements = xml->getNumChildElements();
    if (!numXmlElements)
    {
      context.errorCallback(fileName, T("Empty xml"));
      return Variable::missingValue(outputType);
    }

    XmlElementPtr firstChild = xml->getChildElement(0);
    size_t size = firstChild->getIntAttribute(T("SZ"), 0);
    if (!size)
    {
      context.errorCallback(fileName, T("No size attribute"));
      return Variable::missingValue(outputType);
    }

    size_t firstMoveIndex = (firstChild->hasAttribute(T("B")) || firstChild->hasAttribute(T("W"))) ? 0 : 1;
    if (firstMoveIndex >= numXmlElements)
    {
      context.errorCallback(fileName, T("Empty game"));
      return Variable::missingValue(outputType);
    }

    if (!xml->getChildElement(firstMoveIndex)->hasAttribute(T("B")))
    {
      context.errorCallback(fileName, T("Game does not start with a black move"));
      return Variable::missingValue(outputType);
    }

    //if (numXmlElements > 10)
    //  numXmlElements = 10; // TMP !!! 
    PositiveIntegerPairVectorPtr trajectory = new PositiveIntegerPairVector(numXmlElements - firstMoveIndex);
    bool isBlackTurn = true;
    for (size_t i = firstMoveIndex; i < numXmlElements; ++i)
    {
      String attribute = (isBlackTurn ? T("B") : T("W"));
      XmlElementPtr elt = xml->getChildElement(i);
      if (!elt->hasAttribute(attribute))
      {
        context.errorCallback(fileName, String(T("Could not find ")) + (isBlackTurn ? T("black") : T("white")) + T(" stone at step ") + String((int)i));
        return Variable::missingValue(outputType);
      }

      size_t x, y;

      String value = elt->getStringAttribute(attribute);
      if (value.isEmpty())
        x = y = size;
      else if (value.length() == 2)
      {
        x = letterToIndex(context, value[0]);
        y = letterToIndex(context, value[1]);
        if (x == (size_t)-1 || y == (size_t)-1)
          return Variable::missingValue(outputType);
        if (!(x == size && y == size) && (x >= size || y >= size))
        {
          context.errorCallback(fileName, T("Invalid move: ") + String((int)x) + T(", ") + String((int)y) + T(" (") + value + T(")"));
          return Variable::missingValue(outputType);
        }
      }
      else
      {
        context.errorCallback(fileName, T("Could not parse move: ") + value);
        return Variable::missingValue(outputType);
      }
      

      trajectory->set(i - firstMoveIndex, std::make_pair(x, y));
      isBlackTurn = !isBlackTurn;
    }

    return new Pair(outputType, new GoState(fileName, size), trajectory);
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

// File -> Pair[GoState, Vector[GoAction]]
class LoadSGFFileFunction : public SimpleUnaryFunction
{
public:
  LoadSGFFileFunction() : SimpleUnaryFunction(fileType, pairClass(goStateClass, positiveIntegerPairVectorClass))
  {
    convertFunction = new ConvertSGFXmlToStateAndTrajectory();
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    File file = input.getFile();
    
    XmlElementPtr xml = SGFFileParser(context, file).next().dynamicCast<XmlElement>();
    if (!xml)
      return Variable::missingValue(getOutputType());
    return convertFunction->compute(context, xml); 
  }

  static PairPtr loadStateAndTrajectory(ExecutionContext& context, const File& file)
  {
    XmlElementPtr xml = SGFFileParser(context, file).next().dynamicCast<XmlElement>();
    if (!xml)
      return PairPtr();
    FunctionPtr convertFunction = new ConvertSGFXmlToStateAndTrajectory();
    return convertFunction->compute(context, xml).getObjectAndCast<Pair>();
  }

protected:
  FunctionPtr convertFunction;
};

// File -> Vector[Pair[GoState, Vector[GoAction]]]
class LoadCompositeSGFFileFunction : public SimpleUnaryFunction
{
public:
  LoadCompositeSGFFileFunction(size_t maxCount = 0)
    : SimpleUnaryFunction(fileType, objectVectorClass(pairClass(goStateClass, positiveIntegerPairVectorClass))), maxCount(maxCount)
  {
    convertFunction = new ConvertSGFXmlToStateAndTrajectory();
    convertFunction->initialize(defaultExecutionContext(), xmlElementClass);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    File file = input.getFile();
    
    XmlElementPtr xml = SGFFileParser(context, file).next().dynamicCast<XmlElement>();
    if (!xml)
      return Variable::missingValue(getOutputType());

    size_t n = xml->getNumChildElements();
    ObjectVectorPtr res = new ObjectVector(getOutputType());
    if (maxCount && n > maxCount)
      n = maxCount;
    res->reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
      XmlElementPtr elt = xml->getChildElement(i);
      ObjectPtr stateAndTrajectory = convertFunction->compute(context, elt).getObject();
      if (stateAndTrajectory)
        res->append(stateAndTrajectory);
    }
    return res;
  }

protected:
  size_t maxCount;
  FunctionPtr convertFunction;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_LOAD_SGF_FILE_FUNCTION_H_
