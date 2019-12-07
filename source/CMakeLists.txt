#export CXX=g++-8
cmake_minimum_required(VERSION 3.16)
project(Jde.Dts)
set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../Framework/cmake )
include(SetUpConfigurations)
#Custom######################################################
include_directories( "/home/duffyj/code/libraries/eigen" )
#############################################################
add_library( Jde.Dts SHARED Tree.cpp IDecisionTree.cpp IBoosterParams.cpp ITuning.cpp IDataset.cpp TreeNode.cpp )
#############################################################
add_compile_definitions(THREAD_LOCAL=thread_local)
target_precompile_headers(Jde.Dts
  PRIVATE
		<boost/crc.hpp> 
		<boost/system/error_code.hpp>
		<spdlog/spdlog.h>
		<spdlog/sinks/basic_file_sink.h>
		<spdlog/fmt/ostr.h>
		<Eigen/Dense>
		<Eigen/Sparse>
		<nlohmann/json.hpp>
		"../../Framework/source/TypeDefs.h"
		"../../Framework/source/Exception.h"
		"../../Framework/source/log/Logging.h"
		"../../Framework/source/JdeAssert.h"
		"../../Eigen/source/EMatrix.h"
		"../../MarketLibrary/source/Exports.h"
		"../../MarketLibrary/source/types/proto/requests.pb.h"
)