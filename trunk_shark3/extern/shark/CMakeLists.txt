FIND_PACKAGE( OpenMP QUIET )

#CMAKE_POLICY(SET CMP0003 NEW)

ENABLE_LANGUAGE(C) # Fix small issue with OpenMP on some platforms/versions

#####################################################################
#		Marks the official version                          #
#####################################################################
SET( SHARK_VERSION_MAJOR 3 )
SET( SHARK_VERSION_MINOR 0 )
SET( SHARK_VERSION_PATCH 0 )
set(SHARK_VERSION
  ${SHARK_VERSION_MAJOR}.${SHARK_VERSION_MINOR}.${SHARK_VERSION_PATCH})

ADD_DEFINITIONS(
 -DSHARK_VERSION_MAJOR=${SHARK_VERSION_MAJOR}
 -DSHARK_VERSION_MINOR=${SHARK_VERSION_MINOR}
 -DSHARK_VERSION_PATCH=${SHARK_VERSION_PATCH}
)

CONFIGURE_FILE (
  "include/shark/Core/Shark.h.in"
  "include/shark/Core/Shark.h"
  )

SET( OPT_DYNAMIC_LIBRARY OFF )

IF( OPT_DYNAMIC_LIBRARY )
    set(Boost_USE_STATIC_LIBS OFF)
    add_definitions(-DBOOST_TEST_DYN_LINK)
ELSE( OPT_DYNAMIC_LIBRARY )
    set(Boost_USE_STATIC_LIBS ON)
    SET( LIBRARY_FLAGS STATIC )
ENDIF( OPT_DYNAMIC_LIBRARY )

IF(CMAKE_BUILD_TYPE MATCHES "Debug")
	ADD_DEFINITIONS(-DDEBUG )
	ADD_DEFINITIONS("-DBOOST_UBLAS_TYPE_CHECK=0" )
	#we want to be warned in debug mode
	IF(UNIX)
		IF(CMAKE_COMPILER_IS_GNUCC)
			ADD_DEFINITIONS(-Wall)
		ENDIF(CMAKE_COMPILER_IS_GNUCC)
	ENDIF(UNIX)
ENDIF (CMAKE_BUILD_TYPE MATCHES "Debug")
IF(CMAKE_BUILD_TYPE MATCHES "Release")
	ADD_DEFINITIONS(-DNDEBUG )
	ADD_DEFINITIONS(-DBOOST_UBLAS_NDEBUG )
ENDIF (CMAKE_BUILD_TYPE MATCHES "Release")



#####################################################################
#                Adjust compiler flags and settings for MSVC							#
#####################################################################
IF( MSVC )
    message(STATUS "Win32")
	SET( DISABLE_WARNINGS "/wd4250 /wd4251 /wd4275 /wd4800 /wd4308" )
	#4250: inherit via dominance
	#4251: needs to have dll-interface
	#4275: non-dll interface used as base for dll-interface class
	#4308: Negative integral constant
	SET( ADDITIONAL_COMPILER_FLAGS "${DISABLE_WARNINGS}" )

	STRING( REPLACE "/O2" "/Od" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" )

	SET( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${ADDITIONAL_COMPILER_FLAGS}" )
	SET( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${ADDITIONAL_COMPILER_FLAGS}" )
	SET( CMAKE_CXX_FLAGS_MINSIZERELEASE "${CMAKE_CXX_FLAGS_MINSIZERELEASE} ${ADDITIONAL_COMPILER_FLAGS}" CACHE STRING "compiler-flags" FORCE )
	SET( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${ADDITIONAL_COMPILER_FLAGS}" CACHE STRING "compiler-flags" FORCE )

	SET( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" CACHE STRING "compiler-flags" FORCE )
	SET( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" CACHE STRING "compiler-flags" FORCE )
	SET( CMAKE_CXX_FLAGS_MINSIZERELEASE "${CMAKE_CXX_FLAGS_MINSIZERELEASE}" CACHE STRING "compiler-flags" FORCE )
	SET( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING "compiler-flags" FORCE )

	ADD_DEFINITIONS(-D_USE_MATH_DEFINES)
	ADD_DEFINITIONS(-DNOMINMAX)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE) 
	ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
	ADD_DEFINITIONS(-D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1)
ENDIF( MSVC )


IF( UNIX )
	IF( APPLE )
	  	SET(CMAKE_CXX_CREATE_STATIC_LIBRARY "libtool -static -o <TARGET> <LINK_FLAGS> <OBJECTS>" )
	ENDIF( APPLE )
ENDIF( UNIX )

#SET(LIBRARY_OUTPUT_PATH ${LBCPP_BIN_DIRECTORY}/lib)
INCLUDE_DIRECTORIES( include/ )

SET(ALGORITHMS_STOPPINGCRITERIA_INCLUDES
  include/shark/Algorithms/StoppingCriteria/TrainingError.h
  include/shark/Algorithms/StoppingCriteria/TrainingProgress.h
  include/shark/Algorithms/StoppingCriteria/AbstractStoppingCriterion.h
  include/shark/Algorithms/StoppingCriteria/GeneralizationLoss.h
  include/shark/Algorithms/StoppingCriteria/MaxIterations.h
  include/shark/Algorithms/StoppingCriteria/ValidatedStoppingCriterion.h
  include/shark/Algorithms/StoppingCriteria/GeneralizationQuotient.h
)
SOURCE_SUB_GROUP(Algorithms StoppingCriteria FILES ${ALGORITHMS_STOPPINGCRITERIA_INCLUDES})

SET(UNSUPERVISED_RBM_NEURONLAYERS_INCLUDES
  include/shark/Unsupervised/RBM/Neuronlayers/BinaryLayer.h
  include/shark/Unsupervised/RBM/Neuronlayers/GaussianLayer.h
  include/shark/Unsupervised/RBM/Neuronlayers/TruncatedExponentialLayer.h
)
SOURCE_SUB_GROUP2(Unsupervised RBM Neuronlayers FILES ${UNSUPERVISED_RBM_NEURONLAYERS_INCLUDES})

SET(LINALG_BLAS_INCLUDES
  include/shark/LinAlg/BLAS/VectorTransformations.h
  include/shark/LinAlg/BLAS/Proxy.h
  include/shark/LinAlg/BLAS/fastOperations.h
  include/shark/LinAlg/BLAS/StorageAdaptors.h
  include/shark/LinAlg/BLAS/BlockProducts.h
  include/shark/LinAlg/BLAS/VectorMatrixType.h
  include/shark/LinAlg/BLAS/Initialize.h
#  include/shark/LinAlg/BLAS/ublas.h
  include/shark/LinAlg/BLAS/Tools.h
  include/shark/LinAlg/BLAS/Permutation.h
  include/shark/LinAlg/BLAS/MatrixVectorOperation.h
  include/shark/LinAlg/BLAS/Metrics.h
)
SOURCE_SUB_GROUP(LinAlg BLAS FILES ${LINALG_BLAS_INCLUDES})

SET(CORE_LOGHANDLERS_INCLUDES
  include/shark/Core/LogHandlers/StreamHandler.h
)
SOURCE_SUB_GROUP(Core LogHandlers FILES ${CORE_LOGHANDLERS_INCLUDES})

SET(UNSUPERVISED_RBM_SAMPLING_INCLUDES
  include/shark/Unsupervised/RBM/Sampling/MarkovChain.h
  include/shark/Unsupervised/RBM/Sampling/TemperedMarkovChain.h
  include/shark/Unsupervised/RBM/Sampling/GibbsOperator.h
)
SOURCE_SUB_GROUP2(Unsupervised RBM Sampling FILES ${UNSUPERVISED_RBM_SAMPLING_INCLUDES})

SET(STATISTICS_DISTRIBUTIONS_INCLUDES
  include/shark/Statistics/Distributions/MultiVariateNormalDistribution.h
)
SOURCE_SUB_GROUP(Statistics Distributions FILES ${STATISTICS_DISTRIBUTIONS_INCLUDES})

SET(UNSUPERVISED_RBM_IMPL_INCLUDES
  include/shark/Unsupervised/RBM/Impl/AverageEnergyGradient1x1.h
  include/shark/Unsupervised/RBM/Impl/analytics.h
  include/shark/Unsupervised/RBM/Impl/RBMStructure.h
  include/shark/Unsupervised/RBM/Impl/EnergyInteractionTerm.h
)
SOURCE_SUB_GROUP2(Unsupervised RBM Impl FILES ${UNSUPERVISED_RBM_IMPL_INCLUDES})

SET(CORE_LOGFORMATTERS_INCLUDES
  include/shark/Core/LogFormatters/PrintfLogFormatter.h
)
SOURCE_SUB_GROUP(Core LogFormatters FILES ${CORE_LOGFORMATTERS_INCLUDES})

SET(LINALG_INCLUDES
  include/shark/LinAlg/Cholesky.h
  include/shark/LinAlg/eigenvalues.h
  include/shark/LinAlg/solveSystem.h
  include/shark/LinAlg/fft.h
  include/shark/LinAlg/arrayoptimize.h
  include/shark/LinAlg/VectorStatistics.h
  include/shark/LinAlg/Inverse.h
  include/shark/LinAlg/solveTriangular.h
  include/shark/LinAlg/RQ.h
  include/shark/LinAlg/Base.h
  include/shark/LinAlg/svd.h
  include/shark/LinAlg/rotations.h
)
SOURCE_GROUP(LinAlg FILES ${LINALG_INCLUDES})

SET(CORE_RENDERERS_INCLUDES
  include/shark/Core/Renderers/HighchartRenderer.h
)
SOURCE_SUB_GROUP(Core Renderers FILES ${CORE_RENDERERS_INCLUDES})

SET(ALGORITHMS_DIRECTSEARCH_INCLUDES
  include/shark/Algorithms/DirectSearch/FitnessExtractor.h
  include/shark/Algorithms/DirectSearch/AGE2.h
  include/shark/Algorithms/DirectSearch/CMSA.h
  include/shark/Algorithms/DirectSearch/HypervolumeCalculator.h
  include/shark/Algorithms/DirectSearch/SMS-EMOA.h
  include/shark/Algorithms/DirectSearch/HypervolumeIndicator.h
  include/shark/Algorithms/DirectSearch/ElitistCMA.h
  include/shark/Algorithms/DirectSearch/HypervolumeApproximator.h
  include/shark/Algorithms/DirectSearch/InterruptibleAlgorithmRunner.h
  include/shark/Algorithms/DirectSearch/SteadyStateMOCMA.h
  include/shark/Algorithms/DirectSearch/ParetoDominanceComparator.h
  include/shark/Algorithms/DirectSearch/PAES.h
  include/shark/Algorithms/DirectSearch/GridSearch.h
  include/shark/Algorithms/DirectSearch/FastNonDominatedSort.h
  include/shark/Algorithms/DirectSearch/RankShareComparator.h
  include/shark/Algorithms/DirectSearch/Population.h
  include/shark/Algorithms/DirectSearch/LeastContributorApproximator.hpp
  include/shark/Algorithms/DirectSearch/CMA.h
  include/shark/Algorithms/DirectSearch/AGE.h
  include/shark/Algorithms/DirectSearch/FitnessComparator.h
  include/shark/Algorithms/DirectSearch/TypedIndividual.h
  include/shark/Algorithms/DirectSearch/BoundingBoxCalculator.h
  include/shark/Algorithms/DirectSearch/EA.h
  include/shark/Algorithms/DirectSearch/MOCMA.h
  include/shark/Algorithms/DirectSearch/OnePlusOneES.h
  include/shark/Algorithms/DirectSearch/RealCodedNSGAII.h
  include/shark/Algorithms/DirectSearch/Grid.h
  include/shark/Algorithms/DirectSearch/XNES.h
)
SOURCE_SUB_GROUP(Algorithms DirectSearch FILES ${ALGORITHMS_DIRECTSEARCH_INCLUDES})

SET(STATISTICS_TESTS_INCLUDES
  include/shark/Statistics/Tests/WilcoxonRankSumTest.h
)
SOURCE_SUB_GROUP(Statistics Tests FILES ${STATISTICS_TESTS_INCLUDES})

SET(ALGORITHMS_DIRECTSEARCH_OPERATORS_MUTATION_CMA_INCLUDES
  include/shark/Algorithms/DirectSearch/Operators/Mutation/CMA/Mutator.h
)
SOURCE_SUB_GROUP4(Algorithms DirectSearch Operators Mutation CMA FILES ${ALGORITHMS_DIRECTSEARCH_OPERATORS_MUTATION_CMA_INCLUDES})

SET(OBJECTIVEFUNCTIONS_INCLUDES
  include/shark/ObjectiveFunctions/SpanBoundCSvm.h
  include/shark/ObjectiveFunctions/AbstractCost.h
  include/shark/ObjectiveFunctions/AbstractConstraintHandler.h
  include/shark/ObjectiveFunctions/KernelTargetAlignment.h
  include/shark/ObjectiveFunctions/NegativeAUC.h
  include/shark/ObjectiveFunctions/LooError.h
  include/shark/ObjectiveFunctions/NegativeGaussianProcessEvidence.h
  include/shark/ObjectiveFunctions/DataObjectiveFunction.h
  include/shark/ObjectiveFunctions/DenoisingAutoencoderError.h
  include/shark/ObjectiveFunctions/CombinedObjectiveFunction.h
  include/shark/ObjectiveFunctions/CrossValidationError.h
  include/shark/ObjectiveFunctions/SparseFFNetError.h
  include/shark/ObjectiveFunctions/SvmLogisticInterpretation.h
  include/shark/ObjectiveFunctions/BoxConstraintHandler.h
  include/shark/ObjectiveFunctions/LooErrorCSvm.h
  include/shark/ObjectiveFunctions/JaakkolaHeuristic.h
  include/shark/ObjectiveFunctions/RadiusMarginQuotient.h
  include/shark/ObjectiveFunctions/Regularizer.h
  include/shark/ObjectiveFunctions/ROC.h
  include/shark/ObjectiveFunctions/AbstractMultiObjectiveFunction.h
  include/shark/ObjectiveFunctions/ErrorFunction.h
  include/shark/ObjectiveFunctions/NoisyErrorFunction.h
  include/shark/ObjectiveFunctions/AbstractObjectiveFunction.h
)
SOURCE_GROUP(ObjectiveFunctions FILES ${OBJECTIVEFUNCTIONS_INCLUDES})

SET(LINALG_IMPL_INCLUDES
  include/shark/LinAlg/Impl/Cholesky.inl
  include/shark/LinAlg/Impl/linmin.inl
  include/shark/LinAlg/Impl/svdrank.inl
  include/shark/LinAlg/Impl/rank.inl
  include/shark/LinAlg/Impl/lnsrch.inl
  include/shark/LinAlg/Impl/eigensort.inl
  include/shark/LinAlg/Impl/eigensymmJacobi.inl
  include/shark/LinAlg/Impl/detsymm.inl
  include/shark/LinAlg/Impl/invert.inl
  include/shark/LinAlg/Impl/dlinmin.inl
  include/shark/LinAlg/Impl/pivotingRQ.inl
  include/shark/LinAlg/Impl/fft.inl
  include/shark/LinAlg/Impl/eigensymm.inl
  include/shark/LinAlg/Impl/rankDecomp.inl
  include/shark/LinAlg/Impl/svd.inl
  include/shark/LinAlg/Impl/eigen.inl
  include/shark/LinAlg/Impl/solveTriangular.inl
  include/shark/LinAlg/Impl/eigenerr.inl
  include/shark/LinAlg/Impl/VectorStatistics.inl
  include/shark/LinAlg/Impl/svdsort.inl
  include/shark/LinAlg/Impl/eigensymmJacobi2.inl
  include/shark/LinAlg/Impl/solveSystem.inl
  include/shark/LinAlg/Impl/g_inverse.inl
)
SOURCE_SUB_GROUP(LinAlg Impl FILES ${LINALG_IMPL_INCLUDES})

SET(MODELS_CLUSTERING_INCLUDES
  include/shark/Models/Clustering/SoftClusteringModel.h
  include/shark/Models/Clustering/Centroids.h
  include/shark/Models/Clustering/ClusteringModel.h
  include/shark/Models/Clustering/HardClusteringModel.h
  include/shark/Models/Clustering/AbstractClustering.h
  include/shark/Models/Clustering/HierarchicalClustering.h
)
SOURCE_SUB_GROUP(Models Clustering FILES ${MODELS_CLUSTERING_INCLUDES})

SET(ALGORITHMS_DIRECTSEARCH_OPERATORS_PARAMETERUPDATE_CMA_INCLUDES
  include/shark/Algorithms/DirectSearch/Operators/ParameterUpdate/CMA/Updater.h
)
SOURCE_SUB_GROUP4(Algorithms DirectSearch Operators ParameterUpdate CMA FILES ${ALGORITHMS_DIRECTSEARCH_OPERATORS_PARAMETERUPDATE_CMA_INCLUDES})

SET(FUZZY_LINGUISTICTERMS_INCLUDES
  include/shark/Fuzzy/LinguisticTerms/ComposedLT.h
  include/shark/Fuzzy/LinguisticTerms/TriangularLT.h
  include/shark/Fuzzy/LinguisticTerms/GeneralizedBellLT.h
  include/shark/Fuzzy/LinguisticTerms/CustomizedLT.h
  include/shark/Fuzzy/LinguisticTerms/SigmoidalLT.h
  include/shark/Fuzzy/LinguisticTerms/ConstantLT.h
  include/shark/Fuzzy/LinguisticTerms/BellLT.h
  include/shark/Fuzzy/LinguisticTerms/SingletonLT.h
  include/shark/Fuzzy/LinguisticTerms/TrapezoidLT.h
  include/shark/Fuzzy/LinguisticTerms/InfinityLT.h
)
SOURCE_SUB_GROUP(Fuzzy LinguisticTerms FILES ${FUZZY_LINGUISTICTERMS_INCLUDES})

SET(CORE_UTILITY_INCLUDES
  include/shark/Core/utility/Iterators.h
  include/shark/Core/utility/CopyConst.h
  include/shark/Core/utility/KeyValuePair.h
  include/shark/Core/utility/CanBeCalled.h
  include/shark/Core/utility/Range.h
  include/shark/Core/utility/ZipPair.h
  include/shark/Core/utility/ScopedHandle.h
  include/shark/Core/utility/functional.h
)
SOURCE_SUB_GROUP(Core utility FILES ${CORE_UTILITY_INCLUDES})

SET(FUZZY_FUZZYSETS_INCLUDES
  include/shark/Fuzzy/FuzzySets/ComposedFS.h
  include/shark/Fuzzy/FuzzySets/SingletonFS.h
  include/shark/Fuzzy/FuzzySets/SigmoidalFS.h
  include/shark/Fuzzy/FuzzySets/NDimFS.h
  include/shark/Fuzzy/FuzzySets/InfinityFS.h
  include/shark/Fuzzy/FuzzySets/HomogenousNDimFS.h
  include/shark/Fuzzy/FuzzySets/TrapezoidFS.h
  include/shark/Fuzzy/FuzzySets/ConstantFS.h
  include/shark/Fuzzy/FuzzySets/BellFS.h
  include/shark/Fuzzy/FuzzySets/GeneralizedBellFS.h
  include/shark/Fuzzy/FuzzySets/CustomizedFS.h
  include/shark/Fuzzy/FuzzySets/ComposedNDimFS.h
  include/shark/Fuzzy/FuzzySets/TriangularFS.h
)
SOURCE_SUB_GROUP(Fuzzy FuzzySets FILES ${FUZZY_FUZZYSETS_INCLUDES})

SET(ALGORITHMS_TRAINERS_DISTRIBUTION_INCLUDES
  include/shark/Algorithms/Trainers/Distribution/DistTrainerContainer.h
  include/shark/Algorithms/Trainers/Distribution/NormalTrainer.h
  include/shark/Algorithms/Trainers/Distribution/GenericDistTrainer.h
)
SOURCE_SUB_GROUP2(Algorithms Trainers Distribution FILES ${ALGORITHMS_TRAINERS_DISTRIBUTION_INCLUDES})

SET(UNSUPERVISED_RBM_GRADIENTAPPROXIMATIONS_IMPL_INCLUDES
  include/shark/Unsupervised/RBM/GradientApproximations/Impl/DataEvaluator.h
)
SOURCE_SUB_GROUP3(Unsupervised RBM GradientApproximations Impl FILES ${UNSUPERVISED_RBM_GRADIENTAPPROXIMATIONS_IMPL_INCLUDES})

SET(CORE_SEARCHSPACES_INCLUDES
  include/shark/Core/SearchSpaces/VectorSpace.h
)
SOURCE_SUB_GROUP(Core SearchSpaces FILES ${CORE_SEARCHSPACES_INCLUDES})

SET(DATA_INCLUDES
  include/shark/Data/DataView.h
  include/shark/Data/BatchInterfaceAdaptStruct.h
  include/shark/Data/DataDistribution.h
  include/shark/Data/json_spirit_writer_options.h
  include/shark/Data/Pgm.h
  include/shark/Data/Dataset.h
  include/shark/Data/json_spirit_writer_template.h
  include/shark/Data/Csv.h
  include/shark/Data/json_spirit_reader_template.h
  include/shark/Data/Libsvm.h
  include/shark/Data/HDF5.h
  include/shark/Data/MKLBatchInterface.h
  include/shark/Data/CVDatasetTools.h
  include/shark/Data/json_spirit_error_position.h
  include/shark/Data/json_spirit_value.h
  include/shark/Data/BatchInterface.h
  include/shark/Data/json_spirit_utils.h
  include/shark/Data/PrecomputedMatrix.h
  include/shark/Data/json_spirit_stream_reader.h
)
SOURCE_GROUP(Data FILES ${DATA_INCLUDES})

SET(ALGORITHMS_DIRECTSEARCH_OPERATORS_RECOMBINATION_INCLUDES
  include/shark/Algorithms/DirectSearch/Operators/Recombination/UniformCrossover.h
  include/shark/Algorithms/DirectSearch/Operators/Recombination/OnePointCrossover.h
  include/shark/Algorithms/DirectSearch/Operators/Recombination/GlobalIntermediateRecombination.h
  include/shark/Algorithms/DirectSearch/Operators/Recombination/SimulatedBinaryCrossover.h
)
SOURCE_SUB_GROUP3(Algorithms DirectSearch Operators Recombination FILES ${ALGORITHMS_DIRECTSEARCH_OPERATORS_RECOMBINATION_INCLUDES})

SET(ALGORITHMS_DIRECTSEARCH_OPERATORS_EVALUATION_INCLUDES
  include/shark/Algorithms/DirectSearch/Operators/Evaluation/PenalizingEvaluator.h
)
SOURCE_SUB_GROUP3(Algorithms DirectSearch Operators Evaluation FILES ${ALGORITHMS_DIRECTSEARCH_OPERATORS_EVALUATION_INCLUDES})

SET(UNSUPERVISED_RBM_SAMPLING_IMPL_INCLUDES
  include/shark/Unsupervised/RBM/Sampling/Impl/SampleTypes.h
)
SOURCE_SUB_GROUP3(Unsupervised RBM Sampling Impl FILES ${UNSUPERVISED_RBM_SAMPLING_IMPL_INCLUDES})

SET(UNSUPERVISED_RBM_PROBLEMS_INCLUDES
  include/shark/Unsupervised/RBM/Problems/BarsAndStripes.h
  include/shark/Unsupervised/RBM/Problems/MNIST.h
  include/shark/Unsupervised/RBM/Problems/Shifter.h
  include/shark/Unsupervised/RBM/Problems/DistantModes.h
)
SOURCE_SUB_GROUP2(Unsupervised RBM Problems FILES ${UNSUPERVISED_RBM_PROBLEMS_INCLUDES})

SET(ALGORITHMS_DIRECTSEARCH_OPERATORS_MUTATION_INCLUDES
  include/shark/Algorithms/DirectSearch/Operators/Mutation/BitflipMutator.h
  include/shark/Algorithms/DirectSearch/Operators/Mutation/PolynomialMutation.h
)
SOURCE_SUB_GROUP3(Algorithms DirectSearch Operators Mutation FILES ${ALGORITHMS_DIRECTSEARCH_OPERATORS_MUTATION_INCLUDES})

SET(ALGORITHMS_DIRECTSEARCH_OPERATORS_INITIALIZERS_INCLUDES
  include/shark/Algorithms/DirectSearch/Operators/Initializers/CovarianceMatrixInitializer.h
)
SOURCE_SUB_GROUP3(Algorithms DirectSearch Operators Initializers FILES ${ALGORITHMS_DIRECTSEARCH_OPERATORS_INITIALIZERS_INCLUDES})

SET(CORE_UTILITY_IMPL_INCLUDES
  include/shark/Core/utility/Impl/boost_iterator_facade_fixed.hpp
)
SOURCE_SUB_GROUP2(Core utility Impl FILES ${CORE_UTILITY_IMPL_INCLUDES})

SET(ALGORITHMS_INCLUDES
  include/shark/Algorithms/AbstractMultiObjectiveOptimizer.h
  include/shark/Algorithms/Pegasos.h
  include/shark/Algorithms/AbstractOptimizer.h
  include/shark/Algorithms/AbstractSingleObjectiveOptimizer.h
  include/shark/Algorithms/JaakkolaHeuristic.h
  include/shark/Algorithms/KMeans.h
)
SOURCE_GROUP(Algorithms FILES ${ALGORITHMS_INCLUDES})

SET(ALGORITHMS_LP_INCLUDES
  include/shark/Algorithms/LP/GLPK.h
)
SOURCE_SUB_GROUP(Algorithms LP FILES ${ALGORITHMS_LP_INCLUDES})

SET(ALGORITHMS_DIRECTSEARCH_CMA_INCLUDES
  include/shark/Algorithms/DirectSearch/CMA/Chromosome.h
)
SOURCE_SUB_GROUP2(Algorithms DirectSearch CMA FILES ${ALGORITHMS_DIRECTSEARCH_CMA_INCLUDES})

SET(LINALG_BLAS_IMPL_NUMERIC_BINDINGS_ATLAS_INCLUDES
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/atlas/syrk.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/atlas/trsm.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/atlas/gemm.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/atlas/potrf.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/atlas/trsv.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/atlas/gemv.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/atlas/potri.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/atlas/potrs.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/atlas/cblas_inc.h
)
SOURCE_SUB_GROUP4(LinAlg BLAS Impl numeric_bindings atlas FILES ${LINALG_BLAS_IMPL_NUMERIC_BINDINGS_ATLAS_INCLUDES})

SET(ALGORITHMS_GRADIENTDESCENT_IMPL_INCLUDES
  include/shark/Algorithms/GradientDescent/Impl/ObjectiveFunctionWrapper.h
)
SOURCE_SUB_GROUP2(Algorithms GradientDescent Impl FILES ${ALGORITHMS_GRADIENTDESCENT_IMPL_INCLUDES})

SET(OBJECTIVEFUNCTIONS_BENCHMARKS_INCLUDES
  include/shark/ObjectiveFunctions/Benchmarks/DTLZ1.h
  include/shark/ObjectiveFunctions/Benchmarks/CIGTAB2.h
  include/shark/ObjectiveFunctions/Benchmarks/Discus.h
  include/shark/ObjectiveFunctions/Benchmarks/ZDT6.h
  include/shark/ObjectiveFunctions/Benchmarks/LZ6.h
  include/shark/ObjectiveFunctions/Benchmarks/ZDT2.h
  include/shark/ObjectiveFunctions/Benchmarks/Benchmarks.h
  include/shark/ObjectiveFunctions/Benchmarks/LZ2.h
  include/shark/ObjectiveFunctions/Benchmarks/LZ1.h
  include/shark/ObjectiveFunctions/Benchmarks/Rosenbrock.h
  include/shark/ObjectiveFunctions/Benchmarks/ZDT4.h
  include/shark/ObjectiveFunctions/Benchmarks/Sympart.h
  include/shark/ObjectiveFunctions/Benchmarks/GSP.h
  include/shark/ObjectiveFunctions/Benchmarks/DiffPowers.h
  include/shark/ObjectiveFunctions/Benchmarks/LZ9.h
  include/shark/ObjectiveFunctions/Benchmarks/Ellipsoid.h
  include/shark/ObjectiveFunctions/Benchmarks/LZ3.h
  include/shark/ObjectiveFunctions/Benchmarks/IHR2.h
  include/shark/ObjectiveFunctions/Benchmarks/Ackley.h
  include/shark/ObjectiveFunctions/Benchmarks/DTLZ5.h
  include/shark/ObjectiveFunctions/Benchmarks/ZDT1.h
  include/shark/ObjectiveFunctions/Benchmarks/CigarDiscus.h
  include/shark/ObjectiveFunctions/Benchmarks/DTLZ4.h
  include/shark/ObjectiveFunctions/Benchmarks/LZ4.h
  include/shark/ObjectiveFunctions/Benchmarks/ELLI2.h
  include/shark/ObjectiveFunctions/Benchmarks/Fonseca.h
  include/shark/ObjectiveFunctions/Benchmarks/IHR6.h
  include/shark/ObjectiveFunctions/Benchmarks/DTLZ2.h
  include/shark/ObjectiveFunctions/Benchmarks/IHR1.h
  include/shark/ObjectiveFunctions/Benchmarks/CIGTAB1.h
  include/shark/ObjectiveFunctions/Benchmarks/IHR3.h
  include/shark/ObjectiveFunctions/Benchmarks/DTLZ6.h
  include/shark/ObjectiveFunctions/Benchmarks/LZ8.h
  include/shark/ObjectiveFunctions/Benchmarks/IHR4.h
  include/shark/ObjectiveFunctions/Benchmarks/DTLZ7.h
  include/shark/ObjectiveFunctions/Benchmarks/LZ7.h
  include/shark/ObjectiveFunctions/Benchmarks/DTLZ3.h
  include/shark/ObjectiveFunctions/Benchmarks/Cigar.h
  include/shark/ObjectiveFunctions/Benchmarks/ZDT3.h
  include/shark/ObjectiveFunctions/Benchmarks/ELLI1.h
  include/shark/ObjectiveFunctions/Benchmarks/Sphere.h
  include/shark/ObjectiveFunctions/Benchmarks/LZ5.h
  include/shark/ObjectiveFunctions/Benchmarks/Himmelblau.h
)
SOURCE_SUB_GROUP(ObjectiveFunctions Benchmarks FILES ${OBJECTIVEFUNCTIONS_BENCHMARKS_INCLUDES})

SET(ALGORITHMS_GRADIENTDESCENT_INCLUDES
  include/shark/Algorithms/GradientDescent/SteepestDescent.h
  include/shark/Algorithms/GradientDescent/LineSearch.h
  include/shark/Algorithms/GradientDescent/NoisyRprop.h
  include/shark/Algorithms/GradientDescent/Rprop.h
  include/shark/Algorithms/GradientDescent/IRLS.h
  include/shark/Algorithms/GradientDescent/Quickprop.h
  include/shark/Algorithms/GradientDescent/CG.h
  include/shark/Algorithms/GradientDescent/BFGS.h
)
SOURCE_SUB_GROUP(Algorithms GradientDescent FILES ${ALGORITHMS_GRADIENTDESCENT_INCLUDES})

SET(STATISTICS_INCLUDES
  include/shark/Statistics/Statistics.h
)
SOURCE_GROUP(Statistics FILES ${STATISTICS_INCLUDES})

SET(ALGORITHMS_NEARESTNEIGHBORS_INCLUDES
  include/shark/Algorithms/NearestNeighbors/AbstractNearestNeighbors.h
  include/shark/Algorithms/NearestNeighbors/SimpleNearestNeighbors.h
  include/shark/Algorithms/NearestNeighbors/TreeNearestNeighbors.h
)
SOURCE_SUB_GROUP(Algorithms NearestNeighbors FILES ${ALGORITHMS_NEARESTNEIGHBORS_INCLUDES})

SET(RNG_INCLUDES
  include/shark/Rng/Erlang.h
  include/shark/Rng/Entropy.h
  include/shark/Rng/Uniform.h
  include/shark/Rng/DiscreteUniform.h
  include/shark/Rng/Poisson.h
  include/shark/Rng/Normal.h
  include/shark/Rng/AbstractDistribution.h
  include/shark/Rng/HyperGeometric.h
  include/shark/Rng/Weibull.h
  include/shark/Rng/Binomial.h
  include/shark/Rng/DiffGeometric.h
  include/shark/Rng/KullbackLeiberDivergence.h
  include/shark/Rng/Geometric.h
  include/shark/Rng/LogNormal.h
  include/shark/Rng/TruncatedExponential.h
  include/shark/Rng/NegExponential.h
  include/shark/Rng/Dirichlet.h
  include/shark/Rng/Bernoulli.h
  include/shark/Rng/Rng.h
  include/shark/Rng/GlobalRng.h
  include/shark/Rng/Gamma.h
  include/shark/Rng/Cauchy.h
)
SOURCE_GROUP(Rng FILES ${RNG_INCLUDES})

SET(ALGORITHMS_QP_INCLUDES
  include/shark/Algorithms/QP/QpBoxLinear.h
  include/shark/Algorithms/QP/LRUCache.h
  include/shark/Algorithms/QP/QpSvmDecomp.h
  include/shark/Algorithms/QP/QuadraticProgram.h
  include/shark/Algorithms/QP/QpMcDecomp.h
  include/shark/Algorithms/QP/QpBoxDecomp.h
  include/shark/Algorithms/QP/QpSparseArray.h
  include/shark/Algorithms/QP/QpMcLinear.h
)
SOURCE_SUB_GROUP(Algorithms QP FILES ${ALGORITHMS_QP_INCLUDES})

SET(ALGORITHMS_DIRECTSEARCH_OPERATORS_INITIALIZERS_CMA_INCLUDES
  include/shark/Algorithms/DirectSearch/Operators/Initializers/CMA/Initializer.h
)
SOURCE_SUB_GROUP4(Algorithms DirectSearch Operators Initializers CMA FILES ${ALGORITHMS_DIRECTSEARCH_OPERATORS_INITIALIZERS_CMA_INCLUDES})

SET(CORE_INCLUDES
  include/shark/Core/SharedVector.h
  include/shark/Core/Logger.h
  include/shark/Core/Math.h
  include/shark/Core/Timer.h
  include/shark/Core/Shark.h
  include/shark/Core/State.h
  include/shark/Core/IConfigurable.h
  include/shark/Core/Exception.h
  include/shark/Core/SignalTrap.h
  include/shark/Core/OpenMP.h
  include/shark/Core/Flags.h
  include/shark/Core/AbstractFeasibleRegion.h
  include/shark/Core/Probe.h
  include/shark/Core/Singleton.h
  include/shark/Core/INameable.h
  include/shark/Core/Factory.h
  include/shark/Core/IParameterizable.h
  include/shark/Core/ISerializable.h
  include/shark/Core/ResultSets.h
  include/shark/Core/Chart.h
  include/shark/Core/Derivative.h
)
SOURCE_GROUP(Core FILES ${CORE_INCLUDES})

SET(MODELS_KERNELS_INCLUDES
  include/shark/Models/Kernels/MissingFeaturesKernelExpansion.h
  include/shark/Models/Kernels/MultiTaskKernel.h
  include/shark/Models/Kernels/WeightedSumKernel.h
  include/shark/Models/Kernels/ArdKernel.h
  include/shark/Models/Kernels/MonomialKernel.h
  include/shark/Models/Kernels/ProductKernel.h
  include/shark/Models/Kernels/MklKernel.h
  include/shark/Models/Kernels/ScaledKernel.h
  include/shark/Models/Kernels/KernelExpansion.h
  include/shark/Models/Kernels/AbstractKernelFunction.h
  include/shark/Models/Kernels/NormalizedKernel.h
  include/shark/Models/Kernels/EvalSkipMissingFeatures.h
  include/shark/Models/Kernels/SubrangeKernel.h
  include/shark/Models/Kernels/LinearKernel.h
  include/shark/Models/Kernels/GaussianRbfKernel.h
  include/shark/Models/Kernels/DiscreteKernel.h
  include/shark/Models/Kernels/PolynomialKernel.h
  include/shark/Models/Kernels/CSvmDerivative.h
  include/shark/Models/Kernels/KernelHelpers.h
)
SOURCE_SUB_GROUP(Models Kernels FILES ${MODELS_KERNELS_INCLUDES})

SET(FUZZY_FCL_INCLUDES
  include/shark/Fuzzy/FCL/FuzzyControlLanguageParserFusionBindings.h
  include/shark/Fuzzy/FCL/FuzzyControlLanguageParser.h
)
SOURCE_SUB_GROUP(Fuzzy FCL FILES ${FUZZY_FCL_INCLUDES})

SET(ALGORITHMS_DIRECTSEARCH_EXPERIMENTS_INCLUDES
  include/shark/Algorithms/DirectSearch/Experiments/FrontStore.h
  include/shark/Algorithms/DirectSearch/Experiments/EvaluationCountStoppingCondition.h
  include/shark/Algorithms/DirectSearch/Experiments/Experiment.h
)
SOURCE_SUB_GROUP2(Algorithms DirectSearch Experiments FILES ${ALGORITHMS_DIRECTSEARCH_EXPERIMENTS_INCLUDES})

SET(MODELS_INCLUDES
  include/shark/Models/KalmanFilter.h
  include/shark/Models/SigmoidModel.h
  include/shark/Models/NearestNeighborClassifier.h
  include/shark/Models/RecurrentStructure.h
  include/shark/Models/NearestNeighborRegression.h
  include/shark/Models/LinearNorm.h
  include/shark/Models/Normalizer.h
  include/shark/Models/NBClassifier.h
  include/shark/Models/LinearModel.h
  include/shark/Models/Neurons.h
  include/shark/Models/OneVersusOneClassifier.h
  include/shark/Models/RNNet.h
  include/shark/Models/FFNet.h
  include/shark/Models/SoftNearestNeighborClassifier.h
  include/shark/Models/Converter.h
  include/shark/Models/AbstractModel.h
  include/shark/Models/LinearClassifier.h
  include/shark/Models/OnlineRNNet.h
  include/shark/Models/Softmax.h
  include/shark/Models/CMAC.h
  include/shark/Models/ConcatenatedModel.h
  include/shark/Models/RBFNet.h
)
SOURCE_GROUP(Models FILES ${MODELS_INCLUDES})

SET(MODELS_KERNELS_IMPL_INCLUDES
  include/shark/Models/Kernels/Impl/MklKernelBase.h
)
SOURCE_SUB_GROUP2(Models Kernels Impl FILES ${MODELS_KERNELS_IMPL_INCLUDES})

SET(UNSUPERVISED_RBM_INCLUDES
  include/shark/Unsupervised/RBM/RBM.h
  include/shark/Unsupervised/RBM/analytics.h
  include/shark/Unsupervised/RBM/TruncExpBinaryRBM.h
  include/shark/Unsupervised/RBM/Tags.h
  include/shark/Unsupervised/RBM/Energy.h
  include/shark/Unsupervised/RBM/GaussianBinaryRBM.h
  include/shark/Unsupervised/RBM/BinaryRBM.h
)
SOURCE_SUB_GROUP(Unsupervised RBM FILES ${UNSUPERVISED_RBM_INCLUDES})

SET(DATA_IMPL_INCLUDES
  include/shark/Data/Impl/BoostFusionCopy.hpp
  include/shark/Data/Impl/CVDatasetTools.inl
  include/shark/Data/Impl/BoostFusion151DefineStructInl.hpp
  include/shark/Data/Impl/Dataset.inl
)
SOURCE_SUB_GROUP(Data Impl FILES ${DATA_IMPL_INCLUDES})

SET(ALGORITHMS_TRAINERS_INCLUDES
  include/shark/Algorithms/Trainers/McSvmMMRTrainer.h
  include/shark/Algorithms/Trainers/McSvmWWTrainer.h
  include/shark/Algorithms/Trainers/McSvmCSTrainer.h
  include/shark/Algorithms/Trainers/LassoRegression.h
  include/shark/Algorithms/Trainers/CARTTrainer.h
  include/shark/Algorithms/Trainers/McSvmOVATrainer.h
  include/shark/Algorithms/Trainers/FisherLDA.h
  include/shark/Algorithms/Trainers/AbstractTrainer.h
  include/shark/Algorithms/Trainers/PCA.h
  include/shark/Algorithms/Trainers/SigmoidFit.h
  include/shark/Algorithms/Trainers/LDA.h
  include/shark/Algorithms/Trainers/NormalizeComponentsUnitInterval.h
  include/shark/Algorithms/Trainers/OneClassSvmTrainer.h
  include/shark/Algorithms/Trainers/McSvmLLWTrainer.h
  include/shark/Algorithms/Trainers/RFTrainer.h
  include/shark/Algorithms/Trainers/AbstractSvmTrainer.h
  include/shark/Algorithms/Trainers/NormalizeComponentsWhitening.h
  include/shark/Algorithms/Trainers/McSvmATSTrainer.h
  include/shark/Algorithms/Trainers/NBClassifierTrainer.h
  include/shark/Algorithms/Trainers/OptimizationTrainer.h
  include/shark/Algorithms/Trainers/KernelMeanClassifier.h
  include/shark/Algorithms/Trainers/McSvmADMTrainer.h
  include/shark/Algorithms/Trainers/NormalizeKernelUnitVariance.h
  include/shark/Algorithms/Trainers/McSvmATMTrainer.h
  include/shark/Algorithms/Trainers/NormalizeComponentsUnitVariance.h
  include/shark/Algorithms/Trainers/MissingFeatureSvmTrainer.h
  include/shark/Algorithms/Trainers/CSvmTrainer.h
  include/shark/Algorithms/Trainers/LinearRegression.h
  include/shark/Algorithms/Trainers/Perceptron.h
  include/shark/Algorithms/Trainers/RegularizationNetworkTrainer.h
  include/shark/Algorithms/Trainers/EpsilonSvmTrainer.h
)
SOURCE_SUB_GROUP(Algorithms Trainers FILES ${ALGORITHMS_TRAINERS_INCLUDES})

SET(UNSUPERVISED_RBM_STATESPACES_INCLUDES
  include/shark/Unsupervised/RBM/StateSpaces/RealSpace.h
  include/shark/Unsupervised/RBM/StateSpaces/TwoStateSpace.h
)
SOURCE_SUB_GROUP2(Unsupervised RBM StateSpaces FILES ${UNSUPERVISED_RBM_STATESPACES_INCLUDES})

SET(ALGORITHMS_DIRECTSEARCH_INDICATORS_INCLUDES
  include/shark/Algorithms/DirectSearch/Indicators/InvertedGenerationalDistance.h
  include/shark/Algorithms/DirectSearch/Indicators/MultiplicativeEpsilonIndicator.h
  include/shark/Algorithms/DirectSearch/Indicators/AdditiveEpsilonIndicator.h
)
SOURCE_SUB_GROUP2(Algorithms DirectSearch Indicators FILES ${ALGORITHMS_DIRECTSEARCH_INDICATORS_INCLUDES})

SET(UNSUPERVISED_RBM_GRADIENTAPPROXIMATIONS_INCLUDES
  include/shark/Unsupervised/RBM/GradientApproximations/MultiChainApproximator.h
  include/shark/Unsupervised/RBM/GradientApproximations/ExactGradient.h
  include/shark/Unsupervised/RBM/GradientApproximations/SingleChainApproximator.h
  include/shark/Unsupervised/RBM/GradientApproximations/ContrastiveDivergence.h
)
SOURCE_SUB_GROUP2(Unsupervised RBM GradientApproximations FILES ${UNSUPERVISED_RBM_GRADIENTAPPROXIMATIONS_INCLUDES})

SET(ALGORITHMS_DIRECTSEARCH_OPERATORS_SELECTION_INCLUDES
  include/shark/Algorithms/DirectSearch/Operators/Selection/ApproximatedHypervolumeSelection.h
  include/shark/Algorithms/DirectSearch/Operators/Selection/EPTournamentSelection.h
  include/shark/Algorithms/DirectSearch/Operators/Selection/BinaryTournamentSelection.h
  include/shark/Algorithms/DirectSearch/Operators/Selection/UniformRanking.h
  include/shark/Algorithms/DirectSearch/Operators/Selection/MuPlusLambdaSelection.h
  include/shark/Algorithms/DirectSearch/Operators/Selection/IndicatorBasedSelection.h
  include/shark/Algorithms/DirectSearch/Operators/Selection/RouletteWheelSelection.h
  include/shark/Algorithms/DirectSearch/Operators/Selection/TournamentSelection.h
  include/shark/Algorithms/DirectSearch/Operators/Selection/SteadyStateIndicatorBasedSelection.h
  include/shark/Algorithms/DirectSearch/Operators/Selection/MuKommaLambdaSelection.h
  include/shark/Algorithms/DirectSearch/Operators/Selection/LinearRanking.h
)
SOURCE_SUB_GROUP3(Algorithms DirectSearch Operators Selection FILES ${ALGORITHMS_DIRECTSEARCH_OPERATORS_SELECTION_INCLUDES})

SET(LINALG_BLAS_IMPL_INCLUDES
  include/shark/LinAlg/BLAS/Impl/fast_prod_matrix.inl
  include/shark/LinAlg/BLAS/Impl/sumMatrix.inl
  include/shark/LinAlg/BLAS/Impl/symmRankKUpdate.inl
  include/shark/LinAlg/BLAS/Impl/BinaryExpressions.h
  include/shark/LinAlg/BLAS/Impl/Initialize.h
  include/shark/LinAlg/BLAS/Impl/fast_prod_vector.inl
  include/shark/LinAlg/BLAS/Impl/BlockMatrixVectorOperation.inl
  include/shark/LinAlg/BLAS/Impl/repmat.h
  include/shark/LinAlg/BLAS/Impl/UnaryExpressions.h
)
SOURCE_SUB_GROUP2(LinAlg BLAS Impl FILES ${LINALG_BLAS_IMPL_INCLUDES})

SET(MODELS_IMPL_INCLUDES
  include/shark/Models/Impl/LinearModel.inl
  include/shark/Models/Impl/Neurons.h
)
SOURCE_SUB_GROUP(Models Impl FILES ${MODELS_IMPL_INCLUDES})

SET(CORE_TRAITS_INCLUDES
  include/shark/Core/Traits/IsVector.h
  include/shark/Core/Traits/ProxyReferenceTraits.h
  include/shark/Core/Traits/OptimizerTraits.h
  include/shark/Core/Traits/MultiObjectiveFunctionTraits.h
)
SOURCE_SUB_GROUP(Core Traits FILES ${CORE_TRAITS_INCLUDES})

SET(LINALG_BLAS_IMPL_NUMERIC_BINDINGS_DEFAULT_INCLUDES
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/default/syrk.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/default/trsm.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/default/gemm.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/default/trsv.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/default/gemv.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/default/ublasTags.h
)
SOURCE_SUB_GROUP4(LinAlg BLAS Impl numeric_bindings default FILES ${LINALG_BLAS_IMPL_NUMERIC_BINDINGS_DEFAULT_INCLUDES})

SET(LINALG_BLAS_IMPL_NUMERIC_BINDINGS_INCLUDES
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/syrk.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/trsm.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/gemm.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/trsv.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/gemv.h
)
SOURCE_SUB_GROUP3(LinAlg BLAS Impl numeric_bindings FILES ${LINALG_BLAS_IMPL_NUMERIC_BINDINGS_INCLUDES})

SET(ALGORITHMS_DIRECTSEARCH_TRAITS_INCLUDES
  include/shark/Algorithms/DirectSearch/Traits/FitnessTraits.h
  include/shark/Algorithms/DirectSearch/Traits/QualityIndicatorTraits.h
)
SOURCE_SUB_GROUP2(Algorithms DirectSearch Traits FILES ${ALGORITHMS_DIRECTSEARCH_TRAITS_INCLUDES})

SET(FUZZY_INCLUDES
  include/shark/Fuzzy/RuleBase.h
  include/shark/Fuzzy/Rule.h
  include/shark/Fuzzy/FuzzySet.h
  include/shark/Fuzzy/SugenoIM.h
  include/shark/Fuzzy/CustomIM.h
  include/shark/Fuzzy/LinguisticVariable.h
  include/shark/Fuzzy/SugenoRule.h
  include/shark/Fuzzy/MamdaniIM.h
  include/shark/Fuzzy/LinguisticTerm.h
  include/shark/Fuzzy/Operators.h
  include/shark/Fuzzy/InferenceMachine.h
  include/shark/Fuzzy/Fuzzy.h
  include/shark/Fuzzy/Implication.h
  include/shark/Fuzzy/FuzzyRelation.h
  include/shark/Fuzzy/RuleBaseGenerator.h
)
SOURCE_GROUP(Fuzzy FILES ${FUZZY_INCLUDES})

SET(OBJECTIVEFUNCTIONS_IMPL_INCLUDES
  include/shark/ObjectiveFunctions/Impl/FunctionWrapperBase.h
  include/shark/ObjectiveFunctions/Impl/NoisyErrorFunction.inl
  include/shark/ObjectiveFunctions/Impl/SparseFFNetError.inl
  include/shark/ObjectiveFunctions/Impl/ErrorFunction.inl
)
SOURCE_SUB_GROUP(ObjectiveFunctions Impl FILES ${OBJECTIVEFUNCTIONS_IMPL_INCLUDES})

SET(OBJECTIVEFUNCTIONS_LOSS_INCLUDES
  include/shark/ObjectiveFunctions/Loss/CrossEntropy.h
  include/shark/ObjectiveFunctions/Loss/CrossEntropyIndependent.h
  include/shark/ObjectiveFunctions/Loss/SquaredLoss.h
  include/shark/ObjectiveFunctions/Loss/AbsoluteLoss.h
  include/shark/ObjectiveFunctions/Loss/AbstractLoss.h
  include/shark/ObjectiveFunctions/Loss/NegativeClassificationLogLikelihood.h
  include/shark/ObjectiveFunctions/Loss/DiscreteLoss.h
  include/shark/ObjectiveFunctions/Loss/ZeroOneLoss.h
)
SOURCE_SUB_GROUP(ObjectiveFunctions Loss FILES ${OBJECTIVEFUNCTIONS_LOSS_INCLUDES})

SET(TESTHELPERS_INCLUDES
  include/shark/TestHelpers/Utils.h
)
SOURCE_GROUP(TestHelpers FILES ${TESTHELPERS_INCLUDES})

SET(MODELS_TREES_INCLUDES
  include/shark/Models/Trees/BinaryTree.h
  include/shark/Models/Trees/LCTree.h
  include/shark/Models/Trees/KDTree.h
  include/shark/Models/Trees/CARTClassifier.h
  include/shark/Models/Trees/KHCTree.h
  include/shark/Models/Trees/RFClassifier.h
)
SOURCE_SUB_GROUP(Models Trees FILES ${MODELS_TREES_INCLUDES})

SET(INCLUDES
  include/shark/SharkDefs.h
)

SET(LINALG_BLAS_TRAITS_INCLUDES
  include/shark/LinAlg/BLAS/traits/sparse.hpp
  include/shark/LinAlg/BLAS/traits/vector_raw.hpp
  include/shark/LinAlg/BLAS/traits/matrixContainerMeta.h
  include/shark/LinAlg/BLAS/traits/metafunctions.h
  include/shark/LinAlg/BLAS/traits/vectorContainerMeta.h
  include/shark/LinAlg/BLAS/traits/matrix_raw.hpp
  include/shark/LinAlg/BLAS/traits/MetaBlas.h
)
SOURCE_SUB_GROUP2(LinAlg BLAS traits FILES ${LINALG_BLAS_TRAITS_INCLUDES})

SET(LINALG_BLAS_IMPL_NUMERIC_BINDINGS_GOTOBLAS_INCLUDES
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/gotoblas/syrk.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/gotoblas/trsm.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/gotoblas/gemm.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/gotoblas/trsv.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/gotoblas/gemv.h
  include/shark/LinAlg/BLAS/Impl/numeric_bindings/gotoblas/cblas_inc.h
)
SOURCE_SUB_GROUP4(LinAlg BLAS Impl numeric_bindings gotoblas FILES ${LINALG_BLAS_IMPL_NUMERIC_BINDINGS_GOTOBLAS_INCLUDES})

SET(SHARK_INCLUDES
  ${ALGORITHMS_STOPPINGCRITERIA_INCLUDES}
  ${UNSUPERVISED_RBM_NEURONLAYERS_INCLUDES}
  ${LINALG_BLAS_INCLUDES}
  ${CORE_LOGHANDLERS_INCLUDES}
  ${UNSUPERVISED_RBM_SAMPLING_INCLUDES}
  ${STATISTICS_DISTRIBUTIONS_INCLUDES}
  ${UNSUPERVISED_RBM_IMPL_INCLUDES}
  ${CORE_LOGFORMATTERS_INCLUDES}
  ${LINALG_INCLUDES}
  ${CORE_RENDERERS_INCLUDES}
  ${ALGORITHMS_DIRECTSEARCH_INCLUDES}
  ${STATISTICS_TESTS_INCLUDES}
  ${ALGORITHMS_DIRECTSEARCH_OPERATORS_MUTATION_CMA_INCLUDES}
  ${OBJECTIVEFUNCTIONS_INCLUDES}
  ${LINALG_IMPL_INCLUDES}
  ${MODELS_CLUSTERING_INCLUDES}
  ${ALGORITHMS_DIRECTSEARCH_OPERATORS_PARAMETERUPDATE_CMA_INCLUDES}
  ${FUZZY_LINGUISTICTERMS_INCLUDES}
  ${CORE_UTILITY_INCLUDES}
  ${FUZZY_FUZZYSETS_INCLUDES}
  ${ALGORITHMS_TRAINERS_DISTRIBUTION_INCLUDES}
  ${UNSUPERVISED_RBM_GRADIENTAPPROXIMATIONS_IMPL_INCLUDES}
  ${CORE_SEARCHSPACES_INCLUDES}
  ${DATA_INCLUDES}
  ${ALGORITHMS_DIRECTSEARCH_OPERATORS_RECOMBINATION_INCLUDES}
  ${ALGORITHMS_DIRECTSEARCH_OPERATORS_EVALUATION_INCLUDES}
  ${UNSUPERVISED_RBM_SAMPLING_IMPL_INCLUDES}
  ${UNSUPERVISED_RBM_PROBLEMS_INCLUDES}
  ${ALGORITHMS_DIRECTSEARCH_OPERATORS_MUTATION_INCLUDES}
  ${ALGORITHMS_DIRECTSEARCH_OPERATORS_INITIALIZERS_INCLUDES}
  ${CORE_UTILITY_IMPL_INCLUDES}
  ${ALGORITHMS_INCLUDES}
  ${ALGORITHMS_LP_INCLUDES}
  ${ALGORITHMS_DIRECTSEARCH_CMA_INCLUDES}
  ${LINALG_BLAS_IMPL_NUMERIC_BINDINGS_ATLAS_INCLUDES}
  ${ALGORITHMS_GRADIENTDESCENT_IMPL_INCLUDES}
  ${OBJECTIVEFUNCTIONS_BENCHMARKS_INCLUDES}
  ${ALGORITHMS_GRADIENTDESCENT_INCLUDES}
  ${STATISTICS_INCLUDES}
  ${ALGORITHMS_NEARESTNEIGHBORS_INCLUDES}
  ${RNG_INCLUDES}
  ${ALGORITHMS_QP_INCLUDES}
  ${ALGORITHMS_DIRECTSEARCH_OPERATORS_INITIALIZERS_CMA_INCLUDES}
  ${CORE_INCLUDES}
  ${MODELS_KERNELS_INCLUDES}
  ${FUZZY_FCL_INCLUDES}
  ${ALGORITHMS_DIRECTSEARCH_EXPERIMENTS_INCLUDES}
  ${MODELS_INCLUDES}
  ${MODELS_KERNELS_IMPL_INCLUDES}
  ${UNSUPERVISED_RBM_INCLUDES}
  ${DATA_IMPL_INCLUDES}
  ${ALGORITHMS_TRAINERS_INCLUDES}
  ${UNSUPERVISED_RBM_STATESPACES_INCLUDES}
  ${ALGORITHMS_DIRECTSEARCH_INDICATORS_INCLUDES}
  ${UNSUPERVISED_RBM_GRADIENTAPPROXIMATIONS_INCLUDES}
  ${ALGORITHMS_DIRECTSEARCH_OPERATORS_SELECTION_INCLUDES}
  ${LINALG_BLAS_IMPL_INCLUDES}
  ${MODELS_IMPL_INCLUDES}
  ${CORE_TRAITS_INCLUDES}
  ${LINALG_BLAS_IMPL_NUMERIC_BINDINGS_DEFAULT_INCLUDES}
  ${LINALG_BLAS_IMPL_NUMERIC_BINDINGS_INCLUDES}
  ${ALGORITHMS_DIRECTSEARCH_TRAITS_INCLUDES}
  ${FUZZY_INCLUDES}
  ${OBJECTIVEFUNCTIONS_IMPL_INCLUDES}
  ${OBJECTIVEFUNCTIONS_LOSS_INCLUDES}
  ${TESTHELPERS_INCLUDES}
  ${MODELS_TREES_INCLUDES}
  ${INCLUDES}
  ${LINALG_BLAS_TRAITS_INCLUDES}
  ${LINALG_BLAS_IMPL_NUMERIC_BINDINGS_GOTOBLAS_INCLUDES}
)


SET(ALGORITHMS_GRADIENTDESCENT_SOURCES
  src/Algorithms/GradientDescent/Quickprop.cpp
  src/Algorithms/GradientDescent/CG.cpp
  src/Algorithms/GradientDescent/BFGS.cpp
  src/Algorithms/GradientDescent/IRLS.cpp
  src/Algorithms/GradientDescent/Rprop.cpp
)
SOURCE_SUB_GROUP2(src Algorithms GradientDescent FILES ${ALGORITHMS_GRADIENTDESCENT_SOURCES})

SET(SOURCES
  src/SharkDefs.cpp
)

SET(OBJECTIVEFUNCTIONS_SOURCES
  src/ObjectiveFunctions/ROC.cpp
  src/ObjectiveFunctions/DiscreteLoss.cpp
)
SOURCE_SUB_GROUP(src ObjectiveFunctions FILES ${OBJECTIVEFUNCTIONS_SOURCES})

SET(OBJECTIVEFUNCTIONS_LOSS_SOURCES
  src/ObjectiveFunctions/Loss/NegativeClassificationLogLikelihood.cpp
)
SOURCE_SUB_GROUP2(src ObjectiveFunctions Loss FILES ${OBJECTIVEFUNCTIONS_LOSS_SOURCES})

SET(MODELS_SOURCES
  src/Models/RBFNet.cpp
  src/Models/OnlineRNNet.cpp
  src/Models/SigmoidModel.cpp
  src/Models/CMAC.cpp
  src/Models/Softmax.cpp
  src/Models/KalmanFilter.cpp
  src/Models/LinearClassifier.cpp
  src/Models/LinearNorm.cpp
  src/Models/RNNet.cpp
  src/Models/Converter.cpp
  src/Models/Centroids.cpp
  src/Models/RecurrentStructure.cpp
)
SOURCE_SUB_GROUP(src Models FILES ${MODELS_SOURCES})

SET(ALGORITHMS_SOURCES
  src/Algorithms/PCA.cpp
  src/Algorithms/NoisyRprop.cpp
  src/Algorithms/LDA.cpp
  src/Algorithms/CARTTrainer.cpp
  src/Algorithms/FisherLDA.cpp
  src/Algorithms/LinearRegression.cpp
  src/Algorithms/GLPK.cpp
  src/Algorithms/RFTrainer.cpp
  src/Algorithms/KMeans.cpp
  src/Algorithms/SigmoidFit.cpp
)
SOURCE_SUB_GROUP(src Algorithms FILES ${ALGORITHMS_SOURCES})

SET(CORE_SOURCES
  src/Core/Oracle.cpp
  src/Core/Version.cpp
)
SOURCE_SUB_GROUP(src Core FILES ${CORE_SOURCES})

SET(ALGORITHMS_DIRECTSEARCH_SOURCES
  src/Algorithms/DirectSearch/CMSA.cpp
  src/Algorithms/DirectSearch/CMA.cpp
)
SOURCE_SUB_GROUP2(src Algorithms DirectSearch FILES ${ALGORITHMS_DIRECTSEARCH_SOURCES})

SET(SHARK_SOURCES
  ${ALGORITHMS_GRADIENTDESCENT_SOURCES}
  ${SOURCES}
  ${OBJECTIVEFUNCTIONS_SOURCES}
  ${OBJECTIVEFUNCTIONS_LOSS_SOURCES}
  ${MODELS_SOURCES}
  ${ALGORITHMS_SOURCES}
  ${CORE_SOURCES}
  ${ALGORITHMS_DIRECTSEARCH_SOURCES}
)

SET( SRCS
	${SHARK_INCLUDES}
	${SHARK_SOURCES}
)

IF( WIN32 )
  ADD_LIBRARY( shark ${SRCS} )
  #SET_TARGET_PROPERTIES( shark PROPERTIES DEBUG_PREFIX  )
  #SET_TARGET_PROPERTIES( shark PROPERTIES DEBUG_POSTFIX _debug )
  SET( LINK_LIBRARIES )
ELSE( WIN32 )
  ADD_LIBRARY( shark SHARED ${SRCS} )
  SET_TARGET_PROPERTIES( shark PROPERTIES DEBUG_POSTFIX _debug )
ENDIF( WIN32 )

IF( UNIX AND NOT APPLE )
  SET( LINK_LIBRARIES rt )
ENDIF( UNIX AND NOT APPLE )

TARGET_LINK_LIBRARIES( shark ${LINK_LIBRARIES} )

SET_TARGET_PROPERTIES( shark PROPERTIES
	VERSION ${SHARK_VERSION_MAJOR}.${SHARK_VERSION_MINOR}.{SHARK_VERSION_PATCH}
	SOVERSION ${SHARK_VERSION_MAJOR}.${SHARK_VERSION_MINOR}.{SHARK_VERSION_PATCH}
)

