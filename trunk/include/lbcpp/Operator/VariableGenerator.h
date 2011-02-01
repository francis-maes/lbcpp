/*-----------------------------------------.---------------------------------.
| Filename: VariableGenerator.h            | Base class for Variable         |
| Author  : Francis Maes                   |  Geneators                      |
| Started : 01/02/2011 16:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPERATOR_VARIABLE_GENERATOR_H_
# define LBCPP_OPERATOR_VARIABLE_GENERATOR_H_

# include "Operator.h"

namespace lbcpp
{

class VariableGenerator : public Operator
{
public:

};

typedef ReferenceCountedObjectPtr<VariableGenerator> VariableGeneratorPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_VARIABLE_GENERATOR_H_
