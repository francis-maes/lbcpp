/*-----------------------------------------.---------------------------------.
| Filename: Rosetta.h                      | Rosetta                         |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Dec 2, 2011  10:54:57 AM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_ROSETTA_DATA_ROSETTA_H_
# define LBCPP_PROTEIN_ROSETTA_DATA_ROSETTA_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class Rosetta;
typedef ReferenceCountedObjectPtr<Rosetta> RosettaPtr;

class Rosetta : public Object
{
public:
  Rosetta();
  Rosetta(ExecutionContext& context);
  Rosetta(Rosetta& rosetta);

  void setContext(ExecutionContext& context);

  void init();

  void getLock();
  void releaseLock();

protected:
  CriticalSection* lock;
  ExecutionContextPtr context;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEIN_ROSETTA_DATA_ROSETTA_H_
