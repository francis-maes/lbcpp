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

namespace lbcpp
{

class MultiProteinComponent : public TabbedComponent
{
public:
  MultiProteinComponent(const std::vector< std::pair<String, ProteinPtr> >& proteins)
    : TabbedComponent(TabbedButtonBar::TabsAtBottom), proteins(proteins)
  {
    std::vector<ProteinPtr> proteinsAlone(proteins.size());
    std::vector<String> proteinNames(proteins.size());
    for (size_t i = 0; i < proteinNames.size(); ++i)
    {
      proteinNames[i] = proteins[i].first;
      proteinsAlone[i] = proteins[i].second;
    }

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

      MultiProtein1DConfigurationPtr configuration = new MultiProtein1DConfiguration(proteinNames, sequenceIndex);
      addTab(T("Protein 1D"), Colours::white, new MultiProtein1DComponent(proteinsAlone, configuration), true);
    }
    
    /*
    ** 2D
    */
    {
      std::vector< std::pair<String, size_t> > mapIndex;
      addObjectNameIfExists(T("Ca 8 angstrom"), proteinClass()->findStaticVariable(T("contactMap8Ca")), proteins, mapIndex);
      addObjectNameIfExists(T("Cb 8 angstrom"), proteinClass()->findStaticVariable(T("contactMap8Cb")), proteins, mapIndex);
      
      MultiProtein2DConfigurationPtr configuration = new MultiProtein2DConfiguration(proteinNames, mapIndex);
      addTab(T("Protein 2D"), Colours::white, new MultiProtein2DComponent(proteinsAlone, configuration), true);
    }
  }
    
  juce_UseDebuggingNewOperator

private:
  std::vector< std::pair<String, ProteinPtr> > proteins;

  void addObjectNameIfExists(const String& friendlyName, size_t variableIndex, const std::vector< std::pair<String, ProteinPtr> >& proteinPairs, std::vector< std::pair<String, size_t> >& res)
  {
    for (size_t i = 0; i < proteinPairs.size(); ++i)
      if (proteinPairs[i].second->getVariable(variableIndex))
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
    : MultiProteinComponent(std::vector< std::pair<String, ProteinPtr> >(1, std::make_pair(name, protein))) {}
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_PROTEIN_COMPONENT_H_

