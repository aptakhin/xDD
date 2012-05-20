/** xDDTools */
#include "xdd/scanner.hpp"

#ifdef XDD_UNIVERSAL_SCANNER
#	include <QDir>
#endif

namespace xdd {

void Scanner::add_fast_filter(Filter* filter)
{
	_fast_filters.push_back(filter);
}

void Scanner::start(const QString& path)
{
	_path_len = 0;
	_all_looked_size = 0;
	_soft_stop = false;
	// Add root for file system - it's out start folder
	File* root = File_system::i()->add_file(File((File::ID)-1, path, File::T_DIRECTORY));

#ifdef XDD_WIN32_SCANNER
	wchar_t root_path[MAX_PATH];

	std::wstring wpath = path.toStdWString();

#	ifdef _MSC_VER
	wcscpy_s(root_path, wpath.c_str());
#	else
	wcscpy(root_path, wpath.c_str());
#	endif//#ifdef _MSC_VER
	_path_len = wcslen(root_path);
	uint64 sz = _start(root_path, root, 0);
	root->_set_size(sz);
#endif//#ifdef XDD_WIN32_SCANNER

#ifdef XDD_UNIVERSAL_SCANNER
	_start(root, QDir(QString::fromStdWString(path)));
#endif//#ifdef XDD_WIN32_SCANNER
}


#ifdef XDD_WIN32_SCANNER
/*
Before you've said "WTF code?!" I try to say that it's one of the most 
important method in whole program.
It must be fast:).
*/
uint64 Scanner::_start(wchar_t* path, File* file, int depth)
{
	uint64 full_size = 0;
	WIN32_FIND_DATA file_data;

	path[_path_len++] = L'*', path[_path_len] = 0;// Cat `*` for query

	HANDLE file_handle = FindFirstFile(path, &file_data);

	// Query done. Erase last `*` from path
	path[--_path_len] = 0;

	XDD_ASSERT3(file_handle != INVALID_HANDLE_VALUE,
		"Invalid file handle `" << QString::fromStdWString(path) << "`!",
			return 0);
	
	size_t restore_to_len = _path_len; // Save current length to restore later fast

	do
	{
		if (_soft_stop)
			break;

		File::Type type = File::T_FILE;
		File* goto_file = nullptr;

		uint64 size = 0;

		if (file_data.cFileName[0] == '.' && // Omit `.` and `..`
		   (file_data.cFileName[1] == 0 || file_data.cFileName[1] == '.' && file_data.cFileName[2] == 0))
			continue;

		// Back to original filename
		path[_path_len = restore_to_len] = 0;

		if (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			type = File::T_DIRECTORY;
		else
		{
			size = helper::quad_part(file_data.nFileSizeLow, file_data.nFileSizeHigh);
			full_size += size;// Update size and full folder size
			_all_looked_size += size;
		}

		// Cat new filename
		size_t file_name_sz = wcslen(file_data.cFileName);
		memcpy(path + _path_len, file_data.cFileName, sizeof(wchar_t) * (file_name_sz + 1));
		_path_len += file_name_sz;

		if (type == File::T_DIRECTORY)
			path[_path_len++] = L'\\', path[_path_len] = 0;

		bool add2fs = false;

		if (file != nullptr)
		{
			// Fast filter's work
			const QString* reason = nullptr;
				
			for (Filters::iterator i = _fast_filters.begin(); i != _fast_filters.end(); ++i)
			{
				reason = (*i)->look(file_data);
				if (reason != nullptr)
				{
					add2fs = true;
					break;
				}
			}

			if (add2fs)// Intresting yet?
			{
				File* cur_file = File_system::i()->add_file(
					File(file->Id(), file_data.cFileName, file_name_sz, type));

				// Add file to fs. If successful, set size and go on
				if (cur_file != nullptr && file->add_child(cur_file->Id()))
				{
					goto_file = cur_file, cur_file->_set_size(size);

					if (reason != nullptr && *reason != EMPTY_STR)// Child added. Should set reason if has some
						cur_file->mark_for_delete(reason);
				}
				else
					goto_file = nullptr;					
			}
		}

		if (!add2fs)
		{
			goto_file = nullptr;
			// We calculate size of this directory, but don't add file
			// to file tree
		}

		if (type == File::T_DIRECTORY)
		{
			// Go to subfolder
			uint64 set_size = _start(path, goto_file, depth + 1);

			if (goto_file != nullptr)
				goto_file->_set_size(set_size);
			full_size += set_size;
		}
	}
	while (FindNextFile(file_handle, &file_data) != 0);

	XDD_ASSERT3(GetLastError() == ERROR_NO_MORE_FILES,
		"Error happened while reading `" << QString::fromStdWString(path) << "`!",
		   path[_path_len = restore_to_len] = 0; return full_size);

	FindClose(file_handle);

	path[_path_len = restore_to_len] = 0;// Restore
	return full_size;
}
// — WTF?!
// - xDD
#endif//#ifdef XDD_WIN32_SCANNER

#ifdef XDD_UNIVERSAL_SCANNER
uint64 Scanner::_start(File* file, const QDir& cur_dir)
{
	uint64 full_size = 0;

	QFileInfoList list = cur_dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
	for (int child_i = 0; child_i < list.size(); ++child_i) 
	{
		if (_soft_stop)
			break;

		File* goto_file = nullptr;
		QFileInfo file_data = list.at(child_i);
		File::Type type = File::T_FILE;

		bool add2fs = false;

		uint64 size = file_data.size();
		full_size += size;

		for (Filters::iterator i = _fast_filters.begin(); i != _fast_filters.end(); ++i)
		{
			if ((*i)->look(file_data) == Filter::S_MARK)
			{
				add2fs = true;
				break;
			}
		}

		if (file_data.isDir())
			type = File::T_DIRECTORY;

		if (add2fs)
		{
			File* cur_file = File_system::i()->add_file(
				File(file->Id(), file_data.filename().toStdWString(), type));

			if (cur_file != nullptr && file->add_child(cur_file->Id()))
				goto_file = cur_file, cur_file->_set_size(size);
			else
				goto_file = nullptr;
		}

		if (file_data.isDir())
		{
			uint64 set_size = _start(goto_file, QDir(file_data.absoluteFilePath()));
			if (goto_file != nullptr)
				goto_file->_set_size(set_size);
			full_size += set_size;
		}
	}
	return full_size;
}
#endif

}// namespace xdd
