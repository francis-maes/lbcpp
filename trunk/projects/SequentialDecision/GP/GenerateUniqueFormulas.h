/*-----------------------------------------.---------------------------------.
| Filename: GenerateUniqueFormulas.h       | Generate Unique Formulas        |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2011 19:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_GP_GENERATE_UNIQUE_FORMULAS_H_
# define LBCPP_SEQUENTIAL_DECISION_GP_GENERATE_UNIQUE_FORMULAS_H_

# include <lbcpp/Execution/WorkUnit.h>
# include "GPExpressionBuilder.h"
# include "FormulaSearchProblem.h"
# include "LearningRuleFormulaObjective.h"
# include <algorithm>

namespace lbcpp
{

class GenerateUniqueFormulas : public WorkUnit
{
public:
  GenerateUniqueFormulas() : maxSize(3) {}

  typedef std::vector<int> FormulaKey;
  
  virtual Variable run(ExecutionContext& context)
  {
    std::vector< std::vector<double> > inputSamples;
    problem->sampleInputs(context, 100, inputSamples);
    
    GPExpressionBuilderStatePtr state = problem->makeGPBuilderState(maxSize);
    std::map<FormulaKey, GPExpressionPtr> formulas;
    enumerateAllFormulas(context, state, inputSamples, formulas);

    context.informationCallback("We have " + String((int)formulas.size()) + " formulas");
    return saveFormulasToFile(context, formulas, formulasFile);
  }

  static String formulaKeyToString(const FormulaKey& key)
  {
    String res;
    for (size_t i = 0; i < key.size(); ++i)
      res += String(key[i]) + T(";");
    return res;
  }
  
  void enumerateAllFormulas(ExecutionContext& context, GPExpressionBuilderStatePtr state, const std::vector< std::vector<double> >& inputSamples, std::map<FormulaKey, GPExpressionPtr>& res)
  {
    if (state->isFinalState())
    {
      GPExpressionPtr formula = state->getExpression();
      jassert(formula);
      
      FormulaKey formulaKey;
      if (!problem->makeFormulaKey(formula, inputSamples, formulaKey))
      {
        //context.informationCallback("Invalid formula: " + formula->toShortString());
        return; // invalid formula
      }
      //context.informationCallback("Formula: " + formula->toShortString() + " --> " + formulaKeyToString(formulaKey));
        
      std::map<FormulaKey, GPExpressionPtr>::iterator it = res.find(formulaKey);
      if (it == res.end())
      {
        //context.informationCallback("Formula: " + formula->toShortString());
        res[formulaKey] = formula;
        if (res.size() % 1000 == 0)
          context.informationCallback(String((int)res.size()) + T(" formulas, last formula: ") + formula->toShortString());
      }
      else if (formula->size() < it->second->size())
        it->second = formula; // keep the smallest formula
    }
    else
    {
      ContainerPtr actions = state->getAvailableActions();
      size_t n = actions->getNumElements();
      for (size_t i = 0; i < n; ++i)
      {
        Variable stateBackup;
        Variable action = actions->getElement(i);
        double reward;
        state->performTransition(context, action, reward, &stateBackup);
        enumerateAllFormulas(context, state, inputSamples, res);
        state->undoTransition(context, stateBackup);
      }
    }
  }
  
protected:
  friend class GenerateUniqueFormulasClass;

  FormulaSearchProblemPtr problem;
  size_t maxSize;
  File formulasFile;

  bool saveFormulasToFile(ExecutionContext& context, const std::map<FormulaKey, GPExpressionPtr>& formulas, const File& file) const
  {
    if (file.exists())
      file.deleteFile();
    OutputStream* ostr = file.createOutputStream();
    if (!ostr)
    {
      context.errorCallback(T("Could not open file ") + file.getFullPathName());
      return false;
    }

    for (std::map<FormulaKey, GPExpressionPtr>::const_iterator it = formulas.begin(); it != formulas.end(); ++it)
    {
      *ostr << it->second->toString();
      *ostr << "\n";
    }
    delete ostr;
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_GP_GENERATE_UNIQUE_FORMULAS_H_
