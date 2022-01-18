#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>

using namespace DirectX;

struct BoneInfo {
  XMMATRIX offset;
  XMMATRIX transformation;
};

struct HierarchyNode {
	std::string name;
	XMMATRIX transformation;
	UINT numChildren = 0;
	HierarchyNode* children = nullptr;
};

struct KeyFrame {
	FLOAT time;
	XMVECTOR key;
};

struct BoneKeys {
	std::vector<KeyFrame> positions;
	std::vector<KeyFrame> scaling;
	std::vector<KeyFrame> rotation;
};

template<typename T>
void LoadSkinningVertexInformation(aiMesh* mesh, std::map<std::string, BoneInfo>& boneInfo, T* vertexData) {
	std::vector<UINT> numBonesPerVertex(mesh->mNumVertices, 0U);
	for (UINT meshBoneIndex = 0; meshBoneIndex < mesh->mNumBones; meshBoneIndex++) {
		auto bone = mesh->mBones[meshBoneIndex];
		std::string boneName = bone->mName.C_Str();
		UINT boneId = static_cast<UINT>(std::distance(boneInfo.begin(), boneInfo.find(boneName)));

		for (UINT weightIndex = 0; weightIndex < bone->mNumWeights; weightIndex++) {
			auto weight = bone->mWeights[weightIndex];

			//skip bones with 0 weight
			//ignorar los huesos con un peso de 0
			if (weight.mWeight == 0.0f)
				continue;

			UINT offset = numBonesPerVertex[weight.mVertexId];
			if (offset >= 4) continue;

			//use offset to put the value in the propper slot (x:0, y:1, z:2, w:3)
			//usar el offset para poner el valor en el slot adecuado (x:0, y:1, z:2, w:3)
			uint32_t* boneIdSlot = &vertexData[weight.mVertexId].BoneIds.x + offset;
			float* weightSlot = &vertexData[weight.mVertexId].BoneWeights.x + offset;
			*boneIdSlot = boneId;
			*weightSlot = weight.mWeight;
			numBonesPerVertex[weight.mVertexId]++;
		}
	}
}

void BuildBoneInfo(const aiScene* aiModel, std::map<std::string, BoneInfo>& boneInfo);
void BuildAnimationChannelsKeys(const aiScene* model, std::map<std::string, std::map<std::string, BoneKeys>>& animationChannelsKeys);
void BuildNodesHierarchy(aiNode* node, HierarchyNode* nodeInHierarchy);
void DestroyNodesHierarchy(HierarchyNode* node);

XMMATRIX InterpolateKeys(XMMATRIX(XM_CALLCONV* ToMatrix)(XMVECTOR), XMVECTOR(XM_CALLCONV* Interpolator)(XMVECTOR, XMVECTOR, float), FLOAT time, std::vector<KeyFrame>& keyFrames);
void TraverseNodeHierarchy(FLOAT time, HierarchyNode* node, std::map<std::string, BoneKeys>& boneKeys, std::map<std::string, BoneInfo>& boneInfo, XMMATRIX& rootNodeInverseTransform, XMMATRIX parentTransformation);

template <typename Animated>
void NextAnimation(Animated* animated) {
	auto animation = animated->animationsChannelsKeys.find(animated->currentAnimation);
	animation++;
	if (animation == animated->animationsChannelsKeys.end()) {
		animation = animated->animationsChannelsKeys.begin();
	}
	animated->currentAnimation = animation->first;
	animated->currentAnimationTime = 0.0f;
}

template <typename Animated>
void PreviousAnimation(Animated* animated) {
	auto animation = animated->animationsChannelsKeys.find(animated->currentAnimation);
	if (animation == animated->animationsChannelsKeys.begin()) {
		animation = animated->animationsChannelsKeys.end();
	}
	animation--;
	animated->currentAnimation = animation->first;
	animated->currentAnimationTime = 0.0f;
}