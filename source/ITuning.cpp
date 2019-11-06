#include "stdafx.h"
#include "ITuning.h"
#include "IBooster.h"
#include "IBoosterParams.h"
#include "IDataset.h"
#include "IDecisionTree.h"
#include "ParamResults.h"
#include "../../framework/source/threading/Pool.h"

#define var const auto

namespace Jde::AI::Dts::Tuning
{
	using Eigen::MatrixXf;
	using Eigen::VectorXf;
	constexpr uint TrainingRounds = 1000;
	tuple<uint, vector<double>> CrossValidate(IDecisionTree& decisionTree, const IBoosterParams& parameters, Eigen::MatrixXf& xAll, Math::Vector<>& yAll, uint foldCount, uint trainingRounds, string_view logSuffix)noexcept(false);
	PreviousExecutions ReadPrevious( IDecisionTree& decisionTree, const fs::path& csvFile );

	tuple<IBoosterParamsPtr,bool> TuneParam( IDecisionTree& decisionTree, vector<unique_ptr<Eigen::MatrixXf>>& xs, vector<Math::VPtr<>>& ys, const fs::path& saveStem, uint foldCount, IBoosterParamsPtr& pCurrentRun, PreviousExecutions& previousExecutions, string_view logRemainder )noexcept
	{
		//DBG( "metric='{}'", pCurrentRun->GetMetric() );
		auto parameterSets = pCurrentRun->GetSets();
		uint parameterSetIndex = 0;
		uint executionCount = std::count_if( parameterSets.begin(), parameterSets.end(), [&previousExecutions](auto& parameterSet){return  previousExecutions.find(parameterSet)==previousExecutions.end();} );
//		for( var& parameterSet : parameterSets )
//		{
//			if( previousExecutions.find(parameterSet)==previousExecutions.end() )
//				++executionCount;
//		}
		if( executionCount==0 )
			return make_tuple( pCurrentRun, false );

		var poolThreadCount = 1;//TODO uncomment std::min<uint>( executionCount, std::max<uint>(std::thread::hardware_concurrency()-2,1) );
		{
			Threading::Pool pool( poolThreadCount );
			std::mutex previousExecutionsMutex;
			for( var& pParameterSet : parameterSets )
			{
				DBG0( pParameterSet->to_string() );
				if( previousExecutions.find(pParameterSet)!=previousExecutions.end() )
					continue;
				/*if( parameterSet.NumberOfLeavesValue()<2 )
				{
					GetDefaultLogger()->critical( "parameterSet.NumberOfLeavesValue()<2" );	
					continue;
				}*/
				ASSRT_TR( previousExecutions.find(pParameterSet)==previousExecutions.end() );
				var logSuffix = string("({}")+fmt::format( "/{})({}/{}){}", xs.size(), parameterSetIndex++, parameterSets.size(), string(logRemainder) );
				
				//var logSuffix = fmt::format( "({{}}/{})({}/{})({}/{})({}/{})", xs.size(), parameterSetIndex++, parameterSets.size(), parameterIndex, hasRangeCount, roundIndex, testCount );
				auto func = [&decisionTree, &previousExecutions, &previousExecutionsMutex, &ys, &xs, logSuffix, &pParameterSet, &foldCount, poolThreadCount]()
				{
					uint groupIndex = 0;
					map<uint,vector<double>> groupFolds;
					uint maxIteration = 0;
					vector<Math::VPtr<>>::const_iterator ppY=ys.begin();
					for( vector<unique_ptr<Eigen::MatrixXf>>::const_iterator ppX=xs.begin(); ppX!=xs.end() && ppY!=ys.end(); ++ppX, ++ppY )
					{
						var logSuffix2 = fmt::format( logSuffix, groupIndex );
						pParameterSet->SetThreadCount( poolThreadCount==1 ? std::thread::hardware_concurrency()-2 : 1 );
						tuple<uint,vector<double>> result = CrossValidate( decisionTree, *pParameterSet, *(*ppX), *(*ppY), foldCount, TrainingRounds, logSuffix2 );
						maxIteration = std::max( get<0>(result), maxIteration );
						groupFolds[groupIndex++] = get<1>( result );
					}
					std::unique_lock l( previousExecutionsMutex );
					previousExecutions.emplace( pParameterSet, ParamResults(pParameterSet, maxIteration, groupFolds) );
				};
				pool.Submit( func );	
			}
		}
		ASSERT( previousExecutions.size()>0 );

		var bestParams = SaveTuning( saveStem, previousExecutions );
		return make_tuple( bestParams, true );
	}

	IBoosterParamsPtr Tune( IDecisionTree& decisionTree, vector<unique_ptr<Eigen::MatrixXf>>& xs, vector<Math::VPtr<>>& ys, uint testCount, const fs::path& saveStem, uint foldCount, string_view objective )noexcept(false)
	{
		sp<IBoosterParams> pBestParams;
		auto paramFile = fs::path( saveStem ).replace_extension( ".params" );
		if( fs::exists(paramFile) )
			pBestParams = decisionTree.LoadParams( paramFile );
		//else if( fs::exists(paramStart) )
		//	pBestParams = decisionTree.LoadParams( paramStart );
		else
			pBestParams = decisionTree.LoadDefaultParams( objective );
		//DBG( "BaggingFractionValue={}", pBestParams->BaggingFractionValue() );
		//pBestParams->SetGpu();
		auto csvFile = fs::path(saveStem).replace_extension( ".csv" );
		auto previousExecutions = fs::exists( csvFile ) ? ReadPrevious( decisionTree, csvFile ) : PreviousExecutions();

		for( uint roundIndex=0; roundIndex<testCount; ++roundIndex )
		{
			uint parameterIndex = 0;
			var hasRangeCount = pBestParams->HasRangeCount();
			var parameters = pBestParams->Parameters();
			for( var& changing : parameters )
			{
				var& parameter = *changing.second;
				if( !parameter.HasRange )
					continue;
				if( parameter.Name=="min_sum_hessian_in_leaf" )
				{
					CRITICAL( "Trying to change min_sum_hessian_in_leaf {}", "none" );
					continue;
				}
				auto pCurrentRun = pBestParams->Clone();
				(*pCurrentRun)[parameter.Name]->SetRange();
				//DBG( "BaggingFractionValue={}", pCurrentRun->BaggingFractionValue() );
				var logSuffix = fmt::format( "({}/{})({}/{})", ++parameterIndex, hasRangeCount, roundIndex, testCount );
				pBestParams = get<0>( TuneParam(decisionTree, xs, ys, saveStem, foldCount, pCurrentRun, previousExecutions, logSuffix) );
			}
		}
	 	return pBestParams;
	}
	
	IBoosterParamsPtr TuneOne( IDecisionTree& decisionTree, vector<unique_ptr<Eigen::MatrixXf>>& xs, vector<Math::VPtr<>>& ys, const fs::path& saveStem, uint foldCount, string_view parameterName )noexcept(false)
	{
		var paramFile = fs::path( saveStem ).replace_extension( ".params" );
		IBoosterParamsPtr pBestParams = decisionTree.LoadParams( paramFile );
		pBestParams->SetGpu();
		var csvFile = fs::path(saveStem).replace_extension( ".csv" );
		if( !fs::exists(csvFile) )
			THROW( LogicException(fmt::format("{} was not found", csvFile))  );

		auto previousExecutions = ReadPrevious( decisionTree, csvFile );
		for(uint roundIndex=0;; ++roundIndex)
		{
			auto pCurrentRun = pBestParams->Clone();
			(*pCurrentRun)[parameterName]->SetRange();
			var logSuffix = fmt::format( "({}/{}) - {}", 0, 1, roundIndex );
			var results = TuneParam( decisionTree, xs, ys, saveStem, foldCount, pCurrentRun, previousExecutions, logSuffix );
			pBestParams = get<0>( results );
			if( !get<1>(results) )
				break;
		}
		return pBestParams;
	}

#pragma region CrossValidate
	tuple<uint, vector<double>> CrossValidate(IDecisionTree& decisionTree, const IBoosterParams& parameters, Eigen::MatrixXf& xAll, Math::Vector<>& yAll, uint foldCount, uint trainingRounds, string_view logSuffix)noexcept(false)
	{
		constexpr uint earlyStoppingRounds = 30;
		constexpr uint seed = 1234;
		const uint rowCount = xAll.rows(); const uint columnCount = xAll.cols();
		auto pIndicies = Math::EMatrix::RandomIndexes<-1, uint>( rowCount, seed );
		uint maxIteration = 0;
		vector<double> results(foldCount);
		for( uint foldIndex = 0, validationStart=0; foldIndex < foldCount; ++foldIndex )// fold_index in range(0,fold_count):
		{
			var validationCount = rowCount / foldCount + (foldIndex < rowCount%foldCount ? 1 : 0);
			var trainCount = rowCount - validationCount;
			MatrixXf xTrain{ trainCount, columnCount }, xValidation{ validationCount, columnCount };
			VectorXf yTrain{ trainCount }, yValidation{ validationCount };
			
			uint trainIndex = 0, validationIndex = 0;
			uint maxTrainIndex = 0; uint maxValidationIndex = 0;
			for( uint iRandom = 0; iRandom < rowCount; ++iRandom )//: pIndicies index, rnd_index in enumerate(rnd_indicies):
			{
				//var fromIndex = pIndicies->coeff(iRandom); RANDOM
				var fromIndex = iRandom;
				//var validation = fromIndex % foldCount == foldIndex;
				var validation = fromIndex>=validationStart && fromIndex<validationStart+validationCount;
				auto& data = validation ? xValidation : xTrain;
				auto& labels = validation ? yValidation : yTrain;
				uint& toIndex = validation ? validationIndex : trainIndex;
				uint& maxIndex = validation ? maxValidationIndex : maxTrainIndex;
				maxIndex = std::max( toIndex, maxIndex );
				ASSRT_LT( (uint)data.rows(), toIndex );
				data.row(toIndex) = xAll.row( fromIndex );
				labels(toIndex++) = yAll( fromIndex );
			}
			ASSRT_EQ( trainCount - 1, maxTrainIndex );
			ASSRT_EQ( validationCount - 1, maxValidationIndex );

			//lgb_train =  lgb.Dataset( x, y )
			//lgb_eval = lgb.Dataset( x_validation, y_validation, reference=lgb_train )
			//Math::EMatrix::ExportCsv( "/home/duffyj/test.csv", xValidation, nullptr );
			auto pTrainingDS = std::const_pointer_cast<const IDataset>( decisionTree.CreateDataset(xTrain, yTrain, &parameters, nullptr, nullptr) );
			var pValidationDS = std::const_pointer_cast<const IDataset>( decisionTree.CreateDataset(xValidation, yValidation, &parameters, nullptr, pTrainingDS) );
			auto pBooster = Train( decisionTree, pTrainingDS, parameters, trainingRounds, pValidationDS, earlyStoppingRounds );
			maxIteration = std::max( pBooster->BestIteration(), maxIteration );
			// predictions = model.predict( x_validation, num_iteration=round_count )
			// score = sklearn.metrics.mean_squared_error( np.array(y_validation).astype(np.float32), predictions, sample_weight=None, multioutput='uniform_average' ) #pylint: disable=E1101
			results[foldIndex] = pBooster->BestScore();
			DBG( "{} - {}.{}", pBooster->BestScore(), foldIndex, logSuffix );
			validationStart+=validationCount;
		}
		//fold_array = np.array(fold_results)
		return make_tuple(maxIteration, results);
	}
#pragma endregion
#pragma region Train
	sp<IBooster> Train( IDecisionTree& decisionTree, sp<const IDataset>& pTrain, const IBoosterParams& params, uint count, sp<const IDataset> pValidation, uint earlyStoppingRounds )noexcept(false)
	{
		map<uint,double> results;

		auto pBooster = decisionTree.CreateBooster( params, pTrain, pValidation );// ? pValidation.get() : nullptr
		for( uint i=0; i<count; ++i )
		{
			if( !pBooster->UpdateOneIteration(static_cast<int>(i)) )
				break;
			if( pValidation )
			{
				var result = pBooster->GetEvaluation( true, i )[0];
				results.emplace( i, result );
				if( result<pBooster->BestScore() )
				{
					pBooster->BestScore( result );
					pBooster->SetBestIteration( i );
				}
				else if( i>earlyStoppingRounds && (i-earlyStoppingRounds)>pBooster->BestIteration() )
				{
					GetDefaultLogger()->debug( "({}) - {} early stopping", pBooster->BestIteration(), pBooster->BestScore() );
					break;
				}
			}
		}
		if( pValidation && pBooster->BestIteration()<count-1 )
			pBooster->LoadBestIteration();
//			pBooster->LoadModelFromString( pBooster->to_string(pBooster->BestIteration()) );
		return pBooster;
	}
#pragma endregion
#pragma region SaveTuning
	IBoosterParamsPtr SaveTuning( const fs::path& saveStem, const PreviousExecutions& previousExecutions )noexcept(false)
	{
		ASSRT_GT( (uint)0, previousExecutions.size() );
		multimap<ParamResults,IBoosterParamsPtr> sorted;
		for( var& [pParams, results] : previousExecutions )
		{
			pParams->SetBestIteration( results.BestIteration );
			sorted.emplace( results, pParams );
		}
		var pTop = sorted.begin();
		{
			auto paramPath = saveStem; 
			std::ofstream os;
			os.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			os.open( paramPath.replace_extension(".params") );
			//if( os.fail() )
			//THROW( CodeException(fmt::format("Could not write to {}", paramPath.c_str()), errno) );
			os << *(pTop->second);
		}
		auto resultsPath = saveStem;
		std::ofstream os;
		os.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		os.open( resultsPath.replace_extension(".csv") );
		//if( os.fail() )
		//THROW( CodeException(fmt::format("Could not write to {}", resultsPath.c_str()), errno) );
		os << "average,std_dev,min,max,iteration";
		for( var& parameter : pTop->second->Parameters() )
			os <<","<< parameter.first;
		os << std::endl;
		for( var& [results, pParams] : sorted )
		{
			os << results.Average() << "," << results.StdDeviation() << "," << results.Min() << "," << results.Max() << "," << results.BestIteration;
			for( var& parameter : pParams->Parameters() )
				os << "," << parameter.second->InitialString();
			os << std::endl;
		}
		return pTop->second;
	}

	PreviousExecutions ReadPrevious( IDecisionTree& decisionTree, const fs::path& csvFile )
	{
		std::ifstream is(csvFile);
		ASSRT_TR(is.good());
		string line;
		getline(is, line);
		var columnNameList = StringUtilities::Split(line);
		var columnNames = vector<string>(columnNameList.begin(), columnNameList.end());
		PreviousExecutions previousExecutions;
		while (is.good())
		{
			getline(is, line);
			if (line.size() == 0)
				continue;
			var valueList = StringUtilities::Split(line);
			var values = vector<string>(valueList.begin(), valueList.end());
			ASSRT_EQ(columnNames.size(), values.size());
			auto pParamsOther = decisionTree.LoadDefaultParams( ""sv );
			for (uint i = 5; i < values.size(); ++i)
			{
				var& columnName = columnNames[i];
				shared_ptr<Parameter> pParameter = (*pParamsOther)[columnName];
				if (pParameter->Type() == ParameterType::String)
					dynamic_pointer_cast<TParameter<string>>(pParameter)->Initial = values[i];
				else if (pParameter->Type() == ParameterType::Double)
					dynamic_pointer_cast<TParameter<double>>(pParameter)->Initial = stod(values[i]);
				else if (pParameter->Type() == ParameterType::UInt)
					dynamic_pointer_cast<TParameter<uint>>(pParameter)->Initial = stoul(values[i]);
			}
			pParamsOther->SetBestIteration(stoul(values[4]));
			const ParamResults results(pParamsOther, stod(values[0]), stod(values[1]), stod(values[2]), stod(values[3]), pParamsOther->BestIteration());
			var inserted = previousExecutions.emplace(pParamsOther, results).second;
			if (!inserted)
				CRITICAL("Duplicate parameter sets {}", pParamsOther->to_string()); //ASSRT_TR( inserted );
		}
		return previousExecutions;
	}
}