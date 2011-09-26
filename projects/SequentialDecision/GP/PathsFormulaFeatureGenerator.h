/*-----------------------------------------.---------------------------------.
| Filename: PathsFormulaFeatureGenerator.h | Paths Formula Features          |
| Author  : Francis Maes                   |                                 |
| Started : 26/09/2011 16:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_GP_PATHS_FORMULA_FEATURE_GENERATOR_H_
# define LBCPP_SEQUENTIAL_DECISION_GP_PATHS_FORMULA_FEATURE_GENERATOR_H_

# include "GPExpression.h"
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

  // warning: this is not multi-thread safe
class PathsFormulaFeatureGenerator : public FeatureGenerator
{
public:
  PathsFormulaFeatureGenerator() : FeatureGenerator(true), dictionaryReadOnly(false) {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return gpExpressionClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    pathsEnumeration = new DefaultEnumeration(T("Formula Paths"));
    return pathsEnumeration;
  }

  virtual void computeFeatures(const Variable* in, FeatureGeneratorCallback& callback) const
  {
    GPExpressionPtr formula = in[0].getObjectAndCast<GPExpression>();
    formula = new UnaryGPExpression(gpIdentity, formula); // add Root Node

    std::vector<GPExpressionPtr> stack(1, formula);
    size_t index = 0;
    iterateOverSourceNode(callback, formula, stack, index);
  }

  void iterateOverSourceNode(FeatureGeneratorCallback& callback, const GPExpressionPtr& formula, std::vector<GPExpressionPtr>& sourceStack, size_t& sourceIndex) const
  {
    std::vector<GPExpressionPtr> destStack(1);
    destStack[0] = formula;
    size_t destIndex = 0;
    iterateOverDestNode(callback, sourceStack, destStack, 1, sourceIndex, destIndex);

    ++sourceIndex;

    GPExpressionPtr source = sourceStack.back();
    size_t n = source->getNumSubExpressions();
    for (size_t i = 0; i < n; ++i)
    {
      sourceStack.push_back(source->getSubExpression(i));
      iterateOverSourceNode(callback, formula, sourceStack, sourceIndex);
      sourceStack.pop_back();
    }
  }

  void iterateOverDestNode(FeatureGeneratorCallback& callback, const std::vector<GPExpressionPtr>& sourceStack, std::vector<GPExpressionPtr>& destStack, size_t numCommonNodes, size_t sourceIndex, size_t& destIndex) const
  {
    if (sourceIndex <= destIndex)
      senseFeatureFromTo(callback, sourceStack, destStack, numCommonNodes);
    ++destIndex;

    GPExpressionPtr dest = destStack.back();
    size_t n = dest->getNumSubExpressions();
    for (size_t i = 0; i < n; ++i)
    {
      destStack.push_back(dest->getSubExpression(i));
      bool isCommonNode = (destStack.size() <= sourceStack.size() && sourceStack[destStack.size() - 1] == destStack.back());
      iterateOverDestNode(callback, sourceStack, destStack, numCommonNodes + (isCommonNode ? 1 : 0), sourceIndex, destIndex);
      destStack.pop_back();
    }
  }

  void senseFeatureFromTo(FeatureGeneratorCallback& callback,
                          const std::vector<GPExpressionPtr>& sourceStack,
                          const std::vector<GPExpressionPtr>& destStack,
                          size_t numCommonNodes) const
  {
    jassert(numCommonNodes <= sourceStack.size() && numCommonNodes <= destStack.size());
    jassert(sourceStack[numCommonNodes - 1] == destStack[numCommonNodes - 1]);

    static const String rootSep(T("|"));

    std::vector<String> path;
    path.reserve(sourceStack.size() - numCommonNodes + destStack.size() - numCommonNodes + 3);

    for (int i = (int)sourceStack.size() - 1; i >= (int)numCommonNodes; --i)
    {
      path.push_back(nodeSymbol(sourceStack[i]));
      String link = linkSymbol(sourceStack[i-1], sourceStack[i]);
      if (link.isNotEmpty())
        path.push_back(link);
    }
    path.push_back(rootSep);
    path.push_back(nodeSymbol(sourceStack[numCommonNodes - 1]));
    path.push_back(rootSep);
    for (int i = numCommonNodes; i < (int)destStack.size(); ++i)
    {
      String link = linkSymbol(destStack[i - 1], destStack[i]);
      if (link.isNotEmpty())
        path.push_back(link);
      path.push_back(nodeSymbol(destStack[i]));
    }

    senseFeature(callback, path);
  }

  static String nodeSymbol(const GPExpressionPtr& node)
  {
    UnaryGPExpressionPtr unary = node.dynamicCast<UnaryGPExpression>();
    if (unary)
      return gpPreEnumeration->getElementName(unary->getOperator());
    BinaryGPExpressionPtr binary = node.dynamicCast<BinaryGPExpression>();
    if (binary)
      return gpOperatorEnumeration->getElementName(binary->getOperator());
    return node->toShortString();
  }

  static String linkSymbol(const GPExpressionPtr& parentNode, const GPExpressionPtr& childNode)
  {
    BinaryGPExpressionPtr binary = parentNode.dynamicCast<BinaryGPExpression>();
    if (binary && binary->getOperator() != gpAddition && binary->getOperator() != gpMultiplication &&
        binary->getOperator() != gpMin && binary->getOperator() != gpMax)
    {
      if (childNode == binary->getLeft())
        return "L";
      else if (childNode == binary->getRight())
        return "R";
      else
        jassert(false);
    }
    return String::empty;
  }

  void senseFeature(FeatureGeneratorCallback& callback, const std::vector<String>& path) const
  {
    size_t index = const_cast<PathsFormulaFeatureGenerator* >(this)->getOrAddFeatureIndex(path);
    if (index != (size_t)-1)
      callback.sense(index, 1.0);
  }

  size_t getOrAddFeatureIndex(const std::vector<String>& path)
  {
    String str = pathToString(path);
    int index = pathsEnumeration->findElementByName(str);
    if (index >= 0)
      return (size_t)index;
    str = pathToString(path, true);
    index = pathsEnumeration->findElementByName(str);
    if (index >= 0)
      return (size_t)index;
    if (dictionaryReadOnly)
      return (size_t)-1;

    size_t res = pathsEnumeration->getNumElements();
    pathsEnumeration->addElement(defaultExecutionContext(), str);
    return res;
  }

  static String pathToString(const std::vector<String>& path, bool reverseOrder = false)
  {
    jassert(path.size());

    size_t n = 0;
    for (size_t i = 0; i < path.size(); ++i)
      n += path[i].length() + 1;
    String res;
    res.preallocateStorage(n);
    for (size_t i = 0; i < path.size(); ++i)
    {
      res += path[reverseOrder ? (path.size() - 1 - i) : i];
      if (i < path.size() - 1)
        res += T("-");
    }
    return res;
  }

  void setDictionaryReadOnly(bool readOnly)
    {dictionaryReadOnly = readOnly;}

protected:
  DefaultEnumerationPtr pathsEnumeration;
  bool dictionaryReadOnly;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_GP_PATHS_FORMULA_FEATURE_GENERATOR_H_
