/*-----------------------------------------.---------------------------------.
| Filename: VariableContainer.cpp          | Base class for variable         |
| Author  : Francis Maes                   |    containers                   |
| Started : 28/06/2010 16:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Object/VariableContainer.h>
#include <lbcpp/Object/Vector.h>
#include <lbcpp/Object/SymmetricMatrix.h>
using namespace lbcpp;

VectorPtr VariableContainer::toVector() const
{
  size_t n = size();
  VectorPtr res = new Vector(topLevelClass(), n);
  for (size_t i = 0; i < n; ++i)
    res->setVariable(i, getVariable(i));
  return res;
}

class RandomizedVariableContainer : public DecoratorVariableContainer
{
public:
  RandomizedVariableContainer(VariableContainerPtr target)
    : DecoratorVariableContainer(target)
    {lbcpp::RandomGenerator::getInstance().sampleOrder(target->size(), order);}
   
  RandomizedVariableContainer() {}

  virtual Variable getVariable(size_t index) const
  {
    jassert(order.size() == target->size() && index < order.size());
    return target->getVariable(order[index]);
  }

protected:
  virtual bool load(InputStream& istr)
    {return DecoratorVariableContainer::load(istr) && lbcpp::read(istr, order);}

  virtual void save(OutputStream& ostr) const
    {DecoratorVariableContainer::save(ostr); lbcpp::write(ostr, order);}  

private:
  std::vector<size_t> order;
};

// Creates a randomized version of a dataset.
VariableContainerPtr VariableContainer::randomize() const
  {return new RandomizedVariableContainer(const_cast<VariableContainer* >(this));}

class DuplicatedVariableContainer : public DecoratorVariableContainer
{
public:
  DuplicatedVariableContainer(VariableContainerPtr target, size_t count)
    : DecoratorVariableContainer(target), count(count) {}
  DuplicatedVariableContainer() : count(0) {}
  
  virtual size_t getNumVariables() const
    {return count * target->getNumVariables();}
    
  virtual Variable getVariable(size_t index) const
  {
    jassert(index < target->size() * count);
    return target->getVariable(index % target->size());
  }

protected:
  virtual bool load(InputStream& istr)
    {return DecoratorVariableContainer::load(istr) && lbcpp::read(istr, count);}

  virtual void save(OutputStream& ostr) const
    {DecoratorVariableContainer::save(ostr); lbcpp::write(ostr, count);}  

private:
  size_t count;
};

// Creates a set where each instance is duplicated multiple times.
VariableContainerPtr VariableContainer::duplicate(size_t count) const
  {return new DuplicatedVariableContainer(const_cast<VariableContainer* >(this), count);}

class RangeVariableContainer : public DecoratorVariableContainer
{
public:
  RangeVariableContainer(VariableContainerPtr target, size_t begin, size_t end)
    : DecoratorVariableContainer(target),
    begin(begin), end(end) {jassert(end >= begin);}
  RangeVariableContainer() : begin(0), end(0) {}

  virtual size_t getNumVariables() const
    {return end - begin;}
    
  virtual Variable getVariable(size_t index) const
  {
    index += begin;
    jassert(index < end);
    return target->getVariable(index);
  }

protected:
  virtual bool load(InputStream& istr)
    {return DecoratorVariableContainer::load(istr) && lbcpp::read(istr, begin) && lbcpp::read(istr, end);}

  virtual void save(OutputStream& ostr) const
    {DecoratorVariableContainer::save(ostr); lbcpp::write(ostr, begin); lbcpp::write(ostr, end);}  

private:
  size_t begin, end;
};

// Selects a range.
VariableContainerPtr VariableContainer::range(size_t begin, size_t end) const
  {return new RangeVariableContainer(const_cast<VariableContainer* >(this), begin, end);}

class ExcludeRangeVariableContainer : public DecoratorVariableContainer
{
public:
  ExcludeRangeVariableContainer(VariableContainerPtr target, size_t begin, size_t end)
    : DecoratorVariableContainer(target),
    begin(begin), end(end) {jassert(end >= begin);}
  ExcludeRangeVariableContainer() : begin(0), end(0) {}

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

protected:
  virtual bool load(InputStream& istr)
    {return DecoratorVariableContainer::load(istr) && lbcpp::read(istr, begin) && lbcpp::read(istr, end);}

  virtual void save(OutputStream& ostr) const
    {DecoratorVariableContainer::save(ostr); lbcpp::write(ostr, begin); lbcpp::write(ostr, end);}  
  
private:
  size_t begin, end;
};

// Excludes a range.
VariableContainerPtr VariableContainer::invRange(size_t begin, size_t end) const
  {return new ExcludeRangeVariableContainer(const_cast<VariableContainer* >(this), begin, end);}

// Selects a fold.
VariableContainerPtr VariableContainer::fold(size_t fold, size_t numFolds) const
{
  jassert(numFolds);
  if (!numFolds)
    return VariableContainerPtr();
  double meanFoldSize = size() / (double)numFolds;
  size_t begin = (size_t)(fold * meanFoldSize);
  size_t end = (size_t)((fold + 1) * meanFoldSize);
  return range(begin, end);
}

// Excludes a fold.
VariableContainerPtr VariableContainer::invFold(size_t fold, size_t numFolds) const
{
  jassert(numFolds);
  if (!numFolds)
    return VariableContainerPtr();
  double meanFoldSize = size() / (double)numFolds;
  size_t begin = (size_t)(fold * meanFoldSize);
  size_t end = (size_t)((fold + 1) * meanFoldSize);
  return invRange(begin, end);
}

void declareVariableContainerClasses()
{
  LBCPP_DECLARE_ABSTRACT_CLASS(VariableContainer, Object);

    LBCPP_DECLARE_CLASS(Vector, VariableContainer);
    LBCPP_DECLARE_CLASS(DynamicTypeVector, VariableContainer);
    LBCPP_DECLARE_CLASS(SymmetricMatrix, VariableContainer);

    LBCPP_DECLARE_ABSTRACT_CLASS(DecoratorVariableContainer, VariableContainer);

      LBCPP_DECLARE_CLASS(RangeVariableContainer, DecoratorVariableContainer);
      LBCPP_DECLARE_CLASS(ExcludeRangeVariableContainer, DecoratorVariableContainer);
      LBCPP_DECLARE_CLASS(DuplicatedVariableContainer, DecoratorVariableContainer);
      LBCPP_DECLARE_CLASS(RandomizedVariableContainer, DecoratorVariableContainer);
}
