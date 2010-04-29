INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}
)

ADD_CRALGORITHM_SOURCES(
  InferenceData/CommonFeatureGenerators.lcpp
  InferenceData/Sequence.lcpp
  Protein/ProteinBackboneBondSequence.lcpp
  Protein/Inference/ProteinResidueFeatures.lcpp
  Protein/Inference/ProteinResiduePairFeatures.lcpp
)

SET(LBCPP_INFERENCE_DATA_SOURCES
  InferenceData/CommonFeatureGenerators.h
  InferenceData/AccumulatedScoresMatrix.h
  InferenceData/Sequence.h
  InferenceData/ScalarSequence.h
  InferenceData/ScalarSequence.cpp
  InferenceData/LabelSequence.h
  InferenceData/LabelSequence.cpp
  InferenceData/ScoreVectorSequence.h
  InferenceData/ScoreVectorSequence.cpp
  InferenceData/CartesianCoordinatesSequence.h
  InferenceData/BondCoordinatesSequence.h
  InferenceData/BondCoordinatesSequence.cpp
  InferenceData/ScoreSymmetricMatrix.h
  InferenceData/ScoreSymmetricMatrix.cpp
)

SET(LBCPP_INFERENCE_STEP_SOURCES
  InferenceStep/InferenceStep.h
  InferenceStep/InferenceStep.cpp
  InferenceStep/ClassificationInferenceStep.h
  InferenceStep/RegressionInferenceStep.h
  InferenceStep/SequentialInferenceStep.h
  InferenceStep/SequentialInferenceStep.cpp
  InferenceStep/ParallelInferenceStep.h
  InferenceStep/DecoratorInferenceStep.h
  InferenceStep/DecoratorInferenceStep.cpp
  InferenceStep/ParallelSharedMultiRegressionInferenceStep.h
)

SET(LBCPP_INFERENCE_CONTEXT_SOURCES
  InferenceContext/InferenceVisitor.h
  InferenceContext/InferenceVisitor.cpp
  InferenceContext/InferenceStack.h
  InferenceContext/InferenceContext.h
  InferenceContext/InferenceContext.cpp
  InferenceContext/InferenceCallback.h
  InferenceContext/CancelAfterStepCallback.h
  InferenceContext/CacheInferenceCallback.h
  InferenceContext/ExamplesCreatorCallback.h
)

SET(LBCPP_INFERENCE_LEARNER_SOURCES
  InferenceLearner/InferenceLearnerCallback.h
  InferenceLearner/InferenceLearner.h
  InferenceLearner/InferenceLearner.cpp
  InferenceLearner/GlobalSimulationInferenceLearner.h
  InferenceLearner/SingleStepDeterministicSimulationInferenceLearner.h
)

SET(LBCPP_PROTEIN_GEOMETRY_SOURCES
  Geometry/DihedralAngle.h
  Geometry/Vector3.h
  Geometry/Matrix3.h
  Geometry/Matrix3.cpp  
  Geometry/Matrix4.h
  Geometry/Matrix4.cpp
)

SET(LBCPP_EVALUATOR_SOURCES
  Evaluator/Evaluator.h
  Evaluator/Evaluator.cpp
  Evaluator/ObjectContainerEvaluator.h
  Evaluator/BinaryClassificationConfusionEvaluator.h
  Evaluator/ClassificationAccuracyEvaluator.h
  Evaluator/RegressionErrorEvaluator.h
  Evaluator/ScoreVectorSequenceErrorEvaluator.h
)

SET(LBCPP_PROTEIN_SOURCES
  Protein/AminoAcidDictionary.h
  Protein/AminoAcidDictionary.cpp
  Protein/SecondaryStructureDictionary.h
  Protein/SecondaryStructureDictionary.cpp
  Protein/ProteinBackboneBondSequence.h
  Protein/ProteinResidue.h
  Protein/ProteinResidue.cpp
  Protein/ProteinTertiaryStructure.h
  Protein/ProteinTertiaryStructure.cpp
  Protein/Protein.h
  Protein/Protein.cpp
)

SET(LBCPP_PROTEIN_INFERENCE_STEP_SOURCES
  Protein/Inference/ProteinResidueFeatures.h
  Protein/Inference/ProteinResiduePairFeatures.h
  Protein/Inference/ProteinInferenceStepHelper.h
  Protein/Inference/Protein1DInferenceStep.h
  Protein/Inference/ProteinCAlphaBondSequenceInferenceStep.h
  Protein/Inference/ProteinBackboneBondSequenceInferenceStep.h
  Protein/Inference/ProteinTertiaryStructureInferenceStep.h
  Protein/Inference/Protein2DInferenceStep.h
  Protein/Inference/ProteinContactMapInferenceStep.h
  Protein/Inference/ProteinInference.h
)

SET(LBCPP_PROTEIN_EVALUATION_SOURCES
  Protein/Evaluation/ProteinEvaluationCallback.h
  Protein/Evaluation/ProteinStatisticsCalculator.h
)

SET(LBCPP_PROTEIN_FORMATS_SOURCES
  Protein/Formats/FASTAFileParser.h
  Protein/Formats/FASTAFileGenerator.h
  Protein/Formats/PDBFileParser.h
  Protein/Formats/PDBFileParser.cpp
  Protein/Formats/PDBFileGenerator.h
  Protein/Formats/PDBFileGenerator.cpp
  Protein/Formats/CASPFileGenerator.h
  Protein/Formats/CASPFileGenerator.cpp
  Protein/Formats/OrderDisorderRegionCASPFileGenerator.h
  Protein/Formats/ResidueResidueDistanceCASPFileGenerator.h
  Protein/Formats/TertiaryStructureCASPFileGenerator.h
  Protein/Formats/PSSMFileParser.h
  Protein/Formats/DSSPFileParser.h
)

SET(PROTEINS_CPP_SOURCES
  ${LBCPP_INFERENCE_DATA_SOURCES}
  ${LBCPP_INFERENCE_STEP_SOURCES}
  ${LBCPP_INFERENCE_CONTEXT_SOURCES}
  ${LBCPP_INFERENCE_LEARNER_SOURCES}
  ${LBCPP_PROTEIN_GEOMETRY_SOURCES}
  ${LBCPP_EVALUATOR_SOURCES}
  ${LBCPP_PROTEIN_SOURCES}
  ${LBCPP_PROTEIN_INFERENCE_STEP_SOURCES}
  ${LBCPP_PROTEIN_EVALUATION_SOURCES}
  ${LBCPP_PROTEIN_FORMATS_SOURCES}
  classes.cpp
  InferencePredeclarations.h
)

SET(PROTEINS_SOURCES
  ${CRALGORITHMS_OUTPUT_FILES}
  ${PROTEINS_CPP_SOURCES}
)

SOURCE_GROUP(GeneratedCode FILES ${CRALGORITHMS_OUTPUT_FILES})
SOURCE_GROUP(InferenceData FILES ${LBCPP_INFERENCE_DATA_SOURCES} InferenceData/CommonFeatureGenerators.lcpp InferenceData/Sequence.lcpp)
SOURCE_GROUP(InferenceStep FILES ${LBCPP_INFERENCE_STEP_SOURCES})
SOURCE_GROUP(InferenceContext FILES ${LBCPP_INFERENCE_CONTEXT_SOURCES})
SOURCE_GROUP(InferenceLearner FILES ${LBCPP_INFERENCE_LEARNER_SOURCES})
SOURCE_GROUP(Evaluator FILES ${LBCPP_EVALUATOR_SOURCES})
SOURCE_GROUP(Geometry FILES ${LBCPP_PROTEIN_GEOMETRY_SOURCES})
SOURCE_GROUP(Protein FILES ${LBCPP_PROTEIN_SOURCES} Protein/ProteinBackboneBondSequence.lcpp)
SOURCE_SUB_GROUP(Protein Inference FILES ${LBCPP_PROTEIN_INFERENCE_STEP_SOURCES} Protein/Inference/ProteinResidueFeatures.lcpp Protein/Inference/ProteinResiduePairFeatures.lcpp)
SOURCE_SUB_GROUP(Protein Evaluation FILES ${LBCPP_PROTEIN_EVALUATION_SOURCES})
SOURCE_SUB_GROUP(Protein Formats FILES ${LBCPP_PROTEIN_FORMATS_SOURCES})

ADD_LIBRARY(proteins ${PROTEINS_SOURCES})
TARGET_LINK_LIBRARIES(proteins lbcpp-core)

MACRO(ADD_SINGLE_CPP_EXECUTABLE NAME)
  ADD_EXECUTABLE(${NAME} Programs/${NAME}.cpp)
  TARGET_LINK_LIBRARIES(${NAME} proteins)
ENDMACRO(ADD_SINGLE_CPP_EXECUTABLE)

ADD_SINGLE_CPP_EXECUTABLE(CASPPredictor)
ADD_SINGLE_CPP_EXECUTABLE(ConvertPDBToProtein)
ADD_SINGLE_CPP_EXECUTABLE(ConvertPDBToCASPTS)
ADD_SINGLE_CPP_EXECUTABLE(ComputePSSMs)
ADD_SINGLE_CPP_EXECUTABLE(CompileProteins)
ADD_SINGLE_CPP_EXECUTABLE(ProteinInferenceEvaluator)
ADD_SINGLE_CPP_EXECUTABLE(TertiaryStructureRepresentationTests)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_CREXECUTABLE(SandBox Programs/SandBox.lcpp)
TARGET_LINK_LIBRARIES(SandBox proteins)

ADD_CREXECUTABLE(TrainStepByStepSS Programs/TrainStepByStepSS.lcpp)
TARGET_LINK_LIBRARIES(TrainStepByStepSS proteins)
