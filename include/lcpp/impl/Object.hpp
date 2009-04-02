/*-----------------------------------------.---------------------------------.
| Filename: Object.hpp                     | Base class for static objects   |
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 19:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LCPP_CORE_IMPL_OBJECT_H_
# define LCPP_CORE_IMPL_OBJECT_H_

# include "../ObjectPredeclarations.h"
# include "Macros.hpp"

namespace lcpp {
namespace impl {
 
template<class ExactType>
struct Object
{
  std::string toString() const {return "";}
  std::string getName() const {return "";}
  
  void save(std::ostream& ostr) const {}
  bool load(std::istream& istr) {return true;}

  static void error(const std::string& where, const std::string& what)
    {ErrorHandler::error(where, what);}
    
  static void warning(const std::string& where, const std::string& what)
    {ErrorHandler::warning(where, what);}

protected:
  ExactType& _this()
    {return *(ExactType* )this;}

  const ExactType& _this() const
    {return *(const ExactType* )this;}
};
  
}; /* namespace impl */
}; /* namespace lcpp */

#endif // !LCPP_OBJECT_H_
