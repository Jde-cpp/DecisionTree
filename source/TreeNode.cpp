#include "stdafx.h"
#include "TreeNode.h"
#define var const auto

namespace Jde::AI::Dts
{
	//"  { \"nodeid\": 0, \"depth\": 0, \"split\": \"\", \"split_condition\": -0.17595838, \"yes\": 1, \"no\": 2, \"missing\": 2, \"gain\": 1.18505859, \"cover\": 23636, \"children\": ["
	//"    { \"nodeid\": 1, \"leaf\": -0.0434296243, \"cover\": 7 },"
	//"  { \"nodeid\": 0, \"depth\": 0, \"split\": \"col41\", \"split_condition\": 0.31285727, \"yes\": 1, \"no\": 2, \"missing\": 2, \"gain\": 1.37304688, \"cover\": 23652, \"children\": [\n    { \"nodeid\": 1, \"leaf\": -0.025050504"...
	TreeNode::TreeNode( nlohmann::json json ):
		Id{ json["nodeid"].get<uint>() }
	{
		if( json.find("children")!=json.end() )
		{
			Depth = json["depth"].get<uint>();
			FeatureName = json["split"].get<string>();
			SplitCondition = json["split_condition"].get<double>();
			Gain = json["gain"].get<double>();
			Cover = json["cover"].get<uint>();
			const uint yesNodeId{ json["yes"].get<uint>() };
			const uint noNodeId{ json["no"].get<uint>() };
			const uint missingNodeId{ json["missing"].get<uint>() };
			nlohmann::json children = json["children"];
			for( auto& child : children )
			{
				var pTree = make_shared<TreeNode>( child );
				if( yesNodeId == pTree->Id )
					YesNodePtr = pTree;
				if( noNodeId == pTree->Id )
					NoNodePtr = pTree;
				if( missingNodeId == pTree->Id )
					MissingNodePtr = pTree;
			}
		}
		else
			Leaf = json["leaf"].get<double>();
	}
};