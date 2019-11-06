#include "stdafx.h"
#include "IBoosterParams.h"

#define var const auto

namespace Jde::AI::Dts
{
	template<>
	ParameterType TParameter<double>::Type() const{ return ParameterType::Double; }
	template<>
	ParameterType TParameter<uint>::Type()   const{ return ParameterType::UInt; }
	template<>
	ParameterType TParameter<string>::Type() const{ return ParameterType::String; }

	Parameter::Parameter( string_view name, bool hasRange ):
		Name{name},
		HasRange{hasRange}
	{}
	inline std::ostream& operator<<( std::ostream& os, const Parameter& parameter )noexcept
	{
		os << parameter.ToJson() << std::endl;
		return os;
	}

	nlohmann::json ToJson( const IBoosterParams& params ) noexcept
	{
		nlohmann::json j;
		j["best_iteration"] = params.BestIteration();
		j["parameters"] = nlohmann::json::array();
		for( var& nameParameter : params.Parameters() )
			j["parameters"].push_back( nameParameter.second->ToJson() );
		return j;
	}

	std::ostream& operator<<( std::ostream& os, const IBoosterParams& parameters )noexcept
	{
		os << ToJson(parameters) << endl;
		return os;
	}

	uint IBoosterParams::MaxBinValue()const noexcept
	{
		var pMaxBin = std::dynamic_pointer_cast<TParameter<uint>>( (*this)["max_bin"] );
		return pMaxBin->Initial;
	}

	double IBoosterParams::BaggingFractionValue()const noexcept
	{
		if( _parameters.find( string("bagging_fraction") )==_parameters.end() )
			return 0.0;
		auto pParameter = (*this)["bagging_fraction"];
		return pParameter ? std::dynamic_pointer_cast<TParameter<double>>(pParameter)->Initial : 0.0;
	}
	
	uint IBoosterParams::GetThreadCount()const noexcept
	{
		auto pMaxBin = std::dynamic_pointer_cast<TParameter<uint>>( (*this)[ThreadParamName()] );
		return pMaxBin->Initial;
	}

	void IBoosterParams::SetThreadCount( uint count )const noexcept
	{
		auto pMaxBin = std::dynamic_pointer_cast<TParameter<uint>>( (*this)[ThreadParamName()] );
		pMaxBin->Initial = count;
	}

	const shared_ptr<Parameter>& IBoosterParams::operator[]( string_view name )const noexcept(false)
	{
		var pParameterPair = _parameters.find( string(name) );
		if( pParameterPair==_parameters.end() )
			THROW( Exception("Could not find parameter '{}'", name) );
		return pParameterPair->second;
	}

	shared_ptr<Parameter>& IBoosterParams::operator[]( string_view name )noexcept(false)
	{
		auto pParameterPair = _parameters.find( string(name) );
		if( pParameterPair==_parameters.end() )
			THROW( Exception(fmt::format("Could not find parameter '{}', should have every one.", name)) );
		return pParameterPair->second;
	}
/////////////////////////////////////////
	void IBoosterParams::Read( std::istream& is )noexcept
	{
		nlohmann::json j;
		is >> j;
		_bestIteration = j["best_iteration"];
		for( var& element : j["parameters"] )
		{
			string name = element["name"];
			if( name=="application" )
				name = "objective";
			auto pNameParameter = _parameters.find( name );
			var existing = pNameParameter!=_parameters.end();
			if( StringParams.find(name)!=StringParams.end() )
				_parameters[name] = make_shared<TParameter<string>>( element );
			else
			{
				if( DoubleParams.find(name)!=DoubleParams.end() )
				{
					auto pNew = make_shared<TParameter<double>>( element );
					if( existing )
					{
						auto pExisting = std::dynamic_pointer_cast<TParameter<double>>( pNameParameter->second );
						pExisting->Initial = pNew->Initial;
						pExisting->Values = pNew->Values;
					}
					else
					{
						auto pDefaultDouble = dynamic_cast<const TParameter<double>*>( FindDefault(name) ); ASSRT_NN( pDefaultDouble );
						pNew->Minimum = pDefaultDouble->Minimum;
						pNew->Maximum = pDefaultDouble->Maximum;
						_parameters.emplace( pNew->Name, pNew );
					}
				}
				else if( UIntParams.find(name)!=UIntParams.end() )
				{
					auto pNew = make_shared<TParameter<uint>>( element );
					if( existing )
					{
						auto pExisting = std::dynamic_pointer_cast<TParameter<uint>>( pNameParameter->second );
						pExisting->Initial = pNew->Initial;
						pExisting->Values = pNew->Values;
					}
					else
					{
						auto pDefaultUInt = dynamic_cast<const TParameter<uint>*>( FindDefault(name) ); ASSRT_NN( pDefaultUInt );
						pNew->Minimum = pDefaultUInt->Minimum;
						pNew->Maximum = pDefaultUInt->Maximum;
						_parameters.emplace( pNew->Name, pNew );
					}
				}
				else 
					THROW( LogicException(fmt::format("BoosterParam {} not assocated to a type.", name)) );
			}
		}

	}
	const Parameter* IBoosterParams::FindDefault( string_view name )const noexcept
	{
		const Parameter* pParameter{nullptr};
		if( name==ThreadParamName() )
			pParameter = &ThreadCount;
		else if( name==LearningRate.Name )
			pParameter = &LearningRate;
		else if( name==MaxDepth.Name )
			pParameter = &MaxDepth;
		else if( name==Objective.Name )
			pParameter = &Objective;
	
		return pParameter;
	}

	IBoosterParams::IBoosterParams( const IBoosterParams& other ):
		DoubleParams{ other.DoubleParams },
		StringParams{ other.StringParams },
		UIntParams{ other.UIntParams }
	{
		*this = other;
	}
	IBoosterParams& IBoosterParams::operator=( const IBoosterParams& other )
	{
		if( this!=&other )
		{
			_parameters.clear();
			for( var& [name, pParameter] : other.Parameters() )
				_parameters.emplace( name, pParameter->Clone() );
		}
		return *this;
	}

	IBoosterParams::IBoosterParams( const set<string>& doubleParams, const set<string>& stringParams, const set<string>& uintParams )noexcept:
		DoubleParams{doubleParams},
		StringParams{stringParams},
		UIntParams{uintParams}
	{
		_parameters.emplace( MaxDepth.Name, make_shared<TParameter<uint>>(MaxDepth) );
		_parameters.emplace( LearningRate.Name, make_shared<TParameter<double>>(LearningRate) );
	}

	string IBoosterParams::to_string()const noexcept
	{
		std::ostringstream os;	//'verbose=0 task=train boosting_type=gbdt objective=regression metric=l2 num_leaves=30 feature_fraction=0.69 bagging_fraction=0.81 bagging_freq=2 max_bin=255 max_depth=6 device=gpu'
		os << ThreadParamName() << "=" << GetThreadCount() << " ";
		for( var& [name, pParameter] : _parameters )
		{
			if( name!=ThreadParamName() )
				os << name << "=" << pParameter->InitialString() << " ";
		}
		return os.str();
	}

	bool IBoosterParams::operator<( const IBoosterParams& other )const noexcept
	{
		bool less = false;
		ASSRT_EQ( _parameters.size(), other._parameters.size() );
		for( var& [name, pParameter] : _parameters )
		{
			if( name==ThreadParamName() || name=="device" || name=="task" || name=="verbose" )
				continue;
			var pOther = other._parameters.find( name );
			ASSRT_TR( pOther!=other._parameters.end() );
			//if( name=="bagging_fraction" && std::dynamic_pointer_cast<TParameter<double>>(pParameter)->Initial==.66 )
			//	GetDefaultLogger()->trace( "name:  {}", pParameter->InitialString() );
			var equal = pOther==other._parameters.end() || *pParameter==*pOther->second;
			if( !equal )
			{
	 			less = *pParameter<*pOther->second;
				break;
			}
		}
		return less;
	}

	std::set<IBoosterParamsPtr> IBoosterParams::GetSets()const //def get_parameters( params:dict )->[]:
	{
		uint count = 1;
		map<string,uint> multiParams;
		for( var& [name,pParameter] : _parameters )
		{
			var length = pParameter->Size();
			if( length>1 )
			{
				multiParams[name] = length;
				count = length*count;
			}
		}
		std::set<IBoosterParamsPtr> sets;
		for( uint i=0; i<count; ++i )//for i in range(0, count):
		{
			map<string,uint> multiIndexes;
			uint lastParamIndex = 0;
			for( var& [name,length] : multiParams )
			{
				uint minIndex = lastParamIndex;
				uint maxIndex = length*std::max( minIndex, (uint)1 );
				if( minIndex==0 )
					multiIndexes[name] = i%length;  //#0:0, 1:1, 2:2
				else if( i<minIndex )
					multiIndexes[name] = 0;
				else
				{
					multiIndexes[name] = (i-maxIndex*(i/maxIndex))/minIndex;
					if( multiIndexes[name]==length )
						THROW( Exception("Logic error in GetSets!") );
				}						
				lastParamIndex = maxIndex;
			}
			auto pParameters = Create();
			for( var& [name, pParameter] : _parameters ) //key, value in params.items():
			{
				var length = pParameter->Size();
				(*pParameters)[name] = length==1 ? pParameter->Clone() : (*pParameter)[multiIndexes[name]]->Clone();
			}
			sets.emplace( pParameters );
		}
		return sets;
	}

	uint IBoosterParams::HasRangeCount()const
	{
		//auto p = [](std::pair<string, sp<Parameter>> p){return p.second->HasRange;};
		return std::count_if(_parameters.begin(), _parameters.end(), [](std::pair<string, sp<Parameter>> p){return p.second->HasRange;} ); 
	}
#pragma region Statics
	const TParameter<uint> IBoosterParams::MaxDepth = TParameter<uint>( "max_depth", 6, 1);
	const TParameter<double> IBoosterParams::LearningRate = TParameter<double>( "learning_rate", .05, 0, false );
	
	const TParameter<uint> IBoosterParams::ThreadCount = TParameter<uint>( "num_threads", std::thread::hardware_concurrency() );
	const TParameter<string> IBoosterParams::Objective = TParameter<string>( "objective", "regression" );
	const set<string> IBoosterParams::DefaultDoubleParams{ "learning_rate" };
	const set<string> IBoosterParams::DefaultStringParams{ "objective" };
	const set<string> IBoosterParams::DefaultUIntParams{ "max_depth" };
#pragma endregion

}