/*-----------------------------------------.---------------------------------.
| Filename: Container.h                    | Variable Container base class   |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_CONTAINER_H_
# define LBCPP_OBJECT_CONTAINER_H_

# include "Variable.h"
# include "../Function/Function.h"

namespace lbcpp
{

class Container : public Object
{
public:
  Container(ClassPtr thisClass)
    : Object(thisClass) {}
  Container() {}

  bool isEmpty() const
    {return getNumElements() == 0;}
    
  int findElement(const Variable& value) const;
  TypePtr computeElementsCommonBaseType() const;

  virtual TypePtr getElementsType() const
    {jassert(thisClass); return thisClass->getTemplateArgument(0);}

  virtual size_t getNumElements() const = 0;
  virtual String getElementName(size_t index) const;
  virtual Variable getElement(size_t index) const = 0;
  virtual void setElement(size_t index, const Variable& value) = 0;

  virtual String toString() const;
  VectorPtr toVector() const;

  virtual void saveToXml(XmlElement* xml) const;
  virtual bool loadFromXml(XmlElement* xml, MessageCallback& callback);

public:
  ContainerPtr subset(const std::vector<size_t>& indices) const;

  /**
  ** Applies a @a function to all item contained into the object container.
  **
  ** @param function : function to apply to the current object container.
  ** @param lazyCompute : specify if the function is applicated
  ** immediately on all container items (True), or at each call
  ** to get() or getAndCast() (False).
  **
  ** @return an object container pointer.
  ** @see Function
  */
  ContainerPtr apply(FunctionPtr function, bool lazyCompute = true) const;

  /**
  ** Creates a randomized version of a dataset.
  **
  ** The randomization is lazy: only a mapping between old
  ** indices and new ones is stored and no data is copied.
  **
  ** @returns the randomized Container that has an internal
  ** reference to this one.
  */
  ContainerPtr randomize() const;

  /**
  ** Creates a set where each element is duplicated multiple times.
  **
  ** This function creates a new Container that references this
  ** one. Each element of this Container is duplicated @a count
  ** times in the new Container. For example, if this Container contains
  ** three examples A, B and C, duplicate(4) will contain A, B, C, A,
  ** B, C, A, B, C, A, B and C.
  **
  ** The duplication is lazy and does not consume any memory nor cpu
  ** time.
  **
  ** Duplicating an Container can for example be useful to perform
  ** stochastic training. The following code shows how to perform
  ** 10 passes on the data with a single call to the trainStochastic function:
  ** @code
  ** ClassifierPtr aStochasticClassifier = ...;
  ** ContainerPtr trainingData = ...;
  **
  ** ContainerPtr trainingData10Passes = trainingData->duplicate(10);
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
  ** @returns a new Container that refers this one internally.
  */
  ContainerPtr duplicate(size_t count) const;

  /**
  ** Creates a fold of elements.
  **
  ** This function is typically used in the context of
  ** cross-validation.
  ** The elements are splitted into @a numFolds groups. This
  ** method construct a new Container that refers to the
  ** @a fold 's group.
  **
  ** @param fold : the number of the fold. This number should be in
  ** the interval 0 to numFolds - 1.
  ** @param numFolds : the total number of folds.
  **
  ** @returns a new Container that refers this one internally.
  */
  ContainerPtr fold(size_t fold, size_t numFolds) const;

  /**
  ** Excludes a fold of elements.
  **
  ** This function compute the complementary set of the one
  ** returned by fold().
  ** The returned Container contains all elements except the
  ** one from the @a fold 's group.
  **
  ** @param fold : the number of the fold that will be excluded from
  ** data. This number should be in the interval 0 to numFolds - 1.
  ** @param numFolds : the total number of folds.
  **
  ** @returns a new Container that refers this one internally.
  */
  ContainerPtr invFold(size_t fold, size_t numFolds) const;

  /**
  ** Selects a range of elements.
  **
  ** This functions creates an element set that refer to a range
  ** of examples of this Container. The range is defined through
  ** the begin and end indices that respectively correspond to the
  ** first element inside the range and the first element below the
  ** range.
  **
  **  @param begin : the index of the first element inside the
  **  range.
  **  @param end : the index of the first element below the range.
  **
  **  @returns a new Container that refers to this internally
  **  and whose size is (end - begin).
  **  @see fold
  */
  ContainerPtr range(size_t begin, size_t end) const;

  /** Excludes a range of elements.

      This functions creates the complementary Container
      of range(begin, end), <i>i.e.</i> the Container
      containing all the examples of this except those belonging
      to the range (begin, end).

      @param begin The index of the first element inside the range to exclude.
      @param end The index of the first element below the range to exclude.
      @returns a new Container that refers to this internally and
        whose size is (size() - (end - begin)).
      @see range, invFold
  */
  ContainerPtr invRange(size_t begin, size_t end) const;
};

extern ClassPtr containerClass(TypePtr elementsType);

/**
** @class DecoratorContainer
** @brief Base class for Container decorators.
** This base class is used internally to implement
** the Container::fold(), Container::invFold(),
** Container::range(), ObjectContaner::invRange() and
** Container::randomize() functions.
*/
class DecoratorContainer : public Container
{
public:
  /**
  ** Constructor.
  **
  ** @param target : the decorated Container.
  */
  DecoratorContainer(ContainerPtr target)
    : target(target) {}

  DecoratorContainer() {}

  /**
  ** Container size getter.
  **
  ** @return container size.
  */
  virtual size_t getNumElements() const
    {return target->getNumElements();}

  virtual TypePtr getElementsType() const
    {return target->getElementsType();}

  /**
  ** Gets an element.
  **
  ** @param index : item index.
  **
  ** @return an object pointer.
  */
  virtual Variable getElement(size_t index) const
    {return target->getElement(index);}

  virtual void setElement(size_t index, const Variable& value) const
    {target->setElement(index, value);}

protected:
  friend class DecoratorContainerClass;

  ContainerPtr target; /*!< A pointer to the decorated Container. */
};

typedef ReferenceCountedObjectPtr<DecoratorContainer> DecoratorContainerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_CONTAINER_H_
