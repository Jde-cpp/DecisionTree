#pragma once

namespace Jde::AI::Dts
{
	enum class EFeatureImportance : uint8;
	//struct IBoosterParams;
	//struct IBooster;
	struct IBooster
	{
		virtual MapPtr<string,double> FeatureImportances( EFeatureImportance featureImportance  )const noexcept(false)=0;
		virtual bool HasCoverImportance()const noexcept{ return false; }
		virtual string to_string( uint iterationNumber = 0 )const noexcept(false)=0;
		virtual double Predict( const Math::RowVector<float,-1>& vector )noexcept(false)=0;
		virtual double Predict( const double* pFeatures )noexcept(false)=0;
		uint BestIteration()const noexcept{ return _bestIteration; } virtual void SetBestIteration( uint value )noexcept{_bestIteration = value; }
		virtual const vector<string>& FeatureNames()const noexcept=0;
		double BestScore()const noexcept{ return _bestScore; } void BestScore( double value )noexcept{ _bestScore=value; }
		virtual bool UpdateOneIteration( int index=-1 )noexcept(false)=0;
		virtual vector<double> GetEvaluation( bool validation, uint iteration )const noexcept(false)=0;
		
		virtual void LoadBestIteration()noexcept(false)=0;
		virtual uint ColumnCount()const noexcept{ return FeatureNames().size(); }
		virtual void Save( const fs::path& path )noexcept(false)=0;
	private:
		double _bestScore{ std::numeric_limits<double>::max() };
		uint _bestIteration{0};
	};
	typedef sp<IBooster> IBoosterPtr;
}