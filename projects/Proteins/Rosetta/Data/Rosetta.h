/*-----------------------------------------.---------------------------------.
| Filename: Rosetta.h                      | Rosetta                         |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Dec 2, 2011  10:54:57 AM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_ROSETTA_DATA_ROSETTA_H_
# define LBCPP_PROTEIN_ROSETTA_DATA_ROSETTA_H_

//# define LBCPP_PROTEIN_ROSETTA

namespace lbcpp
{

class Rosetta;
typedef ReferenceCountedObjectPtr<Rosetta> RosettaPtr;

class Rosetta : public Object
{
public:
  Rosetta();
  ~Rosetta();

  void init(ExecutionContext& context, bool verbose, int id, size_t delay);

  void getLock();
  void releaseLock();

protected:
  void setContext(ExecutionContext& context);

  friend class RosettaClass;

  ExecutionContextPtr context;
  CriticalSection* ownLock;
};

extern ClassPtr rosettaClass;

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEIN_ROSETTA_DATA_ROSETTA_H_
