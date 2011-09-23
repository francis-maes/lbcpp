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
# include "LearningRuleFormulaObjective.h"
# include <algorithm>

namespace lbcpp
{

class GenerateUniqueFormulas : public WorkUnit
{
public:
  GenerateUniqueFormulas() : problem("multiArmedBandits"), maxSize(3) {}

  typedef std::vector<int> FormulaKey;
  
  virtual Variable run(ExecutionContext& context)
  {
    std::vector< std::vector<double> > inputSamples;
    makeInputSamples(context, problem, inputSamples);
    
    GPExpressionBuilderStatePtr state = makeGPBuilderState();
    std::map<FormulaKey, GPExpressionPtr> formulas;
    enumerateAllFormulas(context, state, inputSamples, formulas);

    context.informationCallback("We have " + String((int)formulas.size()) + " formulas");
    return saveFormulasToFile(context, formulas, formulasFile);
  }
  
  GPExpressionBuilderStatePtr makeGPBuilderState() const
  {
    EnumerationPtr variables;
    std::vector<GPPre> unaryOperators;
    std::vector<GPOperator> binaryOperators;

    if (problem == T("multiArmedBandits"))
    {
      variables = gpExpressionDiscreteBanditPolicyVariablesEnumeration;
      for (size_t i = gpLog; i <= gpInverse; ++i)
        unaryOperators.push_back((GPPre)i);
      for (size_t i = gpAddition; i <= gpMin; ++i)
        binaryOperators.push_back((GPOperator)i);
    }
    else if (problem == T("binaryClassification"))
    {
      variables = learningRuleFormulaVariablesEnumeration;
      for (size_t i = gpLog; i <= gpAbs; ++i)
        unaryOperators.push_back((GPPre)i);
      for (size_t i = gpAddition; i <= gpLessThan; ++i)
        binaryOperators.push_back((GPOperator)i);
    }
    else
    {
      jassert(false);
      return GPExpressionBuilderStatePtr();
    }

    return new RPNGPExpressionBuilderState("Coucou", variables, FunctionPtr(), maxSize, unaryOperators, binaryOperators);
  }

  void makeInputSamples(ExecutionContext& context, const String& problem, std::vector< std::vector<double> >& res)
  {
    RandomGeneratorPtr random = context.getRandomGenerator();

    if (problem == T("multiArmedBandits"))
    {
      size_t count = 1000;
      res.resize(count);
      for (size_t i = 0; i < count - 2; ++i)
      {
        std::vector<double> input(7);
        input[0] = juce::jmax(1, (int)pow(10.0, random->sampleDouble(0, 5))); // t
        input[1] = random->sampleDouble(0.0, 1.0); // rk1
        input[2] = random->sampleDouble(0.0, 0.5); // sk1
        input[3] = juce::jmax(1, (int)(input[0] * random->sampleDouble(0.0, 1.0))); // tk1
        input[4] = random->sampleDouble(0.0, 1.0); // rk2
        input[5] = random->sampleDouble(0.0, 0.5); // sk2
        input[6] = juce::jmax(1, (int)(input[0] * random->sampleDouble(0.0, 1.0))); // tk2
        res[i] = input;
      }

      std::vector<double> smallest(7);
      smallest[0] = 1.0;
      smallest[1] = smallest[2] = smallest[4] = smallest[5] = 0.0;
      smallest[3] = smallest[6] = 1.0;
      res[count - 2] = smallest;

      std::vector<double> highest(7);
      highest[0] = 100000;
      highest[1] = highest[4] = 1.0;
      highest[2] = highest[5] = 0.5;
      highest[3] = highest[6] = 100000;
      res[count - 1] = highest;
    }
    else if (problem == T("binaryClassification"))
    {
      size_t count = 100;
      res.resize(count);
      for (size_t i = 0; i < count - 1; ++i)
      {
        std::vector<double> input(4);
        input[0] = random->sampleDoubleFromGaussian(0.0, 10.0); // param
        input[1] = random->sampleDoubleFromGaussian(0.0, 1.0);  // feature
        input[2] = random->sampleDoubleFromGaussian(0.0, 10.0); // score
        input[3] = (size_t)pow(10, random->sampleDouble(0.0, 5.0));
        res[i] = input;
      }

      std::vector<double> zero(4);
      zero[0] = 1.0; zero[1] = 0.0; zero[2] = 0.0; zero[3] = 1;
      res[count - 1] = zero;
    }
    else
      jassert(false);
  }
  
  struct CompareSecond
  {
    bool operator() (const std::pair<size_t, double>& a, const std::pair<size_t, double>& b) const
    {
      if (a.second == b.second)
        return a.first < b.first;
      else
        return a.second < b.second;
    }
  };
  
  bool makeFormulaKey(const GPExpressionPtr& expression, const std::vector< std::vector<double> >& inputSamples, FormulaKey& res)
  {
    res.resize(inputSamples.size());

    if (problem == T("multiArmedBandits"))
    {
      for (size_t i = 0; i < inputSamples.size(); ++i)
      {
        std::vector<double> v(4);
        v[0] = inputSamples[i][1];
        v[1] = inputSamples[i][2];
        v[2] = inputSamples[i][3];
        v[3] = inputSamples[i][0];
        double value1 = expression->compute(&v[0]);
        if (!isNumberValid(value1))
          return false;

        v[0] = inputSamples[i][4];
        v[1] = inputSamples[i][5];
        v[2] = inputSamples[i][6];
        v[3] = inputSamples[i][0];
        double value2 = expression->compute(&v[0]);
        if (!isNumberValid(value2))
          return false;

        if (value1 < value2)
          res[i] = 0;
        else if (value1 == value2)
          res[i] = 1;
        else if (value1 > value2)
          res[i] = 2;

        //res[i] = (int)(value1 * 10000) + (int)(value2 * 1000000);
      }
    }
    else if (problem == T("binaryClassification"))
    {
      std::map<size_t, size_t> variableUseCounts;
      expression->getVariableUseCounts(variableUseCounts);
      if (variableUseCounts[1] == 0 || variableUseCounts[2] == 0) // at least feature or score must be used
        return false;
      if (variableUseCounts[3] > 0) // forbid variable "epoch" for the moment
        return false; 

      for (size_t i = 0; i < res.size(); ++i)
      {
        double value = expression->compute(&inputSamples[i][0]);
        if (!isNumberValid(value))
          return false;
        res[i] = (int)(value * 100000);
      }
    }
    else
      jassert(false);
    return true;
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
      if (!makeFormulaKey(formula, inputSamples, formulaKey))
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

  String problem;
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
