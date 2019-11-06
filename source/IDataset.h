#pragma once
#include "Exports.h"
namespace Jde::AI::Dts
{
	struct IBoosterParams;
	struct JDE_DTS_VISIBILITY IDataset
	{
		IDataset( uint rowCount, uint columnCount ); //, const std::vector<string>* pColumnNames
		virtual ~IDataset()=default;
		//IDataset( const Eigen::MatrixXf& matrix, const IBoosterParams* pParams, const std::vector<string>* pColumnNames, const IDataset* pTrainingDataset )noexcept(false);
		//IDataset( const Eigen::MatrixXf& matrix, const Eigen::VectorXf& y, const IBoosterParams* pParams, const std::vector<string>* pColumnNames/*=nullptr*/, const IDataset* pTrainingDataset )noexcept(false);

		//virtual ~IDataset();
//		const HDataset Handle()const { return _handle; }

		const uint RowCount{ 0 };
		const uint ColumnCount{ 0 };
	private:
	//	HDataset _handle;
	};
}