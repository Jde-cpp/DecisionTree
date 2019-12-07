#include "Tree.h"
#include "IBooster.h"
#include "IDecisionTree.h"
#define var const auto

namespace Jde::AI::Dts
{
	//map<uint,map<string,sp<Tree>>> _longTrees;
	//map<uint,map<string,sp<Tree>>> _shortTrees;

	Tree::Tree( sp<IBooster>& pBooster )noexcept(false):
		_boosterPtr{ pBooster }
		//_booster( IO::FileUtilities::ToString(path) )
	{
		var& features = _boosterPtr->FeatureNames();
		_features.reserve( features.size() );
		for( var& feature: features )
			_features.push_back( feature );
	}

	double Tree::Predict( const double* pFeatures )noexcept
	{ 
		return _boosterPtr->Predict( pFeatures );
	}

	bool Tree::HasCoverImportance()const noexcept
	{ 
		return _boosterPtr->HasCoverImportance(); 
	}

	MapPtr<string,double> Tree::FeatureImportances( EFeatureImportance importance )const noexcept(false)
	{
		return _boosterPtr->FeatureImportances( importance );
	}
/*
	sp<ITree> GetPrediction( const IDecisionTree& treeType, const fs::path& path, uint16 minuteStart, bool isLong )
	{
		auto& trees = isLong ? _longTrees : _shortTrees;
		auto pMinuteTree = trees.try_emplace( minuteStart, map<string,sp<Tree>>{} ).first;
		auto pSymbolTrees = pMinuteTree->second.try_emplace( path.string(), shared_ptr<Tree>{} ).first;
		sp<ITree> pTree = pSymbolTrees->second;
		if( pTree==nullptr && fs::exists(path) )
		{
			try
			{
				IBoosterPtr pBooster = treeType.CreateBooster( path );
				pTree = pSymbolTrees->second = make_shared<Tree>( pBooster );
			}
			catch( const Exception& exp )//no trees
			{
				WARN0( exp.what() );
			}
		}
		return pTree;
	}
	*/
}