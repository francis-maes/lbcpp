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

# ifdef LBCPP_PROTEIN_ROSETTA
#  undef T
#  include <core/init.hh>
#  include <utility/vector0.hh>
#  define T JUCE_T
# else // predeclare rosetta
namespace utility {namespace pointer{
  template< typename T > class owning_ptr;
}; };

namespace core { namespace pose {
  class Pose;
  typedef utility::pointer::owning_ptr< Pose > PoseOP;
}; };
# endif // LBCPP_PROTEIN_ROSETTA

namespace lbcpp
{

class Rosetta;
typedef ReferenceCountedObjectPtr<Rosetta> RosettaPtr;

class Rosetta : public Object
{
public:
  Rosetta();
  ~Rosetta();

  void init(ExecutionContext& context, bool verbose = false, int seed = -1);
  static VariableVectorPtr createRosettaPool(ExecutionContext& context, size_t size);

  void getLock();
  void releaseLock();

protected:
  void setContext(ExecutionContext& context);

  void getPoolLock();
  void releasePoolLock();

  friend class RosettaClass;

  ExecutionContextPtr context;
  CriticalSection* ownLock;
  CriticalSection* poolLock;
  size_t nProc;
  size_t id;
  bool isInPool;
};

extern ClassPtr rosettaClass;

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEIN_ROSETTA_DATA_ROSETTA_H_
