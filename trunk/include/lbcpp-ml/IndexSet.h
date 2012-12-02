/*-----------------------------------------.---------------------------------.
| Filename: IndexSet.h                     | Index Set                       |
| Author  : Francis Maes                   |                                 |
| Started : 17/12/2011 13:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_INDEX_SET_H_
# define LBCPP_DATA_INDEX_SET_H_

# include "../Core/RandomGenerator.h"

namespace lbcpp
{

class IndexSet;
typedef ReferenceCountedObjectPtr<IndexSet> IndexSetPtr;

class IndexSet : public Object
{
public:
  IndexSet(size_t begin, size_t end);
  IndexSet(size_t size);
  IndexSet() {}
  
  typedef std::vector<size_t>::iterator iterator;
  typedef std::vector<size_t>::const_iterator const_iterator;

  const_iterator begin() const
    {return v.begin();}

  const_iterator end() const
    {return v.end();}

  size_t size() const
    {return v.size();}

  bool empty() const
    {return v.empty();}

  size_t front() const
    {return v.front();}

  size_t back() const
    {return v.back();}

  void reserve(size_t size)
    {v.reserve(size);}

  void append(size_t index)
    {jassert(v.empty() || index >= v.back()); v.push_back(index);}

  void addInterval(size_t begin, size_t end);

  // Object
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
    {target.staticCast<IndexSet>()->v = v;}

  const std::vector<size_t>& getIndices() const
    {return v;}

  IndexSetPtr sampleBootStrap(RandomGeneratorPtr random) const;
  IndexSetPtr sampleSubset(RandomGeneratorPtr random, size_t subsetSize) const;

protected:
  friend class IndexSetClass;

  std::vector<size_t> v;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_INDEX_SET_H_
