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

class NumericalProteinFeaturesParameters : public Object
{
public:
  NumericalProteinFeaturesParameters()
    : pssmDiscretization(5), pssmEntropyDiscretization(5),
      ss3Discretization(5), ss3EntropyDiscretization(5),
      ss8Discretization(5), ss8EntropyDiscretization(5),
      stalDiscretization(5), stalEntropyDiscretization(5),
      sa20Discretization(5),
      drDiscretization(5),
      residueGlobalFeatures(true), residueWindowSize(15), residueLocalMeanSize(15), residueMediumMeanSize(50)
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

ProteinPredictorParametersPtr numericalProteinPredictorParameters(NumericalProteinFeaturesParametersPtr featuresParameters, LearnerParametersPtr learningParameters);
ProteinPredictorParametersPtr numericalProteinPredictorParameters();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PREDICTOR_PARAMETERS_H_

