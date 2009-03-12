/*-----------------------------------------.---------------------------------.
| Filename: Object.hpp                     | Base class for static objects   |
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 19:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_IMPL_OBJECT_H_
# define CRALGO_IMPL_OBJECT_H_

# include "../Object.h"
# include "Macros.hpp"

namespace cralgo {
namespace impl {
 
template<class ExactType>
struct Object
{
  std::string toString() const {return "";}
  std::string getName() const {return "";}
  
  void save(std::ostream& ostr) const {}
  bool load(std::istream& istr) {return true;}
};
  
}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_OBJECT_H_
