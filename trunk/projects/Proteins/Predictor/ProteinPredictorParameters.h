/*-----------------------------------------.---------------------------------.
| Filename: ProteinPredictorParameters.h   | Protein Predictor Parameters    |
| Author  : Francis Maes                   |                                 |
| Started : 19/02/2011 03:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PREDICTOR_PARAMETERS_H_
# define LBCPP_PROTEIN_PREDICTOR_PARAMETERS_H_

# include <lbcpp/Core/CompositeFunction.h>
# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Learning/Numerical.h>
# include "../Data/Protein.h"
# include "../Data/ProteinFunctions.h"

namespace lbcpp
{

class ProteinPredictorParameters : public Object
{
public:
  virtual void proteinPerception(CompositeFunctionBuilder& builder) const = 0;
  virtual void residueVectorPerception(CompositeFunctionBuilder& builder) const = 0;
  virtual void residuePairVectorPerception(CompositeFunctionBuilder& builder) const = 0;

  // Protein -> ProteinPerception
  virtual FunctionPtr createProteinPerception() const
  {
    FunctionPtr function = new MethodBasedCompositeFunction(refCountedPointerFromThis(this), (FunctionBuildFunction)&ProteinPredictorParameters::proteinPerception);
    function->setBatchLearner(BatchLearnerPtr()); // by default: no learning on perceptions
    return function;
  }
  
  // Protein, ProteinPerception -> Vector[Residue Perception]
  virtual FunctionPtr createResidueVectorPerception() const
  {
    FunctionPtr function = new MethodBasedCompositeFunction(refCountedPointerFromThis(this), (FunctionBuildFunction)&ProteinPredictorParameters::residueVectorPerception);
    function->setBatchLearner(BatchLearnerPtr()); // by default: no learning on perceptions
    return function;
  }
  
  virtual FunctionPtr createResiduePairVectorPerception() const
  {
    FunctionPtr function = new MethodBasedCompositeFunction(refCountedPointerFromThis(this), (FunctionBuildFunction)&ProteinPredictorParameters::residuePairVectorPerception);
    function->setBatchLearner(BatchLearnerPtr()); // by default: no learning on perceptions
    return function;
  }

  virtual FunctionPtr learningMachine(ProteinTarget target) const = 0;
  virtual FunctionPtr binaryClassifier(ProteinTarget target) const
    {return learningMachine(target);}

  virtual FunctionPtr multiClassClassifier(ProteinTarget target) const
    {return learningMachine(target);}

  virtual FunctionPtr regressor(ProteinTarget target) const
    {return learningMachine(target);}

  // Vector[Input], Vector[Supervision] -> Vector[Output]
  virtual FunctionPtr labelVectorPredictor(ProteinTarget target) const
    {return mapContainerFunction(multiClassClassifier(target));}

  virtual FunctionPtr probabilityVectorPredictor(ProteinTarget target) const
    {return mapContainerFunction(binaryClassifier(target));}
  
  virtual FunctionPtr contactMapPredictor(ProteinTarget target) const
    {return mapContainerFunction(binaryClassifier(target));}
  
  virtual FunctionPtr distanceMapPredictor(ProteinTarget target) const
    {return mapContainerFunction(regressor(target));}

  // Features Container x Target supervision -> Predicted target
  virtual FunctionPtr createTargetPredictor(ProteinTarget target) const
  {
    FunctionPtr res;
    if (target == ss3Target || target == ss8Target || target == stalTarget)
      res = labelVectorPredictor(target);
    else if (target == sa20Target || target == drTarget)
      res = probabilityVectorPredictor(target);
    else if (target == cma8Target || target == cmb8Target)
      res = contactMapPredictor(target);
    else
    {
      jassert(false);
      return FunctionPtr();
    }
    return res;
  }
};

typedef ReferenceCountedObjectPtr<ProteinPredictorParameters> ProteinPredictorParametersPtr;

class NumericalProteinFeaturesParameters : public Object
{
public:
  NumericalProteinFeaturesParameters()
    : pssmDiscretization(1), pssmEntropyDiscretization(3),
      ss3Discretization(5), ss3EntropyDiscretization(3),
      ss8Discretization(5), ss8EntropyDiscretization(3),
      stalDiscretization(2), stalEntropyDiscretization(3),
      sa20Discretization(5),
      drDiscretization(5),
      residueGlobalFeatures(true), residueWindowSize(15),
      residueLocalMeanSize(15), residueMediumMeanSize(50),
      residuePairGlobalFeatures(true), residuePairWindowSize(15),
      residuePairLocalMeanSize(15), residuePairMediumMeanSize(50),
      aminoAcidDistanceFeature(true), useIntervalMean(true),
      cartesianProductPrimaryWindowSize(5)
  {
  }

  // pssm
  size_t pssmDiscretization;
  size_t pssmEntropyDiscretization;

  // ss3
  size_t ss3Discretization;
  size_t ss3EntropyDiscretization;

  // ss8
  size_t ss8Discretization;
  size_t ss8EntropyDiscretization;

  // stal
  size_t stalDiscretization;
  size_t stalEntropyDiscretization;

  // sa
  size_t sa20Discretization;

  // dr
  size_t drDiscretization;

  // global
  bool residueGlobalFeatures;
  size_t residueWindowSize;
  size_t residueLocalMeanSize;
  size_t residueMediumMeanSize;
  
  // pair
  bool residuePairGlobalFeatures;
  size_t residuePairWindowSize;
  size_t residuePairLocalMeanSize;
  size_t residuePairMediumMeanSize;
  bool aminoAcidDistanceFeature;
  bool useIntervalMean;
  size_t cartesianProductPrimaryWindowSize;

  virtual String toString() const
    {return T("(") + defaultToStringImplementation(false) + T(")");}

  virtual bool loadFromString(ExecutionContext& context, const String& str)
  {
    if (str.length() < 2 || str[0] != '(' || str[str.length() - 1] != ')')
    {
      context.errorCallback(T("Invalid syntax: ") + str.quoted());
      return false;
    }
    return Object::loadFromString(context, str.substring(1, str.length() - 1));
  }
};

typedef ReferenceCountedObjectPtr<NumericalProteinFeaturesParameters> NumericalProteinFeaturesParametersPtr;

extern ClassPtr numericalProteinFeaturesParametersClass;

ProteinPredictorParametersPtr numericalProteinPredictorParameters(NumericalProteinFeaturesParametersPtr featuresParameters, LearnerParametersPtr learningParameters);
ProteinPredictorParametersPtr numericalProteinPredictorParameters();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PREDICTOR_PARAMETERS_H_

