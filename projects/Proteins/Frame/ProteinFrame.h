/*-----------------------------------------.---------------------------------.
| Filename: ProteinFrame.h                 | Protein Frame                   |
| Author  : Francis Maes                   |                                 |
| Started : 28/01/2011 14:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_FRAME_H_
# define LBCPP_PROTEIN_FRAME_H_

# include <lbcpp/lbcpp.h>
# include "../Data/Protein.h"

namespace lbcpp
{

extern FunctionPtr proteinResidueFeaturesVectorFunction();

class ProteinFrameFactory : public Object
{
public:
  FrameClassPtr createProteinFrameClass(ExecutionContext& context);
  FrameClassPtr createPrimaryResidueFrameClass(ExecutionContext& context, const FrameClassPtr& proteinFrameClass);
  FrameClassPtr createResidueFrameClass(ExecutionContext& context, const FrameClassPtr& proteinFrameClass);
  FrameClassPtr createProteinGlobalFrameClass(ExecutionContext& context, const FrameClassPtr& proteinFrameClass);

  FramePtr createFrame(const ProteinPtr& protein) const;

private:
  FrameClassPtr proteinFrame;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_FRAME_H_
