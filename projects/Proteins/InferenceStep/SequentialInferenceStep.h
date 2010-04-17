/*-----------------------------------------.---------------------------------.
| Filename: SequentialInferenceStep.h      | Base class for sequential       |
| Author  : Francis Maes                   |   inference                     |
| Started : 08/04/2010 18:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_STEP_SEQUENTIAL_H_
# define LBCPP_INFERENCE_STEP_SEQUENTIAL_H_

# include "InferenceStep.h"

namespace lbcpp
{

class SequentialInferenceStep : public InferenceStep
{
public:
  SequentialInferenceStep(const String& name) : InferenceStep(name) {}
  SequentialInferenceStep() {}

  /*
  ** Abstract
  */
  virtual size_t getNumSubSteps() const = 0;
  virtual InferenceStepPtr getSubStep(size_t index) const = 0;

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const
    {return supervision;}

  /*
  ** Object
  */
  virtual String toString() const;

  /*
  ** InferenceStep
  */
  virtual void accept(InferenceVisitorPtr visitor);
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);
};

class VectorSequentialInferenceStep : public SequentialInferenceStep
{
public:
  VectorSequentialInferenceStep(const String& name)
    : SequentialInferenceStep(name) {}

  void appendStep(InferenceStepPtr inference);

  /*
  ** SequentialInferenceStep
  */
  virtual size_t getNumSubSteps() const;
  virtual InferenceStepPtr getSubStep(size_t index) const;

  /*
  ** Object
  */
  virtual bool saveToFile(const File& file) const;
  virtual bool loadFromFile(const File& file);

protected:
  std::vector<InferenceStepPtr> inferenceSteps;
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_SEQUENTIAL_H_
