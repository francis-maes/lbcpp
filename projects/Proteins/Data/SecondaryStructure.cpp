/*-----------------------------------------.---------------------------------.
| Filename: SecondaryStructure.cpp         | Secondary Structure related     |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 01:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "SecondaryStructure.h"
using namespace lbcpp;

SecondaryStructureElement lbcpp::dsspSecondaryStructureToSecondaryStructure(DSSPSecondaryStructureElement element)
{
  if (element == threeTurnHelix || element == alphaHelix)
    return helix;
  if (element == residueInIsolatedBridge || element == extendedStrandInSheet)
    return sheet;
  return other;
}
