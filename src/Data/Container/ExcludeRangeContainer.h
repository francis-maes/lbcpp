/*-----------------------------------------.---------------------------------.
| Filename: ExcludeRangeContainer.h        | Excludes a range of elements    |
| Author  : Francis Maes                   |                                 |
| Started : 23/08/2010 13:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_CONTAINER_EXCLUDE_RANGE_H_
# define LBCPP_DATA_CONTAINER_EXCLUDE_RANGE_H_

# include <lbcpp/Data/Container.h>

namespace lbcpp
{

class ExcludeRangeContainer : public DecoratorContainer
{
public:
  ExcludeRangeContainer(ContainerPtr target, size_t begin, size_t end)
    : DecoratorContainer(target),
    begin(begin), end(end) {jassert(end >= begin);}
  ExcludeRangeContainer() : begin(0), end(0) {}

  virtual size_t getNumElements() const
    {return target->getNumElements() - (end - begin);}
    
  virtual Variable getElement(size_t index) const
  {
    jassert(index < getNumElements());
    if (index < begin)
      return target->getElement(index);
    else
      return target->getElement(index + (end - begin));
  }

  virtual void setElement(size_t index, const Variable& value)
    {jassert(false);}

private:
  size_t begin, end;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_CONTAINER_EXCLUDE_RANGE_H_
