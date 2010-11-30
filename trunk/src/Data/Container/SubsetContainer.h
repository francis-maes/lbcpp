/*-----------------------------------------.---------------------------------.
| Filename: SubsetContainer.h              | Selects a subset of elements    |
| Author  : Francis Maes                   |                                 |
| Started : 23/08/2010 13:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_CONTAINER_SUBSET_H_
# define LBCPP_DATA_CONTAINER_SUBSET_H_

# include <lbcpp/Core/Container.h>

namespace lbcpp
{

class SubsetContainer : public DecoratorContainer
{
public:
  SubsetContainer(ContainerPtr target, const std::vector<size_t>& indices = std::vector<size_t>())
    : DecoratorContainer(target), indices(indices) {}
   
  SubsetContainer() {}

  virtual size_t getNumElements() const
    {return indices.size();}

  virtual Variable getElement(size_t index) const
    {jassert(index < indices.size()); return target->getElement(indices[index]);}

  virtual void setElement(size_t index, const Variable& value)
    {jassert(false);}

private:
  std::vector<size_t> indices;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_CONTAINER_SUBSET_H_
