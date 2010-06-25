/*-----------------------------------------.---------------------------------.
| Filename: ObjectContainer.cpp            | Object RandomAccess Containers  |
| Author  : Francis Maes                   |                                 |
| Started : 08/06/2009 15:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Object/ObjectContainer.h>
#include <lbcpp/Object/ObjectStream.h>
#include <lbcpp/Object/ObjectGraph.h>
#include <lbcpp/Object/ObjectPair.h>
#include <lbcpp/Utilities/RandomGenerator.h>
using namespace lbcpp;

VectorObjectContainerPtr ObjectContainer::toVector() const
{
  VectorObjectContainerPtr res = new VectorObjectContainer(getContentClassName());
  res->reserve(size());
  for (size_t i = 0; i < size(); ++i)
    res->append(get(i));
  return res;
}

class ObjectContainerStream : public ObjectStream
{
public:
  ObjectContainerStream(ObjectContainerPtr container)
    : container(container), position(0) {}
    
  virtual String getContentClassName() const
    {return container->getContentClassName();}

  virtual ObjectPtr next()
    {return position < container->size() ? container->get(position++) : ObjectPtr();}

private:
  ObjectContainerPtr container;
  size_t position;
};

ObjectStreamPtr ObjectContainer::toStream() const
{
  return new ObjectContainerStream(const_cast<ObjectContainer* >(this));
}

class ObjectContainerSequenceGraph : public ObjectGraph
{
public:
  ObjectContainerSequenceGraph(ObjectContainerPtr container)
  {
    if (container->size() >= 1)
    {
      root = container->get(0);
      for (int i = 0; i < (int)container->size() - 1; ++i)
        successors[container->get(i)] = container->get(i + 1);
    }
  }

  virtual size_t getNumRoots() const
    {return root ? 1 : 0;}

  virtual ObjectPtr getRoot(size_t index) const
    {jassert(index < getNumRoots()); return root;}

  virtual size_t getNumSuccessors(ObjectPtr node) const
    {return successors.find(node) != successors.end() ? 1 : 0;}

  virtual ObjectPtr getSuccessor(ObjectPtr node, size_t index) const
  {
    std::map<ObjectPtr, ObjectPtr>::const_iterator it = successors.find(node);
    return it == successors.end() ? ObjectPtr() : it->second;
  }

private:
  ObjectPtr root;
  std::map<ObjectPtr, ObjectPtr> successors;
};

ObjectGraphPtr ObjectContainer::toGraph() const
  {return new ObjectContainerSequenceGraph(const_cast<ObjectContainer* >(this));}

class ApplyFunctionObjectContainer : public DecoratorObjectContainer
{
public:
  ApplyFunctionObjectContainer(ObjectContainerPtr target, ObjectFunctionPtr function)
    : DecoratorObjectContainer(function->getName() + T("(") + target->getName() + T(")"), target), function(function)
    {}

  ApplyFunctionObjectContainer() {}
    
  virtual String getContentClassName() const
    {return function->getOutputClassName(target->getContentClassName());}

  virtual ObjectPtr get(size_t index) const
    {return function->function(target->get(index));}
  
protected:
  virtual bool load(InputStream& istr)
    {return DecoratorObjectContainer::load(istr) && lbcpp::read(istr, function);}

  virtual void save(OutputStream& ostr) const
    {DecoratorObjectContainer::save(ostr); lbcpp::write(ostr, function);}

private:
  ObjectFunctionPtr function;
};


ObjectContainerPtr ObjectContainer::apply(ObjectFunctionPtr function, bool lazyCompute)
{
  if (lazyCompute)
    return new ApplyFunctionObjectContainer(this, function);
  else
  {
    VectorObjectContainerPtr res = new VectorObjectContainer(function->getOutputClassName(getContentClassName()));
    size_t n = size();
    res->reserve(n);
    for (size_t i = 0; i < n; ++i)
      res->append(function->function(get(i)));
    return res;
  }
}

class RandomizedObjectContainer : public DecoratorObjectContainer
{
public:
  RandomizedObjectContainer(ObjectContainerPtr target)
    : DecoratorObjectContainer(T("randomize(") + target->getName() + T(")"), target)
    {lbcpp::RandomGenerator::getInstance().sampleOrder(target->size(), order);}
   
  RandomizedObjectContainer() {}

  virtual ObjectPtr get(size_t index) const
  {
    jassert(order.size() == target->size() && index < order.size());
    return target->get(order[index]);
  }

protected:
  virtual bool load(InputStream& istr)
    {return DecoratorObjectContainer::load(istr) && lbcpp::read(istr, order);}

  virtual void save(OutputStream& ostr) const
    {DecoratorObjectContainer::save(ostr); lbcpp::write(ostr, order);}  

private:
  std::vector<size_t> order;
};

// Creates a randomized version of a dataset.
ObjectContainerPtr ObjectContainer::randomize()
  {return new RandomizedObjectContainer(this);}

class DuplicatedObjectContainer : public DecoratorObjectContainer
{
public:
  DuplicatedObjectContainer(ObjectContainerPtr target, size_t count)
    : DecoratorObjectContainer(T("duplicate(") + target->getName() + T(", ") + lbcpp::toString(count) + T(")"), target), count(count) {}
  DuplicatedObjectContainer() : count(0) {}
  
  virtual size_t size() const
    {return count * target->size();}
    
  virtual ObjectPtr get(size_t index) const
  {
    jassert(index < target->size() * count);
    return target->get(index % target->size());
  }

protected:
  virtual bool load(InputStream& istr)
    {return DecoratorObjectContainer::load(istr) && lbcpp::read(istr, count);}

  virtual void save(OutputStream& ostr) const
    {DecoratorObjectContainer::save(ostr); lbcpp::write(ostr, count);}  

private:
  size_t count;
};

// Creates a set where each instance is duplicated multiple times.
ObjectContainerPtr ObjectContainer::duplicate(size_t count)
  {return new DuplicatedObjectContainer(this, count);}

class RangeObjectContainer : public DecoratorObjectContainer
{
public:
  RangeObjectContainer(ObjectContainerPtr target, size_t begin, size_t end)
    : DecoratorObjectContainer(target->getName() + T(" [") + lbcpp::toString(begin) + T(", ") + lbcpp::toString(end) + T("["), target),
    begin(begin), end(end) {jassert(end >= begin);}
  RangeObjectContainer() : begin(0), end(0) {}

  virtual size_t size() const
    {return end - begin;}
    
  virtual ObjectPtr get(size_t index) const
  {
    index += begin;
    jassert(index < end);
    return target->get(index);
  }

protected:
  virtual bool load(InputStream& istr)
    {return DecoratorObjectContainer::load(istr) && lbcpp::read(istr, begin) && lbcpp::read(istr, end);}

  virtual void save(OutputStream& ostr) const
    {DecoratorObjectContainer::save(ostr); lbcpp::write(ostr, begin); lbcpp::write(ostr, end);}  

private:
  size_t begin, end;
};

// Selects a range.
ObjectContainerPtr ObjectContainer::range(size_t begin, size_t end)
  {return new RangeObjectContainer(this, begin, end);}

class ExcludeRangeObjectContainer : public DecoratorObjectContainer
{
public:
  ExcludeRangeObjectContainer(ObjectContainerPtr target, size_t begin, size_t end)
    : DecoratorObjectContainer(target->getName() + T(" /[") + lbcpp::toString(begin) + T(", ") + lbcpp::toString(end) + T("["), target),
    begin(begin), end(end) {jassert(end >= begin);}
  ExcludeRangeObjectContainer() : begin(0), end(0) {}

  virtual size_t size() const
    {return target->size() - (end - begin);}
    
  virtual ObjectPtr get(size_t index) const
  {
    jassert(index < size());
    if (index < begin)
      return target->get(index);
    else
      return target->get(index + (end - begin));
  }

protected:
  virtual bool load(InputStream& istr)
    {return DecoratorObjectContainer::load(istr) && lbcpp::read(istr, begin) && lbcpp::read(istr, end);}

  virtual void save(OutputStream& ostr) const
    {DecoratorObjectContainer::save(ostr); lbcpp::write(ostr, begin); lbcpp::write(ostr, end);}  
  
private:
  size_t begin, end;
};

// Excludes a range.
ObjectContainerPtr ObjectContainer::invRange(size_t begin, size_t end)
  {return new ExcludeRangeObjectContainer(this, begin, end);}

// Selects a fold.
ObjectContainerPtr ObjectContainer::fold(size_t fold, size_t numFolds)
{
  jassert(numFolds);
  if (!numFolds)
    return ObjectContainerPtr();
  double meanFoldSize = size() / (double)numFolds;
  size_t begin = (size_t)(fold * meanFoldSize);
  size_t end = (size_t)((fold + 1) * meanFoldSize);
  return range(begin, end);
}

// Excludes a fold.
ObjectContainerPtr ObjectContainer::invFold(size_t fold, size_t numFolds)
{
  jassert(numFolds);
  if (!numFolds)
    return ObjectContainerPtr();
  double meanFoldSize = size() / (double)numFolds;
  size_t begin = (size_t)(fold * meanFoldSize);
  size_t end = (size_t)((fold + 1) * meanFoldSize);
  return invRange(begin, end);
}

class BinaryAppendObjectContainer : public ObjectContainer
{
public:
  BinaryAppendObjectContainer(ObjectContainerPtr left, ObjectContainerPtr right)
    : ObjectContainer(left->getName() + T(" U ") + right->getName()), left(left), right(right) {}
    
  virtual String getContentClassName() const
    {jassert(left->getContentClassName() == right->getContentClassName()); return left->getContentClassName();}
  
  virtual size_t size() const
    {return left->size() + right->size();}
    
  virtual ObjectPtr get(size_t index) const
    {return index < left->size() ? left->get(index) : right->get(index - left->size());}

private:
  ObjectContainerPtr left;
  ObjectContainerPtr right;
};

// Append two object containers.
ObjectContainerPtr lbcpp::append(ObjectContainerPtr left, ObjectContainerPtr right)
  {return new BinaryAppendObjectContainer(left, right);}

void declareObjectContainerClasses()
{
  LBCPP_DECLARE_CLASS_LEGACY(VectorObjectContainer);
  LBCPP_DECLARE_CLASS_LEGACY(ApplyFunctionObjectContainer);
  LBCPP_DECLARE_CLASS_LEGACY(RangeObjectContainer);
  LBCPP_DECLARE_CLASS_LEGACY(ExcludeRangeObjectContainer);
  LBCPP_DECLARE_CLASS_LEGACY(DuplicatedObjectContainer);
  LBCPP_DECLARE_CLASS_LEGACY(RandomizedObjectContainer);
  LBCPP_DECLARE_CLASS_LEGACY(ObjectPair);
}
