/*-----------------------------------------.---------------------------------.
| Filename: TuningSAParametersWorkUnit.h   | TuningSAParametersWorkUnit      |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Dec 13, 2011  11:15:08 AM      |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_WORKUNIT_TUNINGSAPARAMETERSWORKUNIT_H_
# define LBCPP_PROTEINS_ROSETTA_WORKUNIT_TUNINGSAPARAMETERSWORKUNIT_H_

# include <lbcpp/Execution/WorkUnit.h>
# include "../RosettaUtils.h"
# include "ProteinOptimizationWorkUnit.h"

namespace lbcpp
{

class TuningSAParametersWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
#ifdef LBCPP_PROTEIN_ROSETTA

    // candidate temperatures
    DenseDoubleVector initialTemps(5, 2);
    initialTemps.setValue(1, 3);
    initialTemps.setValue(2, 4);
    initialTemps.setValue(3, 5);
    initialTemps.setValue(4, 10);
    DenseDoubleVector finalTemps(5, 1);
    finalTemps.setValue(1, 0.8);
    finalTemps.setValue(2, 0.6);
    finalTemps.setValue(3, 0.4);
    finalTemps.setValue(4, 0.2);

    // comparing energies
    std::vector<DenseDoubleVectorPtr> allEnergies;
    std::vector<double> allInitialTemperatures;
    std::vector<double> allFinalTemperatures;

    context.enterScope(T("Trying temperature combinations"));

    for (size_t i = 0; i < initialTemps.getNumElements(); i++)
    {
      for (size_t j = 0; j < finalTemps.getNumElements(); j++)
      {
        context.enterScope(T("Iteration"));

        ProteinOptimizationWorkUnit sa;

        sa.setVariable(0, Variable(inputDirectory));
        sa.setVariable(1, Variable(String(T(""))));
        sa.setVariable(2, Variable(referencesDirectory));
        sa.setVariable(3, Variable(moversDirectory));
        sa.setVariable(4, Variable((int)3));
        sa.setVariable(5, Variable((int)1));
        sa.setVariable(6, Variable((int)1));
        sa.setVariable(7, Variable((int)1));
        sa.setVariable(8, Variable((int)1));
        sa.setVariable(9, Variable((int)10));
        sa.setVariable(10, Variable((double)initialTemps.getValue(i)));
        sa.setVariable(11, Variable((double)finalTemps.getValue(j)));
        sa.setVariable(12, Variable((int)-1));
        sa.setVariable(13, Variable((int)numIterations));

        DenseDoubleVectorPtr energies = sa.run(context).getObjectAndCast<DenseDoubleVector> ();

        allEnergies.push_back(energies);
        allInitialTemperatures.push_back(initialTemps.getValue(i));
        allFinalTemperatures.push_back(finalTemps.getValue(j));

        context.leaveScope();
      }
    }
    context.leaveScope();

    context.enterScope(T("Energy evolution"));

    for (size_t i = 0; i < allEnergies[0]->getNumElements(); i++)
    {
      context.enterScope(T("Energy"));
      context.resultCallback(T("Iteration"), Variable((int)i));
      for (size_t j = 0; j < allEnergies.size(); j++)
      {
        context.resultCallback(String(T("It: ") + String(allInitialTemperatures[j]) + T(", Ft: ")
            + String(allFinalTemperatures[j])), Variable(allEnergies[j]->getValue(i)));
      }
      context.leaveScope();
    }
    context.leaveScope();

    double minEnergy = allEnergies[0]->getValue(allEnergies[0]->getNumElements() - 1);
    size_t indexMinEnergy = 0;

    for (size_t i = 0; i < allEnergies.size(); i++)
    {
      if (allEnergies[i]->getValue(allEnergies[i]->getNumElements() - 1) < minEnergy)
      {
        minEnergy = allEnergies[i]->getValue(allEnergies[i]->getNumElements() - 1);
        indexMinEnergy = i;
      }
    }

    double bestInitialTemperature = allInitialTemperatures[indexMinEnergy];
    double bestFinalTemperature = allFinalTemperatures[indexMinEnergy];

    context.enterScope(String(T("Best initial temperature")));
    context.leaveScope(Variable(bestInitialTemperature));

    context.enterScope(String(T("Best final temperature")));
    context.leaveScope(Variable(bestFinalTemperature));

    context.enterScope(T("Best energy evolution"));

    for (size_t i = 0; i < allEnergies[indexMinEnergy]->getNumElements(); i++)
    {
      context.enterScope(T("Energy"));
      context.resultCallback(T("Iteration"), Variable((int)i));
      context.resultCallback(String(T("It: ") + String(allInitialTemperatures[indexMinEnergy])
          + T(", Ft: ") + String(allFinalTemperatures[indexMinEnergy])), Variable(
          allEnergies[indexMinEnergy]->getValue(i)));
      context.leaveScope();
    }
    context.leaveScope();

    return Variable();

#else
    jassert(false);
    return false;
#endif // LBCPP_PROTEIN_ROSETTA
  }

protected:
  friend class TuningSAParametersWorkUnitClass;

  String inputDirectory;
  String referencesDirectory;
  String moversDirectory;
  size_t numIterations;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_WORKUNIT_TUNINGSAPARAMETERSWORKUNIT_H_
