/*-----------------------------------------.---------------------------------.
| Filename: Predictor.cpp                  | Predictor                       |
| Author  : Julien Becker                  |                                 |
| Started : 10/11/2010 16:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/lbcpp.h>
#include "PredictorProgram.h"
#include "MatlabFileParser.h"

using namespace lbcpp;

#if 0 // broken
class SavePredictionFunction : public Function
{
public:
  SavePredictionFunction(const InferencePtr inference, const File& output) : inference(inference)
  {
    if (output.exists())
      output.deleteFile();
    o = output.createOutputStream();
  }
  
  ~SavePredictionFunction()
    {delete o;}
  
  virtual TypePtr getInputType() const
    {return anyType;}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return inputType;}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    jassert(false); // broken
/*    std::cout << "." << std::flush;
    Variable res = inference->computeFunction(context, input);
    *o << res.toShortString() << "\n";*/
    return input;
  }

protected:
  InferencePtr inference;
  OutputStream* o;
};

#endif // 0

Variable PredictorProgram::run(ExecutionContext& context)
{
#if 0 // broken
  ContainerPtr data = parseDataFile(context, dataFile);
  jassert(data && data->getNumElements());
  std::cout << "Data : " << data->getNumElements() << std::endl;
  InferencePtr inference = Object::createFromFile(context, inferenceFile).staticCast<Inference>();
  jassert(inference);
  
  if (output == File::nonexistent)
    output = File::getCurrentWorkingDirectory().getChildFile(T("prediction.txt"));
  std::cout << "Prediction : " << output.getFullPathName() << std::endl;

  data->apply(context, FunctionPtr(new SavePredictionFunction(inference, output)), Container::sequentialApply);
#endif // 0
  return true;
}