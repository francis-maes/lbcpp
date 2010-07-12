/*-----------------------------------------.---------------------------------.
| Filename: VectorWindowRepresentation.h   | Window Representation           |
| Author  : Francis Maes                   |                                 |
| Started : 12/07/2010 16:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_REPRESENTATION_VECTOR_WINDOW_H_
# define LBCPP_DATA_REPRESENTATION_VECTOR_WINDOW_H_

# include <lbcpp/Data/Representation.h>

namespace lbcpp
{

class VectorWindowRepresentation : public Representation
{
public:
  VectorWindowRepresentation(TypePtr elementsType, size_t windowSize)
    : elementsType(elementsType), windowSize(windowSize) {}
  VectorWindowRepresentation() : windowSize(0) {}

  virtual TypePtr getInputType() const
    {return pairType(vectorClass(elementsType), integerType());}

  virtual size_t getNumOutputVariables() const
    {return windowSize;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return elementsType;}

  virtual String getOutputVariableName(size_t index) const
    {return T("[") + String((int)index - (int)(windowSize / 2)) + T("]");}

  virtual void computeRepresentation(const Variable& input, RepresentationCallbackPtr callback) const
  {
    ContainerPtr container = input[0].getObjectAndCast<Container>();
    int startPosition = input[1].getInteger() - (int)(windowSize / 2);
    
    for (size_t i = 0; i < windowSize; ++i)
    {
      int position = startPosition + (int)i;
      callback->sense(i, position >= 0 && position < (int)container->size()
        ? container->getVariable(position)
        : Variable::missingValue(elementsType));
    }
  }

protected:
  TypePtr elementsType;
  size_t windowSize;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_REPRESENTATION_VECTOR_WINDOW_H_
