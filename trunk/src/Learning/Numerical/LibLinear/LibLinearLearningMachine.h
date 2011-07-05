/*-----------------------------------------.---------------------------------.
| Filename: LibLinearLearningMachine.h     | Wrapper of LibLinear library    |
| Author  : Becker Julien                  |                                 |
| Started : 05/07/2011 16:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_LIBLINEAR_H_
# define LBCPP_LEARNING_NUMERICAL_LIBLINEAR_H_

# include <lbcpp/Core/CompositeFunction.h>
# include <lbcpp/Learning/Numerical.h>
# include "linear.h"

namespace lbcpp
{

extern ClassPtr libLinearLearningMachineClass;

class LibLinearLearningMachine : public Function
{
public:
  LibLinearLearningMachine(double C = 1.f, size_t solverType = L2R_L2LOSS_SVC_DUAL) : C(C), model(NULL), problem(NULL)
  {
    setBatchLearner(libLinearBatchLearner());
  }

  virtual ~LibLinearLearningMachine()
    {destroyModel();}

  virtual TypePtr getSupervisionType() const = 0;
  virtual struct feature_node* getInput(const ObjectPtr& example) const = 0;
  virtual double getSupervision(const ObjectPtr& example) const = 0;

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? getSupervisionType() : (TypePtr)doubleVectorClass();}
 
protected:
  friend class LibLinearLearningMachineClass;
  friend class LibLinearBatchLearner;

  double C;
  size_t solverType;

  struct model*   model;
  struct problem* problem;

  struct parameter createSvmParameters()
  {
    struct parameter param;
    
    param.solver_type = solverType;
    param.C = C;
    param.eps = DBL_MAX;
    param.nr_weight = 0;
    param.weight_label = NULL;
    param.weight = NULL;
    
    if (param.eps == DBL_MAX)
    {
      switch (param.solver_type) {
        case L2R_LR:
        case L2R_L2LOSS_SVC:
          param.eps = 0.01;
          break;
        case L2R_L2LOSS_SVC_DUAL:
        case L2R_L1LOSS_SVC_DUAL:
        case MCSVM_CS:
        case L2R_LR_DUAL:
          param.eps = 0.1;
          break;
        case L1R_L2LOSS_SVC:
        case L1R_LR:
          param.eps = 0.01;
        break;
      }
    }
    
    return param;
  }

  void destroyModel()
  {
    free_and_destroy_model(&model);
    if (problem)
    {
      for (int i = 0; i < problem->l; ++i)
	      delete [] problem->x[i];
      delete [] problem->x;
      delete [] problem->y;
      delete problem;
    }
  }

  static struct feature_node* convertDoubleVector(const DoubleVectorPtr& vector)
  {
    SparseDoubleVectorPtr features = vector->toSparseVector();
    std::pair<size_t, double>* v = features->getValues();
    size_t n = features->getNumValues();
    struct feature_node* res = new struct feature_node[n + 1];
    size_t i;
    for (i = 0; i < n; ++i)
    {
      res[i].index = (int)v->first;
      res[i].value = v->second;
      ++v;
    }
    res[i].index = -1;
    res[i].value = 0;
    return res;
  }
};

typedef ReferenceCountedObjectPtr<LibLinearLearningMachine> LibLinearLearningMachinePtr;

class LibLinearBatchLearner : public BatchLearner
{
public:
  virtual TypePtr getRequiredFunctionType() const
    {return libLinearLearningMachineClass;}

  struct problem* createProblem(const LibLinearLearningMachinePtr& machine, const std::vector<ObjectPtr>& data) const
  {
    struct problem* problem = new struct problem;
    problem->l = data.size();
    problem->x = new struct feature_node*[data.size()];
    problem->y = new int[data.size()];
    problem->bias = -1;
    for (unsigned i = 0; i < data.size(); ++i)
    {
      problem->x[i] = machine->getInput(data[i]);
      problem->y[i] = (int)machine->getSupervision(data[i]);
    }
    return problem;
  }

  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    LibLinearLearningMachinePtr machine = function.staticCast<LibLinearLearningMachine>();

    machine->destroyModel();
    machine->problem = createProblem(machine, trainingData);

    // learn the new model
    struct parameter param = machine->createSvmParameters();
    machine->model = ::train(machine->problem, &param);

    //svm_save_model("toto.model", model);

    // free memory
    destroy_param(&param);
    return true;
  }
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_LIBLINEAR_H_
