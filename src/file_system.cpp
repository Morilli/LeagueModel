#include "file_system.hpp"

#include <map>

std::map<StringView, File*, StringViewCompare>* m_Files = nullptr;

File * FileSystem::GetFile(StringView a_FilePath)
{
	if (m_Files == nullptr)
	{
		m_Files = new std::map<StringView, File*, StringViewCompare>();
		std::atexit([]()
		{
			for (auto& i : *m_Files)
				delete i.second;

			m_Files->clear();
			m_Files = nullptr;
		});
	}

	auto* t_File = new File(a_FilePath);
	(*m_Files)[a_FilePath] = t_File;

	return t_File;
}

void FileSystem::CloseFile(File & a_File)
{
	auto t_FileName = a_File.GetName().Get();
	auto t_Index = m_Files->find(a_File.GetName());
	if (t_Index == m_Files->end()) return;

	delete t_Index->second;
	m_Files->erase(t_Index);
}
