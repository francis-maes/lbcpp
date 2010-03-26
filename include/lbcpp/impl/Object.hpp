/*-----------------------------------------.---------------------------------.
| Filename: Object.hpp                     | Base class for static objects   |
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 19:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_CORE_IMPL_OBJECT_H_
# define LBCPP_CORE_IMPL_OBJECT_H_

# include "../ObjectPredeclarations.h"
# include "Macros.hpp"

namespace lbcpp {
namespace impl {
 
template<class ExactType>
struct Object
{
  String toString() const {return "";}
  String getName() const {return "";}
  
  void save(std::ostream& ostr) const {}
  bool load(std::istream& istr) {return true;}

  static void error(const String& where, const String& what)
    {ErrorHandler::error(where, what);}
    
  static void warning(const String& where, const String& what)
    {ErrorHandler::warning(where, what);}

protected:
  ExactType& _this()
    {return *(ExactType* )this;}

  const ExactType& _this() const
    {return *(const ExactType* )this;}
};
  
}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_H_
