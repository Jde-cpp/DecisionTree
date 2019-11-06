#pragma once
#include "../../framework/source/math/MathUtilities.h"

namespace Jde::AI::Dts
{
	struct ParamResults
	{
		ParamResults(const IBoosterParamsPtr& parameters, uint bestIteration, const map<uint, vector<double>>& groupValues) :
			ParametersPtr{ parameters },
			BestIteration{ bestIteration },
			GroupValues{ groupValues }
		{}
		ParamResults(const IBoosterParamsPtr& parameters, double average, double stdDeviation, double min, double max, uint bestIteration) :
			ParametersPtr{ parameters },
			BestIteration{ bestIteration },
			_average{ average },
			_stdDeviation{ stdDeviation },
			_min{ min },
			_max{ max }
		{}
		const IBoosterParamsPtr ParametersPtr;
		const uint BestIteration;
		const map<uint, vector<double>> GroupValues;
		double Average()const noexcept { CalcResults(); return _average; }
		double StdDeviation()const noexcept { CalcResults(); return _stdDeviation; }
		double Min()const noexcept { CalcResults(); return _min; }
		double Max()const noexcept { CalcResults(); return _max; }
		bool operator<(const ParamResults& other)const noexcept { return Average() < other.Average(); }
	private:
		void CalcResults()const noexcept;
		mutable double _average{ std::numeric_limits<double>::max() };
		mutable double _stdDeviation{ 0.0 };
		mutable double _min{ 0.0 };
		mutable double _max{ 0.0 };
	};

#pragma region ParamResults Implementation
	inline void ParamResults::CalcResults()const noexcept
	{
		if( _average==std::numeric_limits<double>::max() )
		{
			const auto foldCount = GroupValues.size()>0 ? GroupValues.begin()->second.size() : 0;
			vector<double> values; values.reserve( foldCount*GroupValues.size() );
			for( const auto& iterationValues : GroupValues )
			{
				for( const auto& value : iterationValues.second )
					values.push_back( value );
			}
			const auto stats = Math::Statistics( values );
			_average = stats.Average;
			_stdDeviation = std::sqrt( stats.Variance );
			_min = stats.Min;
			_max = stats.Max;
		}
	}
#pragma endregion
}