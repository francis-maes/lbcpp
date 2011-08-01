/*-----------------------------------------.---------------------------------.
| Filename: ProteinPredictorParameters.h   | Protein Predictor Parameters    |
| Author  : Julien Becker                  |                                 |
| Started : 27/07/2011 14:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PREDICTOR_PARAMETERS_DECORATOR_H_
# define LBCPP_PROTEIN_PREDICTOR_PARAMETERS_DECORATOR_H_

# include "ProteinPredictorParameters.h"
# include "Lin09PredictorParameters.h"

namespace lbcpp
{

class DecoratorProteinPredictorParameters : public ProteinPredictorParameters
{
public:
  DecoratorProteinPredictorParameters(ProteinPredictorParametersPtr decorated)
    : decorated(decorated) {}
  DecoratorProteinPredictorParameters() {}
  
  virtual void proteinPerception(CompositeFunctionBuilder& builder) const
    {decorated->proteinPerception(builder);}

  virtual void propertyPerception(CompositeFunctionBuilder& builder) const
    {decorated->propertyPerception(builder);}

  virtual void residueVectorPerception(CompositeFunctionBuilder& builder) const
    {decorated->residueVectorPerception(builder);}

  virtual void residuePairVectorPerception(CompositeFunctionBuilder& builder) const
    {decorated->residuePairVectorPerception(builder);}

  virtual void cysteinResiudePairVectorPerception(CompositeFunctionBuilder& builder) const
    {decorated->cysteinResiudePairVectorPerception(builder);}

  virtual void cysteinSymmetricResiudePairVectorPerception(CompositeFunctionBuilder& builder) const
    {decorated->cysteinSymmetricResiudePairVectorPerception(builder);}

  virtual void cysteinResiudeVectorPerception(CompositeFunctionBuilder& builder) const
    {decorated->cysteinResiudeVectorPerception(builder);}

  virtual FunctionPtr learningMachine(ProteinTarget target) const
    {return decorated->learningMachine(target);}

protected:
  friend class DecoratorProteinPredictorParametersClass;
  
  ProteinPredictorParametersPtr decorated;
};

class GaussianKernelFeatureGenerator : public FeatureGenerator
{
public:
  GaussianKernelFeatureGenerator(double gamma, const std::vector<DoubleVectorPtr>& supportVectors)
    : gamma(gamma), supportVectors(supportVectors)
  {
    supportNorms.resize(supportVectors.size());
    for (size_t i = 0; i < supportVectors.size(); ++i)
      supportNorms[i] = supportVectors[i]->sumOfSquares();
  }
  GaussianKernelFeatureGenerator() {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleVectorClass(enumValueType, doubleType);}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration();
    for (size_t i = 0; i < supportVectors.size(); ++i)
      res->addElement(context, String((int)i));
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    DoubleVectorPtr values = inputs[0].getObjectAndCast<DoubleVector>();
    const size_t n = supportVectors.size();
    for (size_t i = 0; i < n; ++i)
    {
      jassert(supportVectors[i]->getNumElements() == values->getNumElements());

      double norm = supportVectors[i]->sumOfSquares();
      callback.sense(i, exp(-gamma * (norm + supportNorms[i] - 2 * supportVectors[i]->dotProduct(values, 0))));
    }
  }
  
  virtual bool isSparse() const
    {return false;}

protected:
  friend class GaussianKernelFeatureGeneratorClass;

  double gamma;
  std::vector<DoubleVectorPtr> supportVectors;
  std::vector<double> supportNorms;
};

class GaussianKernelPredictorParameters : public DecoratorProteinPredictorParameters
{
public:
  GaussianKernelPredictorParameters(ExecutionContext& context, ProteinPredictorParametersPtr decorated, double gamma, ContainerPtr supportProteins)
    : DecoratorProteinPredictorParameters(decorated)
  {
    FunctionPtr f = lbcppMemberCompositeFunction(DecoratorProteinPredictorParameters, cysteinSymmetricResiudePairVectorPerception);
    if (!f->initialize(context, lin09ProteinPerceptionClass(enumValueType, enumValueType, enumValueType)))
    {
      context.errorCallback(T("GaussianKernelPredictorParameters"), T("Initialization failed !"));
      return;
    }
    
    std::vector<DoubleVectorPtr> supportVectors;
    const size_t numProteins = supportProteins->getNumElements();
    FunctionPtr proteinPerceptionFunction = lbcppMemberCompositeFunction(DecoratorProteinPredictorParameters, proteinPerception);
    if (!proteinPerceptionFunction->initialize(context, proteinClass))
    {
      context.errorCallback(T("GaussianKernelPredictorParameters"), T("Initialization failed !"));
      return;
    }

    for (size_t i = 0; i < numProteins; ++i)
    {
      ProteinPtr protein = supportProteins->getElement(i).getObjectAndCast<Pair>(context)->getVariable(0).getObjectAndCast<Protein>(context);
      jassert(protein);
      ObjectPtr proteinPerception = proteinPerceptionFunction->compute(context, protein).getObject();
      jassert(proteinPerception);
      ContainerPtr featuresVectors = f->compute(context, proteinPerception).getObjectAndCast<Container>();
      if (!featuresVectors)
        continue;
      const size_t n = featuresVectors->getNumElements();
      for (size_t j = 0; j < n; ++j)
      {
        DoubleVectorPtr features = featuresVectors->getElement(j).getObjectAndCast<DoubleVector>();
        if (features)
          supportVectors.push_back(features);
      }
    }
    jassert(supportVectors.size());

    gaussianKernel = new GaussianKernelFeatureGenerator(gamma, supportVectors);
  }
  GaussianKernelPredictorParameters() {}

  virtual void cysteinSymmetricResiudePairVectorPerception(CompositeFunctionBuilder& builder) const
  {
    size_t proteinPerception = builder.addInput(lin09ProteinPerceptionClass(enumValueType, enumValueType, enumValueType));

    size_t features = builder.addFunction(lbcppMemberCompositeFunction(Lin09PredictorParameters, cysteinSymmetricResiudePairVectorPerception), proteinPerception);

    builder.addFunction(mapNFunction(gaussianKernel), features, T("Gaussian"));
  }

protected:
  friend class GaussianKernelPredictorParametersClass;

  FeatureGeneratorPtr gaussianKernel;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PREDICTOR_PARAMETERS_DECORATOR_H_
