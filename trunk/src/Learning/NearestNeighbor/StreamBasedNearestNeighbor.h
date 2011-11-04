/*-----------------------------------------.---------------------------------.
| Filename: StreamBasedNearestNeighbor.h   | Stream Based Nearest Neighbor   |
| Author  : Julien Becker                  |                                 |
| Started : 24/09/2011 13:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_STREAM_BASED_NEAREST_NEIGHBOR_H_
# define LBCPP_STREAM_BASED_NEAREST_NEIGHBOR_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/Learning/NearestNeighbor.h>
# include <lbcpp/Data/Stream.h>

namespace lbcpp
{

extern BatchLearnerPtr streamBasedNearestNeighborBatchLearner();

class StreamBasedNearestNeighbor : public Function
{
public:
  StreamBasedNearestNeighbor(const StreamPtr& stream, size_t numNeighbors, bool includeTheNearestNeighbor);
  StreamBasedNearestNeighbor(size_t numNeighbors, bool includeTheNearestNeighbor);

  virtual TypePtr getSupervisionType() const = 0;

  /* Function */
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)doubleVectorClass() : getSupervisionType();}

  virtual String getOutputPostFix() const
    {return String((int)numNeighbors) + T("-StreamNN");}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

protected:
  friend class StreamBasedNearestNeighborClass;
  friend class StreamBasedNearestNeighborBatchLearner;

  typedef std::multimap<double, Variable> ScoresMap;

  StreamPtr stream;
  size_t numNeighbors;
  bool includeTheNearestNeighbor;
  std::vector<double> standardDeviation;

  StreamBasedNearestNeighbor() {}

  virtual Variable computeOutput(ScoresMap& scoredIndices) const = 0;
};

typedef ReferenceCountedObjectPtr<StreamBasedNearestNeighbor> StreamBasedNearestNeighborPtr;
extern ClassPtr streamBasedNearestNeighborClass;

class ClassificationStreamBasedNearestNeighbor : public StreamBasedNearestNeighbor
{
public:
  ClassificationStreamBasedNearestNeighbor(const StreamPtr& stream, size_t numNeighbors, bool includeTheNearestNeighbor)
    : StreamBasedNearestNeighbor(stream, numNeighbors, includeTheNearestNeighbor) {}

  ClassificationStreamBasedNearestNeighbor(size_t numNeighbors, bool includeTheNearestNeighbor)
    : StreamBasedNearestNeighbor(numNeighbors, includeTheNearestNeighbor) {}

  virtual TypePtr getSupervisionType() const
    {return doubleVectorClass(enumValueType, probabilityType);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    enumeration = inputVariables[1]->getType()->getTemplateArgument(0).staticCast<Enumeration>();
    return denseDoubleVectorClass(enumeration, probabilityType);
  }

protected:
  friend class ClassificationStreamBasedNearestNeighborClass;

  EnumerationPtr enumeration;

  ClassificationStreamBasedNearestNeighbor() {}

  virtual Variable computeOutput(ScoresMap& scoredIndices) const;
};

class BinaryClassificationStreamBasedNearestNeighbor : public StreamBasedNearestNeighbor
{
public:
  BinaryClassificationStreamBasedNearestNeighbor(const StreamPtr& stream, size_t numNeighbors, bool includeTheNearestNeighbor)
    : StreamBasedNearestNeighbor(stream, numNeighbors, includeTheNearestNeighbor) {}

  BinaryClassificationStreamBasedNearestNeighbor(size_t numNeighbors, bool includeTheNearestNeighbor)
    : StreamBasedNearestNeighbor(numNeighbors, includeTheNearestNeighbor) {}

  virtual TypePtr getSupervisionType() const
    {return sumType(booleanType, probabilityType);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}

protected:
  friend class BinaryClassificationStreamBasedNearestNeighborClass;

  BinaryClassificationStreamBasedNearestNeighbor() {}

  virtual Variable computeOutput(ScoresMap& scoredIndices) const;
};

class StreamBasedNearestNeighborBatchLearner : public BatchLearner
{
public:
  virtual TypePtr getRequiredFunctionType() const
    {return streamBasedNearestNeighborClass;}
  
  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_STREAM_BASED_NEAREST_NEIGHBOR_H_
