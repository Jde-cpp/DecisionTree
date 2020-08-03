#pragma once
#include "Exports.h"
#include <functional>

namespace Jde::AI::Dts
{
#pragma region ParameterType
	enum class ParameterType : uint8
	{
		None=0,
		String=1,
		Double=2,
		UInt=3
	};
#pragma endregion
#pragma region Parameter
	struct JDE_DTS_VISIBILITY Parameter
	{
		Parameter( string_view name, bool hasRange=false );
		//Parameter( std::istream& is );
		string Name;
		bool HasRange;
		virtual sp<Parameter> Clone()const noexcept=0;
		virtual sp<Parameter> operator[]( uint index )const noexcept(false)=0;
		virtual bool operator<( const Parameter& other )const noexcept=0;
		virtual bool operator==( const Parameter& other )const noexcept=0;

		virtual string InitialString()const=0;
		virtual bool HaveDelta()const=0;
		virtual string DeltaString()const=0;
		virtual nlohmann::json ToJson()const noexcept=0;
		virtual uint Size()const=0;//{return 0;}
		//virtual void SetDelta( string delta )noexcept;
		//virtual void SetInitial( string initial )noexcept;
		//nlohmann::json ToJson()const;
		virtual ParameterType Type()const=0;
		virtual void SetRange()=0;
	};
	inline std::ostream& operator<<( std::ostream& os, const Parameter& parameter )noexcept;
#pragma endregion
#pragma region TParameter
	template<typename T>
	struct TParameter : public Parameter
	{
		TParameter( string_view name, const T& initial );
		TParameter( string_view name, const T& initial, const T& delta, bool hasRange=true );
		TParameter( string_view name, const T& initial, const T& delta, T minimum, T maximum, bool hasRange=true );
		TParameter( const nlohmann::json& j )noexcept;
		virtual ~TParameter()=default;
		sp<Parameter> Clone()const noexcept override;
		string InitialString()const override;
		string DeltaString()const override;
		virtual nlohmann::json ToJson()const noexcept override;
		//void SetInitial( string initial )noexcept;
		void SetRange()override;
		uint Size()const override{ return Values.size()==0 ? 1 : Values.size(); }
		bool HaveDelta()const override;
		JDE_DTS_VISIBILITY ParameterType Type()const override;

		sp<Parameter> operator[]( uint index )const noexcept(false) override;
		bool operator<( const Parameter& other )const noexcept override;
		bool operator==( const Parameter& other )const noexcept override;
		T Initial;
		T Delta;
		T Minimum;
		T Maximum;
		std::vector<T> Values;
	};
#pragma endregion
#pragma region IBoosterParams
	struct JDE_DTS_VISIBILITY IBoosterParams
	{
		virtual ~IBoosterParams(){}
		virtual sp<IBoosterParams> Create()const noexcept=0;
		virtual sp<IBoosterParams> Clone()const noexcept=0;

		virtual string to_string()const noexcept;
		IBoosterParams& operator=( const IBoosterParams& );

		const shared_ptr<Parameter>& operator[]( string_view name )const noexcept(false);
		shared_ptr<Parameter>& operator[]( string_view name )noexcept(false);
		const map<string,sp<Parameter>>& Parameters()const{return _parameters;}
		void Read( std::istream& is )noexcept;
		std::set<sp<IBoosterParams>> GetSets()const;
		//virtual void FixObjective( TParameter<string>& parameter )const noexcept{};
		virtual const Parameter* FindDefault( string_view name )const noexcept;
		virtual string GetMetric()const noexcept=0; virtual void SetMetric( string_view metric )noexcept=0;
		uint BestIteration()const noexcept{ return _bestIteration; } void SetBestIteration(uint value)const noexcept{_bestIteration=value;}
		uint MaxBinValue()const noexcept;
		uint GetThreadCount()const noexcept;void SetThreadCount( uint count )const noexcept;/*const because not significant*/
		virtual string DeviceValue()const noexcept=0; virtual void SetCpu()const noexcept=0; virtual void SetGpu()const noexcept=0;/*const because not significant*/
		virtual string_view ThreadParamName()const noexcept=0;

		uint HasRangeCount()const;
		double BaggingFractionValue()const noexcept;

		bool operator<( const IBoosterParams& other )const noexcept;

	protected:
		IBoosterParams()=delete;
		IBoosterParams( const IBoosterParams& other );
		IBoosterParams( const set<string>& doubleParams, const set<string>& stringParams, const set<string>& uintParams )noexcept;

		const static set<string> DefaultDoubleParams;
		const static set<string> DefaultStringParams;
		const static set<string> DefaultUIntParams;

		const static TParameter<uint> ThreadCount;
		const static TParameter<string> Objective;

		const set<string>& DoubleParams;
		const set<string>& StringParams;
		const set<string>& UIntParams;
	private:
		mutable uint _bestIteration{0};

		const static TParameter<double> LearningRate;
		const static TParameter<uint> MaxDepth;
	protected:
		map<string,sp<Parameter>> _parameters;
	};
	std::ostream& operator<<( std::ostream& os, const IBoosterParams& parameter )noexcept;

	typedef sp<IBoosterParams> IBoosterParamsPtr;
	struct BoosterParamPtrCompare
	{
		//using is_transparent = void;
		bool operator()( const IBoosterParamsPtr& a, const IBoosterParamsPtr& b ) const{ return *a<*b; }
		bool operator()( const IBoosterParamsPtr& a, const IBoosterParams& b ) const{return *a < b;}
		bool operator()( const IBoosterParams& a, const IBoosterParamsPtr& b ) const{return a < *b;}
	};
#pragma endregion
	template<typename T>
	TParameter<T>::TParameter( string_view name, const T& initial ):
		TParameter{ name, initial, T{}, false }
	{}

	template<>
	inline TParameter<string>::TParameter( string_view name, const string& initial, const string& delta, bool hasRange ):
		TParameter{ name, initial, delta, string(), string(), hasRange }
	{}

	template<typename T>
	TParameter<T>::TParameter( string_view name, const T& initial, const T& delta, bool hasRange ):
		TParameter{ name, initial, delta, 0, std::numeric_limits<T>::max(), hasRange }
	{}

	template<typename T>
	TParameter<T>::TParameter( string_view name, const T& initial, const T& delta, T minimum, T maximum, bool hasRange ):
		Parameter{ name, hasRange },
		Initial{ initial },
		Delta{ delta },
		Minimum{ minimum },
		Maximum{ maximum },
		Values{ initial }
	{}


	template<typename T>
	TParameter<T>::TParameter( const nlohmann::json& j )noexcept:
		Parameter( j["name"].get<std::string>() )//
	{
		HasRange = j["has_range"];
		Initial = j["initial"];
		if( j.find("delta") != j.end() )
			Delta = j["delta"];
	}
	template<typename T>
	nlohmann::json TParameter<T>::ToJson()const noexcept
	{
		nlohmann::json j;
		j["name"] = Name;
		j["has_range"] = HasRange;
		j["initial"] = Initial;
		if( HaveDelta() )
			j["delta"] = Delta;
		return j;
	}

	template<typename T>
	sp<Parameter> TParameter<T>::Clone()const noexcept
	{
		return make_shared<TParameter<T>>( *this );
	}
	template<>
	inline void TParameter<string>::SetRange()
	{
		Values.clear();
		Values.push_back( Initial );
	}
	template<typename T>
	void TParameter<T>::SetRange()
	{
		Values.clear();
		const auto haveDelta = HaveDelta();
		if( haveDelta && Initial!=Minimum )
		{
			const auto value = Initial-Delta;
			if( value>=Minimum )
				Values.push_back( value );
		}
		if( Name!="num_leaves" || Initial>1 )
			Values.push_back( Initial );
		if( haveDelta )
		{
			const auto value = Initial+Delta;
			if( value<=Maximum )
				Values.push_back( value );
		}
	}
	template<>
	inline bool TParameter<string>::HaveDelta()const{ return false; }
	template<typename T>
	inline bool TParameter<T>::HaveDelta()const{ return Delta!=0; }

	template<typename T>
	string TParameter<T>::InitialString()const
	{
		//constexpr std::string_view typeName = Jde::GetTypeName<T>();
		//GetDefaultLogger()->debug( typeName );
		std::ostringstream os;
		os << Initial;
		return os.str();
	}
	template<typename T>
	string TParameter<T>::DeltaString()const
	{
		string delta;
		if( HaveDelta() )
		{
			std::ostringstream os;
			os << Delta;
			delta = os.str();
		}
		return delta;
	}

	template<typename T>
	sp<Parameter> TParameter<T>::operator[]( uint index )const noexcept(false)
	{
		ASSERT( index<Values.size() );
		return make_shared<TParameter<T>>( Name, Values[index], Delta, HasRange );
	}

	template<>
	inline bool TParameter<double>::operator==( const Parameter& other )const noexcept
	{
		const auto& otherDerived = dynamic_cast<const TParameter<double>&>( other );
		return std::abs(otherDerived.Initial-Initial)<std::numeric_limits<double>::epsilon()*32.0;
	}

	template<>
	inline bool TParameter<double>::operator<( const Parameter& other )const noexcept
	{
		const auto& otherDerived = dynamic_cast<const TParameter<double>&>( other );
		return !(*this==other) && otherDerived.Initial-Initial>std::numeric_limits<double>::epsilon()*32.0;
	}

	template<typename T>
	bool TParameter<T>::operator<( const Parameter& other )const noexcept
	{
		const auto& otherDerived = dynamic_cast<const TParameter<T>&>( other );
		return Initial<otherDerived.Initial;
	}
	template<typename T>
	bool TParameter<T>::operator==( const Parameter& other )const noexcept
	{
		const auto& otherDerived = dynamic_cast<const TParameter<T>&>( other );
		return Initial==otherDerived.Initial;
	}


}