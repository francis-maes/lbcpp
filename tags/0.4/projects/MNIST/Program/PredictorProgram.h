#ifndef LBCPP_MNIST_PREDICTOR_H_
# define LBCPP_MNIST_PREDICTOR_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class PredictorProgram : public WorkUnit
{
public:
  virtual String toString() const
    {return T("Predictor");}
  
  virtual Variable run(ExecutionContext& context);

protected:
  friend class PredictorProgramClass;
  
  File dataFile;
  File inferenceFile;
  File output;
};

};

#endif // !LBCPP_MNIST_PREDICTOR_H_
