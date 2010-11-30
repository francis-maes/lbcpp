/*-----------------------------------------.---------------------------------.
| Filename: SegmentedContainer.h           | SegmentedContainer              |
| Author  : Francis Maes                   |                                 |
| Started : 17/10/2010 21:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_CONTAINER_SEGMENTED_H_
# define LBCPP_DATA_CONTAINER_SEGMENTED_H_

# include <lbcpp/Core/Container.h>

namespace lbcpp
{

class SegmentedContainer : public Container
{
public:
  SegmentedContainer(ContainerPtr container)
    : sourceContainer(container)
  {
    size_t n = container->getNumElements();
    size_t startIndex = 0;
    TypePtr elementsType = container->getElementsType();
    while (startIndex < n)
    {
      Variable value = container->getElement(startIndex);
      size_t endIndex;
      for (endIndex = startIndex + 1; endIndex < n; ++endIndex)
        if (container->getElement(endIndex) != value)
          break;
      segments.push_back(new ContainerSegment(elementsType, startIndex, endIndex - startIndex, value));
      startIndex = endIndex;
    }
  }

  SegmentedContainer() {}

  virtual TypePtr getElementsType() const
    {return containerSegmentClass(sourceContainer->getElementsType());}

  virtual size_t getNumElements() const
    {return segments.size();}

  virtual Variable getElement(size_t index) const
    {return segments[index];}

  virtual void setElement(size_t index, const Variable& value)
    {jassert(false);}

private:
  friend class SegmentedContainerClass;

  ContainerPtr sourceContainer;
  std::vector<ContainerSegmentPtr> segments;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_CONTAINER_SEGMENTED_H_
