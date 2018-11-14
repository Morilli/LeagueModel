#include "league/skeleton.hpp"
#include "league/skin.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>

League::Skeleton::Skeleton(League::Skin & a_Skin) :
	m_Skin(a_Skin)
{
}

void RecursiveInvertGlobalMatrices(const glm::mat4& a_Parent, League::Skeleton::Bone& a_Bone)
{
	auto t_Global = a_Parent * a_Bone.LocalMatrix;
	a_Bone.GlobalMatrix = t_Global;
	a_Bone.InverseGlobalMatrix = glm::inverse(t_Global);

	for (auto t_Child : a_Bone.Children)
		RecursiveInvertGlobalMatrices(t_Global, *t_Child);
}

void League::Skeleton::Load(StringView a_FilePath, OnLoadFunction a_OnLoadFunction, void * a_Argument)
{
	auto* t_File = FileSystem::GetFile(a_FilePath);

	struct LoadData
	{
		LoadData(Skeleton* a_Target, OnLoadFunction a_Function, void* a_Argument) :
			Target(a_Target), OnLoadFunction(a_Function), Argument(a_Argument)
		{}

		Skeleton* Target;
		OnLoadFunction OnLoadFunction;
		void* Argument;
	};
	auto* t_LoadData = new LoadData(this, a_OnLoadFunction, a_Argument);

	t_File->Load([](File* a_File, File::LoadState a_LoadState, void* a_Argument)
	{
		auto* t_LoadData = (LoadData*)a_Argument;
		auto* t_Skeleton = (Skeleton*)t_LoadData->Target;

		if (a_LoadState != File::LoadState::Loaded)
		{
			t_Skeleton->m_State = a_LoadState;
			if (t_LoadData->OnLoadFunction) t_LoadData->OnLoadFunction(*t_Skeleton, t_LoadData->Argument);

			FileSystem::CloseFile(*a_File);
			delete t_LoadData;
			return;
		}

		size_t t_Offset = 0;
		uint32_t t_Signature;
		Skeleton::Type t_SkeletonType;
		a_File->Get(t_Signature, t_Offset);
		a_File->Get(t_SkeletonType, t_Offset);

		File::LoadState t_State;
		switch (t_SkeletonType)
		{
		case Skeleton::Type::Classic:
			t_State = t_Skeleton->ReadClassic(*a_File, t_Offset);

		case Skeleton::Type::Version2:
			t_State = t_Skeleton->ReadVersion2(*a_File, t_Offset);
		};

		// Post process if successful
		if (t_State == File::LoadState::Loaded)
		{
			for (auto& t_Bone : t_Skeleton->m_Bones)
			{
				if (t_Bone.ParentID != -1) continue;
				RecursiveInvertGlobalMatrices(glm::identity<glm::mat4>(), t_Bone);
			}
		}

		t_Skeleton->m_State = t_State;
		if (t_LoadData->OnLoadFunction) t_LoadData->OnLoadFunction(*t_Skeleton, t_LoadData->Argument);
		FileSystem::CloseFile(*a_File);
		delete t_LoadData;
	}, t_LoadData);
}

const std::vector<League::Skeleton::Bone>& League::Skeleton::GetBones() const
{
	return m_Bones;
}

const League::Skeleton::Bone* League::Skeleton::GetBone(StringView a_Name) const
{
	for (const auto& t_Bone : m_Bones)
		if (t_Bone.Name == a_Name)
			return &t_Bone;

	return nullptr;
}

File::LoadState League::Skeleton::ReadClassic(File& a_File, size_t& a_Offset)
{
	printf("We haven't tested classic skeletons yet!\n");
	throw 0; // Not yet tested
}

File::LoadState League::Skeleton::ReadVersion2(File& a_File, size_t& a_Offset)
{
	uint32_t t_Version;
	a_File.Get(t_Version, a_Offset);

	a_Offset += sizeof(uint16_t);

	uint16_t t_BoneCount;
	a_File.Get(t_BoneCount, a_Offset);

	uint32_t t_BoneIndexCount;
	a_File.Get(t_BoneIndexCount, a_Offset);

	uint16_t t_DataOffset;
	a_File.Get(t_DataOffset, a_Offset);

	a_Offset += sizeof(uint16_t);

	uint32_t t_BoneIndexMapOffset;
	a_File.Get(t_BoneIndexMapOffset, a_Offset);

	uint32_t t_BoneIndicesOffset;
	a_File.Get(t_BoneIndicesOffset, a_Offset);

	a_Offset += sizeof(uint32_t);
	a_Offset += sizeof(uint32_t);

	uint32_t t_BoneNamesOffset;
	a_File.Get(t_BoneNamesOffset, a_Offset);

	a_Offset = t_DataOffset;

	m_Bones.resize(t_BoneCount);
	for (int i = 0; i < t_BoneCount; ++i)
	{
		Bone& t_Bone = m_Bones[i];
		a_Offset += sizeof(uint16_t);

		a_File.Get(t_Bone.ID, a_Offset);
		if (t_Bone.ID != i)
		{
			printf("League Skeleton noticed an unexpected ID value for bone %d: %d\n", i, t_Bone.ID);
			return File::LoadState::FailedToLoad;
		}

		a_File.Get(t_Bone.ParentID, a_Offset);
		t_Bone.Parent = t_Bone.ParentID >= 0 ? &m_Bones[t_Bone.ParentID] : nullptr;

		a_Offset += sizeof(uint16_t);

		a_File.Get(t_Bone.Hash, a_Offset);

		a_Offset += sizeof(uint32_t);

		glm::vec3 t_Position;
		a_File.Get(t_Position.x, a_Offset);
		a_File.Get(t_Position.y, a_Offset);
		a_File.Get(t_Position.z, a_Offset);

		glm::vec3 t_Scale;
		a_File.Get(t_Scale.x, a_Offset);
		a_File.Get(t_Scale.y, a_Offset);
		a_File.Get(t_Scale.z, a_Offset);

		glm::quat t_Rotation;
		a_File.Get(t_Rotation.x, a_Offset);
		a_File.Get(t_Rotation.y, a_Offset);
		a_File.Get(t_Rotation.z, a_Offset);
		a_File.Get(t_Rotation.w, a_Offset);

		a_Offset += 44;

		t_Bone.LocalMatrix = glm::translate(t_Position) * glm::mat4_cast(t_Rotation) * glm::scale(t_Scale);

		if (t_Bone.ParentID != -1)
			m_Bones[t_Bone.ParentID].Children.push_back(&t_Bone);
	} 

	a_Offset = t_BoneIndexMapOffset;

	std::map<uint32_t, uint32_t> t_BoneIndexMap;
	for (int i = 0; i < t_BoneCount; i++)
	{
		uint32_t t_SkeletonID;
		a_File.Get(t_SkeletonID, a_Offset);

		uint32_t t_AnimationID;
		a_File.Get(t_AnimationID, a_Offset);

		t_BoneIndexMap[t_AnimationID] = t_SkeletonID;
	}

	a_Offset = t_BoneIndicesOffset;
	std::vector<uint16_t> t_BoneIndices(t_BoneIndexCount);
	for (int i = 0; i < t_BoneIndexCount; i++)
		a_File.Get(t_BoneIndices[i], a_Offset);
	
	a_Offset = t_BoneNamesOffset;

	// Get file part with bone names
	size_t t_NameChunkSize = 32 * t_BoneCount;
	std::vector<uint8_t> t_Start(t_NameChunkSize);
	memset(t_Start.data(), 0, t_NameChunkSize);
	a_File.Read(t_Start.data(), t_NameChunkSize, a_Offset);

	// Go through all the names and store them on our bones.
	char* t_Pointer = (char*)t_Start.data();
	for (int i = 0; i < t_BoneCount; ++i)
	{
		m_Bones[i].Name = t_Pointer;
		size_t t_NameLength = strlen(t_Pointer);
		std::string t_Name = t_Pointer;
		t_Pointer += t_NameLength;
		while (*t_Pointer == 0) t_Pointer++; // eat all \0s
	}

	// Fix the bone indices on the skin
	for (auto& t_IndexVector : m_Skin.m_BoneIndices)
		for (int i = 0; i < t_IndexVector.length(); i++)
			t_IndexVector[i] = t_BoneIndices[t_IndexVector[i]];

	return File::LoadState::Loaded;
}