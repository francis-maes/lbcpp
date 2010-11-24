/*-----------------------------------------.---------------------------------.
| Filename: Container.cpp                  | Base class for variable         |
| Author  : Francis Maes                   |    containers                   |
| Started : 28/06/2010 16:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Data/Container.h>
#include <lbcpp/Data/RandomGenerator.h>
#include <lbcpp/Data/XmlSerialisation.h>
#include <lbcpp/Data/Vector.h>
#include <lbcpp/Data/SymmetricMatrix.h>
#include <lbcpp/Execution/ThreadPool.h>
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

void Container::clone(const ObjectPtr& target) const
{
  Object::clone(target);
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

ContainerPtr Container::apply(FunctionPtr function, bool lazyCompute) const
{
  if (lazyCompute)
    return applyFunctionContainer(refCountedPointerFromThis(this), function);
  else
  {
    size_t n = getNumElements();
    VectorPtr res = vector(function->getOutputType(getElementsType()), n);
    for (size_t i = 0; i < n; ++i)
      res->setElement(i, function->compute(getElement(i)));
    return res;
  }
}

class ApplyFunctionInContainerWorkUnit : public WorkUnit
{
public:
  ApplyFunctionInContainerWorkUnit(ContainerPtr source, FunctionPtr function, ContainerPtr target, size_t index)
    : WorkUnit(T("apply ") + function->toString() + T(" ") + String((int)index)), source(source), function(function), target(target), index(index) {}

protected:
  ContainerPtr source;
  FunctionPtr function;
  ContainerPtr target;
  size_t index;

  virtual bool run(ExecutionContext& context)
    {target->setElement(index, function->compute(source->getElement(index))); return true;}
};

ContainerPtr Container::apply(FunctionPtr function, ThreadPoolPtr pool) const
{
  size_t n = getNumElements();
  VectorPtr res = vector(function->getOutputType(getElementsType()), n);
  std::vector<WorkUnitPtr> workUnits(n);
  for (size_t i = 0; i < n; ++i)
    workUnits[i] = new ApplyFunctionInContainerWorkUnit(refCountedPointerFromThis(this), function, res, i);
  pool->addWorkUnitsAndWaitExecution(workUnits, 10);
  return res;
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
