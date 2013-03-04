/*------------------------------------------.---------------------------------.
 | Filename: ColoVariableEncoder.h          | Compiler Optimization Level     |
 | Author  : Denny Verbeeck                 | Optimization Variable Encoder   |
 | Started : 22/02/2013 21:10               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef ML_COLO_VARIABLE_ENCODER_H_
# define ML_COLO_VARIABLE_ENCODER_H_

# include <ml/ExpressionDomain.h>
# include <ml/VariableEncoder.h>
# include "ColoProblem.h"

namespace lbcpp
{
  
class ColoVariableEncoder : public VariableEncoder
{
public:
  ColoVariableEncoder() : numFlags(0) {}

  virtual void createEncodingVariables(ExecutionContext& context, DomainPtr domain, ExpressionDomainPtr res)
  {
    ColoDomainPtr coloDomain = domain.staticCast<ColoDomain>();
    numFlags = coloDomain->getNumFlags();
    for (size_t i = 0; i < numFlags; ++i)
      res->addInput(positiveIntegerClass, "flag" + string((int)i+1));
    for (size_t i = 0; i < numFlags; ++i)
      for (size_t j = 0; j < numFlags; ++j)
        res->addInput(positiveIntegerClass, "flag" + string((int)i+1) + "before" + string((int)j+1));
  }

  virtual void encodeIntoVariables(ExecutionContext& context, ObjectPtr solution, std::vector<ObjectPtr>& res)
  {
    ColoObjectPtr coloObject = solution.staticCast<ColoObject>();
    std::vector<size_t> counts(numFlags * (numFlags + 1));
    for (size_t j = 0; j < coloObject->getLength(); ++j)
    {
      size_t flag = coloObject->getFlag(j);
      counts[flag]++;
      for (size_t i = 0; i < j; ++i)
      {
        size_t previousFlag = coloObject->getFlag(i);
        counts[numFlags + previousFlag * numFlags + flag]++;
      }
    }
    
    res.resize(counts.size());
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = getPositiveInteger(counts[i]);
  }
  
private:
  size_t numFlags;
  std::vector<ObjectPtr> integers; // avoid allocating dozens of PositiveIntegers
  
  ObjectPtr getPositiveInteger(size_t i)
  {
    while (i >= integers.size())
    {
      ObjectPtr posInt = new PositiveInteger(integers.size());
      integers.push_back(posInt);
    }
    return integers[i];
  }
};

extern VariableEncoderPtr coloVariableEncoder();
  
};
#endif
