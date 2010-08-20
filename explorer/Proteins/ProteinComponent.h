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

  virtual Component* createComponentForVariable(const Variable& variable, const String& name)
  {
    ClassPtr proteinClass = lbcpp::proteinClass();
    jassert(variable.getType()->canBeCastedTo(pairType(anyType(), stringType())));
    String tabName = variable[1].getString();
    return new VariableTreeComponent(getPerception(tabName)->compute(input), name + T(" ") + tabName);
  }

protected:
  Variable input;
  PerceptionPtr perception;

  PerceptionPtr getPerception(const String& tabName) const
  {
    if (tabName == T("Attributes"))
      return perception->flatten();
    else if (tabName == T("Features"))
      return PerceptionPtr(new ConvertToFeaturesPerception(perception));
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
    ProteinInferenceFactory factory;
    perception = factory.createResiduePerception(String::empty);
  }
};

class ProteinPerceptionComponent : public PerceptionComponent
{
public:
  ProteinPerceptionComponent(const Variable& protein)
    : PerceptionComponent(protein)
  {
    ProteinInferenceFactory factory;
    perception = factory.createProteinPerception();
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
  
  virtual Component* createComponentForVariable(const Variable& variable, const String& name)
  {
    ClassPtr proteinClass = lbcpp::proteinClass();

    jassert(variable.getType()->canBeCastedTo(pairType(anyType(), stringType())));
    String tabName = variable[1].getString();
    if (tabName == T("Data"))
      return new VariableTreeComponent(proteins[0], names[0]);
    else if (tabName == T("Perception"))
      return new ProteinPerceptionComponent(proteins[0]);
    else if (tabName == T("Protein 1D"))
    {
      std::vector< std::pair<String, size_t> > sequenceIndex;
      size_t n = proteinClass->getNumStaticVariables();
      for (size_t i = 0; i < n; ++i)
      {
        TypePtr type = proteinClass->getStaticVariableType(i);
        if (type->inheritsFrom(vectorClass(anyType())))
        {
          String friendlyName = Protein::getTargetFriendlyName(i);
          if (friendlyName.contains(T("Distance Map")) || friendlyName.contains(T("Contact Map")))
            continue; // FIXME: bug in Type::inheritsFrom when using template types

          addObjectNameIfExists(friendlyName, i, sequenceIndex);
        }
      }

      MultiProtein1DConfigurationPtr configuration = new MultiProtein1DConfiguration(names, sequenceIndex);
      return new MultiProtein1DComponent(proteins, configuration);
    }
    else if (tabName == T("Protein 2D"))
    {
      std::vector< std::pair<String, size_t> > mapIndex;
      addObjectNameIfExists(T("Ca 8 angstrom"), proteinClass->findStaticVariable(T("contactMap8Ca")), mapIndex);
      addObjectNameIfExists(T("Cb 8 angstrom"), proteinClass->findStaticVariable(T("contactMap8Cb")), mapIndex);
      
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
      if (proteins[i]->getVariable(variableIndex))
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

