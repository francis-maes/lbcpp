/*-----------------------------------------.---------------------------------.
| Filename: RangeContainer.h               | Selects a range of elements     |
| Author  : Francis Maes                   |                                 |
| Started : 23/08/2010 13:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_CONTAINER_RANGE_H_
# define LBCPP_DATA_CONTAINER_RANGE_H_

# include <lbcpp/Data/Container.h>

namespace lbcpp
{

class RangeContainer : public DecoratorContainer
{
public:
  RangeContainer(ContainerPtr target, size_t begin, size_t end)
    : DecoratorContainer(target),
    begin(begin), end(end) {jassert(end >= begin);}
  RangeContainer() : begin(0), end(0) {}

  virtual size_t getNumElements() const
    {return end - begin;}
    
  virtual Variable getElement(size_t index) const
  {
    index += begin;
    jassert(index < end);
    return target->getElement(index);
  }

  virtual void setElement(size_t index, const Variable& value)
    {jassert(false);}

private:
  size_t begin, end;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_CONTAINER_RANGE_H_
