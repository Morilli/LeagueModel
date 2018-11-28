#pragma once
#include <string>
#include <file_system.hpp>

#include <glm/glm.hpp>
#include <map>

namespace League
{
	class Bin
	{
	public:
		class ValueStorage;

		using OnLoadFunction = void(*)(League::Bin& a_Bin, void* a_Argument);
		using FindConditionFunction = bool(*)(const ValueStorage& a_Value, void* a_UserData);

		void Load(std::string a_FilePath, OnLoadFunction a_OnLoadFunction = nullptr, void* a_Argument = nullptr);
		std::string GetAsJSON() const;

		File::LoadState GetLoadState() const { return m_State; }

		class ValueStorage
		{
		public:
			enum Type : uint8_t
			{
				U16Vec3 = 0,
				Bool = 1,
				S8 = 2,
				U8 = 3,
				S16 = 4,
				U16 = 5,
				S32 = 6,
				U32 = 7,
				S64 = 8,
				U64 = 9,
				Float = 10,
				FVec2 = 11,
				FVec3 = 12,
				FVec4 = 13,
				Mat4 = 14,
				RGBA = 15,
				StringT = 16,
				Hash = 17,
				Container = 18,
				Struct = 19,
				Embedded = 20,
				Link = 21,
				Array = 22,
				Map = 23,
				Padding = 24,
			};

			template<typename T> T Read() const { return *(T*)m_Data; }

			void DebugPrint();
			std::string GetAsJSON(bool a_ForceToString, bool a_ExposeHash) const;

			std::vector<const ValueStorage*> Find(FindConditionFunction a_Function, void* a_UserData) const;

			bool Is(const std::string& a_Name) const;
			uint32_t GetHash() const { return m_Hash; }
			Type GetType() const { return m_Type; }
			const uint8_t* GetData() const { return m_Data; }
			const void* GetPointer() const { return m_Pointer; }

			const ValueStorage* Get(std::string a_Name) const;

			friend class League::Bin;
		protected:
			void FetchDataFromFile(File* a_File, size_t& a_Offset);

			Type m_Type;
			uint8_t m_Data[256] = { 0 };
			void* m_Pointer = nullptr;
			uint32_t m_Hash = 0;
		};

		std::vector<const ValueStorage*> Find(FindConditionFunction a_Function, void* a_UserData = nullptr) const;
		const ValueStorage* Get(std::string a_Name) const;

		friend class League::Bin::ValueStorage;
	protected:
		static void GetStorageData(File* a_File, League::Bin::ValueStorage& t_Storage, size_t& t_Offset);

	private:
		static size_t GetSizeByType(League::Bin::ValueStorage::Type a_Type);

		File::LoadState m_State = File::LoadState::NotLoaded;

		std::map<uint32_t, std::vector<ValueStorage>> m_Values;
		std::vector<std::string> m_LinkedFiles;
	};
}