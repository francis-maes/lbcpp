/*-----------------------------------------.---------------------------------.
| Filename: DecisionTree.h                 | Decision Trees Header           |
| Author  : Julien Becker                  |                                 |
| Started : 16/02/2011 16:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DECISION_TREE_H_
# define LBCPP_DECISION_TREE_H_

# include "../Core/Function.h"

namespace lbcpp
{

class ExtraTreesFunction : public Function
{
public:
  ExtraTreesFunction(size_t numTrees,
                    size_t numAttributeSamplesPerSplit,
                    size_t minimumSizeForSplitting)
  : numTrees(numTrees), 
    numAttributeSamplesPerSplit(numAttributeSamplesPerSplit),
    minimumSizeForSplitting(minimumSizeForSplitting) {}

  /* ExtraTreeFunction */
  size_t getNumTrees() const
    {return numTrees;}
  
  size_t getNumAttributeSamplesPerSplit() const
    {return numAttributeSamplesPerSplit;}
  
  size_t getMinimumSizeForSplitting() const
    {return minimumSizeForSplitting;}
  
  virtual TypePtr getSupervisionType() const = 0;

  /* Function */
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)containerClass(anyType) : getSupervisionType();}

  virtual String getOutputPostFix() const
    {return T("ExtraTrees");}

protected:
  friend class ExtraTreesFunctionClass;

  size_t numTrees;
  size_t numAttributeSamplesPerSplit;
  size_t minimumSizeForSplitting;

  ExtraTreesFunction() {}
};

typedef ReferenceCountedObjectPtr<ExtraTreesFunction> ExtraTreesFunctionPtr;

extern ExtraTreesFunctionPtr regressionExtraTree(size_t numTrees = 100,
                                                 size_t numAttributeSamplesPerSplit = 10,
                                                 size_t minimumSizeForSplitting = 0,
                                                 bool useLowMemory = false,
                                                 const File& saveTreesToFilePrefix = File::nonexistent);

extern ExtraTreesFunctionPtr binaryClassificationExtraTree(size_t numTrees = 100,
                                                           size_t numAttributeSamplesPerSplit = 10,
                                                           size_t minimumSizeForSplitting = 0,
                                                           bool useLowMemory = false,
                                                           const File& saveTreesToFilePrefix = File::nonexistent);

extern ExtraTreesFunctionPtr classificationExtraTree(size_t numTrees = 100,
                                                     size_t numAttributeSamplesPerSplit = 10,
                                                     size_t minimumSizeForSplitting = 0,
                                                     bool useLowMemory = false,
                                                     const File& saveTreesToFilePrefix = File::nonexistent);

extern FunctionPtr extraTreeLearningMachine(size_t numTrees = 100,
                                                   size_t numAttributeSamplesPerSplit = 10,
                                                   size_t minimumSizeForSplitting = 0,
                                                   bool useLowMemory = false,
                                                   const File& saveTreesToFilePrefix = File::nonexistent);


}; /* namespace lbcpp */

#endif //!LBCPP_DECISION_TREE_H_
