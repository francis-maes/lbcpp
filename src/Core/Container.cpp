/*-----------------------------------------.---------------------------------.
| Filename: Container.cpp                  | Base class for variable         |
| Author  : Francis Maes                   |    containers                   |
| Started : 28/06/2010 16:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Core/Container.h>
#include <lbcpp/Data/RandomGenerator.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Core/Vector.h>
#include <lbcpp/Data/SymmetricMatrix.h>
#include <lbcpp/Execution/WorkUnit.h>
using namespace lbcpp;

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
  String res = T("[");
  size_t n = getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    res += getElement(i).toShortString();
    if (i < n - 1)
      res += T(",\n  ");
  }
  return res + T("]");
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
  {return T("[") + String((int)index) + T("]");}

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
  TypePtr elementsType = getElementsType();
  for (size_t i = 0; i < n; ++i)
  {
    Variable element = getElement(i);
    if (!element.isMissingValue())
      exporter.saveElement(i, getElement(i), elementsType);
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
    : description(function->getDescription(source->getElement(index))), source(source),
      function(function), target(target), index(index), progressionUnit(function->getInputType()->getName() + T("s")){}

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
    {target->setElement(index, function->computeFunction(context, source->getElement(index))); return Variable();}
};

ContainerPtr Container::apply(ExecutionContext& context, FunctionPtr function, ApplyComputeMode computeMode, const String& workUnitName) const
{
  if (computeMode == lazyApply)
    return applyFunctionContainer(refCountedPointerFromThis(this), function);
  else
  {
    size_t n = getNumElements();
    VectorPtr res = vector(function->getOutputType(getElementsType()), n);
    if (computeMode == sequentialApply)
    {
      for (size_t i = 0; i < n; ++i)
        res->setElement(i, function->computeFunction(context, getElement(i)));
    }
    else if (computeMode == parallelApply)
    {
      CompositeWorkUnitPtr workUnits(new CompositeWorkUnit(workUnitName.isEmpty() ? function->toString() : workUnitName, n));
      for (size_t i = 0; i < n; ++i)
        workUnits->setWorkUnit(i, new ApplyFunctionInContainerWorkUnit(refCountedPointerFromThis(this), function, res, i));
      workUnits->setProgressionUnit(function->getInputType()->getName() + T("s"));
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
