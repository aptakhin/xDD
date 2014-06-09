/** xDDTools */
#pragma once

#include "xdd/common.hpp"
#include "xdd/filters.hpp"
#include "xdd/file.hpp"

namespace xdd {

class File;

class File_system
{
public:
	File_system();
	~File_system();

	QString full_path_of(const File& file) const;

	void full_path_of(const File& file, QString& full_path) const;
	void full_path_of(const File& file, wchar_t* full_path, size_t* len) const;

	File* add_file(const File& file);

	/// O(1) access-time
	File* file_with_id(File::ID id);
	/// O(1) access-time
	const File* file_with_id(File::ID id) const;

	static File_system* i();
	uint32 num_files() const;

	File* root();
	const File* root() const;

	void flush_and_ready_async();

protected:
	void full_path_of_impl(const File& file, QString& path) const;
	void full_path_of_impl(const File& file, wchar_t* full_path, size_t* len) const;

protected:
	static File_system* instance_;

	int init_bucket_size_;

	std::deque<File> files_;
};

}// namespace xdd
