/** xDDTools */
#include "xdd/file_system.hpp"
#include <QThread>

namespace xdd {

File_system* File_system::instance_ = 0;

File_system::File_system()
:	init_bucket_size_(1 << 10),
	files_(new Bucket_vector<File>(init_bucket_size_))
{
	XDD_ASSERT3(!File_system::instance_,
		"Singleton of File_system is already created!",
			return);
	File_system::instance_ = this;
}

File_system::~File_system()
{
	delete[] files_;

	XDD_ASSERT3(File_system::instance_ == this,
		"Problem while deleting File_system! Another singleton was created!",
			return);
	File_system::instance_ = 0;
}

File_system* File_system::i()
{
	XDD_ASSERT2(File_system::instance_,
		"Singleton of File_system wasn't created!");
	return File_system::instance_;
}

QString File_system::full_path_of(const File& file) const
{
	QString full_path;
	File_system::full_path_of_impl(file, full_path);
	return full_path;
}

void File_system::full_path_of(const File& file, QString& full_path) const
{
	full_path_of_impl(file, full_path);
}

void File_system::full_path_of(const File& file, wchar_t* full_path, size_t* len) const
{
	*len = 0;
	full_path_of_impl(file, full_path, len);
}

void File_system::full_path_of_impl(const File& file, wchar_t* full_path, size_t* len) const
{
	if (!file.is_root())
		full_path_of_impl((*files_)[file.parent_id()], full_path, len);
	size_t add_sz = file.name().size();
	if (*len != 0)
		full_path[*len] = L'\\', full_path[++(*len)] = '\0';
	memcpy(full_path + *len, (wchar_t*) file.name().utf16(), sizeof(wchar_t) * (*len + add_sz + 1));
	*len = *len + add_sz;
}

void File_system::full_path_of_impl(const File& file, QString& path) const
{
	if (!file.is_root())
		full_path_of_impl((*files_)[file.parent_id()], path);
	if (path.length() != 0)
		path += L'\\';
	path += file.name();
}

File* File_system::file_with_id(File::ID id)
{
	return &(*files_)[id];
}

const File* File_system::file_with_id(File::ID id) const 
{
	return &(*files_)[id];
}

File* File_system::add_file(const File& file)
{
	// TODO: ATOMIC! or don't use more than one thread:)
	uint32 files = files_->size();
	files_->push_back(file);
	files_->back()._set_id(files);
	return &files_->back();
}

uint32 File_system::num_files() const
{
	return files_->size();
}

File* File_system::root()
{
	return &(*files_)[0];
}

const File* File_system::root() const
{
	return &(*files_)[0];
}

void File_system::flush_and_ready_async()
{
	// Save old pointer to old buckets
	Bucket_vector<File>* _old_files = files_;

	// Create new buckets
	files_ = new Bucket_vector<File>(init_bucket_size_);

	class Flush_thread : public QThread
	{
	public:
		Bucket_vector<File>* _to_flush;
		Flush_thread(Bucket_vector<File>* to_flush) : _to_flush(to_flush) {}
		void run()
		{
			delete _to_flush;
		}
	};
	// Now ready to work with new buckets and delete async old buckets
	Flush_thread* flush_thread = new Flush_thread(_old_files);
	flush_thread->start();
}

}// namespace xdd
