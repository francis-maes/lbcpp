/*-----------------------------------------.---------------------------------.
| Filename: ProteinComponent.h             | Components for proteins         |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 18:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_PROTEIN_COMPONENT_H_
# define EXPLORER_COMPONENTS_PROTEIN_COMPONENT_H_

# include "MultiProtein1DComponent.h"
# include "MultiProtein2DComponent.h"
# include "../Utilities/VariableSelector.h"
# include "../Components/VariableTreeComponent.h"

namespace lbcpp
{

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

class PerceptionComponent : public TabbedVariableSelectorComponent
{
public:
  PerceptionComponent(const Variable& input, PerceptionPtr perception = PerceptionPtr())
    : TabbedVariableSelectorComponent(input), input(input), perception(perception)
  {
    addTab(T("Perception"), Colours::white);
    addTab(T("Attributes"), Colours::white);
    addTab(T("Features"), Colours::white);
  }

  virtual Component* createComponentForVariable(ExecutionContext& context, const Variable& variable, const String& name)
  {
    ClassPtr proteinClass = lbcpp::proteinClass;
    jassert(variable.getType()->canBeCastedTo(pairClass(anyType, stringType)));
    String tabName = variable[1].getString();
    VariableTreeOptions options;
    options.showMissingVariables = false;
    return new VariableTreeComponent(getPerception(tabName)->computeFunction(context, input), name + T(" ") + tabName, options);
  }

protected:
  Variable input;
  PerceptionPtr perception;

  std::vector<size_t> makeBinaryConjunction(size_t index1, size_t index2) const
    {std::vector<size_t> res(2); res[0] = index1; res[1] = index2; return res;}

  PerceptionPtr getPerception(const String& tabName) const
  {
    if (tabName == T("Attributes"))
      return flattenPerception(perception);
    else if (tabName == T("Features"))
    {
      return flattenPerception(perceptionToFeatures(perception));
      /*
      PerceptionPtr collapsedFeatures = collapsePerception(perceptionToFeatures(perception));

      std::vector< std::vector<size_t> > selectedConjunctions;
      for (size_t i = 0; i < collapsedFeatures->getNumOutputVariables(); ++i)
        selectedConjunctions.push_back(std::vector<size_t>(1, i));

      selectedConjunctions.push_back(makeBinaryConjunction(0, 1));
      selectedConjunctions.push_back(makeBinaryConjunction(5, 10));
      selectedConjunctions.push_back(makeBinaryConjunction(10, 15));

      return selectAndMakeConjunctionFeatures(collapsedFeatures, selectedConjunctions);*/
    }
    else
      return perception;
  }
};

class ResiduePerceptionComponent : public PerceptionComponent
{
public:
  ResiduePerceptionComponent(const Variable& proteinAndPosition)
    : PerceptionComponent(proteinAndPosition)
  {
    ProteinInferenceFactory factory(*silentExecutionContext);
    perception = factory.createResiduePerception(String::empty);
  }
};

class ResiduePairPerceptionComponent : public PerceptionComponent
{
public:
  ResiduePairPerceptionComponent(const Variable& proteinAndPositions)
    : PerceptionComponent(proteinAndPositions)
  {
    ProteinInferenceFactory factory(*silentExecutionContext);
    perception = factory.createResiduePairPerception(String::empty);
  }
};

class ProteinPerceptionComponent : public PerceptionComponent
{
public:
  ProteinPerceptionComponent(const Variable& protein)
    : PerceptionComponent(protein)
  {
    ProteinInferenceFactory factory(*silentExecutionContext);
    perception = factory.createProteinPerception(String::empty);
  }
};

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

class MultiProteinComponent : public TabbedVariableSelectorComponent
{
public:
  MultiProteinComponent(const std::vector<ProteinPtr>& proteins, const std::vector<String>& names)
    : TabbedVariableSelectorComponent(proteins.size() == 1 ? proteins[0] : Variable()), proteins(proteins), names(names)
  {
    if (proteins.size() == 1)
    {
      addTab(T("Data"), Colours::white);
      addTab(T("Perception"), Colours::white);
    }
    addTab(T("Protein 1D"), Colours::white);
    addTab(T("Protein 2D"), Colours::white);
  }
  
  virtual Component* createComponentForVariable(ExecutionContext& context, const Variable& variable, const String& name)
  {
    ClassPtr proteinClass = lbcpp::proteinClass;

    jassert(variable.getType()->canBeCastedTo(pairClass(anyType, stringType)));
    String tabName = variable[1].getString();
    if (tabName == T("Data"))
      return new VariableTreeComponent(proteins[0], names[0]);
    else if (tabName == T("Perception"))
      return new ProteinPerceptionComponent(proteins[0]);
    else if (tabName == T("Protein 1D"))
    {
      std::vector< std::pair<String, size_t> > sequenceIndex;
      size_t n = proteinClass->getObjectNumVariables();
      for (size_t i = 0; i < n; ++i)
      {
        TypePtr type = proteinClass->getObjectVariableType(i);
        if (type->inheritsFrom(genericVectorClass(anyType))
         || type->inheritsFrom(objectVectorClass(discreteProbabilityDistributionClass(anyType))))
        {
          String friendlyName = proteinClass->getObjectVariableDescription(i);
          addObjectNameIfExists(friendlyName, i, sequenceIndex);
        }
      }

      MultiProtein1DConfigurationPtr configuration = new MultiProtein1DConfiguration(names, sequenceIndex);
      return new MultiProtein1DComponent(proteins, configuration);
    }
    else if (tabName == T("Protein 2D"))
    {
      std::vector< std::pair<String, size_t> > mapIndex;
      addObjectNameIfExists(T("Ca 8 angstrom"), proteinClass->findObjectVariable(T("contactMap8Ca")), mapIndex);
      addObjectNameIfExists(T("Cb 8 angstrom"), proteinClass->findObjectVariable(T("contactMap8Cb")), mapIndex);
      
      MultiProtein2DConfigurationPtr configuration = new MultiProtein2DConfiguration(names, mapIndex);
      return new MultiProtein2DComponent(proteins, configuration);
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
      if (proteins[i]->getTargetOrComputeIfMissing(variableIndex).exists())
      {
        res.push_back(std::make_pair(friendlyName, variableIndex));
        return;
      }
  }  
};

class ProteinComponent : public MultiProteinComponent
{
public:
  ProteinComponent(ProteinPtr protein, const String& name)
    : MultiProteinComponent(std::vector<ProteinPtr>(1, protein), std::vector<String>(1, name)) {}
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_PROTEIN_COMPONENT_H_

