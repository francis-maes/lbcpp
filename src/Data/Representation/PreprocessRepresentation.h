/*-----------------------------------------.---------------------------------.
| Filename: PreprocessRepresentation.h     | Add a preprocessor to a         |
| Author  : Francis Maes                   |  Representation                 |
| Started : 12/07/2010 16:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_REPRESENTATION_PREPROCESS_H_
# define LBCPP_DATA_REPRESENTATION_PREPROCESS_H_

# include <lbcpp/Data/Representation.h>

namespace lbcpp
{

class PreprocessRepresentation : public DecoratorRepresentation
{
public:
  PreprocessRepresentation() {}

  PreprocessRepresentation(FunctionPtr preProcessingFunction, RepresentationPtr representation)
    : DecoratorRepresentation(representation), preProcessingFunction(preProcessingFunction)
    {}

  virtual TypePtr getInputType() const
    {return preProcessingFunction->getInputType();}

  virtual TypePtr getOutputType() const
    {return decorated->getOutputType();}

  virtual void computeRepresentation(const Variable& input, RepresentationCallbackPtr callback) const
  {
    if (checkInheritance(preProcessingFunction->getOutputType(input.getType()), decorated->getInputType()))
      DecoratorRepresentation::computeRepresentation(preProcessingFunction->compute(input), callback);
  }

protected:
  FunctionPtr preProcessingFunction;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_REPRESENTATION_PREPROCESS_H_
