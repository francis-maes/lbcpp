/*-----------------------------------------.---------------------------------.
| Filename: Container.cpp                  | Base class for variable         |
| Author  : Francis Maes                   |    containers                   |
| Started : 28/06/2010 16:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Data/Container.h>
#include <lbcpp/Data/Vector.h>
#include <lbcpp/Data/SymmetricMatrix.h>
#include <lbcpp/Data/Function.h>
using namespace lbcpp;

VectorPtr Container::toVector() const
{
  size_t n = getNumVariables();
  VectorPtr res = new Vector(getElementsType(), n);
  for (size_t i = 0; i < n; ++i)
    res->setVariable(i, getVariable(i));
  return res;
}

String Container::toString() const
  {return T("[") + variablesToString(T(",\n  ")) + T("]");}

int Container::find(const Variable& value) const
{
  size_t n = getNumVariables();
  for (size_t i = 0; i < n; ++i)
    if (getVariable(i) == value)
      return (int)i;
  return -1;
}

class ApplyFunctionContainer : public DecoratorContainer
{
public:
  ApplyFunctionContainer(ContainerPtr target, FunctionPtr function)
    : DecoratorContainer(target), function(function)
    {checkInheritance(target->getElementsType(), function->getInputType());}

  ApplyFunctionContainer() {}
    
  virtual TypePtr getElementsType() const
    {return function->getOutputType(target->getElementsType());}

  virtual Variable getVariable(size_t index) const
    {return function->compute(target->getVariable(index));}

  virtual void setVariable(size_t index, const Variable& value) const
    {jassert(false);}

private:
  FunctionPtr function;
};

ContainerPtr Container::apply(FunctionPtr function, bool lazyCompute) const
{
  if (lazyCompute)
    return new ApplyFunctionContainer(refCountedPointerFromThis(this), function);
  else
  {
    size_t n = size();
    VectorPtr res = new Vector(function->getOutputType(getElementsType()), n);
    for (size_t i = 0; i < n; ++i)
      res->setVariable(i, function->compute(getVariable(i)));
    return res;
  }
}

class SubsetContainer : public DecoratorContainer
{
public:
  SubsetContainer(ContainerPtr target, const std::vector<size_t>& indices = std::vector<size_t>())
    : DecoratorContainer(target), indices(indices) {}
   
  SubsetContainer() {}

  virtual size_t getNumVariables() const
    {return indices.size();}

  virtual Variable getVariable(size_t index) const
    {jassert(index < indices.size()); return target->getVariable(indices[index]);}

private:
  std::vector<size_t> indices;
};

ContainerPtr Container::subset(const std::vector<size_t>& indices) const
  {return new SubsetContainer(refCountedPointerFromThis(this), indices);}

// Creates a randomized version of a dataset.
ContainerPtr Container::randomize() const
{
  std::vector<size_t> indices;
  lbcpp::RandomGenerator::getInstance().sampleOrder(size(), indices);
  return subset(indices);
}

class DuplicatedContainer : public DecoratorContainer
{
public:
  DuplicatedContainer(ContainerPtr target, size_t count)
    : DecoratorContainer(target), count(count) {}
  DuplicatedContainer() : count(0) {}
  
  virtual size_t getNumVariables() const
    {return count * target->getNumVariables();}
    
  virtual Variable getVariable(size_t index) const
  {
    jassert(index < target->size() * count);
    return target->getVariable(index % target->size());
  }

private:
  size_t count;
};

// Creates a set where each instance is duplicated multiple times.
ContainerPtr Container::duplicate(size_t count) const
  {return new DuplicatedContainer(refCountedPointerFromThis(this), count);}

class RangeContainer : public DecoratorContainer
{
public:
  RangeContainer(ContainerPtr target, size_t begin, size_t end)
    : DecoratorContainer(target),
    begin(begin), end(end) {jassert(end >= begin);}
  RangeContainer() : begin(0), end(0) {}

  virtual size_t getNumVariables() const
    {return end - begin;}
    
  virtual Variable getVariable(size_t index) const
  {
    index += begin;
    jassert(index < end);
    return target->getVariable(index);
  }

private:
  size_t begin, end;
};

// Selects a range.
ContainerPtr Container::range(size_t begin, size_t end) const
  {return new RangeContainer(refCountedPointerFromThis(this), begin, end);}

class ExcludeRangeContainer : public DecoratorContainer
{
public:
  ExcludeRangeContainer(ContainerPtr target, size_t begin, size_t end)
    : DecoratorContainer(target),
    begin(begin), end(end) {jassert(end >= begin);}
  ExcludeRangeContainer() : begin(0), end(0) {}

  virtual size_t getNumVariables() const
    {return target->size() - (end - begin);}
    
  virtual Variable getVariable(size_t index) const
  {
    jassert(index < size());
    if (index < begin)
      return target->getVariable(index);
    else
      return target->getVariable(index + (end - begin));
  }

private:
  size_t begin, end;
};

// Excludes a range.
ContainerPtr Container::invRange(size_t begin, size_t end) const
  {return new ExcludeRangeContainer(refCountedPointerFromThis(this), begin, end);}

// Selects a fold.
ContainerPtr Container::fold(size_t fold, size_t numFolds) const
{
  jassert(numFolds);
  if (!numFolds)
    return ContainerPtr();
  double meanFoldSize = size() / (double)numFolds;
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
  double meanFoldSize = size() / (double)numFolds;
  size_t begin = (size_t)(fold * meanFoldSize);
  size_t end = (size_t)((fold + 1) * meanFoldSize);
  return invRange(begin, end);
}

ClassPtr lbcpp::containerClass()
  {static TypeCache cache(T("Container")); return cache();}

class ContainerClass : public DefaultClass
{
public:
  ContainerClass() : DefaultClass(T("Container"), T("Object")) {}
  
  virtual Variable getSubVariable(const VariableValue& value, size_t index) const
    {return value.getObject()->getVariable(index);}

  virtual void setSubVariable(const VariableValue& value, size_t index, const Variable& subValue) const
    {value.getObject()->setVariable(index, subValue);}
};

void declareContainerClasses()
{
  Type::declare(new ContainerClass());

    LBCPP_DECLARE_TEMPLATE_CLASS(Vector, 1, Container);
    LBCPP_DECLARE_CLASS(BooleanVector, Container);
    LBCPP_DECLARE_CLASS(DynamicObject, Container);
    LBCPP_DECLARE_TEMPLATE_CLASS(SymmetricMatrix, 1, Container);
      LBCPP_DECLARE_TEMPLATE_CLASS(SymmetricMatrixRow, 1, Container);

    LBCPP_DECLARE_ABSTRACT_CLASS(DecoratorContainer, Container);

      LBCPP_DECLARE_CLASS(ApplyFunctionContainer, DecoratorContainer);
      LBCPP_DECLARE_CLASS(RangeContainer, DecoratorContainer);
      LBCPP_DECLARE_CLASS(ExcludeRangeContainer, DecoratorContainer);
      LBCPP_DECLARE_CLASS(DuplicatedContainer, DecoratorContainer);
      LBCPP_DECLARE_CLASS(SubsetContainer, DecoratorContainer);
}
