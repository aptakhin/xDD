
#include "xdd/manager.hpp"
#include "xdd/file.hpp"
#include <QFileInfo>
#include <QFileIconProvider>

namespace xdd {

Scan_manager* Scan_manager::instance_ = nullptr;

Scan_manager::Scan_manager()
:	fs_(nullptr),
	thread_(this),
	ready_(false),
	time_exec_(0),
	start_time_(0),
	soft_stop_(false)
{
	XDD_ASSERT3(!Scan_manager::instance_,
		"Singleton of Scan_manager is already created!",
			return);

	Scan_manager::instance_ = this;
	Logger::i().wl();
	XDD_LOG("Scan manager initalized");
}

Scan_manager::~Scan_manager()
{
	XDD_ASSERT3(Scan_manager::instance_ == this,
		"Problem while deleting Scan_manager! Another singleton was created!",
			return);
	XDD_LOG("Scan manager destroyed");
	Logger::i().wl();
	Scan_manager::instance_ = 0;
}

Scan_manager* Scan_manager::i() 
{
	XDD_ASSERT3(Scan_manager::instance_,
		"Singleton of Scan_manager wasn't yet created!",
			return nullptr);
	return instance_; 
}

void Scan_manager::start_scan_thread(const Scan_files_param& param)
{
	XDD_LOG("Start scanning thread");
	thread_.set_params(param);
	thread_.start();
}

void Scan_manager::stop_scan_thread()
{
	soft_stop_ = true;
	scanner_.sig_soft_stop();
	fs_->flush_and_ready_async();
}

bool Scan_manager::is_scan_finished() const
{
	return ready_;
}

void Scan_manager::prepare_for_scan()
{
	if (!fs_)
		fs_ = new File_system;
	
	fs_->flush_and_ready_async();
	ready_ = false;
}

void Scan_manager::scan(const Scan_files_param& param)
{
	soft_stop_ = false;
	ready_ = false;
	
	QString start = param.start_path;

	if (!start.endsWith('\\'))// add required last slash
		start += '\\';

	ready_ = false;
	stat_.update(start);
	XDD_LOG("Scanner started at: " << start);
	start_time_ = helper::get_ms_time();
	scanner_.start(start);
	if (!soft_stop_)
	{
		uint64 end_time = helper::get_ms_time();
		time_exec_ = end_time - start_time_;
		start_time_ = 0;
		XDD_LOG("Scanner stopped in " << helper::format_time_ms(time_exec_));

		File* root = fs_->root();
		prepare_files(root);
	
		ready_ = true;
		emit scan_finished();
	}
}

const File_system* Scan_manager::fs() const
{
	XDD_ASSERT2(fs_, "File system wasn't yet initialized!");
	return fs_;
}

void Scan_manager::update_timer()
{
	emit update_info();
}

void Scan_manager::flush()
{
	ready_ = false;
	fs_->flush_and_ready_async();
}

void Scan_manager::prepare_files(File* file)
{
	file->sort_size_desc();
	for (size_t i = 0; i < file->num_children(); ++i)
		prepare_files(file->i_child(i));
}

void Scan_manager::Call_scan::run()
{
	if (mgr_ != nullptr)
		mgr_->scan(params_);
}

uint64 Scan_manager::approx_scan_time_left() const
{
	if (start_time_ == 0)
		return 0;

	uint64 time_done = helper::get_ms_time() - start_time_;

	if (time_done < 1000)
		return 0;

	uint64 looked_size = scanner_.get_current_looked_size();
	uint64 speed = looked_size / (time_done / 1000);

	std::cout << helper::format_size(looked_size).toStdString() << "/" << helper::format_size(stat_.full_disk_size()).toStdString() << " " << helper::format_size(speed) << std::endl;
	uint64 t = (stat_.full_disk_size() - looked_size) / speed * SECOND_MS;
	return t;
}


void Clean_manager::make_clean(Action action)
{
	XDD_LOG("Cleaning");
	make_clean_rec(File_system::i()->root(), action);
}

void Clean_manager::make_clean_rec(const File* file, Action action)
{
	children_each_rec(file, [action] (const File* file) {
		if (!file->is_directory() && file->for_delete())
		{
			if (action == Clean_manager::MOVE_TO_RECYCLE_BIN)
				move_file_to_recycle_bin(file);
			else if (action == Clean_manager::REMOVE)
				remove_file(file);

			// TODO: Check if there is no more file files in directory. Remove/move it too
		}
	});
}

void Clean_manager::move_file_to_recycle_bin(const File* file)
{
#ifdef XDD_WIN32_CODE
	wchar_t path[MAX_PATH + 1] = L"";
	size_t len = 0;
	File_system::i()->full_path_of(*file, path, &len);
	path[len + 1] = L'\0';// Path have to be double-zeroed \0\0. Oh, yep.

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
	not_implemented("Can't move to recycle bin on this platform!");
#endif//#ifdef XDD_WIN32_CODE
}
void Clean_manager::remove_file(const File* file)
{
#ifdef XDD_WIN32_CODE
	QString tmp;
	File_system::i()->full_path_of(*file, tmp);
	const wchar_t* path = (wchar_t*) tmp.utf16();
	DeleteFile(path);
#else
	not_implemented("Can't delete files on this platform!");
#endif//#ifdef XDD_WIN32_CODE
}

void Clean_manager::remove_directory(const File* file)
{
#ifdef XDD_WIN32_CODE
	QString tmp;
	File_system::i()->full_path_of(*file, tmp);
	const wchar_t* path = (const wchar_t*) tmp.utf16();
	RemoveDirectory(path);
#else
	not_implemented("Can't move to recycle bin!");
#endif//#ifdef XDD_WIN32_CODE
}

File_system_stat::File_system_stat()
:	full_disk_size_(0),
	free_disk_size_(0)
{
}

void File_system_stat::update(const QString& path)
{
	ULARGE_INTEGER free_bytes_available, total_number_of_bytes, total_number_of_free_bytes;

	GetDiskFreeSpaceEx((const wchar_t*) path.utf16(),
		&free_bytes_available, &total_number_of_bytes, &total_number_of_free_bytes);

	full_disk_size_ = total_number_of_bytes.QuadPart;
	free_disk_size_ =  free_bytes_available.QuadPart;
}

File_histogram_manager::File_histogram_manager()
:	hist_(0, MEGABYTE, KILOBYTE/4)
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
		hist_.add(file->size());
}

const File_histogram_manager::File_hist& File_histogram_manager::histogram() const
{
	return hist_;
}

const File_system* File_histogram_manager::fs()
{
	return Scan_manager::i()->fs();
}

} // namespace xdd