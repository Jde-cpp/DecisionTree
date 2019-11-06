#include "Exports.h"

namespace Jde::AI::Dts
{
	//"  { \"nodeid\": 0, \"depth\": 0, \"split\": \"\", \"split_condition\": -0.17595838, \"yes\": 1, \"no\": 2, \"missing\": 2, \"gain\": 1.18505859, \"cover\": 23636, \"children\": ["
	//"    { \"nodeid\": 1, \"leaf\": -0.0434296243, \"cover\": 7 },"
	struct JDE_DTS_VISIBILITY TreeNode
	{
		TreeNode( nlohmann::json json );
		uint Id{ std::numeric_limits<uint>::max() };
		uint Depth{ std::numeric_limits<uint>::max() };
		string FeatureName;
		double SplitCondition{0.0};
		double Leaf{0.0};
		sp<TreeNode> YesNodePtr;
		sp<TreeNode> NoNodePtr;
		sp<TreeNode> MissingNodePtr;
		double Gain{0.0};
		uint Cover{0};
		bool IsLeaf()const noexcept{ return !YesNodePtr; }
	private:
/*		uint _yesNodeId{0};
		uint _noNodeId{0};
		uint _missingNodeId{0};*/
	};
}