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
  MultiProteinComponent(const std::vector< std::pair<String, ProteinPtr> >& proteinPairs)
    : TabbedComponent(TabbedButtonBar::TabsAtBottom), proteinPairs(proteinPairs)
  {
    std::vector<ProteinPtr> proteins(proteinPairs.size());
    std::vector<String> proteinNames(proteinPairs.size());
    for (size_t i = 0; i < proteinNames.size(); ++i)
    {
      proteinNames[i] = proteinPairs[i].first;
      proteins[i] = proteinPairs[i].second;
    }

    /*
    ** 1D
    */
    {
      std::vector< std::pair<String, size_t> > sequenceIndex;
      // TODO use introspection :-D
      addObjectNameIfExists(T("Primary structure"), T("primaryStructure"), proteinPairs, sequenceIndex);
      addObjectNameIfExists(T("PSSM"), T("positionSpecificScoringMatrix"), proteinPairs, sequenceIndex);
      addObjectNameIfExists(T("Secondary structure 3"), T("secondaryStructure"), proteinPairs, sequenceIndex);
      addObjectNameIfExists(T("Secondary structure 8"), T("dsspSecondaryStructure"), proteinPairs, sequenceIndex);
      addObjectNameIfExists(T("Solvent accesibility"), T("solventAccesibility"), proteinPairs, sequenceIndex);
      addObjectNameIfExists(T("Solvent accesibility 20%"), T("solventAccesibilityAt20p"), proteinPairs, sequenceIndex);    
      addObjectNameIfExists(T("Disorder region Probabilities"), T("disorderRegions"), proteinPairs, sequenceIndex);
      addObjectNameIfExists(T("Structural alphabet"), T("structuralAlphabetSequence"), proteinPairs, sequenceIndex);

      MultiProtein1DConfigurationPtr configuration = new MultiProtein1DConfiguration(proteinNames, sequenceIndex);
      addTab(T("Protein 1D"), Colours::white, new MultiProtein1DComponent(proteins, configuration), true);
    }
    
    /*
    ** 2D
    */
    {
      std::vector< std::pair<String, size_t> > mapIndex;
      addObjectNameIfExists(T("Ca 8 angstrom"), T("contactMap8Ca"), proteinPairs, mapIndex);
      addObjectNameIfExists(T("Cb 8 angstrom"), T("contactMap8Cb"), proteinPairs, mapIndex);
      
      MultiProtein2DConfigurationPtr configuration = new MultiProtein2DConfiguration(proteinNames, mapIndex);
      addTab(T("Protein 2D"), Colours::white, new MultiProtein2DComponent(proteins, configuration), true);
    }
  }
    
  juce_UseDebuggingNewOperator

private:
  std::vector< std::pair<String, ProteinPtr> > proteinPairs;

  void addObjectNameIfExists(const String& friendlyName, const String& name, const std::vector< std::pair<String, ProteinPtr> >& proteinPairs, std::vector< std::pair<String, size_t> >& res)
  {
    jassert(proteinClass()->findStaticVariable(name) != -1);
    size_t variableIndex = proteinClass()->findStaticVariable(name) -1; // FIXME: -1 correspond to baseClass.staticVariable
    std::cout << "Testing friendly name: " << friendlyName << " Index: " << variableIndex << std::endl;
    for (size_t i = 0; i < proteinPairs.size(); ++i)
      if (proteinPairs[i].second->getVariable(variableIndex))
      {
        std::cout << "Accepted: " << friendlyName << std::endl;
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

