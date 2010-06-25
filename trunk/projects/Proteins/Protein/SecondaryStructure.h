/*-----------------------------------------.---------------------------------.
| Filename: SecondaryStructure.h           | Secondary Structure related     |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 01:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_SECONDARY_STRUCTURE_H_
# define LBCPP_PROTEINS_SECONDARY_STRUCTURE_H_

# include <lbcpp/Object/Class.h>

namespace lbcpp
{

enum SecondaryStructureElement
{
  helix = 0, // G (threeTurnHelix) or H (alphaHelix)
  sheet, // B (residueInIsolatedBridge) or E (extendedStrandInSheet )
  other  // I, T, S or C
};

enum DSSPSecondaryStructureElement
{
  threeTurnHelix = 0,        // G
  alphaHelix,                // H
  piHelix,                   // I
  hydrogenBondedTurn,        // T
  extendedStrandInSheet,     // E
  residueInIsolatedBridge,   // B
  bend,                      // S
  coil                       // C -
};

/*enum StructuralAlphabetElement
{
  A = 0, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z, a
};*/

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_SECONDARY_STRUCTURE_H_
