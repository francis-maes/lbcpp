/*-----------------------------------------.---------------------------------.
| Filename: LinearRegressor.h              | Linear Regressors               |
| Author  : Francis Maes                   |                                 |
| Started : 15/02/2010 20:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_LINEAR_REGRESSOR_H_
# define LBCPP_LEARNING_NUMERICAL_LINEAR_REGRESSOR_H_

# include <lbcpp/Core/CompositeFunction.h>
# include <lbcpp/Learning/Numerical.h>
# include "libsvm.h"

namespace lbcpp
{

extern ClassPtr libSVMLearningMachineClass;

class LibSVMLearningMachine : public Function
{
public:
  LibSVMLearningMachine(double C = 0.1, size_t kernelType = 0, size_t kernelDegree = 1, double kernelGamma = 0.1, double kernelCoef0 = 0.0)
      : C(C), kernelType(kernelType), kernelDegree(kernelDegree), kernelGamma(kernelGamma), kernelCoef0(kernelCoef0), model(NULL), problem(NULL)
  {
    setBatchLearner(libSVMBatchLearner());
  }

  virtual ~LibSVMLearningMachine()
    {destroyModel();}

  virtual TypePtr getSupervisionType() const = 0;
  virtual size_t getSvmType() const = 0;
  virtual svm_node* getInput(const ObjectPtr& example) const = 0;
  virtual double getSupervision(const ObjectPtr& example) const = 0;

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? getSupervisionType() : (TypePtr)doubleVectorClass();}
 
protected:
  friend class LibSVMLearningMachineClass;
  friend class LibSVMBatchLearner;

  double C;
  size_t kernelType;
  size_t kernelDegree;
  double kernelGamma;
  double kernelCoef0;

  svm_model*   model;
  svm_problem* problem;
  
  svm_parameter createSvmParameters()
  {
    svm_parameter parameter;

    parameter.svm_type = getSvmType();
    parameter.kernel_type = kernelType;
    parameter.degree = kernelDegree;
    parameter.gamma = kernelGamma; // 1 / k
    parameter.coef0 = kernelCoef0;
    parameter.nu = 0.5;
    parameter.cache_size = 40;
    parameter.C = C;
    parameter.eps = 0.001;
    parameter.p = 0.1;
    parameter.shrinking = 1;
    parameter.probability = 1; // probability estimate
    parameter.nr_weight = 0;
    parameter.weight_label = NULL;
    parameter.weight = NULL;

    return parameter;
  }

  void destroyModel()
  {
    svm_free_and_destroy_model(&model);
    if (problem)
    {
      for (int i = 0; i < problem->l; ++i)
	      delete [] problem->x[i];
      delete [] problem->x;
      delete [] problem->y;
      delete problem;
    }
  }

  static svm_node* convertDoubleVector(const DoubleVectorPtr& vector)
  {
    SparseDoubleVectorPtr features = vector->toSparseVector();
    std::pair<size_t, double>* v = features->getValues();
    size_t n = features->getNumValues();
    svm_node* res = new svm_node[n + 1];
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

typedef ReferenceCountedObjectPtr<LibSVMLearningMachine> LibSVMLearningMachinePtr;

class LibSVMBatchLearner : public BatchLearner
{
public:
  virtual TypePtr getRequiredFunctionType() const
    {return libSVMLearningMachineClass;}

  svm_problem* createProblem(const LibSVMLearningMachinePtr& machine, const std::vector<ObjectPtr>& data) const
  {
    typedef svm_node* svm_node_ptr;

    svm_problem* problem = new struct svm_problem();
    problem->l = data.size();
    problem->x = new svm_node_ptr[data.size()];
    problem->y = new double[data.size()];
    for (unsigned i = 0; i < data.size(); ++i)
    {
      problem->x[i] = machine->getInput(data[i]);
      problem->y[i] = machine->getSupervision(data[i]);
    }
    return problem;
  }

  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    LibSVMLearningMachinePtr machine = function.staticCast<LibSVMLearningMachine>();

    machine->destroyModel();
    machine->problem = createProblem(machine, trainingData);

    // learn the new model
    svm_parameter parameter = machine->createSvmParameters();
    machine->model = svm_train(context, machine->problem, &parameter);

    //svm_save_model("toto.model", model);

    // free memory
    svm_destroy_param(&parameter);
    return true;
  }
};

class LibSVMClassifier : public LibSVMLearningMachine
{
public:
  LibSVMClassifier(double C, size_t kernelType, size_t kernelDegree, double kernelGamma, double kernelCoef0)
    : LibSVMLearningMachine(C, kernelType, kernelDegree, kernelGamma, kernelCoef0) {}
  LibSVMClassifier() {}

  virtual TypePtr getSupervisionType() const
    {return sumType(denseDoubleVectorClass(), enumValueType);}

  virtual size_t getSvmType() const
    {return C_SVC;}

  virtual svm_node* getInput(const ObjectPtr& example) const
    {return convertDoubleVector(example->getVariable(0).getObjectAndCast<DoubleVector>());}

  virtual double getSupervision(const ObjectPtr& example) const
  {
    Variable supervision = example->getVariable(1);
    if (supervision.isEnumeration())
      return supervision.toDouble();
    else
    {
      DoubleVectorPtr vector = supervision.dynamicCast<DoubleVector>();
      if (vector)
        return (double)vector->getIndexOfMaximumValue();
      else
      {
        jassert(false);
        return 0.0;
      }
    }
  }

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    TypePtr supervisionType = inputVariables[1]->getType();
    if (supervisionType.isInstanceOf<Enumeration>())
      labels = supervisionType.staticCast<Enumeration>();
    else
    {
      labels = DoubleVector::getElementsEnumeration(supervisionType);
      jassert(labels);
    }
    return denseDoubleVectorClass(labels, probabilityType);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    if (!model)
      return Variable();

    DoubleVectorPtr input = inputs[0].getObjectAndCast<DoubleVector>();

    // predict probabilities
    std::vector<double> probs(model->nr_class);
    svm_node* node = convertDoubleVector(input);
    svm_predict_probability(model, &node[0], &probs[0]);
    delete [] node;

    // reorder classes and return probabilities
    DenseDoubleVectorPtr res = new DenseDoubleVector(getOutputType());
    std::vector<int> labelIndices(model->nr_class);
    svm_get_labels(model, &labelIndices[0]);
    for (int i = 0; i < model->nr_class; ++i)
      res->setValue(labelIndices[i], probs[i]);
    return res;
  }

protected:
  EnumerationPtr labels;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_LINEAR_REGRESSOR_H_
