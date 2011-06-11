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
  virtual void propertyPerception(CompositeFunctionBuilder& builder) const = 0;
  virtual void residueVectorPerception(CompositeFunctionBuilder& builder) const = 0;
  virtual void residuePairVectorPerception(CompositeFunctionBuilder& builder) const = 0;
  virtual void cysteinResiudePairVectorPerception(CompositeFunctionBuilder& builder) const = 0;
  virtual void cysteinResiudeVectorPerception(CompositeFunctionBuilder& builder) const = 0;

  // Protein -> ProteinPerception
  virtual FunctionPtr createProteinPerception() const
  {
    FunctionPtr function = lbcppMemberCompositeFunction(ProteinPredictorParameters, proteinPerception);
    function->setBatchLearner(BatchLearnerPtr()); // by default: no learning on perceptions
    return function;
  }
  
  // ProteinPerception -> DoubleVector
  virtual FunctionPtr createGlobalPerception() const
  {
    FunctionPtr function = lbcppMemberCompositeFunction(ProteinPredictorParameters, propertyPerception);
    function->setBatchLearner(BatchLearnerPtr()); // by default: no learning on perceptions
    return function;
  }

  // ProteinPerception -> Vector[Residue Perception]
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
    FunctionPtr function = lbcppMemberCompositeFunction(ProteinPredictorParameters, cysteinResiudePairVectorPerception);
    function->setBatchLearner(BatchLearnerPtr()); // by default: no learning on perceptions
    return function;
  }
  
  virtual FunctionPtr createCysteinBondingStateVectorPerception() const
  {
    FunctionPtr function = lbcppMemberCompositeFunction(ProteinPredictorParameters, cysteinResiudeVectorPerception);
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
  
  virtual FunctionPtr labelPropertyPredictor(ProteinTarget target) const
    {return multiClassClassifier(target);}

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
    else if (target == cbpTarget)
      res = labelPropertyPredictor(target);
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
      : 
      proteinLengthDiscretization(20), numCysteinsDiscretization(5),
      bondingPropertyDiscretization(5), bondingPropertyEntropyDiscretization(0),

      pssmDiscretization(4), pssmEntropyDiscretization(6),
      ss3Discretization(9), ss3EntropyDiscretization(2),
      ss8Discretization(12), ss8EntropyDiscretization(4),
      stalDiscretization(2), stalEntropyDiscretization(5),
      sa20Discretization(7),
      drDiscretization(7),

      dsbDiscretization(5), dsbNormalizedDiscretization(3),
      dsbEntropyDiscretization(5),
      dsbPairWindowRows(7), dsbPairWindowColumns(7),
      dsbCartesianCbpSize(7), dsbCartesianCbsSize(3),

      cbsWindowSize(9), cbsDiscretization(5),
      cbsSeparationProfilSize(7), cbsSeparationProfilDiscretization(5),
      cbsRatioDiscretization(5),
      useCartesianCBPvsCBS(true),

      residueGlobalMeanFeatures(true), residueWindowSize(9),
      residueLocalMeanSize(18), residueMediumMeanSize(90),
      
      // residue pair
      aminoAcidDistanceDiscretization(5), useIntervalMean(false),
      cartesianProductPrimaryWindowSize(0)
  {
  }

  // protein
  size_t proteinLengthDiscretization;
  size_t numCysteinsDiscretization;
  size_t bondingPropertyDiscretization;
  size_t bondingPropertyEntropyDiscretization;
  
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
  size_t dsbEntropyDiscretization;
  size_t dsbPairWindowRows;
  size_t dsbPairWindowColumns;
  size_t dsbCartesianCbpSize;
  size_t dsbCartesianCbsSize;
  
  // cbs
  size_t cbsWindowSize;
  size_t cbsDiscretization;
  size_t cbsSeparationProfilSize;
  size_t cbsSeparationProfilDiscretization;
  size_t cbsRatioDiscretization;
  bool useCartesianCBPvsCBS;

  // global
  bool residueGlobalMeanFeatures;
  size_t residueWindowSize;
  size_t residueLocalMeanSize;
  size_t residueMediumMeanSize;

  // pair
  size_t aminoAcidDistanceDiscretization;
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

ProteinPredictorParametersPtr numericalCysteinPredictorParameters();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PREDICTOR_PARAMETERS_H_
