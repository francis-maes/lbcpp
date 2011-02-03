/*-----------------------------------------.---------------------------------.
| Filename: SegmentContainerOperator.h     | Segment Container Operator      |
| Author  : Francis Maes                   |                                 |
| Started : 01/02/2011 13:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_OPERATOR_SEGMENT_CONTAINER_H_
# define LBCPP_FUNCTION_OPERATOR_SEGMENT_CONTAINER_H_

# include <lbcpp/Operator/Operator.h>
# include <lbcpp/Core/Vector.h>
# include <lbcpp/Core/DynamicObject.h>

namespace lbcpp
{

extern ClassPtr segmentClass(TypePtr elementType);
extern ClassPtr segmentContainerClass(TypePtr elementsType);

class SegmentContainer;
typedef ReferenceCountedObjectPtr<SegmentContainer> SegmentContainerPtr;

/*
** Segment
*/
namespace impl
{
  struct Segment
  {
    Segment(size_t beginIndex = 0, size_t endIndex = 0, const Variable& value = Variable())
      : beginIndex(beginIndex), endIndex(endIndex), value(value) {}

    size_t beginIndex;
    size_t endIndex;
    Variable value;

    TypePtr getElementType() const
      {return value.getType();}
  };
};

class Segment : public Object
{
public:
  Segment(const impl::Segment& segment, SegmentContainerPtr owner)
    : Object(segmentClass(segment.getElementType())), segment(segment), owner(owner) {}
  Segment() {}

  TypePtr getElementType() const
    {return segment.getElementType();}

  size_t getBeginIndex() const
    {return segment.beginIndex;}

  size_t getEndIndex() const
    {return segment.endIndex;}

  size_t getLength() const
    {return segment.endIndex - segment.beginIndex;}

  const Variable& getValue() const
    {return segment.value;}

  const SegmentContainerPtr& getOwner() const
    {return owner;}

  virtual String toShortString() const
    {return T("[") + String((int)segment.beginIndex) + T(", ") + String((int)segment.endIndex) + T("[ ") + segment.value.toShortString();}

private:
  friend class SegmentClass;

  impl::Segment segment;
  SegmentContainerPtr owner;
};

/*
** SegmentContainer
*/
class SegmentContainer : public Container
{
public:
  SegmentContainer(TypePtr elementsType, size_t numElements)
    : Container(segmentContainerClass(elementsType)), elementToSegmentIndex(numElements, (size_t)-1) {}
  SegmentContainer(ClassPtr thisClass)
    : Container(thisClass) {}
  SegmentContainer() {}

  size_t getInputNumElements() const
    {return elementToSegmentIndex.size();}

  void setInputNumElements(size_t numElements)
    {elementToSegmentIndex.resize(numElements, (size_t)-1);}

  size_t getNumSegments() const
    {return segments.size();}

  const impl::Segment& getSegment(size_t index) const
    {jassert(index < segments.size()); return segments[index];}

  virtual size_t getNumElements() const
    {return segments.size();}

  virtual Variable getElement(size_t index) const
    {return new Segment(segments[index], refCountedPointerFromThis(this));}

  virtual void setElement(size_t index, const Variable& value)
    {jassert(false);}

  void appendSegment(size_t beginIndex, size_t endIndex, const Variable& value)
  {
    jassert(beginIndex < endIndex && endIndex <= elementToSegmentIndex.size());
    for (size_t i = beginIndex; i < endIndex; ++i)
      elementToSegmentIndex[i] = segments.size();
    segments.push_back(impl::Segment(beginIndex, endIndex, value));
  }

private:
  std::vector<size_t> elementToSegmentIndex;
  std::vector<impl::Segment> segments;
};

/*
** SegmentContainerOperator
*/
// Container[T] => SegmentContainer[T]
class SegmentContainerOperator : public Function
{
public:
  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    if (!checkNumInputs(context, 1))
      return VariableSignaturePtr();
    VariableSignaturePtr inputVariable = getInputVariable(0);
    TypePtr elementsType;
    if (!getContainerElementsType(context, inputVariable->getType(), elementsType))
      return VariableSignaturePtr();
    return new VariableSignature(vectorClass(segmentContainerClass(elementsType)), inputVariable->getName() + T("Segmented"), inputVariable->getShortName() + T("s"));
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const ContainerPtr& container = inputs[0].getObjectAndCast<Container>();
    size_t n = container->getNumElements();

    SegmentContainerPtr res(new SegmentContainer(getOutputType()));
    res->setInputNumElements(n);
    size_t beginIndex = 0;
    while (beginIndex < n)
    {
      Variable value = container->getElement(beginIndex);
      size_t endIndex;
      for (endIndex = beginIndex + 1; endIndex < n; ++endIndex)
        if (container->getElement(endIndex) != value)
          break;
      res->appendSegment(beginIndex, endIndex, value);
      beginIndex = endIndex;
    }
    return res;
  };

protected:
  friend class SegmentContainerOperatorClass;

  TypePtr elementsType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_OPERATOR_SEGMENT_CONTAINER_H_
