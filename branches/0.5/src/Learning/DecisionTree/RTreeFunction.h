/*-----------------------------------------.---------------------------------.
| Filename: RTreeFunction.h                | Wrapper of ExtraTrees           |
| Author  : Julien Becker                  | implemented by Pierre Geurts    |
| Started : 16/02/2011 11:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DECISION_TREE_R_TREE_FUNCTION_H_
# define LBCPP_DECISION_TREE_R_TREE_FUNCTION_H_

# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/Learning/DecisionTree.h>
# include <lbcpp/Learning/BatchLearner.h>

namespace lbcpp
{

extern BatchLearnerPtr rTreeBatchLearner(bool verbose = false);

class RTreeFunction : public ExtraTreesFunction
{
public:
  RTreeFunction(size_t numTrees,
                size_t numAttributeSamplesPerSplit,
                size_t minimumSizeForSplitting)
  : ExtraTreesFunction(numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting)
    {setBatchLearner(filterUnsupervisedExamplesBatchLearner(rTreeBatchLearner()));}

  void setTrees(ObjectPtr trees)
    {this->trees = trees;}
  
  void setVerbosity(bool verbose)
    {setBatchLearner(filterUnsupervisedExamplesBatchLearner(rTreeBatchLearner(verbose)));}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

  const std::vector<double>& getVariableImportances() const
    {return variableImportances;}

  void setVariableImportances(const std::vector<double>& varImp)
    {variableImportances = varImp;}

  void getRankedMapping(bool useRankAsIndex, std::vector<size_t>& result) const
  {
    std::multimap<double, size_t> mapRanks;
    for (size_t i = 0; i < variableImportances.size(); ++i)
      mapRanks.insert(std::pair<double, size_t>(variableImportances[i], i));

    result.resize(variableImportances.size());
    std::multimap<double, size_t>::reverse_iterator it = mapRanks.rbegin();
    for (size_t i = 0; it != mapRanks.rend(); ++it, ++i)
    {
      if (useRankAsIndex)
        result[i] = it->second;
      else
        result[it->second] = i;
    }
  }
  
  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);

  bool saveTreesToBinaryFile(ExecutionContext& context, const File& file) const;
  bool loadTreesFromBinaryFile(ExecutionContext& context, const File& file);

  // Private
  static void* computeCoreTableOf(ExecutionContext& context, const Variable& input);
  static void deleteCoreTable(void* coreTable);
  Variable makePredictionFromCoreTable(ExecutionContext& context, void* coreTable);

protected:
  friend class RTreeFunctionClass;
  
  ObjectPtr trees;
  std::vector<double> variableImportances;

  RTreeFunction() {}
};

extern ClassPtr rTreeFunctionClass;
typedef ReferenceCountedObjectPtr<RTreeFunction> RTreeFunctionPtr;

/****** Batch Learner ******/
class RTreeBatchLearner : public BatchLearner
{
public:
  RTreeBatchLearner(bool verbose = false)
    : verbose(verbose) {}

  virtual TypePtr getRequiredFunctionType() const
    {return rTreeFunctionClass;}
  
  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const;

protected:
  friend class RTreeBatchLearnerClass;

  bool verbose;
};

/****** Specialisation ******/
class ClassificationRTreeFunction : public RTreeFunction
{
public:
  ClassificationRTreeFunction(size_t numTrees,
                              size_t numAttributeSamplesPerSplit,
                              size_t minimumSizeForSplitting)
    : RTreeFunction(numTrees,
                    numAttributeSamplesPerSplit,
                    minimumSizeForSplitting) {}
  
  virtual TypePtr getSupervisionType() const
    {return sumType(enumValueType, doubleVectorClass(enumValueType, probabilityType));}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    if (inputVariables[1]->getType()->inheritsFrom(enumValueType))
      return denseDoubleVectorClass(inputVariables[1]->getType(), probabilityType);
    return inputVariables[1]->getType();
  }

protected:
  friend class ClassificationRTreeFunctionClass;

  ClassificationRTreeFunction() {}
};
  
class RegressionRTreeFunction : public RTreeFunction
{
public:
  RegressionRTreeFunction(size_t numTrees,
                          size_t numAttributeSamplesPerSplit,
                          size_t minimumSizeForSplitting)
    : RTreeFunction(numTrees,
                    numAttributeSamplesPerSplit,
                    minimumSizeForSplitting) {}
  
  virtual TypePtr getSupervisionType() const
    {return doubleType;}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}

protected:
  friend class RegressionRTreeFunctionClass;

  RegressionRTreeFunction() {}
};

class BinaryRTreeFunction : public RTreeFunction
{
public:
  BinaryRTreeFunction(size_t numTrees,
                      size_t numAttributeSamplesPerSplit,
                      size_t minimumSizeForSplitting)
    : RTreeFunction(numTrees,
                    numAttributeSamplesPerSplit,
                    minimumSizeForSplitting) {}
  
  virtual TypePtr getSupervisionType() const
    {return sumType(probabilityType, booleanType);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}

protected:
  friend class BinaryRTreeFunctionClass;

  BinaryRTreeFunction() {}
};

}; /* namespace lbcpp */

#endif // !LBCPP_DECISION_TREE_R_TREE_FUNCTION_H_
