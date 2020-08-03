#pragma once
#include "Exports.h"


namespace Jde::AI::Dts
{
	enum class EFeatureImportance : uint8;
	struct IDecisionTree;
	struct ITree
	{
		virtual double Predict( const double* pFeatures )noexcept=0;
		virtual const string_view* FeatureNames()const noexcept=0;
		virtual size_t FeatureCount()const noexcept=0;
		virtual MapPtr<string,double> FeatureImportances( EFeatureImportance importance )const=0;
		virtual bool HasCoverImportance()const noexcept=0;
	};
	struct IBooster;
	struct Tree : public ITree
	{
		Tree( sp<IBooster>& pBooster )noexcept(false);
		virtual ~Tree()=default;
		double Predict( const double* pFeatures )noexcept override;
		const string_view* FeatureNames()const noexcept override{ return _features.data(); }
		size_t FeatureCount()const noexcept override{ return _features.size(); }
		map<string,int> FeatureImportance();
		MapPtr<string,double> FeatureImportances( EFeatureImportance importance )const noexcept(false) override;
		bool HasCoverImportance()const noexcept override;
	private:
		sp<IBooster> _boosterPtr;
		vector<string_view> _features;
	};

	template<size_t TFeatureCount>
	struct TreeBase : public ITree
	{
		constexpr TreeBase( const std::array<string_view,TFeatureCount>& featureCount );
		const std::array<string_view,TFeatureCount> Features;

		const string_view* FeatureNames()const noexcept override{ return Features.data(); }
		size_t FeatureCount()const noexcept{return FeatureCnt;}
		constexpr static size_t FeatureCnt{TFeatureCount};
	};
	template<size_t TFeatureCount>
	constexpr TreeBase<TFeatureCount>::TreeBase( const std::array<string_view,TFeatureCount>& features ):
		Features{features}
	{}

/*
	template<class TTree>
	ITree& ITree::GetInstance()noexcept
	{
		if( _pInstance==nullptr )
			_pInstance = std::unique_ptr<TTree>( new TTree() );
		return *_pInstance;
	}
*/
	//ITree* GetPrediction( string_view namespaceName );
	//JDE_DTS_VISIBILITY sp<ITree> GetPrediction( const IDecisionTree& treeType, const fs::path& path, uint16 minuteStart, bool isLong );
	//JDE_DTS_VISIBILITY void ClearTrees( uint16 minuteStart );
}
