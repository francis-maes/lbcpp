/*-----------------------------------------.---------------------------------.
| Filename: ProteinComponent.h             | Components for proteins         |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 18:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_USER_INTERFACE_PROTEIN_H_
# define LBCPP_PROTEINS_USER_INTERFACE_PROTEIN_H_

# include "MultiProtein1DComponent.h"
# include "MultiProtein2DComponent.h"
# include <lbcpp/UserInterface/VariableSelector.h>
# include "../Predictor/ProteinPredictor.h"
# include "../Predictor/NumericalCysteinPredictorParameters.h"
# include "../Predictor/Lin09PredictorParameters.h"
# include "../Predictor/LargeProteinPredictorParameters.h"

namespace lbcpp
{

class MultiProteinComponent : public TabbedVariableSelectorComponent
{
public:
  MultiProteinComponent(const std::vector<ProteinPtr>& proteins, const std::vector<String>& names)
    : TabbedVariableSelectorComponent(proteins.size() == 1 ? proteins[0] : Variable()), proteins(proteins), names(names)
    {initializeTabs();}
  
  MultiProteinComponent(ContainerPtr container, const String& name)
    : TabbedVariableSelectorComponent(container)
  {
    jassert(container->getElementsType()->inheritsFrom(proteinClass));
    size_t n = container->getNumElements();
    proteins.reserve(n);
    names.reserve(n);
    size_t length = (size_t)-1;
    for (size_t i = 0; i < n; ++i)
    {
      ObjectPtr object = container->getElement(i).getObject();
      ProteinPtr protein = object.dynamicCast<Protein>();
      jassert(protein);

      if (i == 0)
        length = protein->getLength();
      else if (protein->getLength() != length)
        continue;

      proteins.push_back(protein);
      names.push_back(String((int)i) + T(" - ") + protein->getName());
    }
    initializeTabs();
  }
      
  void initializeTabs()
  {
    if (proteins.size() == 1)
      addTab(T("Data"), Colours::white);

    addTab(T("Protein 1D"), Colours::white);
    addTab(T("Protein 2D"), Colours::white);
    
    if (proteins.size() == 1)
    {
      addTab(T("Residue Features"), Colours::white);
      addTab(T("Residue Pair Features"), Colours::red);
      addTab(T("Cystein 0D"), Colours::white);
      addTab(T("Cystein 1D"), Colours::white);
      addTab(T("Cystein 2D"), Colours::white);
      addTab(T("Compute Missing"), Colours::pink);
    }
  }

  virtual Component* createComponentForVariable(ExecutionContext& context, const Variable& variable, const String& tabName)
  {
    ClassPtr proteinClass = lbcpp::proteinClass;

    if (tabName == T("Data"))
      return userInterfaceManager().createVariableTreeView(context, proteins[0], names[0]);
    else if (tabName == T("Compute Missing"))
    {
      const size_t n = proteinClass->getNumMemberVariables();
      for (size_t i = 0; i < n; ++i)
        proteins[0]->getTargetOrComputeIfMissing(context, i);
      return userInterfaceManager().createVariableTreeView(context, proteins[0], names[0]);
    }
    else if (tabName == T("Cystein 0D"))
    {
      ProteinPredictorParametersPtr predictorParameters = numericalCysteinPredictorParameters();

      FunctionPtr proteinfunction = predictorParameters->createProteinPerception();
      proteinfunction->initialize(context, (TypePtr)proteinClass);
      Variable proteinPerception = proteinfunction->compute(context, proteins[0]);

      FunctionPtr residuefunction = predictorParameters->createGlobalPerception();
      residuefunction->initialize(context, proteinfunction->getOutputType());
      Variable description = residuefunction->compute(context, proteinPerception);
      
      return userInterfaceManager().createVariableTreeView(context, description);
    }
    else if (tabName == T("Cystein 1D"))
    {
      ProteinPredictorParametersPtr predictorParameters = numericalCysteinPredictorParameters();
      
      FunctionPtr proteinfunction = predictorParameters->createProteinPerception();
      proteinfunction->initialize(context, (TypePtr)proteinClass);
      Variable proteinPerception = proteinfunction->compute(context, proteins[0]);
      
      FunctionPtr residuefunction = predictorParameters->createCysteinBondingStateVectorPerception();
      residuefunction->initialize(context, proteinfunction->getOutputType());
      Variable description = residuefunction->compute(context, proteinPerception);
      
      return userInterfaceManager().createVariableTreeView(context, description);
    }
    else if (tabName == T("Cystein 2D"))
    {
      Lin09ParametersPtr lin09 = new Lin09Parameters();
      lin09->pssmWindowSize = 15;
      lin09->separationProfilSize = 9;
      lin09->usePositionDifference = true;
      lin09->pssmLocalHistogramSize = 100;
      
      Lin09PredictorParametersPtr lin09Pred = new Lin09PredictorParameters(lin09);
      lin09Pred->useLibSVM = false;
      lin09Pred->useLaRank = false;
      lin09Pred->useLibLinear = false;
      lin09Pred->useAddBias = true;
      
      lin09Pred->C = 1.4;
      lin09Pred->kernelGamma = -4.6;
      
      ProteinPredictorParametersPtr predictorParameters = lin09Pred;

      FunctionPtr proteinfunction = predictorParameters->createProteinPerception();
      proteinfunction->initialize(context, (TypePtr)proteinClass);
      Variable proteinPerception = proteinfunction->compute(context, proteins[0]);
      
      FunctionPtr residuefunction = predictorParameters->createDisulfideResiduePairVectorPerception();
      residuefunction->initialize(context, proteinfunction->getOutputType());
      Variable description = residuefunction->compute(context, proteinPerception);
      
      return userInterfaceManager().createVariableTreeView(context, description);
    }
    else if (tabName == T("Residue Pair Features"))
    {
      NumericalProteinFeaturesParametersPtr featuresParameters = new NumericalProteinFeaturesParameters();
      ProteinPredictorParametersPtr predictorParameters = numericalProteinPredictorParameters(featuresParameters, new StochasticGDParameters());
      
      FunctionPtr proteinfunction = predictorParameters->createProteinPerception();
      proteinfunction->initialize(context, (TypePtr)proteinClass);
      Variable proteinPerception = proteinfunction->compute(context, proteins[0]);
      
      FunctionPtr residuefunction = predictorParameters->createResiduePairVectorPerception();
      residuefunction->initialize(context, proteinfunction->getOutputType());
      Variable description = residuefunction->compute(context, proteinPerception);

      return userInterfaceManager().createVariableTreeView(context, description);
    }
    else if (tabName == T("Residue Features"))
    {
      LargeProteinParametersPtr featureParameters = new LargeProteinParameters();
      featureParameters->useProteinLength = true;
      featureParameters->useNumCysteins = true;

      featureParameters->useAminoAcidGlobalHistogram = true;
      featureParameters->usePSSMGlobalHistogram = true;
      featureParameters->useSS3GlobalHistogram = true;
      featureParameters->useSS8GlobalHistogram = true;
      featureParameters->useSAGlobalHistogram = true;
      featureParameters->useDRGlobalHistogram = true;
      featureParameters->useSTALGlobalHistogram = true;

      featureParameters->aminoAcidWindowSize = 10;
      featureParameters->pssmWindowSize = 10;
      featureParameters->ss3WindowSize = 10;
      featureParameters->ss8WindowSize = 10;
      featureParameters->saWindowSize = 10;
      featureParameters->drWindowSize = 10;
      featureParameters->stalWindowSize = 10;

      featureParameters->aminoAcidLocalHistogramSize = 10;
      featureParameters->pssmLocalHistogramSize = 10;
      featureParameters->ss3LocalHistogramSize = 10;
      featureParameters->ss8LocalHistogramSize = 10;
      featureParameters->saLocalHistogramSize = 10;
      featureParameters->drLocalHistogramSize = 10;
      featureParameters->stalLocalHistogramSize = 10;

      LargeProteinPredictorParametersPtr predictorParameters = new LargeProteinPredictorParameters(featureParameters, true);
      
      FunctionPtr proteinfunction = predictorParameters->createProteinPerception();
      proteinfunction->initialize(context, (TypePtr)proteinClass);
      Variable proteinPerception = proteinfunction->compute(context, proteins[0]);
      
      FunctionPtr residuefunction = predictorParameters->createResidueVectorPerception();
      residuefunction->initialize(context, proteinfunction->getOutputType());
      Variable description = residuefunction->compute(context, proteinPerception);

      return userInterfaceManager().createVariableTreeView(context, description);
    }
    else if (tabName == T("Protein 1D"))
    {
      std::vector< std::pair<String, size_t> > sequenceIndex;
      size_t n = proteinClass->getNumMemberVariables();
      for (size_t i = 0; i < n; ++i)
      {
        TypePtr type = proteinClass->getMemberVariableType(i);
        if (type->inheritsFrom(doubleVectorClass(enumValueType, probabilityType))
            || type->inheritsFrom(objectVectorClass(doubleVectorClass()))
            || type->inheritsFrom(genericVectorClass(aminoAcidTypeEnumeration)))
        {
          String friendlyName = proteinClass->getMemberVariableDescription(i);
          addObjectNameIfExists(friendlyName, i, sequenceIndex);
        }
      }

      MultiProtein1DConfigurationPtr configuration = new MultiProtein1DConfiguration(names, sequenceIndex);
      return new MultiProtein1DComponent(context, proteins, configuration);
    }
    else if (tabName == T("Protein 2D"))
    {
      std::vector< std::pair<String, size_t> > mapIndex;
      addObjectNameIfExists(T("Ca 8 angstrom"), proteinClass->findMemberVariable(T("contactMap8Ca")), mapIndex);
      addObjectNameIfExists(T("Cb 8 angstrom"), proteinClass->findMemberVariable(T("contactMap8Cb")), mapIndex);
      addObjectNameIfExists(T("Disulfide Bonds"), proteinClass->findMemberVariable(T("disulfideBonds")), mapIndex);
      
      MultiProtein2DConfigurationPtr configuration = new MultiProtein2DConfiguration(names, mapIndex);
      return new MultiProtein2DComponent(context, proteins, configuration);
    }

    jassert(false);
    return NULL;
  }

  juce_UseDebuggingNewOperator

protected:
  std::vector<ProteinPtr> proteins;
  std::vector<String> names;

  void addObjectNameIfExists(const String& friendlyName, size_t variableIndex, std::vector< std::pair<String, size_t> >& res)
  {
    for (size_t i = 0; i < proteins.size(); ++i)
      if (proteins[i]->getTargetOrComputeIfMissing(defaultExecutionContext(), variableIndex).exists())
      {
        res.push_back(std::make_pair(friendlyName, variableIndex));
        return;
      }
  }  
};

class ProteinComponent : public MultiProteinComponent
{
public:
  ProteinComponent(ProteinPtr protein, const String& name = String::empty)
    : MultiProteinComponent(std::vector<ProteinPtr>(1, protein), std::vector<String>(1, name.isEmpty() ? protein->getName() : name)) {}
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_USER_INTERFACE_PROTEIN_H_

