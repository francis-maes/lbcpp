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
    std::vector<ProteinPtr> proteinObjects(proteins.size());
    std::vector<String> proteinNames(proteins.size());
    for (size_t i = 0; i < proteinNames.size(); ++i)
    {
      proteinNames[i] = proteins[i].first;
      proteinObjects[i] = proteins[i].second;
    }

    /*
    ** 1D
    */
    {
      std::vector< std::pair<String, String> > sequenceNames;
      addObjectNameIfExists(T("Primary structure"), T("AminoAcidSequence"), proteins, sequenceNames);
      addObjectNameIfExists(T("PSSM"), T("PositionSpecificScoringMatrix"), proteins, sequenceNames);
      addObjectNameIfExists(T("Secondary structure 3"), T("SecondaryStructureSequence"), proteins, sequenceNames);
      addObjectNameIfExists(T("Secondary structure 3 Probabilities"), T("SecondaryStructureProbabilities"), proteins, sequenceNames);
      addObjectNameIfExists(T("Secondary structure 8"), T("DSSPSecondaryStructureSequence"), proteins, sequenceNames);
      addObjectNameIfExists(T("Solvent accesibility"), T("NormalizedSolventAccessibilitySequence"), proteins, sequenceNames);
      addObjectNameIfExists(T("Solvent accesibility 20%"), T("SolventAccessibilityThreshold20"), proteins, sequenceNames);    
      addObjectNameIfExists(T("Solvent accesibility 25%"), T("SolventAccessibilityThreshold25"), proteins, sequenceNames);    
      addObjectNameIfExists(T("Disorder region"), T("DisorderSequence"), proteins, sequenceNames);
      addObjectNameIfExists(T("Disorder region Probabilities"), T("DisorderProbabilitySequence"), proteins, sequenceNames);
      addObjectNameIfExists(T("Structural alphabet"), T("StructuralAlphabetSequence"), proteins, sequenceNames);

      MultiProtein1DConfigurationPtr configuration = new MultiProtein1DConfiguration(proteinNames, sequenceNames);
      addTab(T("Protein 1D"), Colours::white, new MultiProtein1DComponent(proteinObjects, configuration), true);
    }
    
    /*
    ** 2D
    */
    {
      std::vector< std::pair<String, String> > mapNames;
      addObjectNameIfExists(T("Ca 8 angstrom"), T("ResidueResidueContactMatrix8Ca"), proteins, mapNames);
      addObjectNameIfExists(T("Cb 8 angstrom"), T("ResidueResidueContactMatrix8Cb"), proteins, mapNames);
      
      MultiProtein2DConfigurationPtr configuration = new MultiProtein2DConfiguration(proteinNames, mapNames);
      addTab(T("Protein 2D"), Colours::white, new MultiProtein2DComponent(proteinObjects, configuration), true);
    }
  }
    
  juce_UseDebuggingNewOperator

private:
  std::vector< std::pair<String, ProteinPtr> > proteins;

  void addObjectNameIfExists(const String& friendlyName, const String& name, const std::vector< std::pair<String, ProteinPtr> >& proteins, std::vector< std::pair<String, String> >& res)
  {
    for (size_t i = 0; i < proteins.size(); ++i)
      if (proteins[i].second->getObject(name))
      {
        res.push_back(std::make_pair(friendlyName, name));
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

