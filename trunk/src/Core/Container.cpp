/*-----------------------------------------.---------------------------------.
| Filename: Container.cpp                  | Base class for variable         |
| Author  : Francis Maes                   |    containers                   |
| Started : 28/06/2010 16:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/Container.h>
#include <lbcpp/Data/RandomGenerator.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Core/Vector.h>
#include <lbcpp/Data/SymmetricMatrix.h>
#include <lbcpp/Core/Function.h>
#include <lbcpp/Execution/WorkUnit.h>
#include <algorithm>
using namespace lbcpp;

TypePtr Container::getTemplateParameter(TypePtr type)
{
  TypePtr dvType = type->findBaseTypeFromTemplateName(T("Container"));
  jassert(dvType && dvType->getNumTemplateArguments() == 1);
  TypePtr res = dvType->getTemplateArgument(0);
  jassert(res);
  return res;
}

bool Container::getTemplateParameter(ExecutionContext& context, TypePtr type, TypePtr& res)
{
  TypePtr dvType = type->findBaseTypeFromTemplateName(T("Container"));
  if (!dvType)
  {
    context.errorCallback(type->getName() + T(" is not a Container"));
    return false;
  }
  jassert(dvType->getNumTemplateArguments() == 1);
  res = dvType->getTemplateArgument(0);
  return true;
}

VectorPtr Container::toVector() const
{
  size_t n = getNumElements();
  VectorPtr res = vector(getElementsType(), n);
  for (size_t i = 0; i < n; ++i)
    res->setElement(i, getElement(i));
  return res;
}

String Container::toString() const
{
  String res;
  size_t n = getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    res += getElement(i).toString();
    if (i < n - 1)
      res += T(", ");
  }
  return res;
}

String Container::toShortString() const
{
  size_t n = getNumElements(); 
  if (n == 0)
    return T("<empty>");
  if (n < 10)
  {
    String res;
    for (size_t i = 0; i < n; ++i)
    {
      res += getElement(i).toShortString();
      if (i < n - 1)
        res += T(", ");
    }
    return res;
  }
  else
    return String((int)n) + T(" elements...");
}

void Container::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  Object::clone(context, target);
  ContainerPtr targetContainer = target.staticCast<Container>();
  size_t n = getNumElements();
  for (size_t i = 0; i < n; ++i)
    targetContainer->setElement(i, getElement(i));
}

String Container::getElementName(size_t index) const
  {return getElementsEnumeration()->getElementName(index);}

int Container::findElement(const Variable& value) const
{
  size_t n = getNumElements();
  for (size_t i = 0; i < n; ++i)
    if (getElement(i) == value)
      return (int)i;
  return -1;
}

TypePtr Container::computeElementsCommonBaseType() const
{
  size_t n = getNumElements();
  if (n == 0)
    return topLevelType;
  TypePtr type = getElement(0).getType();
  for (size_t i = 1; i < n; ++i)
  {
    type = Type::findCommonBaseType(type, getElement(i).getType());
    if (type == topLevelType)
      break;
  }
  return type;
}

void Container::saveToXml(XmlExporter& exporter) const
{
  Object::saveToXml(exporter);
  size_t n = getNumElements();
  exporter.setAttribute(T("size"), (int)n);
  //TypePtr elementsType = getElementsType();
  TypePtr actuallyType = computeElementsCommonBaseType();
  for (size_t i = 0; i < n; ++i)
  {
    Variable element = getElement(i);
    if (!element.isMissingValue())
      exporter.saveElement(i, getElement(i), actuallyType);
  }
}

bool Container::loadFromXml(XmlImporter& importer)
{
  if (!Object::loadFromXml(importer))
    return false;

  TypePtr elementsType = getElementsType();

  forEachXmlChildElementWithTagName(*importer.getCurrentElement(), child, T("element"))
  {
    int index = child->getIntAttribute(T("index"), -1);
    if (index < 0)
    {
      importer.errorMessage(T("Container::loadFromXml"), T("Invalid index for element: ") + String(index));
      return false;
    }
    
    Variable value = importer.loadVariable(child, elementsType);
    setElement((size_t)index, value);
  }
  return true;
}

struct OrderContainerFunction
{
  OrderContainerFunction(const ContainerPtr& container, size_t variableIndex, bool increasingOrder)
    : container(container), variableIndex(variableIndex), increasingOrder(increasingOrder) {}

  ContainerPtr container;
  size_t variableIndex;
  bool increasingOrder;
  
  bool operator ()(size_t first, size_t second) const
  {
    Variable a = container->getElement(first).getObject()->getVariable(variableIndex);
    Variable b = container->getElement(second).getObject()->getVariable(variableIndex);
    return increasingOrder ? (a > b) : (a < b);
  }
};

void Container::makeOrder(size_t variableIndex, bool increasingOrder, std::vector<size_t>& res) const
{
  size_t n = getNumElements();
  res.resize(n);
  for (size_t i = 0; i < n; ++i)
    res[i] = i;
  OrderContainerFunction order(refCountedPointerFromThis(this), variableIndex, increasingOrder);
  std::sort(res.begin(), res.end(), order);
}

namespace lbcpp
{
  extern DecoratorContainerPtr rangeContainer(ContainerPtr target, size_t begin, size_t end);
  extern DecoratorContainerPtr excludeRangeContainer(ContainerPtr target, size_t begin, size_t end);
  extern DecoratorContainerPtr duplicatedContainer(ContainerPtr target, size_t count);
  extern DecoratorContainerPtr subsetContainer(ContainerPtr target, const std::vector<size_t>& indices);
  extern DecoratorContainerPtr applyFunctionContainer(ContainerPtr container, FunctionPtr function);
};

class ApplyFunctionInContainerWorkUnit : public WorkUnit
{
public:
  ApplyFunctionInContainerWorkUnit(ContainerPtr source, FunctionPtr function, ContainerPtr target, size_t index)
    : description(T("Applying function")), source(source),
      function(function), target(target), index(index), progressionUnit(function->getRequiredInputType(0, 1)->getName() + T("s")){}

  virtual String toString() const
    {return description;}

protected:
  String description;
  ContainerPtr source;
  FunctionPtr function;
  ContainerPtr target;
  size_t index;
  String progressionUnit;

  virtual Variable run(ExecutionContext& context)
    {target->setElement(index, function->compute(context, source->getElement(index))); return Variable();}
};

ContainerPtr Container::apply(ExecutionContext& context, FunctionPtr function, ApplyComputeMode computeMode, const String& workUnitName) const
{
  if (!function->initialize(context, getElementsType()))
    return ContainerPtr();

  if (computeMode == lazyApply)
    return applyFunctionContainer(refCountedPointerFromThis(this), function);
  else
  {
    size_t n = getNumElements();
    VectorPtr res = vector(function->getOutputType(), n);
    if (computeMode == sequentialApply)
    {
      for (size_t i = 0; i < n; ++i)
        res->setElement(i, function->compute(context, getElement(i)));
    }
    else if (computeMode == parallelApply)
    {
      CompositeWorkUnitPtr workUnits(new CompositeWorkUnit(workUnitName.isEmpty() ? function->toString() : workUnitName, n));
      for (size_t i = 0; i < n; ++i)
        workUnits->setWorkUnit(i, new ApplyFunctionInContainerWorkUnit(refCountedPointerFromThis(this), function, res, i));
      workUnits->setProgressionUnit(function->getOutputType()->getName() + T("s"));
      context.run(workUnits);
    }
    else
      jassert(false);
    return res;
  }
}

ContainerPtr Container::subset(const std::vector<size_t>& indices) const
  {return subsetContainer(refCountedPointerFromThis(this), indices);}

// Creates a randomized version of a dataset.
ContainerPtr Container::randomize() const
{
  std::vector<size_t> indices;
  lbcpp::RandomGenerator::getInstance()->sampleOrder(getNumElements(), indices);
  return subset(indices);
}

// Creates a set where each instance is duplicated multiple times.
ContainerPtr Container::duplicate(size_t count) const
  {return duplicatedContainer(refCountedPointerFromThis(this), count);}

// Selects a range.  
ContainerPtr Container::range(size_t begin, size_t end) const
  {return rangeContainer(refCountedPointerFromThis(this), begin, end);}

// Excludes a range.
ContainerPtr Container::invRange(size_t begin, size_t end) const
  {return excludeRangeContainer(refCountedPointerFromThis(this), begin, end);}

// Selects a fold.
ContainerPtr Container::fold(size_t fold, size_t numFolds) const
{
  jassert(numFolds);
  if (!numFolds)
    return ContainerPtr();
  double meanFoldSize = getNumElements() / (double)numFolds;
  size_t begin = (size_t)(fold * meanFoldSize);
  size_t end = (size_t)((fold + 1) * meanFoldSize);
  return range(begin, end);
}

// Excludes a fold.
ContainerPtr Container::invFold(size_t fold, size_t numFolds) const
{
  jassert(numFolds);
  if (!numFolds)
    return ContainerPtr();
  double meanFoldSize = getNumElements() / (double)numFolds;
  size_t begin = (size_t)(fold * meanFoldSize);
  size_t end = (size_t)((fold + 1) * meanFoldSize);
  return invRange(begin, end);
}
