/*-----------------------------------------.---------------------------------.
| Filename: Predictor.cpp                  | Predictor                       |
| Author  : Julien Becker                  |                                 |
| Started : 10/11/2010 16:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "PredictorProgram.h"
#include "MatlabFileParser.h"

using namespace lbcpp;

class SavePredictionFunction : public Function
{
public:
  SavePredictionFunction(const InferencePtr inference, const File& output) : inference(inference), context(singleThreadedInferenceContext())
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
  
  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    std::cout << "." << std::flush;
    Variable res = context->predict(inference, input);
    *o << res.toShortString() << "\n";
    return input;
  }

protected:
  InferencePtr inference;

private:
  InferenceContextPtr context;
  OutputStream* o;
};

bool PredictorProgram::run(ExecutionContext& context)
{
  ContainerPtr data = parseDataFile(dataFile);
  jassert(data && data->getNumElements());
  std::cout << "Data : " << data->getNumElements() << std::endl;
  InferencePtr inference = Object::createFromFile(inferenceFile).staticCast<Inference>();
  jassert(inference);
  
  if (output == File::nonexistent)
    output = File::getCurrentWorkingDirectory().getChildFile(T("prediction.txt"));
  std::cout << "Prediction : " << output.getFullPathName() << std::endl;

  data->apply(FunctionPtr(new SavePredictionFunction(inference, output)), false);  
  return true;
}
