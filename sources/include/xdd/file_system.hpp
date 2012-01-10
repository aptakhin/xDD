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
	:   _buckets(), 
	 	_bucket_size(bucket_size),
		_last_bucket(nullptr)
	{
		extend();// add first bucket at creation
	}

	~Bucket_vector()
	{
		clear();
	}

	void push_back(const T& obj)
	{
		if (_last_bucket->size == _bucket_size)
			extend();// No place. add another one bucket.

		_last_bucket->base[_last_bucket->size++] = obj;
	}

	T& operator [] (uint64 i)
	{
		uint bucket = uint(i / _bucket_size);
		uint local  = uint(i - bucket * _bucket_size);
		return _buckets[bucket].base[local];
	}

	const T& operator [] (uint64 i) const
	{
		uint64 bucket = uint(i / _bucket_size);
		uint64 local  = uint(i - bucket * _bucket_size);
		return _buckets[bucket].base[local];
	}

	T& back()
	{
		return _last_bucket->base[_last_bucket->size - 1];
	}

	uint32 size() const
	{
		// So easy, because there are no deleting interface, yep.
		return (_buckets.size() - 1) * _bucket_size + _last_bucket->size;
	}

	void clear()
	{
		XDD_LOG("File system started to clean up");
		uint64 start_time = helper::get_ms_time();

		Buckets::iterator i = _buckets.begin();
		while (i != _buckets.end()) // Delete everything
		{
			delete[] i->base;
			i->base = nullptr, i->size = 0;
			i = _buckets.erase(i);
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
		bucket.base = new T[_bucket_size];
		bucket.size = 0;
		_buckets.push_back(bucket);
		_last_bucket = &_buckets.back();
	}

protected:

	typedef std::vector<Bucket> Buckets;
	Buckets _buckets;

	/// size of each bucket vector
	const uint32 _bucket_size;

	Bucket* _last_bucket;
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
	void _full_path_of(const File& file, QString& path) const;
	void _full_path_of(const File& file, wchar_t* full_path, size_t* len) const;

protected:
	static File_system* _instance;

	int _init_bucket_size;

	Bucket_vector<File>* _files;
};

}// namespace xdd
