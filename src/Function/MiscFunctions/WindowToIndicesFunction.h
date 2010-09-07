/*-----------------------------------------.---------------------------------.
| Filename: WindowToIndicesFunction.h      | Window To Indicies Function     |
| Author  : Julien Becker                  |                                 |
| Started : 06/10/2010 20:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_FUNCTION_WINDOW_TO_INDICES_H_
# define LBCPP_DATA_FUNCTION_WINDOW_TO_INDICES_H_

# include <lbcpp/Function/Function.h>

namespace lbcpp
{
  
class WindowToIndicesFunction : public Function
{
public:
  WindowToIndicesFunction(size_t windowSize = 0)
    : windowSize(windowSize)
  {}
  
  virtual TypePtr getInputType() const
    {return pairType(vectorClass(anyType()), integerType());}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return pairType(inputType->getTemplateArgument(0), pairType(integerType(), integerType()));}
                     
  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    int startPosition = input[1].getInteger() - (int)(windowSize / 2);
    return Variable::pair(input[0], Variable::pair(startPosition, startPosition + windowSize));
  }
 
private:
  friend class WindowToIndicesFunctionClass;

  size_t windowSize;
};
                     
}; /* namespace lbcpp */
                     
#endif // !LBCPP_DATA_FUNCTION_WINDOW_TO_INDICES_H_
