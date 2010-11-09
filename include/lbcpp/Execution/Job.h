/*-----------------------------------------.---------------------------------.
| Filename: Job.h                          | Job                             |
| Author  : Francis Maes                   |                                 |
| Started : 08/10/2010 18:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_JOB_H_
# define LBCPP_EXECUTION_JOB_H_

# include "../Data/Object.h"
# include "../Data/Cache.h"
# include <list>

namespace lbcpp
{

class Job : public NameableObject
{
public:
  Job(const String& name)
    : NameableObject(name), jobShouldExit(false) {}
  Job() : jobShouldExit(false) {}

  virtual String getCurrentStatus() const = 0;

  virtual bool runJob(String& failureReason) = 0;

  bool shouldExit() const
    {return jobShouldExit;}

  void signalJobShouldExit()
    {jobShouldExit = true;}

private:
  friend class JobClass;

  bool volatile jobShouldExit;
};

typedef ReferenceCountedObjectPtr<Job> JobPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_JOB_H_
