#include "Exports.h"

namespace Jde::AI::Dts
{
	struct IBooster;
	struct IBoosterParams;
	struct BoosterParamPtrCompare;
	struct IDataset;
	struct IDecisionTree;
	struct ParamResults;
namespace Tuning
{
	typedef sp<IBoosterParams> IBoosterParamsPtr;

	JDE_DTS_VISIBILITY IBoosterParamsPtr Tune( IDecisionTree& decisionTree, vector<unique_ptr<Eigen::MatrixXf>>& xs, vector<Math::VPtr<>>& ys, uint testCount, const fs::path& saveStem, uint foldCount, string_view objective )noexcept(false);
	IBoosterParamsPtr TuneOne( IDecisionTree& decisionTree, vector<unique_ptr<Eigen::MatrixXf>>& xs, vector<Math::VPtr<>>& ys, const fs::path& saveStem, uint foldCount, string_view parameterName )noexcept(false);

	typedef map<IBoosterParamsPtr, ParamResults, BoosterParamPtrCompare> PreviousExecutions;
	IBoosterParamsPtr SaveTuning( const fs::path& saveStem, const PreviousExecutions& previousExecutions )noexcept(false);

	sp<IBooster> Train( IDecisionTree& decisionTree, sp<const IDataset>& pTrain, const IBoosterParams& params, uint count, sp<const IDataset> pValidation=nullptr, uint earlyStoppingRounds=std::numeric_limits<uint>::max() )noexcept(false);
}}
