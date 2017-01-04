// \brief
//		class Model implementation
//

#include <cassert>
#include <iostream>
#include <queue>
#include <vector>
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Model.h"
#include "SkinMesh.h"


//===========================================================================================
// Load Model File.

// Load assimp context
struct FAssimpLoadContext
{
	std::string		AssetFolder;
	const aiScene	*Scene;
	FModelRef		Model;
};


static void print_tables(int Deeps)
{
	for (int i = 0; i < Deeps; i++)
	{
		std::cout << "    ";
	}
}

static glm::mat4 ConvertToGLMatrix(const aiMatrix4x4 &aiMat)
{
	return glm::mat4(aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
		aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
		aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
		aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4);
}

// breath travel the node tree
static void Assimp_ProcessHierarchy(FAssimpLoadContext &Context)
{
	FNodeHierarchyRef NewSkeleton = new FNodeHierarchy();

	FModel* CurrentModel = Context.Model;
	aiNode* RootNode = Context.Scene->mRootNode;
	std::queue<aiNode*>	 NodeQueue;
	std::map<aiNode*, int> Node2Index;

	NodeQueue.push(RootNode);
	while (!NodeQueue.empty())
	{
		aiNode *Current = NodeQueue.front(); NodeQueue.pop();
		int ParentIdx = NODE_INDEX_NONE;
		
		if (Current->mParent)
		{
			ParentIdx = Node2Index[Current->mParent];
		}

		FNode NewNode;
		NewNode.ParentIdx = ParentIdx;
		NewNode.NodeName = Current->mName.C_Str();
		NewNode.LocalMat = ConvertToGLMatrix(Current->mTransformation);

		int CurrentNodeIdx = NewSkeleton->AddNode(NewNode);
		Node2Index[Current] = CurrentNodeIdx;
		
		// Process each mesh located at the current node
		for (GLuint i = 0; i < Current->mNumMeshes; i++)
		{
			// The node object only contains indices to index the actual objects in the scene. 
			// The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			int MeshDataIdx = Current->mMeshes[i];
			CurrentModel->MeshInstances.push_back(FMeshInstance(CurrentNodeIdx, MeshDataIdx));
		}

		// next children
		for (int k = 0; k < Current->mNumChildren; k++)
		{
			NodeQueue.push(Current->mChildren[k]);
		} // end for
	} // while

	NewSkeleton->CalculateNodesModelMatrix();

	Context.Model->OrgNodeHierarchy = NewSkeleton;
}

// process node animation sequence
static void Assimp_ProcessNodeAnimSequences(FAssimpLoadContext &Context)
{
	const aiScene* Scene = Context.Scene;

	if (!Scene->mNumAnimations)
	{
		return;
	}

	for (unsigned int AnimsIdx = 0; AnimsIdx < Scene->mNumAnimations; AnimsIdx++)
	{
		aiAnimation *CurrentAnim = Scene->mAnimations[AnimsIdx];

		FNodeAnimationSequenceRef AnimSeq = new FNodeAnimationSequence();
		AnimSeq->SeqName = CurrentAnim->mName.C_Str();
		AnimSeq->Duration = CurrentAnim->mDuration;
		AnimSeq->CachedTrans.resize(CurrentAnim->mNumChannels);

		for (unsigned int TrackIdx = 0; TrackIdx < CurrentAnim->mNumChannels; TrackIdx++)
		{
			aiNodeAnim* NodeAnim = CurrentAnim->mChannels[TrackIdx];

			FNodeAnimRef NewTrack = new FNodeAnim();
			NewTrack->NodeName = NodeAnim->mNodeName.C_Str();

			// handle the key-frames
			// POSITION KEYS
			for (unsigned int k = 0; k < NodeAnim->mNumPositionKeys; k++)
			{
				FPositionKey PositionKey;
				PositionKey.Time = NodeAnim->mPositionKeys[k].mTime;

				const aiVector3D& aiV3 = NodeAnim->mPositionKeys[k].mValue;
				PositionKey.Value = glm::vec3(aiV3.x, aiV3.y, aiV3.z);

				NewTrack->PositionKeys.push_back(PositionKey);
			} // end for k

			  // SCALING KEYS
			for (unsigned int k = 0; k < NodeAnim->mNumScalingKeys; k++)
			{
				FScalingKey ScalingKey;
				ScalingKey.Time = NodeAnim->mScalingKeys[k].mTime;

				const aiVector3D& aiV3 = NodeAnim->mScalingKeys[k].mValue;
				ScalingKey.Value = glm::vec3(aiV3.x, aiV3.y, aiV3.z);

				NewTrack->ScalingKeys.push_back(ScalingKey);
			} // end for k

			  // ROTATION KEYS
			for (unsigned int k = 0; k < NodeAnim->mNumRotationKeys; k++)
			{
				FRotationKey RotateKey;
				RotateKey.Time = NodeAnim->mRotationKeys[k].mTime;

				const aiQuaternion& aiQuat = NodeAnim->mRotationKeys[k].mValue;
				RotateKey.Value = glm::quat(aiQuat.w, aiQuat.x, aiQuat.y, aiQuat.z); //w, xyz

				NewTrack->RotationKeys.push_back(RotateKey);
			} // end for k

			AnimSeq->Tracks.push_back(NewTrack);
			AnimSeq->CachedTrans[TrackIdx].NodeName = NewTrack->NodeName;
		} // end for TrackIdx

		Context.Model->NodeAnimSequences.push_back(AnimSeq);
	} // end for AnimsCnt
}

// recursive process scene node
static void Assimp_TravelScene_Recursive(FAssimpLoadContext &Context, aiNode* CurrentNode, int Deeps)
{
	print_tables(Deeps);
	std::cout << CurrentNode->mName.C_Str();

	if (CurrentNode->mNumMeshes) {
		std::cout << "  with meshes<";
	}
	// Process each mesh located at the current node
	for (GLuint i = 0; i < CurrentNode->mNumMeshes; i++)
	{
		// The node object only contains indices to index the actual objects in the scene. 
		// The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = Context.Scene->mMeshes[CurrentNode->mMeshes[i]];
		std::cout << mesh->mName.C_Str() << ", ";
	}
	if (CurrentNode->mNumMeshes) {
		std::cout << ">";
	}
	std::cout << std::endl;

	Deeps++;
	// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (GLuint i = 0; i < CurrentNode->mNumChildren; i++)
	{
		Assimp_TravelScene_Recursive(Context, CurrentNode->mChildren[i], Deeps);
	}
}

// Display Hierarchy. For Debugging
static void Assimp_DisplayHierarchy(FAssimpLoadContext &Context)
{
	Assimp_TravelScene_Recursive(Context, Context.Scene->mRootNode, 0);
}

// process a mesh
static void Assimp_Process_One_Mesh(FAssimpLoadContext &Context, aiMesh *InMesh)
{
	std::vector<FVertex>	Vertexes;
	std::vector<GLuint>		Indices;

	const unsigned int kNumVerts = InMesh->mNumVertices;
	Vertexes.reserve(kNumVerts);
	for (unsigned int Index = 0; Index < kNumVerts; Index++)
	{
		FVertex V;

		V.Position.x = InMesh->mVertices[Index].x;
		V.Position.y = InMesh->mVertices[Index].y;
		V.Position.z = InMesh->mVertices[Index].z;

		V.Normal.x = InMesh->mNormals[Index].x;
		V.Normal.y = InMesh->mNormals[Index].y;
		V.Normal.z = InMesh->mNormals[Index].z;

		V.Tangent.x = InMesh->mTangents[Index].x;
		V.Tangent.y = InMesh->mTangents[Index].y;
		V.Tangent.z = InMesh->mTangents[Index].z;

		V.Bitangent.x = InMesh->mBitangents[Index].x;
		V.Bitangent.y = InMesh->mBitangents[Index].y;
		V.Bitangent.z = InMesh->mBitangents[Index].z;

		// UV channel 0
		if (InMesh->mTextureCoords[0])
		{
			V.TexCoords.x = InMesh->mTextureCoords[0][Index].x;
			V.TexCoords.y = InMesh->mTextureCoords[0][Index].y;
		}

		Vertexes.push_back(V);
	} // end for Index

	const unsigned int kNumFaces = InMesh->mNumFaces;
	Indices.reserve(kNumFaces * 3);
	for (unsigned int Index = 0; Index < kNumFaces; Index++)
	{
		const aiFace &Face = InMesh->mFaces[Index];
		assert(Face.mNumIndices);

		Indices.push_back(Face.mIndices[0]);
		Indices.push_back(Face.mIndices[1]);
		Indices.push_back(Face.mIndices[2]);
	} // end for Index

	FMaterialRef Material = Context.Model->Materials[InMesh->mMaterialIndex];
	FVertexBufferRef VBufferRef = new FVertexBuffer();
	FIndexBufferRef IBufferRef = new FIndexBuffer();

	VBufferRef->FillBuffer(Vertexes);
	IBufferRef->FillBuffer(Indices);

	// Check is a skeleton blend mesh.
	if (InMesh->mNumBones == 0)
	{
		FMeshRef NewMesh = new FMesh(Material, VBufferRef, IBufferRef, GL_TRIANGLES);
		Context.Model->Meshes.push_back(NewMesh);

		std::cout << "Info: Import A Static Mesh" << InMesh->mName.C_Str() << std::endl;
	}
	else
	{
		std::vector<FVertexSkin> SkinVertexInfo(Vertexes.size());
		std::vector<unsigned int> VertexNextInfo(Vertexes.size(), 0);

		std::vector<FMeshBone>	MeshBones;

		for (unsigned int Index = 0; Index < InMesh->mNumBones; Index++)
		{
			const aiBone* aiBonePtr = InMesh->mBones[Index];
			assert(aiBonePtr);

			MeshBones.push_back(FMeshBone(aiBonePtr->mName.C_Str(), ConvertToGLMatrix(aiBonePtr->mOffsetMatrix)));

			// get the vertex detail
			for (unsigned int k = 0; k < aiBonePtr->mNumWeights; k++)
			{
				const aiVertexWeight &VertWeight = aiBonePtr->mWeights[k];
				const unsigned int mVertexId = VertWeight.mVertexId;
				const float weight = VertWeight.mWeight;

				unsigned int &NextSlot = VertexNextInfo[mVertexId];
				FVertexSkin &SkinInfo = SkinVertexInfo[mVertexId];

				assert(NextSlot < (sizeof(SkinInfo.Indices) / sizeof(SkinInfo.Indices[0])));
				SkinInfo.Indices[NextSlot] = Index;
				SkinInfo.Weights[NextSlot] = weight;
				// update to the next slot
				NextSlot++;
			} // end for k
		} // end for Index

		// DEBUG
		if (0)
		{
			std::cout << "------------------VERTEX SKIN INFO---------------------" << std::endl;
			std::cout << "BONES ARRAY: Bones Count=" << MeshBones.size() << std::endl;
			for (unsigned int Index = 0; Index < MeshBones.size(); Index++)
			{
				std::cout << " " << MeshBones[Index].BoneName << ", ";
			} // end for Index

			std::cout << std::endl;
		
			std::cout << " Vertex Blend Info: " << std::endl;
			for (unsigned int Index = 0; Index < SkinVertexInfo.size(); Index++)
			{
				std::cout << "Vertex " << Index << ": BoneIdxs="
					<< (unsigned int)SkinVertexInfo[Index].Indices[0] << ", "
					<< (unsigned int)SkinVertexInfo[Index].Indices[1] << ", "
					<< (unsigned int)SkinVertexInfo[Index].Indices[2] << ", "
					<< (unsigned int)SkinVertexInfo[Index].Indices[3] << "; "
					<< "Weights="
					<< SkinVertexInfo[Index].Weights[0] << ", "
					<< SkinVertexInfo[Index].Weights[1] << ", "
					<< SkinVertexInfo[Index].Weights[2] << ", "
					<< SkinVertexInfo[Index].Weights[3] << std::endl;
			}
		}


		FVertexSkinBufferRef VSkinBufferRef = new FVertexSkinBuffer();
		VSkinBufferRef->FillBuffer(SkinVertexInfo);

		FSkinMeshRef NewMesh = new FSkinMesh(Material, VBufferRef, VSkinBufferRef, IBufferRef, GL_TRIANGLES, MeshBones);
		Context.Model->Meshes.push_back(NewMesh.DeRef());

		std::cout << "Info: Import A Skinning Mesh" << InMesh->mName.C_Str() << std::endl;
	}
}

static void Assimp_PorcessMeshes(FAssimpLoadContext &Context)
{
	const aiScene *scene = Context.Scene;

	for (unsigned int Index = 0; Index < scene->mNumMeshes; Index++)
	{
		Assimp_Process_One_Mesh(Context, scene->mMeshes[Index]);
	}
}

// process material texture
static FTexture2DRef Assimp_ProcessMaterialTexture(FAssimpLoadContext &Context, aiMaterial *InMaterial, aiTextureType InType)
{
	unsigned int nCount = InMaterial->GetTextureCount(InType);
	if (nCount <= 0)
	{
		return nullptr;
	}

	aiString Filename;
	InMaterial->GetTexture(InType, 0, &Filename);

	std::string Pathname = Context.AssetFolder + '/' + Filename.C_Str();

	std::cout << "Assimp_ProcessMaterialTexture: " << Pathname.c_str() << std::endl;
	return FTexture2D::CreateTexture(Pathname);
}

// process materials
static void Assimp_ProcessMaterials(FAssimpLoadContext &Context)
{
	const aiScene *scene = Context.Scene;
	FModel *NewModel = Context.Model;

	for (unsigned int Index = 0; Index < scene->mNumMaterials; Index++)
	{
		aiMaterial* material = scene->mMaterials[Index];
		assert(material);

		FMaterialRef NewMaterial = new FMaterial();

		NewMaterial->TexDiffuse = Assimp_ProcessMaterialTexture(Context, material, aiTextureType_DIFFUSE);
		NewMaterial->TexSpecular = Assimp_ProcessMaterialTexture(Context, material, aiTextureType_SPECULAR);
		NewMaterial->TexNormal = Assimp_ProcessMaterialTexture(Context, material, aiTextureType_NORMALS);

		NewModel->Materials.push_back(NewMaterial);
	} // end for Index

}

FModelRef FModel::CreateModel(const std::string &InFilename)
{
	// Read file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(InFilename, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_LimitBoneWeights);
	// Check for errors
	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return nullptr;
	}

	FModelRef NewModel = new FModel();
	NewModel->AssetPathname = InFilename;

	// Retrieve the directory path of the filepath
	std::string AssetFolder = InFilename.substr(0, InFilename.find_last_of('/'));

	FAssimpLoadContext Context;
	Context.AssetFolder = AssetFolder;
	Context.Scene = scene;
	Context.Model = NewModel;

	// Process Materials;
	Assimp_ProcessMaterials(Context);

	// Process Meshes
	Assimp_PorcessMeshes(Context);

	// Process Hierarchy
	Assimp_ProcessHierarchy(Context);

	// Process Animation Sequence
	Assimp_ProcessNodeAnimSequences(Context);

	// Debug Display Hierarchy
	Assimp_DisplayHierarchy(Context);

	return NewModel;
}

// Create Plane
FModelRef FModel::CreatePlane(const char* InDiffuseTex)
{
	std::vector<FVertex>	Vertexes;
	std::vector<GLuint>		Indices;


	GLfloat planeVertices[] = {
		// Positions            // Normals           // Texture Coords
		25.0f, -0.5f,  25.0f,  0.0f,  1.0f,  0.0f,  25.0f, 0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f,  1.0f,  0.0f,  0.0f,  25.0f,
		-25.0f, -0.5f,  25.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,

		25.0f, -0.5f,  25.0f,  0.0f,  1.0f,  0.0f,  25.0f, 0.0f,
		25.0f, -0.5f, -25.0f,  0.0f,  1.0f,  0.0f,  25.0f, 25.0f,
		-25.0f, -0.5f, -25.0f,  0.0f,  1.0f,  0.0f,  0.0f,  25.0f
	};

	for (GLuint Index = 0; Index < (sizeof(planeVertices)/(sizeof(float)*8)); Index++)
	{
		FVertex V;

		V.Position.x = planeVertices[Index * 8 + 0];
		V.Position.y = planeVertices[Index * 8 + 1];
		V.Position.z = planeVertices[Index * 8 + 2];
		V.Normal.x = planeVertices[Index * 8 + 3];
		V.Normal.y = planeVertices[Index * 8 + 4];
		V.Normal.z = planeVertices[Index * 8 + 5];
		V.TexCoords.x = planeVertices[Index * 8 + 6];
		V.TexCoords.y = planeVertices[Index * 8 + 7];

		Vertexes.push_back(V);
		Indices.push_back(Index);
	} // end for Index

	FMaterialRef NewMaterial = new FMaterial();
	NewMaterial->TexDiffuse = FTexture2D::CreateTexture(InDiffuseTex);

	FVertexBufferRef VBufferRef = new FVertexBuffer();
	FIndexBufferRef IBufferRef = new FIndexBuffer();

	VBufferRef->FillBuffer(Vertexes);
	IBufferRef->FillBuffer(Indices);

	FModelRef NewModel = new FModel();
	NewModel->Meshes.push_back(new FMesh(NewMaterial, VBufferRef, IBufferRef, GL_TRIANGLES));
	NewModel->MeshInstances.push_back(FMeshInstance(NODE_INDEX_NONE, 0));
	return NewModel;
}

// create cube
FModelRef FModel::CreateCube(const char *InDiffuseTex)
{
	std::vector<FVertex>	Vertexes;
	std::vector<GLuint>		Indices;


	GLfloat cubeVertices[] = {
		// Positions            // Normals           // Texture Coords
		// Back face
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
		0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
		0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,  // top-right
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // bottom-left
		-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,// top-left
														  // Front face
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
		0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
		0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // top-right
		0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
		-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom-left
														   // Left face
		-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
		-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-left
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
		-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
														  // Right face
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
		0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-right
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // top-left
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
														 // Bottom face
		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
		0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,// bottom-left
		0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
		-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
															// Top face
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
		0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
		0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right
		0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
		-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f // bottom-left
	};

	for (GLuint Index = 0; Index < (sizeof(cubeVertices)/(sizeof(float)*8)); Index++)
	{
		FVertex V;

		V.Position.x = cubeVertices[Index * 8 + 0];
		V.Position.y = cubeVertices[Index * 8 + 1];
		V.Position.z = cubeVertices[Index * 8 + 2];
		V.Normal.x = cubeVertices[Index * 8 + 3];
		V.Normal.y = cubeVertices[Index * 8 + 4];
		V.Normal.z = cubeVertices[Index * 8 + 5];
		V.TexCoords.x = cubeVertices[Index * 8 + 6];
		V.TexCoords.y = cubeVertices[Index * 8 + 7];

		Vertexes.push_back(V);
		Indices.push_back(Index);
	} // end for Index

	FMaterialRef NewMaterial = new FMaterial();
	NewMaterial->TexDiffuse = FTexture2D::CreateTexture(InDiffuseTex);

	FVertexBufferRef VBufferRef = new FVertexBuffer();
	FIndexBufferRef IBufferRef = new FIndexBuffer();

	VBufferRef->FillBuffer(Vertexes);
	IBufferRef->FillBuffer(Indices);

	FModelRef NewModel = new FModel();
	NewModel->Meshes.push_back(new FMesh(NewMaterial, VBufferRef, IBufferRef, GL_TRIANGLES));
	NewModel->MeshInstances.push_back(FMeshInstance(NODE_INDEX_NONE, 0));

	return NewModel;
}


//========================================================================================================
// Model Operators

void FModel::InitRHI()
{
	for (size_t Index = 0; Index < Meshes.size(); Index++)
	{
		Meshes[Index]->InitRHI();
	}
}

void FModel::ReleaseRHI()
{
	for (size_t Index = 0; Index < Meshes.size(); Index++)
	{
		Meshes[Index]->ReleaseRHI();
	}
}

void FModel::Draw(const FViewContext &InViewContext, FRenderPolicy &InPolicy)
{
	for (size_t Index = 0; Index < MeshInstances.size(); Index++)
	{
		FMeshRef Mesh = Meshes[MeshInstances[Index].MeshIdx];
		Mesh->Draw(InViewContext, InPolicy, *this, MeshInstances[Index]);
	}
}

void FModel::Tick(float deltTime)
{
	if (SeqPlayedIndex == NODE_INDEX_NONE)
	{
		return;
	}

	FNodeAnimationSequenceRef AnimSeq = NodeAnimSequences[SeqPlayedIndex];
	assert(IsValidRef(AnimSeq));

	if (IsPlaying)
	{
		AnimSeq->CachedCalculateTransform(TimeElapse);

		TimeElapse += deltTime;
		if (TimeElapse > AnimSeq->GetDuration())
		{
			TimeElapse = 0.f;
		}

		// UPDATE the hierarchy
		FNodeHierarchyRef TargetHierarchy = PlayingHierarchy;
		const FNodeAnimationSequence::CachedNodeTransArray& CachedTrans = AnimSeq->GetCachedTransform();
		for (unsigned int kNode = 0; kNode < CachedTrans.size(); kNode++)
		{
			const FNodeAnimationSequence::CachedNodeTrans &Element = CachedTrans[kNode];

			int NodeIdx = TargetHierarchy->GetNodeIndex(Element.NodeName);
			assert(NodeIdx != NODE_INDEX_NONE);
			FNode &Node = TargetHierarchy->GetNode(NodeIdx);
			Node.LocalMat = Element.TransMat;

		} // end for

		PlayingHierarchy->CalculateNodesModelMatrix();
	}
}

bool FModel::Play(int InSequence)
{
	if (InSequence < 0 || InSequence >= NodeAnimSequences.size())
	{
		std::cout << "Play Sequence is not exist: " << InSequence << std::endl;

		PlayingHierarchy = nullptr;
		SeqPlayedIndex = NODE_INDEX_NONE;
		TimeElapse = 0.f;
		IsPlaying = false;

		return false;
	}

	PlayingHierarchy = OrgNodeHierarchy->Clone();
	SeqPlayedIndex = InSequence;
	TimeElapse = 0.f;
	IsPlaying = true;

	return true;
}

void FModel::Pause(bool InbPause)
{
	IsPlaying = InbPause;
}

//========================================================================================================
// Animation Operators

void FNodeHierarchy::CalculateNodesModelMatrix()
{
	for (size_t k = 0; k < NodesArray.size(); k++)
	{
		FNode &Node = NodesArray[k];

		if (Node.ParentIdx != NODE_INDEX_NONE)
		{
			const FNode &Parent = NodesArray[Node.ParentIdx];
			Node.ModelMat = Parent.ModelMat * Node.LocalMat;
		}
		else
		{
			Node.ModelMat = Node.LocalMat;
		}
	} // end for k

	// FOR DEBUG
	UpdateDebugHierarchyLines();
}

const std::vector<FSimpleVertex>& FNodeHierarchy::GetDebugHierarchyLines()
{
	unsigned int LinesCount = (NodesArray.size() - 1) << 1;
	if (DebugHierarchyLines.size() != LinesCount)
	{
		DebugHierarchyLines.resize(LinesCount);
	}

	return DebugHierarchyLines;
}

void FNodeHierarchy::UpdateDebugHierarchyLines()
{
	unsigned int LinesCount = (NodesArray.size() - 1) << 1;
	if (DebugHierarchyLines.size() != LinesCount)
	{
		DebugHierarchyLines.resize(LinesCount);
	}

	for (size_t k = 0, Index=0; k < NodesArray.size(); k++)
	{
		FNode &Node = NodesArray[k];
		
		if (Node.ParentIdx != NODE_INDEX_NONE)
		{
			const FNode &Parent = NodesArray[Node.ParentIdx];
			
			const glm::mat4::col_type &T0 = Parent.ModelMat[3];
			const glm::mat4::col_type &T1 = Node.ModelMat[3];

			DebugHierarchyLines[Index++].Position = glm::vec3(T0.x, T0.y, T0.z);
			DebugHierarchyLines[Index++].Position = glm::vec3(T1.x, T1.y, T1.z);
		}
	} // end for k
}

//////////////////////////////////////////////////////////////////////////
glm::mat4 FNodeAnim::GetTransformMatrix(double InTime)
{
	glm::vec3 position, scale;
	glm::quat rotate;
	unsigned int k;
	int dest;

	// POSITION
	for (k = 0, dest = -1; k < PositionKeys.size(); k++)
	{
		if (PositionKeys[k].Time > InTime)
		{
			break;
		}

		dest++;
	} // end for k

	if (dest < 0)
	{
		position = PositionKeys[0].Value;
	}
	else if (dest >= (PositionKeys.size() - 1))
	{
		dest = (PositionKeys.size() - 1);
		position = PositionKeys[dest].Value;
	}
	else
	{
		float t = (InTime - PositionKeys[dest].Time) / (PositionKeys[dest + 1].Time - PositionKeys[dest].Time);
		position = PositionKeys[dest].Value * (1.0f - t) + PositionKeys[dest + 1].Value * t;
	}

	// SCALING
	for (k = 0, dest = -1; k < ScalingKeys.size(); k++)
	{
		if (ScalingKeys[k].Time > InTime)
		{
			break;
		}

		dest++;
	} // end for k

	if (dest < 0)
	{
		scale = ScalingKeys[0].Value;
	}
	else if (dest >= (ScalingKeys.size() - 1))
	{
		dest = (ScalingKeys.size() - 1);
		scale = ScalingKeys[dest].Value;
	}
	else
	{
		float t = (InTime - ScalingKeys[dest].Time) / (ScalingKeys[dest + 1].Time - ScalingKeys[dest].Time);
		scale = ScalingKeys[dest].Value * (1.0f - t) + ScalingKeys[dest + 1].Value * t;
	}

	// ROTATION
	for (k = 0, dest = -1; k < RotationKeys.size(); k++)
	{
		if (RotationKeys[k].Time > InTime)
		{
			break;
		}

		dest++;
	} // end for k

	if (dest < 0)
	{
		rotate = RotationKeys[0].Value;
	}
	else if (dest >= (RotationKeys.size() - 1))
	{
		dest = (RotationKeys.size() - 1);
		rotate = RotationKeys[dest].Value;
	}
	else
	{
		float t = (InTime - RotationKeys[dest].Time) / (RotationKeys[dest + 1].Time - RotationKeys[dest].Time);
		rotate = slerp(RotationKeys[dest].Value, RotationKeys[dest + 1].Value, t);
	}

	return glm::translate(glm::mat4(), position) * glm::scale(glm::mat4(), scale) * ((glm::mat4)(rotate));
}

void FNodeAnimationSequence::CachedCalculateTransform(double InTime)
{
	if (InTime < 0.0) {
		InTime = 0.0;
	}
	else if (InTime > Duration) {
		InTime = Duration;
	}

	for (size_t k = 0; k < Tracks.size(); k++)
	{
		FNodeAnimRef Element = Tracks[k];

		CachedTrans[k].TransMat = Element->GetTransformMatrix(InTime);
	} // end for k
}
