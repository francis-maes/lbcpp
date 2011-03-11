/*-----------------------------------------.---------------------------------.
 | Filename: OrderedMap.h                   | OrderedMap of Variables         |
 | Author  : Arnaud Schoofs                 |                                 |
 | Started : 10/03/2011 21:41               |                                 |
 `------------------------------------------/                                 |
                                            |                                 |
                                            `--------------------------------*/

#ifndef LBCPP_CORE_ORDERED_MAP_H_
# define LBCPP_CORE_ORDERED_MAP_H_

# include "Map.h"

namespace lbcpp
{
  
class OrderedMap : public Map
{
public:
  OrderedMap(ClassPtr thisClass) : Map(thisClass) {}
  OrderedMap() {}


  lbcpp_UseDebuggingNewOperator

};


}; /* namespace lbcpp */

#endif // !LBCPP_CORE_ORDERED_MAP_H_
