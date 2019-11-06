#pragma once
#include "Exports.h"


namespace Jde::AI::Dts
{
	struct ITree;
	struct Tree;
	struct IBooster;
	struct IBoosterParams;
	struct IDataset;
	typedef sp<IBoosterParams> IBoosterParamsPtr;
	
	struct JDE_DTS_VISIBILITY IDecisionTree
	{
		IDecisionTree( string_view paramFilePrefix );
		virtual sp<IBooster> CreateBooster( const fs::path& file )const noexcept(false)=0;
		virtual sp<IBooster> CreateBooster( const IBoosterParams& params, sp<const IDataset>& pTrain, sp<const IDataset> pValidation )=0;
		sp<ITree> GetPrediction( const fs::path& path, uint16 minuteStart, bool isLong );
		virtual fs::path BaseDir()const noexcept;
		virtual IBoosterParamsPtr LoadParams( const fs::path& file )const noexcept(false)=0;
		IBoosterParamsPtr LoadParams( uint featureLength, string_view metric )noexcept(false);
		virtual IBoosterParamsPtr LoadDefaultParams( string_view objective )const noexcept(false)=0;
		virtual string_view DefaultRegression()const noexcept=0;
		sp<IBooster> Train( const Eigen::MatrixXf& x, const Eigen::VectorXf& y, const IBoosterParams& params, uint count, const std::vector<string>& columnNames )noexcept(false);
		virtual sp<IDataset> CreateDataset( const Eigen::MatrixXf& matrix, const Eigen::VectorXf& y, const IBoosterParams* pParams, const std::vector<string>* pColumnNames/*=nullptr*/, shared_ptr<const IDataset> pTrainingDataset )=0;
		void ClearTrees( uint16 minuteStart );
		
		const string ParamFilePrefix;
		static sp<IDecisionTree> Create( const fs::path& path );
	private:
		map<uint,map<string,sp<Tree>>> _longTrees;
		map<uint,map<string,sp<Tree>>> _shortTrees;
	};
}