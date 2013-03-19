/*-----------------------------------------.---------------------------------.
 | Filename: VariableEncoder.h              | Variable Encoder                |
 | Author  : Denny Verbeeck                 |                                 |
 | Started : 22/02/2013 16:44               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef ML_VARIABLE_ENCODER_H_
# define ML_VARIABLE_ENCODER_H_

# include "predeclarations.h"

namespace lbcpp
{

class VariableEncoder : public Object
{
public:
  virtual void createEncodingVariables(ExecutionContext& context, DomainPtr domain, ExpressionDomainPtr res) = 0;
  virtual void encodeIntoVariables(ExecutionContext& context, ObjectPtr object, std::vector<ObjectPtr>& res) = 0;  
};

extern VariableEncoderPtr scalarVectorVariableEncoder();

}; /* namespace lbcpp */

#endif  // !ML_VARIABLE_ENCODER_H_
