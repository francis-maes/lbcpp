/*-----------------------------------------.---------------------------------.
| Filename: DataStream.h                   | Data Stream                     |
| Author  : Denny Verbeeck                 | Represents a stream of data for |
| Started : 12/12/2013 08:58               | e.g. incremental learning       |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_DATA_STREAM_H_
# define ML_DATA_STREAM_H_

# include <ml/Sampler.h>
# include <ml/Problem.h>

namespace lbcpp
{

/** The DataStream class represents a stream of data. Objects can be
 *  retrieved one at a time.
 */
class DataStream : public Object
{
public:
  virtual bool hasNext() const = 0;   /*< Returns true if another object is ready to be retrieved */
  virtual ObjectPtr next(ExecutionContext& context) = 0;       /*< This method returns the next object in the data stream  */
};

typedef ReferenceCountedObjectPtr<DataStream> DataStreamPtr;

/** \brief This class creates a DataStream from a Problem.
 *  A Sampler is used to generate inputs for the Problem objectives.
 *  The amount of objects that can be drawn from this DataStream
 *  can be limited.
 */
class ProblemDataStream : public DataStream
{
public:
  /** Constructor
   *  \param problem The Problem from which samples will be taken
   *  \param sampler The Sampler used to generate inputs for problem
   *  \param numObjects The amount of samples that can be drawn from this DataStream, 0 means infinite
   */
  ProblemDataStream(ProblemPtr problem, SamplerPtr sampler, size_t numObjects = 0) :
    problem(problem), sampler(sampler), numObjects(numObjects), numObjectsDrawn(0) {}
  ProblemDataStream() {}

  virtual bool hasNext() const
  {
    if (!numObjects)
      return true;
    return numObjectsDrawn < numObjects;
  }

  virtual ObjectPtr next(ExecutionContext& context)
  {
    if (numObjectsDrawn == numObjects)
      return ObjectPtr();
    ++numObjectsDrawn;
    return problem->evaluate(context, sampler->sample(context));
  }

protected:
  friend class ProblemDataStreamClass;

  ProblemPtr problem;
  SamplerPtr sampler;
  size_t numObjects;
  size_t numObjectsDrawn;
};

} /* namespace lbcpp */


#endif //!ML_DATA_STREAM_H_
