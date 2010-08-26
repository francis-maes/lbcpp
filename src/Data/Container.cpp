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
    return variableType();
  TypePtr type = getElement(0).getType();
  for (size_t i = 1; i < n; ++i)
  {
    type = Type::findCommonBaseType(type, getElement(i).getType());
    if (type == topLevelType())
      break;
  }
  return type;
}

void Container::saveToXml(XmlExporter& exporter) const
{
  Object::saveToXml(exporter);
  size_t n = getNumElements();
  exporter.setAttribute(T("size"), (int)n);
  for (size_t i = 0; i < n; ++i)
  {
    Variable element = getElement(i);
    if (!element.isMissingValue())
      exporter.saveElement(i, getElement(i));
  }
}

bool Container::loadFromXml(XmlElement* xml, MessageCallback& callback)
{
  if (!Object::loadFromXml(xml, callback))
    return false;

  for (XmlElement* child = xml->getFirstChildElement(); child; child = child->getNextElement())
    if (child->getTagName() == T("element"))
    {
      int index = child->getIntAttribute(T("index"), -1);
      if (index < 0)
      {
        callback.errorMessage(T("Container::loadFromXml"), T("Invalid index for element: ") + String(index));
        return false;
      }
      Variable value = Variable::createFromXml(child, callback);
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
