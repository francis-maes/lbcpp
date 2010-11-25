/*-----------------------------------------.---------------------------------.
 | Filename: ProteinInferenceEvaluat...h    | ProteinInferenceEvaluator       |
 | Author  : Julien Becker                  | WorkUnit                        |
 | Started : 25/11/2010 11:20               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_PROTEIN_INFERENCE_EVALUATOR_WORK_UNIT_H_
# define LBCPP_PROTEINS_PROTEIN_INFERENCE_EVALUATOR_WORK_UNIT_H_
# include <lbcpp/lbcpp.h>

namespace lbcpp
{
  
class ProteinInferenceEvaluatorWorkUnit : public WorkUnit
{
public:
  ProteinInferenceEvaluatorWorkUnit() : numThreads(1) {}
  
  virtual String toString() const
    {return T("Take an learned inference and save prediction \
              from an input protein directory to an output directory.");}
  
  virtual bool run(ExecutionContext& context);

protected:
  friend class ProteinInferenceEvaluatorWorkUnitClass;
  
  File inputDirectory;
  File outputDirectory;
  File inferenceFile;
  
  size_t numThreads;
};

};

#endif //!LBCPP_PROTEINS_PROTEIN_INFERENCE_EVALUATOR_WORK_UNIT_H_
