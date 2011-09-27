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
  GenerateUniqueFormulas() : maxSize(3), numSamples(100),
    numFinalStates(0), numFormulas(0), numInvalidFormulas(0), numFormulaClasses(0) {}
 
  virtual Variable run(ExecutionContext& context)
  {
    SuperFormulaPool pool(context, problem, numSamples);
    
    if (!pool.addAllFormulasUpToSize(context, maxSize, numFinalStates))
      return false;

    // results
    numFormulas = pool.getNumFormulas();
    numInvalidFormulas = pool.getNumInvalidFormulas();
    numFormulaClasses = pool.getNumFormulaClasses();

    return formulasFile == File() || saveFormulasToFile(context, pool, formulasFile);
  }

protected:
  friend class GenerateUniqueFormulasClass;

  FormulaSearchProblemPtr problem;
  size_t maxSize;
  size_t numSamples;
  File formulasFile;

  size_t numFinalStates;
  size_t numFormulas;
  size_t numInvalidFormulas;
  size_t numFormulaClasses;

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
