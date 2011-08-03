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

extern ClassPtr decoratorProteinPredictorParametersClass;

class DecoratorProteinPredictorParameters : public ProteinPredictorParameters
{
public:
  DecoratorProteinPredictorParameters(ProteinPredictorParametersPtr decorated)
    : decorated(decorated) {setThisClass(decoratorProteinPredictorParametersClass);}
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
  
  virtual FunctionPtr createTargetPredictor(ProteinTarget target) const
    {return decorated->createTargetPredictor(target);}

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
/*    std::cout << "Support Vector : " << std::endl;
    for (size_t i = 0; i < supportVectors.size(); ++i)
      std::cout << supportVectors[i]->toString() << std::endl;
    std::cout << std::endl;
*/    
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
    
    DenseDoubleVectorPtr denseVector = new DenseDoubleVector(values->getElementsEnumeration(), values->getElementsType());
    const size_t numElements = values->getNumElements();
    double* ptr = denseVector->getValuePointer(0);
    for (size_t i = 0; i < numElements; ++i, ++ptr)
      *ptr = values->getElement(i).exists() ? values->getElement(i).getDouble() : 0.0;
/*
    std::cout << "Support Vector 0: ";
    for (size_t i = 0; i < supportVectors[0]->getNumElements(); ++i)
      std::cout << supportVectors[0]->getElement(i).toString() << " ";
    std::cout << std::endl;

    std::cout << "Input Vector: ";
    for (size_t i = 0; i < numElements; ++i)
      std::cout << denseVector->getElement(i).toString() << " ";
    std::cout << std::endl;

    std::cout << "Dot Product: " << supportVectors[0]->dotProduct(denseVector, 0) << std::endl;

    std::cout << "Norm of Support Vector 0: " << supportNorms[0] << " - Norm of Input: " <<  denseVector->sumOfSquares() << std::endl;

    std::cout << "Gaussian Values:";
*/    for (size_t i = 0; i < n; ++i)
    {
      jassert(supportVectors[i]->getNumElements() == values->getNumElements());

      const double norm = denseVector->sumOfSquares();
      const double res = exp(-gamma * (norm + supportNorms[i] - 2 * supportVectors[i]->dotProduct(denseVector, 0)));
      //std::cout << res << " ";
      callback.sense(i, res);
    }
    //std::cout << std::endl;
    //jassertfalse;
  }
  
  virtual bool isSparse() const
    {return false;}

protected:
  friend class GaussianKernelFeatureGeneratorClass;

  double gamma;
  std::vector<DoubleVectorPtr> supportVectors;
  std::vector<double> supportNorms;
};

extern ClassPtr gaussianKernelPredictorParametersClass;

class GaussianKernelPredictorParameters : public DecoratorProteinPredictorParameters
{
public:
  GaussianKernelPredictorParameters(ProteinPredictorParametersPtr decorated)
    : DecoratorProteinPredictorParameters(decorated)
  {
    setThisClass(gaussianKernelPredictorParametersClass);
  }
  
  void initialize(ExecutionContext& context, double gamma, ContainerPtr supportProteins)
  {
    context.enterScope(T("Initialisation of Gaussian Kernel"));
    //FunctionPtr f = lbcppMemberCompositeFunction(DecoratorProteinPredictorParameters, cysteinSymmetricResiudePairVectorPerception);
    FunctionPtr f = CompositeFunctionPtr(new MethodBasedCompositeFunction(refCountedPointerFromThis(decorated.get()),  (CompositeFunctionBuilderFunction)(&ProteinPredictorParameters::cysteinSymmetricResiudePairVectorPerception)));
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

      if (i == 0 && !f->initialize(context, proteinPerception->getClass()))
      {
        context.errorCallback(T("GaussianKernelPredictorParameters"), T("Initialization failed !"));
        return;
      }

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
    context.resultCallback(T("Num. Proteins"), numProteins);
    context.resultCallback(T("Num. Support Vectors"), supportVectors.size());
    context.resultCallback(T("Num. Base Features"), supportVectors[0]->getNumElements());
    context.leaveScope();
  }
  GaussianKernelPredictorParameters() {}

  virtual void cysteinSymmetricResiudePairVectorPerception(CompositeFunctionBuilder& builder) const
  {
//    size_t proteinPerception = builder.addInput(lin09ProteinPerceptionClass(enumValueType, enumValueType, enumValueType));
//    ProteinPredictorParametersPtr f = CompositeFunctionPtr(new MethodBasedCompositeFunction(refCountedPointerFromThis(decorated.get()),  (CompositeFunctionBuilderFunction)(&ProteinPredictorParameters::cysteinSymmetricResiudePairVectorPerception)));
//    size_t features = builder.addFunction(f, proteinPerception);
    DecoratorProteinPredictorParameters::cysteinSymmetricResiudePairVectorPerception(builder);
    builder.finishSelectionWithFunction(mapNFunction(gaussianKernel), T("Gaussian"));
  }

protected:
  friend class GaussianKernelPredictorParametersClass;

  FeatureGeneratorPtr gaussianKernel;
};

typedef ReferenceCountedObjectPtr<GaussianKernelPredictorParameters> GaussianKernelPredictorParametersPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PREDICTOR_PARAMETERS_DECORATOR_H_
