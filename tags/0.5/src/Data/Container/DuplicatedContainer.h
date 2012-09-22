/*-----------------------------------------.---------------------------------.
| Filename: DuplicatedContainer.h          | Duplicates a Container          |
| Author  : Francis Maes                   |                                 |
| Started : 23/08/2010 13:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_CONTAINER_DUPLICATED_H_
# define LBCPP_DATA_CONTAINER_DUPLICATED_H_

# include <lbcpp/Core/Container.h>

namespace lbcpp
{

class DuplicatedContainer : public DecoratorContainer
{
public:
  DuplicatedContainer(ContainerPtr target, size_t count)
    : DecoratorContainer(target), count(count) {}
  DuplicatedContainer() : count(0) {}
  
  virtual size_t getNumElements() const
    {return count * target->getNumElements();}
    
  virtual Variable getElement(size_t index) const
  {
    jassert(index < target->getNumElements() * count);
    return target->getElement(index % target->getNumElements());
  }

  virtual void setElement(size_t index, const Variable& value)
    {jassert(false);}

private:
  size_t count;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_CONTAINER_DUPLICATED_H_
