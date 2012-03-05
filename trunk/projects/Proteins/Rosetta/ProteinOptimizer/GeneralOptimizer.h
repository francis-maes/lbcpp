/*-----------------------------------------.---------------------------------.
| Filename: GeneralOptimizer.h             | GeneralOptimizer                |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Mar 1, 2012  9:20:54 AM        |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_ROSETTA_PROTEINOPTIMIZER_GENERALOPTIMIZER_H_
# define LBCPP_PROTEIN_ROSETTA_PROTEINOPTIMIZER_GENERALOPTIMIZER_H_

namespace lbcpp
{

class GeneralOptimizerState : public Object
{
public:
  virtual double getObjective() const = 0;

protected:
  friend class GeneralOptimizerStateClass;
};

typedef ReferenceCountedObjectPtr<GeneralOptimizerState> GeneralOptimizerStatePtr;

class GeneralOptimizerStateModifier : public Object
{
public:
  virtual GeneralOptimizerStatePtr getModifiedState(GeneralOptimizerStatePtr& state) = 0;

protected:
  friend class GeneralOptimizerStateModifierClass;
};

typedef ReferenceCountedObjectPtr<GeneralOptimizerStateModifier> GeneralOptimizerStateModifierPtr;

class GeneralOptimizerStoppingCriterion : public Object
{
public:
  virtual bool performNext(size_t iteration, GeneralOptimizerStatePtr& state, GeneralOptimizerStatePtr& bestState) = 0;

protected:
  friend class GeneralOptimizerStoppingCriterionClass;

};

typedef ReferenceCountedObjectPtr<GeneralOptimizerStoppingCriterion> GeneralOptimizerStoppingCriterionPtr;

class GeneralOptimizer : public Object
{
public:
  GeneralOptimizer(const GeneralOptimizerStatePtr& initialState,
                   const GeneralOptimizerStateModifierPtr& modifier,
                   const GeneralOptimizerStoppingCriterionPtr& stoppingCriterion)
    : iteration(0),
      initialState(initialState),
      bestState(initialState),
      modifier(modifier),
      stoppingCriterion(stoppingCriterion) {}

  /*
   * Optimization
   */
  void optimize()
  {
//    while (stoppingCriterion->performNext(iteration, state, bestState))
//    {
//      performNextIteration();
//      updateBestState();
//      iteration++;
    }
  }

  /*
   * Solution
   */
  GeneralOptimizerStatePtr getBestState()
    {return bestState;}

protected:
  virtual void performNextIteration() = 0;
  virtual void updateBestState() = 0;

  friend class GeneralOptimizerClass;

  size_t iteration;
  GeneralOptimizerStatePtr initialState;
  GeneralOptimizerStatePtr bestState;
  GeneralOptimizerStateModifierPtr modifier;
  GeneralOptimizerStoppingCriterionPtr stoppingCriterion;
};

typedef ReferenceCountedObjectPtr<GeneralOptimizer> GeneralOptimizerPtr;

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEIN_ROSETTA_PROTEINOPTIMIZER_GENERALOPTIMIZER_H_

