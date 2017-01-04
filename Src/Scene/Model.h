// \brief
//		Model class, a collection of meshes.
//

#ifndef __JETX_SCENE_MODEL_H__
#define __JETX_SCENE_MODEL_H__

#include <vector>
#include <string>
#include <map>

#include <Common/RefCounting.h>
#include "Mesh.h"
#include "Render.h"

#define NODE_INDEX_NONE		(-1)
#define MESH_INDEX_NONE		(-1)

// Node & Hierarchy In the Model
class FNode
{
public:
	FNode() : ParentIdx(NODE_INDEX_NONE)
	{}

	FNode(const std::string &InName, int InParent, const glm::mat4 &InLocal)
		: ParentIdx(InParent)
		, NodeName(InName)
		, LocalMat(InLocal)
	{
	}

	int			ParentIdx;
	std::string	NodeName;
	glm::mat4	LocalMat;
	glm::mat4	ModelMat;
};

// Node hierarchy
class FNodeHierarchy;
typedef TRefCountPtr<FNodeHierarchy>	FNodeHierarchyRef;

class FNodeHierarchy : public FRefCountedObject
{
public:
	FNodeHierarchy()
	{}

	// Add a node, return the index
	int AddNode(const FNode &InNode)
	{
		int Index = NodesArray.size();
		NodesArray.push_back(InNode);
		Name2Index[InNode.NodeName] = Index;

		return Index;
	}

	// Get node
	const FNode& GetNode(int Index) const
	{
		return NodesArray[Index];
	}

	FNode& GetNode(int Index)
	{
		return NodesArray[Index];
	}

	unsigned int GetNodesCount() const
	{
		return NodesArray.size();
	}

	FNodeHierarchyRef Clone() const
	{
		return new FNodeHierarchy(*this);
	}

	// Get node index
	int GetNodeIndex(const std::string &InName)
	{
		std::map<std::string, int>::iterator itr = Name2Index.find(InName);
		if (itr != Name2Index.end())
		{
			return itr->second;
		}

		return NODE_INDEX_NONE;
	}

	void CalculateNodesModelMatrix();

	const std::vector<FSimpleVertex>& GetDebugHierarchyLines();

protected:
	void UpdateDebugHierarchyLines();

protected:
	std::vector<FNode>			NodesArray;
	std::map<std::string, int>	Name2Index;

	std::vector<FSimpleVertex>	DebugHierarchyLines;
};



//\brief
//	A key-frame data 
class FKeyFrame_Vector
{
public:
	double		Time;
	glm::vec3	Value;
};

class FKeyFrame_Rotation
{
public:
	double		Time;
	glm::quat	Value;
};

typedef FKeyFrame_Vector	FPositionKey;
typedef FKeyFrame_Vector	FScalingKey;
typedef FKeyFrame_Rotation	FRotationKey;


//\brief 
//	node animation
// contains all the key-frames of a node.
class FNodeAnim : public FRefCountedObject
{
public:
	FNodeAnim()
	{}

	// get the transform matrix at a time.
	glm::mat4 GetTransformMatrix(double InTime);

public:
	std::string		NodeName;

	std::vector<FPositionKey>		PositionKeys; // NOTE: the count of keys of the three parts maybe are not equal
	std::vector<FRotationKey>		RotationKeys;
	std::vector<FScalingKey>		ScalingKeys;
};
typedef TRefCountPtr<FNodeAnim>		FNodeAnimRef;


// Node Animation Sequence
// Contains all the nodes's animation data
class FNodeAnimationSequence : public FRefCountedObject
{
public:
	struct CachedNodeTrans
	{
		std::string		NodeName;
		glm::mat4		TransMat;
	};
	typedef std::vector<CachedNodeTrans>	CachedNodeTransArray;

	FNodeAnimationSequence()
	{}

	void CachedCalculateTransform(double InTime);

	const CachedNodeTransArray& GetCachedTransform() const
	{
		return CachedTrans;
	}

	const std::string& GetName() const { return SeqName; }
	double GetDuration() const { return Duration; }

public:
	std::string		SeqName;
	double			Duration;

	std::vector<FNodeAnimRef>	Tracks;  // a track represent one node anims.

	std::vector<CachedNodeTrans> CachedTrans;
};
typedef TRefCountPtr<FNodeAnimationSequence> FNodeAnimationSequenceRef;


//======================================================================================
// Model
//======================================================================================

class FModel;
typedef TRefCountPtr<FModel>	FModelRef;

struct FMeshInstance
{
	FMeshInstance()
		: NodeIdx(NODE_INDEX_NONE)
		, MeshIdx(MESH_INDEX_NONE)
	{}

	FMeshInstance(int InNodeIdx, int InMeshIdx)
		: NodeIdx(InNodeIdx)
		, MeshIdx(InMeshIdx)
	{}

	int		NodeIdx;
	int		MeshIdx;
};

// class Model
class FModel : public FRefCountedObject
{
public:
	FModel()
		: TimeElapse(0.f)
		, IsPlaying(false)
		, SeqPlayedIndex(NODE_INDEX_NONE)
	{}

	static FModelRef CreateModel(const std::string &InFilename);
	static FModelRef CreatePlane(const char *InDiffuseTex);
	static FModelRef CreateCube(const char *InDiffuseTex);

	void Draw(const FViewContext &InViewContext, FRenderPolicy &InPolicy);

	void InitRHI();
	void ReleaseRHI();


	FNodeHierarchyRef GetNodeHierarchy() const
	{
		return IsValidRef(PlayingHierarchy) ? PlayingHierarchy : OrgNodeHierarchy;
	}

	unsigned int GetAnimationSequeceCount() const { return NodeAnimSequences.size(); }

	void Tick(float deltTime);
	bool Play(int InSequence);
	void Pause(bool InbPause);

public:
	std::vector<FMaterialRef>	Materials;
	std::vector<FMeshRef>		Meshes;
	std::string					AssetPathname;

	std::vector<FNodeAnimationSequenceRef>	NodeAnimSequences;
	FNodeHierarchyRef			OrgNodeHierarchy;  // Origin Hierarchy
	
	// Mesh Instances
	std::vector<FMeshInstance>	MeshInstances;

	// PlayInstance Information
	FNodeHierarchyRef	PlayingHierarchy;
	float				TimeElapse;
	bool				IsPlaying;
	int					SeqPlayedIndex;
};

#endif // __JETX_SCENE_MODEL_H__
