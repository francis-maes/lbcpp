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
  virtual void disulfideResiduePairVectorPerception(CompositeFunctionBuilder& builder) const = 0;
  virtual void cysteinBondingStateVectorPerception(CompositeFunctionBuilder& builder) const = 0;

  // Protein -> ProteinPerception
  virtual FunctionPtr createProteinPerception() const
  {
    FunctionPtr function = lbcppMemberCompositeFunction(ProteinPredictorParameters, proteinPerception);
    function->setBatchLearner(BatchLearnerPtr()); // by default: no learning on perceptions
    return function;
  }

  // Protein, ProteinPerception -> Vector[Residue Perception]
  virtual FunctionPtr createResidueVectorPerception() const
  {
    FunctionPtr function = lbcppMemberCompositeFunction(ProteinPredictorParameters, residueVectorPerception);
    function->setBatchLearner(BatchLearnerPtr()); // by default: no learning on perceptions
    return function;
  }

  virtual FunctionPtr createResiduePairVectorPerception() const
  {
    FunctionPtr function = lbcppMemberCompositeFunction(ProteinPredictorParameters, residuePairVectorPerception);
    function->setBatchLearner(BatchLearnerPtr()); // by default: no learning on perceptions
    return function;
  }
  
  virtual FunctionPtr createDisulfideResiduePairVectorPerception() const
  {
    FunctionPtr function = lbcppMemberCompositeFunction(ProteinPredictorParameters, disulfideResiduePairVectorPerception);
    function->setBatchLearner(BatchLearnerPtr()); // by default: no learning on perceptions
    return function;
  }
  
  virtual FunctionPtr createCysteinBondingStateVectorPerception() const
  {
    FunctionPtr function = lbcppMemberCompositeFunction(ProteinPredictorParameters, cysteinBondingStateVectorPerception);
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
    {return mapNContainerFunction(multiClassClassifier(target));}

  virtual FunctionPtr probabilityVectorPredictor(ProteinTarget target) const
    {return mapNContainerFunction(binaryClassifier(target));}

  virtual FunctionPtr contactMapPredictor(ProteinTarget target) const
    {return mapNSymmetricMatrixFunction(binaryClassifier(target));}

  virtual FunctionPtr distanceMapPredictor(ProteinTarget target) const
    {return mapNSymmetricMatrixFunction(regressor(target));}
  
  virtual FunctionPtr disulfideBondPredictor(ProteinTarget target) const
    {return mapNSymmetricMatrixFunction(binaryClassifier(target), 1);}

  // Features Container x Target supervision -> Predicted target
  virtual FunctionPtr createTargetPredictor(ProteinTarget target) const
  {
    FunctionPtr res;
    if (target == ss3Target || target == ss8Target || target == stalTarget)
      res = labelVectorPredictor(target);
    else if (target == sa20Target || target == drTarget || target == cbsTarget)
      res = probabilityVectorPredictor(target);
    else if (target == cma8Target || target == cmb8Target)
      res = contactMapPredictor(target);
    else if (target == dsbTarget)
      res = disulfideBondPredictor(target);
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
      : pssmDiscretization(4), pssmEntropyDiscretization(6),
      ss3Discretization(9), ss3EntropyDiscretization(2),
      ss8Discretization(12), ss8EntropyDiscretization(4),
      stalDiscretization(2), stalEntropyDiscretization(5),
      sa20Discretization(7),
      drDiscretization(7),
  
      dsbDiscretization(3), dsbNormalizedDiscretization(3),
      dsbWindowRows(3), dsbWindowColumns(3), dsbEntropyDiscretization(4),
      dsbPairWindowRows(3), dsbPairWindowColumns(3),

      residueGlobalFeatures(true), residueWindowSize(9),
      residueLocalMeanSize(18), residueMediumMeanSize(90),
      residuePairGlobalFeatures(true), residuePairWindowSize(15),
      residuePairLocalMeanSize(15), residuePairMediumMeanSize(50),
      aminoAcidDistanceFeature(true), useIntervalMean(true),
      cartesianProductPrimaryWindowSize(0)
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
  
  // dsb
  size_t dsbDiscretization;
  size_t dsbNormalizedDiscretization;
  size_t dsbWindowRows;
  size_t dsbWindowColumns;
  size_t dsbEntropyDiscretization;
  size_t dsbPairWindowRows;
  size_t dsbPairWindowColumns;

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
