/*
using std::array;
using std::make_unique;
using std::map;
using std::string_view;
*/
#pragma warning( disable : 4245) 
#include <boost/crc.hpp> 
#pragma warning( default : 4245) 
#include <boost/system/error_code.hpp>
#ifndef __INTELLISENSE__
	#include <spdlog/spdlog.h>
	#include <spdlog/sinks/basic_file_sink.h>
	#include <spdlog/fmt/ostr.h>
#endif

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <nlohmann/json.hpp>
#include "../../Framework/source/TypeDefs.h"
#include "../../Framework/source/Exception.h"
#include "../../Framework/source/log/Logging.h"
#include "../../Framework/source/JdeAssert.h"
#include "../../Eigen/source/EMatrix.h"
#include "../../MarketLibrary/source/Exports.h"//for requests.pb.h
#include "../../MarketLibrary/source/types/proto/requests.pb.h"
//#include "TypeDefs.h"