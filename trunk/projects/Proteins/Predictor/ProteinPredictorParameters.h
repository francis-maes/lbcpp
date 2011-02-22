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
# include "../Data/Protein.h"
# include "../Data/ProteinFunctions.h"

namespace lbcpp
{

class ProteinPredictorParameters : public Object
{
public:
  virtual void residueVectorPerception(CompositeFunctionBuilder& builder) const = 0;

  // Protein -> Vector[Residue Perception]
  virtual FunctionPtr createResidueVectorPerception() const
  {
    FunctionPtr function = new MethodBasedCompositeFunction(refCountedPointerFromThis(this), (FunctionBuildFunction)&ProteinPredictorParameters::residueVectorPerception);
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

  // Features Container x Target supervision -> Predicted target
  virtual FunctionPtr createTargetPredictor(ProteinTarget target) const
  {
    FunctionPtr res;
    if (target == ss3Target || target == ss8Target || target == stalTarget)
      res = labelVectorPredictor(target);
    else if (target == sa20Target || target == drTarget)
      res = probabilityVectorPredictor(target);
    else
    {
      jassert(false);
      return FunctionPtr();
    }
    return res;
  }
};

typedef ReferenceCountedObjectPtr<ProteinPredictorParameters> ProteinPredictorParametersPtr;

class NumericalProteinPredictorParameters : public ProteinPredictorParameters
{
public:
  // Features
  virtual void primaryResidueFeatures(CompositeFunctionBuilder& builder) const;
  virtual void primaryResidueFeaturesVector(CompositeFunctionBuilder& builder) const;
  virtual void residueFeatures(CompositeFunctionBuilder& builder) const;
  virtual void residueFeaturesVector(CompositeFunctionBuilder& builder) const;

  virtual void residueVectorPerception(CompositeFunctionBuilder& builder) const
    {residueFeaturesVector(builder);}

  typedef void (NumericalProteinPredictorParameters::*ThisClassFunctionBuildFunction)(CompositeFunctionBuilder& builder) const; 

  FunctionPtr function(ThisClassFunctionBuildFunction buildFunc) const
    {return function((FunctionBuildFunction)buildFunc);}

  FunctionPtr function(FunctionBuildFunction buildFunc) const
    {return new MethodBasedCompositeFunction(refCountedPointerFromThis(this), buildFunc);}

  // Learning Machine
  virtual FunctionPtr learningMachine(ProteinTarget target) const;
};

typedef ReferenceCountedObjectPtr<NumericalProteinPredictorParameters> NumericalProteinPredictorParametersPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PREDICTOR_PARAMETERS_H_

