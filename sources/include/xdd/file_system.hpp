/** xDDTools */
#pragma once

#include "xdd/common.hpp"
#include "xdd/filters.hpp"
#include "xdd/file.hpp"

namespace xdd {

class File;

/** Vector with fixed size buckets, that doesn't do wasteless copies because it doesn't need these copies. */
template <typename T>
class Bucket_vector
{
protected:
	struct Bucket
	{
		T* base;
		uint32 size;

		Bucket() : base(nullptr), size(0) {}
		Bucket(const Bucket& cpy) : base(cpy.base), size(cpy.size) {}
	};

public:

	Bucket_vector(uint32 bucket_size)
	:   buckets_(), 
	 	bucket_size_(bucket_size),
		last_bucket_(nullptr)
	{
		extend();// add first bucket at creation
	}

	~Bucket_vector()
	{
		clear();
	}

	void push_back(const T& obj)
	{
		if (last_bucket_->size == bucket_size_)
			extend();// No place. add another one bucket.

		last_bucket_->base[last_bucket_->size++] = obj;
	}

	T& operator [] (uint64 i)
	{
		uint bucket = uint(i / bucket_size_);
		uint local  = uint(i - bucket * bucket_size_);
		return buckets_[bucket].base[local];
	}

	const T& operator [] (uint64 i) const
	{
		uint64 bucket = uint(i / bucket_size_);
		uint64 local  = uint(i - bucket * bucket_size_);
		return buckets_[bucket].base[local];
	}

	T& back()
	{
		return last_bucket_->base[last_bucket_->size - 1];
	}

	uint32 size() const
	{
		// So easy, because there are no deleting interface, yep.
		return (buckets_.size() - 1) * bucket_size_ + last_bucket_->size;
	}

	void clear()
	{
		XDD_LOG("File system started to clean up");
		uint64 start_time = helper::get_ms_time();

		Buckets::iterator i = buckets_.begin();
		while (i != buckets_.end()) // Delete everything
		{
			delete[] i->base;
			i->base = nullptr, i->size = 0;
			i = buckets_.erase(i);
		}
		// Everything was removed. It's now time to add first bucket.
		extend();

		uint64 end_time = helper::get_ms_time();
		XDD_LOG("File system cleaned in " << helper::format_time_ms(uint32(end_time - start_time)));
	}

protected:

	void extend()
	{
		Bucket bucket;
		bucket.base = new T[bucket_size_];
		bucket.size = 0;
		buckets_.push_back(bucket);
		last_bucket_ = &buckets_.back();
	}

protected:

	typedef std::vector<Bucket> Buckets;
	Buckets buckets_;

	/// size of each bucket vector
	const uint32 bucket_size_;

	Bucket* last_bucket_;
};

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

	Bucket_vector<File>* files_;
};

}// namespace xdd
