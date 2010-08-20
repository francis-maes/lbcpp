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

class MultiProteinComponent : public TabbedVariableSelectorComponent
{
public:
  MultiProteinComponent(const std::vector<ProteinPtr>& proteins, const std::vector<String>& names)
    : proteins(proteins), names(names)
  {
    if (proteins.size() == 1)
      addTab(T("Protein Data"), Colours::white);
    addTab(T("Protein 1D"), Colours::white);
    addTab(T("Protein 2D"), Colours::white);
  }
  
  virtual Component* createComponentForVariable(const Variable& variable, const String& name)
  {
    ClassPtr proteinClass = lbcpp::proteinClass();

    String tabName = variable.getString();
    if (tabName == T("Protein Data"))
      return new VariableTreeComponent(proteins[0], names[0]);
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

#if 0
class MultiProteinComponent : public TabbedComponent
{
public:
  MultiProteinComponent(const std::vector<ProteinPtr>& proteins, const std::vector<String>& names)
    : TabbedComponent(TabbedButtonBar::TabsAtBottom)
  {
    /*
    ** 1D
    */
    {
      std::vector< std::pair<String, size_t> > sequenceIndex;
      ClassPtr proteinClass = lbcpp::proteinClass();
      size_t n = proteinClass->getNumStaticVariables();
      for (size_t i = 0; i < n; ++i)
      {
        TypePtr type = proteinClass->getStaticVariableType(i);
        if (type->inheritsFrom(vectorClass(anyType())))
        {
          String friendlyName = Protein::getTargetFriendlyName(i);
          if (friendlyName.contains(T("Distance Map")) || friendlyName.contains(T("Contact Map")))
            continue; // FIXME: bug in Type::inheritsFrom when using template types

          addObjectNameIfExists(friendlyName, i, proteins, sequenceIndex);
        }
      }

      MultiProtein1DConfigurationPtr configuration = new MultiProtein1DConfiguration(names, sequenceIndex);
      
      Component* protein1dComponent = new MultiProtein1DComponent(proteins, configuration);
      
      Variable variable;
      if (proteins.size() == 1)
        variable = Variable(proteins[0]);

      Component* tabComponent = /*new VariableSelectorAndContentComponent(variable,*/ protein1dComponent;
      addTab(T("Protein 1D"), Colours::white, tabComponent, true);
    }
    
    /*
    ** 2D
    */
    {
      std::vector< std::pair<String, size_t> > mapIndex;
      addObjectNameIfExists(T("Ca 8 angstrom"), proteinClass()->findStaticVariable(T("contactMap8Ca")), proteins, mapIndex);
      addObjectNameIfExists(T("Cb 8 angstrom"), proteinClass()->findStaticVariable(T("contactMap8Cb")), proteins, mapIndex);
      
      MultiProtein2DConfigurationPtr configuration = new MultiProtein2DConfiguration(names, mapIndex);
      addTab(T("Protein 2D"), Colours::white, new MultiProtein2DComponent(proteins, configuration), true);
    }
  }
    
  juce_UseDebuggingNewOperator

private:
  void addObjectNameIfExists(const String& friendlyName, size_t variableIndex, const std::vector<ProteinPtr>& proteins, std::vector< std::pair<String, size_t> >& res)
  {
    for (size_t i = 0; i < proteins.size(); ++i)
      if (proteins[i]->getVariable(variableIndex))
      {
        res.push_back(std::make_pair(friendlyName, variableIndex));
        return;
      }
  }  
};
#endif // 0

class ProteinComponent : public MultiProteinComponent
{
public:
  ProteinComponent(ProteinPtr protein, const String& name)
    : MultiProteinComponent(std::vector<ProteinPtr>(1, protein), std::vector<String>(1, name)) {}
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_PROTEIN_COMPONENT_H_

