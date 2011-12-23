
#include "xdd/manager.hpp"
#include <QFileInfo>
#include <QFileIconProvider>

namespace xdd {

Scan_manager* Scan_manager::_instance = nullptr;

Scan_manager::Scan_manager()
:	_fs(nullptr),
	_thread(this),
	_ready(false),
	_time_exec(0)
{
	XDD_ASSERT3(!Scan_manager::_instance,
        L"Singleton of Scan_manager is already created!",
            return);

    Scan_manager::_instance = this;
	Logger::i().wl();
	XDD_LOG("Scan manager initalized");
}

Scan_manager::~Scan_manager()
{
    XDD_ASSERT3(Scan_manager::_instance == this,
        L"Problem while deleting Scan_manager! Another singleton was created!",
            return);
	XDD_LOG("Scan manager destroyed");
	Logger::i().wl();
    Scan_manager::_instance = 0;
}

Scan_manager* Scan_manager::i() 
{
	XDD_ASSERT3(Scan_manager::_instance,
        L"Singleton of Scan_manager wasn't yet created!",
            return nullptr);
	return _instance; 
}

void Scan_manager::start_scan_thread(const Scan_files_param& param)
{
	XDD_LOG("Start scanning thread");
	_thread.set_params(param);
	_thread.start();
}

bool Scan_manager::is_scan_finished() const
{
	return _ready;
}

void Scan_manager::prepare_for_scan()
{
	if (!_fs)
	{
		_fs = new File_system;
	}
	else
		_fs->flush_and_ready_async();
		
	_ready = false;

	
}

void Scan_manager::scan(const Scan_files_param& param)
{
	_ready = false;
	
	String start = param.start_path;

	if (*(--start.end()) != L'\\')// add required last slash
		start += L'\\';

	_ready = false;
	XDD_LOG("Scanner started at: " << start);

    DWORD start_time = GetTickCount();
    _scanner.start(start);
    DWORD end_time = GetTickCount();
    _time_exec = uint64(end_time - start_time);
	XDD_LOG("Scanner stopped in " << helper::format_time_ms(_time_exec));

	File* root = _fs->root();
	form_files(root);
	
	_stat.update(_fs);
	
	_ready = true;
	emit scan_finished();
}

const File_system* Scan_manager::fs() const
{
	XDD_ASSERT2(_fs, L"File system wasn't yet initialized!");
	return _fs;
}

void Scan_manager::update_timer()
{
	emit update_info();
}

void Scan_manager::flush()
{
	_ready = false;
	_fs->flush_and_ready_async();
}

void Scan_manager::form_files(File* file)
{
    file->sort_size_desc();
    for (size_t i = 0; i < file->num_children(); ++i)
		form_files(file->i_child(i));
}

void Scan_manager::Call_scan::run()
{
	if (_mgr != nullptr)
		_mgr->scan(_params);
}

void Clean_manager::make_clean(Action action)
{
	XDD_LOG("Start cleaning");
	make_clean_rec(fs()->root(), action);
}

void Clean_manager::make_clean_rec(File* file, Action action)
{
	//
	// TODO: Admin rights

	if (file->is_directory())
	{
		size_t sz = file->num_children();
		for (size_t i = 0; i < sz; ++i)
			make_clean_rec(file->i_child(i), action);
		// TODO: Check if there is no more file files in directory. Remove/move it too
	}
	else if (file->For_delete())
	{
		if (action == A_MOVE_TO_RECYCLE_BIN)
			move_file_to_recycle_bin(file);
		else if (action == A_REMOVE)
			remove_file(file);
	}
}

void Clean_manager::move_file_to_recycle_bin(File* file)
{
#ifdef XDD_WIN32_CODE
	WCHAR path[MAX_PATH + 1] = L"";
	size_t len = 0;
	fs()->full_path_of(*file, path, &len);
	path[++len] = L'\0';// Path have to be double-zeroed \0\0. Oh, yep.

	SHFILEOPSTRUCTW fileop;
    fileop.hwnd = 0;
    fileop.wFunc = FO_DELETE;
    fileop.pFrom = path;
    fileop.pTo = NULL;
    fileop.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
    fileop.fAnyOperationsAborted = 0;
    fileop.hNameMappings = 0;
    fileop.lpszProgressTitle = NULL;
    SHFileOperationW(&fileop);
#else
	not_implemented("Can't delete files on this platform!");
#endif//#ifdef XDD_WIN32_CODE
}
void Clean_manager::remove_file(File* file)
{
#ifdef XDD_WIN32_CODE
	String tmp;
	fs()->full_path_of(*file, tmp);
	const wchar_t* path = tmp.c_str();
	DeleteFile(path);
#else
	not_implemented("Can't move to recycle bin on this platform!");
#endif//#ifdef XDD_WIN32_CODE
}

void Clean_manager::remove_directory(File* file)
{
#ifdef XDD_WIN32_CODE
	String tmp;
	fs()->full_path_of(*file, tmp);
	const wchar_t* path = tmp.c_str();
	RemoveDirectory(path);
#else
	not_implemented("Can't move to recycle bin!");
#endif//#ifdef XDD_WIN32_CODE
}

File_system* Clean_manager::fs()
{
	return const_cast<File_system*>(Scan_manager::i()->fs());
}

File_system_stat::File_system_stat()
:	_full_disk_size(0),
	_free_disk_size(0)
{
}

void File_system_stat::update(const File_system* fs)
{
	ULARGE_INTEGER free_bytes_available, total_number_of_bytes, total_number_of_free_bytes;

	GetDiskFreeSpaceEx(fs->root()->name().c_str(),
		&free_bytes_available, &total_number_of_bytes, &total_number_of_free_bytes);

	_full_disk_size = total_number_of_bytes.QuadPart;
	_free_disk_size =  free_bytes_available.QuadPart;
}

File_histogram_manager::File_histogram_manager()
:	_hist(0, MEGABYTE, KILOBYTE/4)
{
}

void File_histogram_manager::go()
{
	go(fs()->root());
}

void File_histogram_manager::go(const File* file)
{
	if (file->is_directory())
	{
		size_t sz = file->num_children();
		for (size_t i = 0; i < sz; ++i)
			go(file->i_child(i));
	}
	else 
		_hist.add(file->size());
}

const File_histogram_manager::File_hist& File_histogram_manager::histogram() const
{
	return _hist;
}

const File_system* File_histogram_manager::fs()
{
	return Scan_manager::i()->fs();
}


} // namespace xdd