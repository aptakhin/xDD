/** xDDTools */
#include "xdd/Scanner.hpp"

#ifdef XDD_UNIVERSAL_SCANNER
#include <QDir>
#endif

namespace xdd {

void Scanner::add_fast_filter(Filter* filter)
{
    _fast_filters.push_back(filter);
}

void Scanner::start(const String& path)
{
	_path_len = 0;
	// add root for file system - it's out start folder
    File* root = File_system::i()->add_file(File((File::ID)-1, path, File::T_DIRECTORY));

#ifdef XDD_WIN32_SCANNER
	wchar_t root_path[MAX_PATH];
#	ifdef _MSC_VER
    wcscpy_s(root_path, path.c_str());
#	else
	wcscpy(root_path, path.c_str());
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
Before you've said "WTF code?!" I try to to say that it's one of the most 
useful method in whole program.
It must be fast:).
*/
uint64 Scanner::_start(wchar_t* path, File* file, int depth)
{
    uint64 full_size = 0;
    WIN32_FIND_DATA file_data;

	path[_path_len++] = L'*', path[_path_len] = 0;// Cat `*` for query

	HANDLE file_handle = FindFirstFile(path, &file_data);

    XDD_ASSERT3(file_handle != INVALID_HANDLE_VALUE,
        L"Invalid file handle `" << path << "`!",
			path[--_path_len] = 0; 
			return full_size);

    // Query done. Erase last `*` from path
    path[--_path_len] = 0;
	size_t restore_to_len = _path_len;// Save current length to restore fast

    do
    {
        File::Type type = File::T_FILE;
		File* goto_file = nullptr;

        uint64 size = 0;

		if (file_data.cFileName[0] == '.' && 
		   (file_data.cFileName[1] == 0 || file_data.cFileName[1] == '.' && file_data.cFileName[2] == 0))
			continue;

		// back to original filename
		path[_path_len = restore_to_len] = 0;

        if (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            type = File::T_DIRECTORY;
        else
        {
            size = helper::quad_part(file_data.nFileSizeLow, file_data.nFileSizeHigh);
            full_size += size;// update size and full folder size
        }

		// cat new filename
		size_t file_name_sz = wcslen(file_data.cFileName);
		memcpy(path + _path_len, file_data.cFileName, sizeof(wchar_t) * (file_name_sz + 1));
		_path_len += file_name_sz;

		if (type == File::T_DIRECTORY)
			path[_path_len++] = L'\\', path[_path_len] = 0;

        bool add2fs = false;

        if (file != nullptr)
        {
			// Fast filter's work
			const String* reason = nullptr;
                
            for (Filters::iterator i = _fast_filters.begin(); i != _fast_filters.end(); ++i)
            {
				reason = (*i)->look(file_data);
				if (reason != nullptr)
                {
                    add2fs = true;
                    break;
                }
            }

            if (add2fs)
                add2fs = does_look_at(file_data, depth + 1);

            if (add2fs)// Intresting yet?
            {
                File* cur_file = File_system::i()->add_file(
                    File(file->Id(), file_data.cFileName, file_name_sz, type));

				// Add file to fs. If successful, set size and go on
				if (cur_file != nullptr && file->add_child(cur_file->Id()))
				{
					goto_file = cur_file, cur_file->_set_size(size);
					cur_file->mark_for_delete(reason);// Child added. Can set reason
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
			// go to subfolder
            uint64 set_size = _start(path, goto_file, depth + 1);
            if (goto_file != nullptr)
                goto_file->_set_size(set_size);
            full_size += set_size;
        }
    }
    while (FindNextFile(file_handle, &file_data) != 0);

    XDD_ASSERT3(GetLastError() == ERROR_NO_MORE_FILES,
        L"Error happened while reading `" << path << "`!",
           path[restore_to_len] = 0; _path_len = restore_to_len; return full_size);

    FindClose(file_handle);

	path[restore_to_len] = 0, _path_len = restore_to_len;// Restore
    return full_size;
}
// WTF?!!
#endif//#ifdef XDD_WIN32_SCANNER

#ifdef XDD_UNIVERSAL_SCANNER
uint64 Scanner::_start(File* file, const QDir& cur_dir)
{
	uint64 full_size = 0;

    QFileInfoList list = cur_dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (int child_i = 0; child_i < list.size(); ++child_i) 
	{
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
			add2fs = does_look_at(file_data, 0);

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

bool Scanner::does_look_at(const file_data&, int)
{
    return true;
}

}// namespace xdd
