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
# include "ProteinPerceptionComponent.h"
# include <lbcpp/UserInterface/VariableSelector.h>
# include "../Predictor/ProteinPredictor.h"

# include "../../../src/Function/Evaluator/Utilities.h"
# include "../../../src/UserInterface/Component/ContainerCurveEditor.h"

namespace lbcpp
{

class ROCScoreObjectComponent : public ContainerCurveEditor
{
public:
  ROCScoreObjectComponent(ROCScoreObjectPtr rocScore, const String& name)
    : ContainerCurveEditor(defaultExecutionContext(), rocScore->createROCCurveElements(), createCurveEditorConfiguration())
  {
  }

protected:
  ContainerCurveEditorConfigurationPtr createCurveEditorConfiguration() const
  {
    ContainerCurveEditorConfigurationPtr res = new ContainerCurveEditorConfiguration(rocScoreObjectElementClass);
    res->setXAxis(new CurveAxisConfiguration(0.0, 1.0, T("False positive rate"), false));
    res->setYAxis(new CurveAxisConfiguration(0.0, 1.0, T("True positive rate"), false));
    return res;
  }
};

class ConfusionMatrixComponent : public juce::Component, public ComponentWithPreferedSize
{
public:
  ConfusionMatrixComponent(BinaryClassificationConfusionMatrixPtr confusionMatrix, const String& name)
    : confusionMatrix(confusionMatrix)
  {}

  virtual int getDefaultWidth() const
    {return 300;}

  virtual int getDefaultHeight() const
    {return 300;}

  virtual void paint(juce::Graphics& g)
  {
    const int width = getWidth();
    const int center = width / 2;
    const int cellWidth = 100;
    const int cellHeight = 50;
    
    const Font defaultFont = g.getCurrentFont();
    
    size_t yCursor = 20;
    g.setFont(defaultFont.getHeight() + 4, Font::bold);
    g.drawText(T("Confusion Matrix"), 0, yCursor, width, 20, Justification::horizontallyCentred, true);
    g.setFont(defaultFont);
    yCursor += 30;

    g.drawText(T("Actual value"), center - cellWidth, yCursor, cellWidth * 2, 20, Justification::horizontallyCentred, true);
    yCursor += 20;
    g.drawText(T("true"), center - cellWidth, yCursor, cellWidth, 20, Justification::horizontallyCentred, true);
    g.drawText(T("false"), center, yCursor, cellWidth, 20, Justification::horizontallyCentred, true);
    yCursor += 20;

    g.drawTextAsPath(T("Outcome"), juce::AffineTransform::rotation(-M_PI / 2.).translated(center - cellWidth - 10, yCursor + cellHeight * 1.5));
    g.drawTextAsPath(T("   true"), juce::AffineTransform::rotation(M_PI / 2.).translated(center + cellWidth + 10, yCursor));
    g.drawTextAsPath(T("   false"), juce::AffineTransform::rotation(M_PI / 2.).translated(center + cellWidth + 10, yCursor + cellHeight));

    g.drawRect(center - cellWidth, yCursor, cellWidth, cellHeight);
    g.drawText(String((int)confusionMatrix->getTruePositives()), center - cellWidth, yCursor, cellWidth, cellHeight, Justification::centred, true);
    g.drawRect(center - 1, yCursor, cellWidth, cellHeight);
    g.drawText(String((int)confusionMatrix->getFalsePositives()), center, yCursor, cellWidth, cellHeight, Justification::centred, true);
    yCursor += cellHeight;
    g.drawRect(center - cellWidth, yCursor - 1, cellWidth, cellHeight);
    g.drawText(String((int)confusionMatrix->getFalseNegatives()), center - cellWidth, yCursor, cellWidth, cellHeight, Justification::centred, true);
    g.drawRect(center - 1, yCursor - 1, cellWidth, cellHeight);
    g.drawText(String((int)confusionMatrix->getTrueNegatives()), center, yCursor, cellWidth, cellHeight, Justification::centred, true);
    yCursor += cellHeight;

    const int labelWidth = 150;
    const int valueWidth = 50;
    const int rowCenter = center + valueWidth;

    yCursor += 10;
    g.setFont(defaultFont.getHeight() + 4, Font::bold);
    g.drawText(T("Measures"), 0, yCursor, width, 20, Justification::horizontallyCentred, true);
    g.setFont(defaultFont);
    yCursor += 30;
    g.drawText(T("Accuracy "), rowCenter - labelWidth, yCursor, labelWidth, 20, Justification::left, true);
    g.drawText(String(confusionMatrix->computeAccuracy(), 4), rowCenter, yCursor, valueWidth, 20, Justification::horizontallyCentred, true);    
    yCursor += 20;
    g.drawText(T("F1 Score"), rowCenter - labelWidth, yCursor, labelWidth, 20, Justification::left, true);
    g.drawText(String(confusionMatrix->computeF1Score(), 4), rowCenter, yCursor, valueWidth, 20, Justification::horizontallyCentred, true);    
    yCursor += 20;
    g.drawText(T("Precision"), rowCenter - labelWidth, yCursor, labelWidth, 20, Justification::left, true);
    g.drawText(String(confusionMatrix->computePrecision(), 4), rowCenter, yCursor, valueWidth, 20, Justification::horizontallyCentred, true);    
    yCursor += 20;
    g.drawText(T("Recall (Sensitivity)"), rowCenter - labelWidth, yCursor, labelWidth, 20, Justification::left, true);
    g.drawText(String(confusionMatrix->computeRecall(), 4), rowCenter, yCursor, valueWidth, 20, Justification::horizontallyCentred, true);    
    yCursor += 20;
    g.drawText(T("Specificity"), rowCenter - labelWidth, yCursor, labelWidth, 20, Justification::left, true);
    g.drawText(String(confusionMatrix->computeSpecificity(), 4), rowCenter, yCursor, valueWidth, 20, Justification::horizontallyCentred, true);    
    yCursor += 20;
    g.drawText(T("Matthews Correlation"), rowCenter - labelWidth, yCursor, labelWidth, 20, Justification::left, true);
    g.drawText(String(confusionMatrix->computeMatthewsCorrelation(), 4), rowCenter, yCursor, valueWidth, 20, Justification::horizontallyCentred, true);    
  }

protected:
  BinaryClassificationConfusionMatrixPtr confusionMatrix;
};

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
    {
      addTab(T("Data"), Colours::white);
      //addTab(T("Perception"), Colours::white);
    }
    addTab(T("Protein 1D"), Colours::white);
    addTab(T("Protein 2D"), Colours::white);
    
    if (proteins.size() == 1)
    {
      addTab(T("Residue Features"), Colours::white);
      addTab(T("Residue Pair Features"), Colours::red);
      addTab(T("Disulfide Pair Features"), Colours::red);
      addTab(T("Cystein BS Features"), Colours::red);
      addTab(T("Compute Missing"), Colours::pink);
    }
  }

  virtual Component* createComponentForVariable(ExecutionContext& context, const Variable& variable, const String& tabName)
  {
    ClassPtr proteinClass = lbcpp::proteinClass;

    if (tabName == T("Data"))
      return userInterfaceManager().createVariableTreeView(context, proteins[0], names[0]);
//    else if (tabName == T("Perception"))
//      return new ProteinPerceptionComponent(proteins[0]);
    else if (tabName == T("Compute Missing"))
    {
      const size_t n = proteinClass->getNumMemberVariables();
      for (size_t i = 0; i < n; ++i)
        proteins[0]->getTargetOrComputeIfMissing(context, i);
      return userInterfaceManager().createVariableTreeView(context, proteins[0], names[0]);
    }
    else if (tabName == T("Cystein BS Features"))
    {
      NumericalProteinFeaturesParametersPtr featuresParameters = new NumericalProteinFeaturesParameters();
      ProteinPredictorParametersPtr predictorParameters = numericalProteinPredictorParameters(featuresParameters, new StochasticGDParameters());
      
      FunctionPtr proteinfunction = predictorParameters->createProteinPerception();
      proteinfunction->initialize(context, (TypePtr)proteinClass);
      Variable proteinPerception = proteinfunction->compute(context, proteins[0]);
      
      FunctionPtr residuefunction = predictorParameters->createCysteinBondingStateVectorPerception();
      residuefunction->initialize(context, proteinfunction->getOutputType());
      Variable description = residuefunction->compute(context, proteinPerception);
      
      return userInterfaceManager().createVariableTreeView(context, description);
    }
    else if (tabName == T("Disulfide Pair Features"))
    {
      NumericalProteinFeaturesParametersPtr featuresParameters = new NumericalProteinFeaturesParameters();
      ProteinPredictorParametersPtr predictorParameters = numericalProteinPredictorParameters(featuresParameters, new StochasticGDParameters());
      
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
      NumericalProteinFeaturesParametersPtr featuresParameters = new NumericalProteinFeaturesParameters();
     /* featuresParameters->pssmDiscretization = 1;
      featuresParameters->pssmEntropyDiscretization = 5;
      featuresParameters->ss3Discretization = 1;
      featuresParameters->ss8Discretization = 2;
      featuresParameters->drDiscretization = 5;
      featuresParameters->sa20Discretization = 1;

      featuresParameters->residueWindowSize = 5;
      featuresParameters->residueGlobalFeatures = false;
      featuresParameters->residueLocalMeanSize = 8;*/

      ProteinPredictorParametersPtr predictorParameters = numericalProteinPredictorParameters(featuresParameters, new StochasticGDParameters());

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

