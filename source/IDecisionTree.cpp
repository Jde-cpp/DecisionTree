#include "IDecisionTree.h"
#include "../../Framework/source/io/File.h"
#include "../../Framework/source/Settings.h"
#include "../../Framework/source/StringUtilities.h"
#include "../../Framework/source/Dll.h"
#include "Tree.h"
#include "ITuning.h"
#include "c_api.h"
#define var const auto

namespace Jde::AI::Dts
{
	IDecisionTree::IDecisionTree( string_view paramFilePrefix ):
		ParamFilePrefix{ paramFilePrefix }
	{}

	IBoosterParamsPtr IDecisionTree::LoadParams( uint featureLength, string_view metric )noexcept(false)
	{
		var pDirFiles = IO::FileUtilities::GetDirectory( BaseDir() );
		static map<uint,fs::path> files;
		if( !files.size() )
		{
			for( var& file : *pDirFiles )
			{
				var stem = file.path().stem().string();
				//var metricSplit = StringUtilities::Split( metric, ':' );
				if( stem.size()>3 && file.path().extension()==".params" && StringUtilities::StartsWith(stem, ParamFilePrefix) && std::isdigit(stem[3]) )
				{
					uint featureCount = StringUtilities::TryTo<uint>( stem.substr(3), 0 );
					if( featureCount!=0 )
						files.emplace( featureCount, file.path() );
				}
			}
		}
		var metricSplit = StringUtilities::Split( metric, ':' );
		auto fileName = BaseDir()/fmt::format( "{}_{}.params", ParamFilePrefix, metricSplit[metricSplit.size()-1] );
		if( files.size() && featureLength<81 )
		{
			WARN( "Looking for featureLength={}", featureLength );
			auto pLowerBound = files.lower_bound( featureLength );
			if( pLowerBound!=files.begin() && (pLowerBound==files.end() || pLowerBound->first!=featureLength) )
				--pLowerBound;
			fileName = pLowerBound->second;
		}
		return LoadParams( fileName );
	}

	sp<ITree> IDecisionTree::GetPrediction( const fs::path& path, uint16 minuteStart, bool isLong )
	{
		auto& trees = isLong ? _longTrees : _shortTrees;
		auto pMinuteTree = trees.try_emplace( minuteStart, map<string,sp<Tree>>{} ).first;
		auto pSymbolTrees = pMinuteTree->second.try_emplace( path.string(), shared_ptr<Tree>{} ).first;
		shared_ptr<ITree> pTree{ pSymbolTrees->second };//

		if( !pTree && fs::exists(path) )
		{
			try
			{
				sp<IBooster> pBooster = CreateBooster( path );
				pTree = pSymbolTrees->second = make_shared<Tree>( pBooster );
			}
			catch( const Exception& exp )//no trees
			{
				WARN0( exp.what() );
			}
		}
		return pTree;
	}

	void IDecisionTree::ClearTrees( uint16 minuteStart )
	{
		auto func = [minuteStart]( map<uint,map<string,sp<Tree>>>& trees )
		{
			auto pPrev = trees.end();
			for( auto pMinuteSymbols = trees.begin(); pMinuteSymbols!=trees.end(); ++pMinuteSymbols )
			{
				if( pMinuteSymbols->first<minuteStart && pPrev!=trees.end() )
				{
					trees.erase( pPrev );
					break;
				}
				pPrev = pMinuteSymbols;
			}
		};
		func( _longTrees );
		func( _shortTrees );
	}
	sp<IBooster> IDecisionTree::Train( const Eigen::MatrixXf& x, const Eigen::VectorXf& y, const IBoosterParams& params, uint count, const std::vector<string>& columnNames )noexcept(false)
	{
		auto pDataset = std::const_pointer_cast<const IDataset>( CreateDataset(x, y, &params, &columnNames, nullptr) );
		return Tuning::Train( *this, pDataset, params, count );
	}
	fs::path IDecisionTree::BaseDir()const noexcept
	{
		return Settings::Global().Get<fs::path>( "DtsBaseDir", fs::path{"/mnt/2TB/dts"} );
	}
#pragma region Create
	class DecisionTreeApi
	{
		DllHelper _dll;//initiaized first
	public:
		DecisionTreeApi( const fs::path& path ):
			_dll{ path },
			GetDecisionTreeFunction{ _dll["GetDecisionTree"] }
		{}
		decltype(GetDecisionTree) *GetDecisionTreeFunction;
	};

	sp<IDecisionTree> IDecisionTree::Create( const fs::path& path )
	{
		static map<string,unique_ptr<DecisionTreeApi>> _decisionTrees;
		auto& pApi = _decisionTrees.emplace( path.string(), unique_ptr<DecisionTreeApi>{ new DecisionTreeApi{path} } ).first->second;
		return shared_ptr<Jde::AI::Dts::IDecisionTree>{ pApi->GetDecisionTreeFunction() };
	}
#pragma endregion
}
