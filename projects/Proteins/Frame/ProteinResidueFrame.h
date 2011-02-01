/*-----------------------------------------.---------------------------------.
| Filename: ProteinResidueFrame.h          | Protein Residue Frame           |
| Author  : Francis Maes                   |                                 |
| Started : 01/02/2011 14:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_RESIDUE_FRAME_H_
# define LBCPP_PROTEIN_RESIDUE_FRAME_H_

# include "ProteinFrame.h"

namespace lbcpp
{

class ProteinResidueFrame : public Object
{
public:
  ProteinResidueFrame(ProteinFrame* proteinFrame, size_t position)
    : proteinFrame(proteinFrame), position(position)
  {
  /*  FunctionPtr windowOperator = windowPerception(aminoAcidTypeEnumeration, 15);
    jassert(windowOperator);
    primaryStructureWindow = windowOperator->computeFunction(defaultExecutionContext(), new Pair(proteinFrame->getPrimaryStructure(), position)).getObject();
    */
  }

  ProteinResidueFrame() : position(0) {}

private:
  friend class ProteinResidueFrameClass;

  ProteinFrame* proteinFrame;

  size_t position;
  ObjectPtr primaryStructureWindow;
  ObjectPtr primaryStructureWindowFeatures;
};

extern ClassPtr proteinResidueFrameClass;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_RESIDUE_FRAME_H_

