/*-----------------------------------------.---------------------------------.
| Filename: VariableContainer.h            | Variable Container base class   |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_VARIABLE_CONTAINER_H_
# define LBCPP_OBJECT_VARIABLE_CONTAINER_H_

# include "Variable.h"
# include "../ObjectPredeclarations.h"

namespace lbcpp
{

class VariableContainer : public Object
{
public:
  VariableContainer(ClassPtr thisClass)
    : Object(thisClass) {}
  VariableContainer() {}

  bool empty() const
    {return getNumVariables() == 0;}

  size_t size() const
    {return getNumVariables();}

  virtual TypePtr getStaticType() const
    {return topLevelType();}

  virtual String toString() const;

  VectorPtr toVector() const;

public:
  VariableContainerPtr subset(const std::vector<size_t>& indices) const;

  /**
  ** Creates a randomized version of a dataset.
  **
  ** The randomization is lazy: only a mapping between old
  ** indices and new ones is stored and no data is copied.
  **
  ** @returns the randomized VariableContainer that has an internal
  ** reference to this one.
  */
  VariableContainerPtr randomize() const;

  /**
  ** Creates a set where each element is duplicated multiple times.
  **
  ** This function creates a new VariableContainer that references this
  ** one. Each element of this VariableContainer is duplicated @a count
  ** times in the new VariableContainer. For example, if this VariableContainer contains
  ** three examples A, B and C, duplicate(4) will contain A, B, C, A,
  ** B, C, A, B, C, A, B and C.
  **
  ** The duplication is lazy and does not consume any memory nor cpu
  ** time.
  **
  ** Duplicating an VariableContainer can for example be useful to perform
  ** stochastic training. The following code shows how to perform
  ** 10 passes on the data with a single call to the trainStochastic function:
  ** @code
  ** ClassifierPtr aStochasticClassifier = ...;
  ** VariableContainerPtr trainingData = ...;
  **
  ** VariableContainerPtr trainingData10Passes = trainingData->duplicate(10);
  ** if (randomize)
  **  trainingData10Passes = trainingData10Passes->randomize();
  ** aStochasticClassifier->trainStochastic(trainingData10Passes);
  ** @endcode
  **
  ** Note that one can alternatively use a loop that calls ten
  ** times the trainStochastic function.
  **
  ** @param count : The number of times the element set is
  ** duplicated.
  ** @returns a new VariableContainer that refers this one internally.
  */
  VariableContainerPtr duplicate(size_t count) const;

  /**
  ** Creates a fold of elements.
  **
  ** This function is typically used in the context of
  ** cross-validation.
  ** The elements are splitted into @a numFolds groups. This
  ** method construct a new VariableContainer that refers to the
  ** @a fold 's group.
  **
  ** @param fold : the number of the fold. This number should be in
  ** the interval 0 to numFolds - 1.
  ** @param numFolds : the total number of folds.
  **
  ** @returns a new VariableContainer that refers this one internally.
  */
  VariableContainerPtr fold(size_t fold, size_t numFolds) const;

  /**
  ** Excludes a fold of elements.
  **
  ** This function compute the complementary set of the one
  ** returned by fold().
  ** The returned VariableContainer contains all elements except the
  ** one from the @a fold 's group.
  **
  ** @param fold : the number of the fold that will be excluded from
  ** data. This number should be in the interval 0 to numFolds - 1.
  ** @param numFolds : the total number of folds.
  **
  ** @returns a new VariableContainer that refers this one internally.
  */
  VariableContainerPtr invFold(size_t fold, size_t numFolds) const;

  /**
  ** Selects a range of elements.
  **
  ** This functions creates an element set that refer to a range
  ** of examples of this VariableContainer. The range is defined through
  ** the begin and end indices that respectively correspond to the
  ** first element inside the range and the first element below the
  ** range.
  **
  **  @param begin : the index of the first element inside the
  **  range.
  **  @param end : the index of the first element below the range.
  **
  **  @returns a new VariableContainer that refers to this internally
  **  and whose size is (end - begin).
  **  @see fold
  */
  VariableContainerPtr range(size_t begin, size_t end) const;

  /** Excludes a range of elements.

      This functions creates the complementary VariableContainer
      of range(begin, end), <i>i.e.</i> the VariableContainer
      containing all the examples of this except those belonging
      to the range (begin, end).

      @param begin The index of the first element inside the range to exclude.
      @param end The index of the first element below the range to exclude.
      @returns a new VariableContainer that refers to this internally and
        whose size is (size() - (end - begin)).
      @see range, invFold
  */
  VariableContainerPtr invRange(size_t begin, size_t end) const;
};

extern ClassPtr variableContainerClass();

/**
** @class DecoratorVariableContainer
** @brief Base class for VariableContainer decorators.
** This base class is used internally to implement
** the VariableContainer::fold(), VariableContainer::invFold(),
** VariableContainer::range(), ObjectContaner::invRange() and
** VariableContainer::randomize() functions.
*/
class DecoratorVariableContainer : public VariableContainer
{
public:
  /**
  ** Constructor.
  **
  ** @param target : the decorated VariableContainer.
  */
  DecoratorVariableContainer(VariableContainerPtr target)
    : target(target) {}

  DecoratorVariableContainer() {}

  /**
  ** Container size getter.
  **
  ** @return container size.
  */
  virtual size_t getNumVariables() const
    {return target->getNumVariables();}

  virtual TypePtr getStaticType() const
    {return target->getStaticType();}

  /**
  ** Gets an element.
  **
  ** @param index : item index.
  **
  ** @return an object pointer.
  */
  virtual Variable getVariable(size_t index) const
    {return target->getVariable(index);}

  virtual void setVariable(size_t index, const Variable& value) const
    {target->setVariable(index, value);}

protected:
  VariableContainerPtr target; /*!< A pointer to the decorated VariableContainer. */
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_VARIABLE_CONTAINER_H_
