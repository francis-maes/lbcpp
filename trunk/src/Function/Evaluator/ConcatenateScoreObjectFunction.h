/*-----------------------------------------.---------------------------------.
| Filename: ConcatenateScoreObjectFunct...h| Concatenate Score Object        |
| Author  : Julien Becker                  |  Function                       |
| Started : 23/02/2011 11:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_MISC_CONCATENATE_SCORE_OBJECT_H_
# define LBCPP_FUNCTION_MISC_CONCATENATE_SCORE_OBJECT_H_

# include <lbcpp/Function/Evaluator.h>

namespace lbcpp
{

class ConcatenateScoreObjectFunction : public Function
{
public:
  virtual size_t getMinimumNumRequiredInputs() const
    {return 0;}
  
  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return scoreObjectClass;}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return scoreObjectClass;}
  
protected:
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    size_t numInputs = getNumInputs();
    CompositeScoreObjectPtr res = new CompositeScoreObject();
    for (size_t i = 0; i < numInputs; ++i)
      res->pushScoreObject(inputs[i].getObjectAndCast<ScoreObject>());
    return res;
  }
};
  
};

#endif //!LBCPP_FUNCTION_MISC_CONCATENATE_SCORE_OBJECT_H_
