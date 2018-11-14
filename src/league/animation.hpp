#pragma once

#include <file_system.hpp>
#include <string.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace League 
{
	class Skeleton;

	class Animation
	{
	public:
		using OnLoadFunction = void(*)(League::Animation& a_Animation, void* a_Argument);
		Animation(Skeleton& a_Skeleton);

		void Load(StringView a_FilePath, OnLoadFunction a_OnLoadFunction = nullptr, void* a_Argument = nullptr);
		
		struct Bone
		{
			template<typename T>
			struct Frame
			{
				Frame(float a_Time, T a_FrameData) :
					Time(a_Time), FrameData(a_FrameData)
				{
				}

				float Time;
				T FrameData;
			};
			using TranslationFrame = Frame<glm::vec3>;
			using RotationFrame = Frame<glm::quat>;
			using ScaleFrame = Frame<glm::vec3>;

			StringView Name;
			std::vector<TranslationFrame> Translation;
			std::vector<RotationFrame> Rotation;
			std::vector<ScaleFrame> Scale;
		};

		File::LoadState GetLoadState() const { return m_State; }
		const std::vector<Bone>& GetBones() const { return m_Bones; }

		const Animation::Bone* GetBone(StringView a_Name) const;
		float GetDuration() const { return m_Duration; }
	private:
		File::LoadState m_State = File::LoadState::NotLoaded;

		File::LoadState LoadVersion1(const std::map<uint32_t, StringView>& a_BoneNameHashes, File& a_File, size_t& a_Offset);

		float m_FPS, m_Duration;
		std::vector<Bone> m_Bones;
		Skeleton& m_Skeleton;
	};
}