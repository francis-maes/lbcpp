#ifndef LBCPP_MNIST_PROGRAM_PROGRAM_H_
# define LBCPP_MNIST_PROGRAM_PROGRAM_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class MNISTProgram : public Program
{
public:
  virtual String toString() const
    {return T("Program developed for the challenge of the course ELEN0062, at University of Liège, in 2010.");}
  
  virtual int runProgram(MessageCallback& callback);
  
protected:
  friend class MNISTProgramClass;

  File learningFile;
  File testingFile;

private:
  ContainerPtr learningData;
  ContainerPtr testingData;
  
  ContainerPtr loadDataFromFile(const File& file);
  bool loadData();
};

};

#endif // !LBCPP_MNIST_PROGRAM_PROGRAM_H_