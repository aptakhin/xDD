/** xDDTools */
#pragma once

#include "xdd/common.hpp"
#include "xdd/filters.hpp"
#include <QIcon>

namespace xdd {

const String EMPTY_STR = L"";
const String USER_WANTS_STR = L"User selected";

class File
{

public:
	/// Unique global file id
    typedef uint32 ID;

    enum Type
    {
        T_FILE,
        T_DIRECTORY
    };

	enum Field
	{
		F_NAME,
		F_SIZE,
		F_DELETE_REASON
	};

	typedef std::vector<File::ID> Files;

public:
    File();
    File(const File& cpy);
    File(File::ID parent, const String& name, Type type);
	File(File::ID parent, const wchar_t* name, size_t len, Type type);
	~File();

    void operator = (const File& cpy);

	const String& name() const { return _name; }
	File::ID Id() const { return _id; }

    bool is_root() const { return _parent == (ID)-1; }
    bool is_directory() const { return _type == T_DIRECTORY; }

    uint64 size() const { return _size; }
    void _set_size(uint64 size) { _size = size; }

    File::ID parent_id() const { return _parent; }
	File* parent();
	const File* parent() const;

    bool add_child(const File::ID& file);

	void sort_size_desc();

	bool has_children() const { return !_children.empty(); }
	size_t num_children() const { return _children.size(); }

	const Files& children() const { return _children; }

	File::ID i_child_id(size_t i) const { return _children[i]; }
	File* i_child(size_t i);
	const File* i_child(size_t i) const;

	size_t number_of(File::ID id) const;
	bool has_child(File::ID id) const;

	/** Marks to delete all child files with reason. 
		empty string means not to delete. Other values show the reason of deleting.
		Remember that File doesn't copy string to object, it just saves pointer to string!
		Don't push there nullptr pointer! Valid pointer object must be pushed. */
	void mark_for_delete(const String* reason);

	const String& delete_reason() const { return *_reason_delete; }

	bool for_delete() const;

	const Files& files_to_delete() const { return _to_delete; }

	size_t num_files_to_delete() const;
	File* i_file_to_delete(size_t i) const;
	size_t number_of_deleted(const File* file) const;
	bool has_to_delete(const File* file) const;
	bool has_files_to_delete() const;

	/// Called by primary child (not child of child)
	void child_marked_for_delete(const File* child);

	void sort_marked_for_delete(Field field, Sort_order order);

	void _add_child_to_delete_list(const File* child);
	void _remove_child_from_delete_list(const File* child);

	void _remove_all_children_from_delete_list();

	bool _has_cached_icon() const;
	void _set_cached_icon(const QIcon& icon);
	const QIcon& _cached_icon() const;

	bool has_for_delete_cache() const;
	bool update_has_for_delete_cache_rec() const;

	template <typename Acc, typename T>
	Acc deleted_children_accum(Acc accumulate, T fun) const
	{
		for (Files::const_iterator i = _to_delete.begin(); i != _to_delete.end(); ++i)
			accumulate(fun(File_system::i()->file_with_id(*i)));
		return accumulate;
	}

	template <typename T>
	bool deleted_children_if_any(T fun) const
	{
		bool result = false;
		for (Files::const_iterator i = _to_delete.begin(); i != _to_delete.end(); ++i)
		{
			if (fun(File_system::i()->file_with_id(*i)))
				result = true;
		}
		return result;
	}

	template <typename T>
	bool children_if_any(T fun) const
	{
		bool result = false;
		for (Files::const_iterator i = _children.begin(); i != _children.end(); ++i)
		{
			if (fun(File_system::i()->file_with_id(*i)))
				result = true;
		}
		return result;
	}

protected:
	friend class File_system;

	/// Only parent can call this
	void _parent_marks_for_delete(const String& reason);

    void _set_id(File::ID id) { _id = id; }

	Files::iterator child_to_delete_iter_by_id(File::ID id);
	
protected:
    File::ID _parent;
    File::ID _id;
    String _name;
    Type _type;

    uint64 _size;

    Files _children;

	const String* _reason_delete;

	mutable QIcon _icon_cache;

	Files _to_delete;

	mutable bool _has_for_delete_cache;
};

template <typename F>
void files_each(const File::Files& files, const F& functor)
{
	fun::each(files, [&functor] (File::ID id) {
		functor(File_system::i()->file_with_id(id));
	});
}

template <typename F>
void children_each_rec(const File* file, const F& functor)
{
	files_each(file->children(), [file, &functor] (const File* file) {
		if (file->is_directory())
		{
			children_each_rec(file, functor);
		}
		functor(file);
	});
}

template <typename F>
void to_delete_each_rec(const File* file, const F& functor)
{
	files_each(file->files_to_delete(), [file, &functor] (const File* file) {
		if (file->is_directory())
		{
			to_delete_each_rec(file, functor);
		}
		functor(file);
	});
}

}// namespace xdd
