#pragma once

namespace Jde::AI::Dts
{
	enum class EFeatureImportance : uint8
	{	
		None=0x0, 
		Gain=0x1, 
		Cover=0x2, 
		Total=0x4, 
		TotalGain=0x5, 
		TotalCover=0x6, 
		Weight=0x8 
	};
	inline EFeatureImportance operator&(EFeatureImportance a, EFeatureImportance b){ return (EFeatureImportance)( (uint8)a & (uint8)b ); }
}