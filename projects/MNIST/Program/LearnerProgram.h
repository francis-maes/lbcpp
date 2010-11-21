#ifndef LBCPP_LEARNER_PROGRAM_PROGRAM_H_
# define LBCPP_LEARNER_PROGRAM_PROGRAM_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class LearnerProgram : public Program
{
public:
  LearnerProgram() : output(File::getCurrentWorkingDirectory().getChildFile(T("result"))),
                     numIterations(50), binarizationThreshold(0.05), regularizer(0.0),
                     numTrees(100), numAttr(10), splitSize(2)
  {}
  
  virtual String toString() const
    {return T("Program developed for the challenge of the course ELEN0062, at University of Liege, in 2010.");}
  
  virtual int runProgram(MessageCallback& callback);
  
protected:
  friend class LearnerProgramClass;

  File learningFile;
  File testingFile;
  File output;
  
  size_t numIterations;
  double binarizationThreshold;
  
  double regularizer;

  size_t numTrees;
  size_t numAttr;
  size_t splitSize;

private:
  ContainerPtr learningData;
  ContainerPtr testingData;
  
  bool loadData();
  InferenceOnlineLearnerPtr createOnlineLearner() const;
};

};

#endif // !LBCPP_LEARNER_PROGRAM_PROGRAM_H_
