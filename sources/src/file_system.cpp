/** xDDTools */
#include "xdd/File_system.hpp"
#include <QThread>

namespace xdd {

File_system* File_system::_instance = 0;

File_system::File_system()
:	_init_bucket_size(1 << 10),
	_files(new Bucket_vector<File>(_init_bucket_size))
{
	XDD_ASSERT3(!File_system::_instance,
		"Singleton of File_system is already created!",
			return);
	File_system::_instance = this;
}

File_system::~File_system()
{
	delete[] _files;

	XDD_ASSERT3(File_system::_instance == this,
		"Problem while deleting File_system! Another singleton was created!",
			return);
	File_system::_instance = 0;
}

File_system* File_system::i()
{
	XDD_ASSERT2(File_system::_instance,
		"Singleton of File_system wasn't created!");
	return File_system::_instance;
}

QString File_system::full_path_of(const File& file) const
{
	QString full_path;
	File_system::_full_path_of(file, full_path);
	return full_path;
}

void File_system::full_path_of(const File& file, QString& full_path) const
{
	_full_path_of(file, full_path);
}

void File_system::full_path_of(const File& file, wchar_t* full_path, size_t* len) const
{
	*len = 0;
	_full_path_of(file, full_path, len);
}

void File_system::_full_path_of(const File& file, wchar_t* full_path, size_t* len) const
{
	if (!file.is_root())
		_full_path_of((*_files)[file.parent_id()], full_path, len);
	size_t add_sz = file.name().size();
	if (*len != 0)
		full_path[*len] = L'\\', full_path[++(*len)] = '\0';
	memcpy(full_path + *len, file.name().toStdWString().c_str(), sizeof(wchar_t) * (*len + add_sz + 1));
	*len = *len + add_sz;
}

void File_system::_full_path_of(const File& file, QString& path) const
{
	if (!file.is_root())
		_full_path_of((*_files)[file.parent_id()], path);
	if (path.length() != 0)
		path += L'\\';
	path += file.name();
}

File* File_system::file_with_id(File::ID id)
{
	return &(*_files)[id];
}

const File* File_system::file_with_id(File::ID id) const 
{
	return &(*_files)[id];
}

File* File_system::add_file(const File& file)
{
	// TODO: ATOMIC! or don't use more than one thread:)
	uint32 files = _files->size();
	_files->push_back(file);
	_files->back()._set_id(files);
	return &_files->back();
}

uint32 File_system::num_files() const
{
	return _files->size();
}

File* File_system::root()
{
	return &(*_files)[0];
}

const File* File_system::root() const
{
	return &(*_files)[0];
}

void File_system::flush_and_ready_async()
{
	// Save old pointer to old buckets
	Bucket_vector<File>* _old_files = _files;

	// Create new buckets
	_files = new Bucket_vector<File>(_init_bucket_size);

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
