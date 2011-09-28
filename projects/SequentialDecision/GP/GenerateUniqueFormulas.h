/*-----------------------------------------.---------------------------------.
| Filename: GenerateUniqueFormulas.h       | Generate Unique Formulas        |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2011 19:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_GP_GENERATE_UNIQUE_FORMULAS_H_
# define LBCPP_SEQUENTIAL_DECISION_GP_GENERATE_UNIQUE_FORMULAS_H_

# include "FormulaLearnAndSearch.h" // SuperFormulaPool

namespace lbcpp
{

class GenerateUniqueFormulas : public WorkUnit
{
public:
  GenerateUniqueFormulas() : maxSize(3), numSamples(100) {}
 
  virtual Variable run(ExecutionContext& context)
  {
    context.getRandomGenerator()->setSeed((juce::uint32)(juce::Time::getMillisecondCounterHiRes() * 100.0));

    SuperFormulaPool pool(context, problem, numSamples);
   
    size_t numFinalStates = 0;
    if (!pool.addAllFormulasUpToSize(context, maxSize, numFinalStates))
      return false;

    // results
    context.resultCallback(T("numFinalStates"), numFinalStates);
    context.resultCallback(T("numFormulas"), pool.getNumFormulas());
    context.resultCallback(T("numInvalidFormulas"), pool.getNumInvalidFormulas());
    context.resultCallback(T("numFormulaClasses"), pool.getNumFormulaClasses());

    if (formulasFile != File() && !saveFormulasToFile(context, pool, formulasFile))
      return false;
    
    return new Pair(pool.getNumFormulaClasses(), pool.getNumInvalidFormulas());
  }

protected:
  friend class GenerateUniqueFormulasClass;

  FormulaSearchProblemPtr problem;
  size_t maxSize;
  size_t numSamples;
  File formulasFile;

  bool saveFormulasToFile(ExecutionContext& context, SuperFormulaPool& pool, const File& file) const
  {
    if (file.exists())
      file.deleteFile();
    OutputStream* ostr = file.createOutputStream();
    if (!ostr)
    {
      context.errorCallback(T("Could not open file ") + file.getFullPathName());
      return false;
    }

    size_t n = pool.getNumFormulaClasses();
    for (size_t i = 0; i < n; ++i)
    {
      *ostr << pool.getFormulaClassExpression(i)->toString();
      *ostr << "\n";
    }
    delete ostr;
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_GP_GENERATE_UNIQUE_FORMULAS_H_
