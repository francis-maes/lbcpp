/*-----------------------------------------.---------------------------------.
| Filename: ProteinPerceptionComponent.h   | Components for protein          |
| Author  : Francis Maes                   |   perceptions                   |
| Started : 16/12/2010 16:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_USER_INTERFACE_PROTEIN_PERCEPTION_H_
# define LBCPP_PROTEINS_USER_INTERFACE_PROTEIN_PERCEPTION_H_

# include <lbcpp/UserInterface/VariableSelector.h>

namespace lbcpp
{

class PerceptionComponent : public TabbedVariableSelectorComponent
{
public:
  PerceptionComponent(const Variable& input, const String& name)
    : TabbedVariableSelectorComponent(input), input(input)
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
    return userInterfaceManager().createVariableTreeView(context, getPerception(tabName)->computeFunction(context, input), name + T(" ") + tabName);
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
  ResiduePerceptionComponent(const PairPtr& proteinAndPosition, const String& name = String::empty)
    : PerceptionComponent(proteinAndPosition, name)
  {
    ProteinInferenceFactory factory(defaultExecutionContext());
    perception = factory.createResiduePerception(String::empty);
  }
};

class ResiduePairPerceptionComponent : public PerceptionComponent
{
public:
  ResiduePairPerceptionComponent(const PairPtr& proteinAndPositions, const String& name = String::empty)
    : PerceptionComponent(proteinAndPositions, name)
  {
    ProteinInferenceFactory factory(defaultExecutionContext());
    perception = factory.createResiduePairPerception(String::empty);
  }
};

class ProteinPerceptionComponent : public PerceptionComponent
{
public:
  ProteinPerceptionComponent(const ProteinPtr& protein, const String& name = String::empty)
    : PerceptionComponent(protein, name)
  {
    ProteinInferenceFactory factory(defaultExecutionContext());
    perception = factory.createProteinPerception(String::empty);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_USER_INTERFACE_PROTEIN_PERCEPTION_H_

