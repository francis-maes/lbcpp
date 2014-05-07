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
 *  A Sampler is used to generate inputs for the Problem.
 */
class ProblemDataStream : public DataStream
{
public:
  /** Constructor
   *  \param problem The Problem from which samples will be taken
   *  \param sampler The Sampler used to generate inputs for problem
   */
  ProblemDataStream(ProblemPtr problem, SamplerPtr sampler) :
    problem(problem), sampler(sampler) {}
  ProblemDataStream() {}

  virtual bool hasNext() const
    {return true;}

  virtual ObjectPtr next(ExecutionContext& context)
    {return problem->evaluate(context, sampler->sample(context));}

protected:
  friend class ProblemDataStreamClass;

  ProblemPtr problem;
  SamplerPtr sampler;
};



} /* namespace lbcpp */


#endif //!ML_DATA_STREAM_H_
