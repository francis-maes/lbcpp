/*-----------------------------------------.---------------------------------.
| Filename: PreprocessPerception.h         | Add a preprocessor to a         |
| Author  : Francis Maes                   |  Perception                     |
| Started : 12/07/2010 16:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_PERCEPTION_PREPROCESS_H_
# define LBCPP_DATA_PERCEPTION_PREPROCESS_H_

# include <lbcpp/Function/Perception.h>

namespace lbcpp
{

class PreprocessPerception : public DecoratorPerception
{
public:
  PreprocessPerception() {}

  PreprocessPerception(FunctionPtr preProcessingFunction, PerceptionPtr perception)
    : DecoratorPerception(perception), preProcessingFunction(preProcessingFunction)
    {}

  virtual TypePtr getInputType() const
    {return preProcessingFunction->getInputType();}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    if (checkInheritance(preProcessingFunction->getOutputType(input.getType()), decorated->getInputType()))
      DecoratorPerception::computePerception(preProcessingFunction->compute(input), callback);
  }

protected:
  friend class PreprocessPerceptionClass;

  FunctionPtr preProcessingFunction;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_PREPROCESS_H_
