/*-----------------------------------------.---------------------------------.
| Filename: LaRankLearningMachine.h        | LaRank Library Learning Machine |
| Author  : Julien Becker                  |                                 |
| Started : 06/07/2011 15:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_LARANK_LEARNING_MACHINE_H_
# define LBCPP_LEARNING_NUMERICAL_LARANK_LEARNING_MACHINE_H_

# include <lbcpp/Core/CompositeFunction.h>
# include <lbcpp/Learning/Numerical.h>
# include "LaRank.h"

namespace lbcpp
{

extern ClassPtr laRankLearningMachineClass;

class LaRankLearningMachine;
typedef ReferenceCountedObjectPtr<LaRankLearningMachine> LaRankLearningMachinePtr;

class LaRankLearningMachine : public Function
{
public:
  LaRankLearningMachine(double C = 0.1, LaRankKernelType kernelType = laRankLinearKernel, size_t kernelDegree = 1, double kernelGamma = 0.1, double kernelCoef0 = 0.0)
  : C(C), kernelType(kernelType), kernelDegree(kernelDegree), kernelGamma(kernelGamma), kernelCoef0(kernelCoef0), model(NULL)
  {
    setBatchLearner(laRankBatchLearner());
  }

  virtual ~LaRankLearningMachine()
    {destroyModel();}

  virtual TypePtr getSupervisionType() const = 0;
  virtual size_t getNumClasses() const = 0;
  virtual void getInput(const ObjectPtr& example, SVector& result) const = 0;
  virtual int getSupervision(const ObjectPtr& example) const = 0;

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? getSupervisionType() : (TypePtr)doubleVectorClass();}
 
protected:
  friend class LaRankLearningMachineClass;
  friend class LaRankBatchLearner;

  struct LaRankConfiguration
  {
    LaRankLearningMachinePtr machine;
    Exampler trainingData;
    SVector testingData;
    double normTestingData;
  } configuration;
  
  double C;
  size_t kernelType;
  size_t kernelDegree;
  double kernelGamma;
  double kernelCoef0;

  Machine* model;

  void destroyModel()
  {
    model->destroy();
  }

  static void convertDoubleVector(const DoubleVectorPtr& vector, SVector& result)
  {
    result.clear();
    SparseDoubleVectorPtr features = vector->toSparseVector();
    std::pair<size_t, double>* v = features->getValues();
    size_t n = features->getNumValues();
    for (size_t i = 0; i < n; ++i)
    {
      result.set(v->first, v->second);
      ++v;
    }
  }
  
  static double linearKernel(int i, int j, void* config)
  {
    jassert(config);
    LaRankConfiguration* laRank = (LaRankConfiguration*)config;
    SVector xi = i == -1 ? laRank->testingData : laRank->trainingData.data[i].inpt;
    SVector xj = laRank->trainingData.data[j].inpt;
    return dot(xi, xj);
  }
  
  static double polynomialKernel(int i, int j, void* config)
  {
    jassert(config);
    LaRankConfiguration* laRank = (LaRankConfiguration*)config;
    SVector xi = i == -1 ? laRank->testingData : laRank->trainingData.data[i].inpt;
    SVector xj = laRank->trainingData.data[j].inpt;
    return pow(laRank->machine->kernelGamma * dot(xi, xj) + laRank->machine->kernelCoef0, (double)laRank->machine->kernelDegree);
  }
  
  static double rbfKernel(int i, int j, void* config)
  {
    jassert(config);
    LaRankConfiguration* laRank = (LaRankConfiguration*)config;
    SVector xi = i == -1 ? laRank->testingData : laRank->trainingData.data[i].inpt;
    SVector xj = laRank->trainingData.data[j].inpt;
    const double normXi = i == -1 ? laRank->normTestingData : laRank->trainingData.data[i].norm;
    const double normXj = laRank->trainingData.data[j].norm;
    return exp(-laRank->machine->kernelGamma * (normXi + normXj - 2 * dot(xi, xj)));
  }
};

class LaRankBatchLearner : public BatchLearner
{
public:
  virtual TypePtr getRequiredFunctionType() const
    {return laRankLearningMachineClass;}

  void createExampler(const LaRankLearningMachinePtr& machine, const std::vector<ObjectPtr>& data, Exampler& db) const
  {
    db.nb_ex = data.size();
    db.max_index = data.size() ? data[0]->getVariable(0).getObjectAndCast<DoubleVector>()->getNumElements() : 0;
    db.nb_labels = machine->getNumClasses();
    db.data.resize(data.size());
    for (size_t i = 0; i < data.size(); ++i)
    {
      machine->getInput(data[i], db.data[i].inpt);
      db.data[i].cls = machine->getSupervision(data[i]);
      db.data[i].norm = dot(db.data[i].inpt, db.data[i].inpt);
    }
  };

  Machine* createMachine(const LaRankLearningMachinePtr& machine, void* config) const
  {
    Machine* svm = create_larank(config);
    svm->tau = 0.0001;
    svm->C = machine->C;
    svm->cache = 64;

    switch (machine->kernelType) {
      case laRankLinearKernel:
        svm->kfunc = &LaRankLearningMachine::linearKernel;
        break;
      case laRankPolynomialKernel:
        svm->kfunc = &LaRankLearningMachine::polynomialKernel;
        break;
      case laRankRBFKernel:
        svm->kfunc = &LaRankLearningMachine::rbfKernel;
        break;
      default:
        jassertfalse;
    }

    return svm;
  }

  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    LaRankLearningMachinePtr machine = function.staticCast<LaRankLearningMachine>();
    machine->configuration.machine = machine;

    createExampler(machine, trainingData, machine->configuration.trainingData);
    Machine* svm = createMachine(machine, (void*)&machine->configuration);

    size_t iteration = 0;
    double gap = DBL_MAX;
    while (gap > svm->C)
    {
      for (size_t i = 0; i < trainingData.size(); ++i)
        svm->add((int)i, machine->configuration.trainingData.data[i].cls);
      gap = svm->computeGap();
      context.enterScope(T("Iteration ") + String((int)iteration));
      context.resultCallback(T("Iteration"), iteration);
      context.resultCallback(T("Gap"), gap);
      context.resultCallback(T("C"), svm->C);
      context.leaveScope(gap);
      ++iteration;
    }

    machine->model = svm;
    return true;
  }
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_LARANK_LEARNING_MACHINE_H_
