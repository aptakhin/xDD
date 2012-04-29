/** xDDTools */
#include "xdd/file.hpp"
#include "xdd/file_system.hpp"
#include "xdd/manager.hpp"

#include <QFileInfo>
#include <QFileIconProvider>

#ifdef XDD_UNIVERSAL_SCANNER
#	include <QDir>
#endif

namespace xdd {

File::File()
:	_parent(0),
	_id(0),
	_name(),
	_type(T_FILE),
	_size(0),
	_reason_delete(&EMPTY_STR),
	_has_for_delete_cache(false)
{
}

File::File(const File& cpy)
:	_parent(cpy._parent),
	_id(cpy._id),
	_name(cpy._name),
	_type(cpy._type),
	_size(cpy._size),
	_reason_delete(&EMPTY_STR),
	_has_for_delete_cache(false)
{
}

File::File(File::ID parent, const QString& name, Type type)
:   _parent(parent),
	_id(0),
	_name(name),
	_type(type),
	_size(0),
	_reason_delete(&EMPTY_STR),
	_has_for_delete_cache(false)
{
}

File::File(File::ID parent, const wchar_t* name, size_t len, Type type)
:	_parent(parent),
	_id(0),
	_name(QString::fromWCharArray(name, len)),
	_type(type),
	_size(0),
	_reason_delete(&EMPTY_STR),
	_has_for_delete_cache(false)
{
}

void File::operator = (const File& cpy)
{
	_parent			      = cpy._parent;
	_id					  = cpy._id;
	_name				  = cpy._name;
	_type				  = cpy._type;
	_size				  = cpy._size;
	_reason_delete		  = cpy._reason_delete;
	_has_for_delete_cache = cpy._has_for_delete_cache;
}

File::~File()
{
}

bool File::add_child(const File::ID& file_id)
{
	_children.push_back(file_id);
	return true;
}

void File::sort_size_desc()
{
	struct file_size_comparer
	{
		bool operator () (File::ID a, File::ID b)
		{
			File_system* fs = File_system::i();
			return fs->file_with_id(a)->size() > fs->file_with_id(b)->size();
		}
	} pred;

	if (!_children.empty())
		std::sort(_children.begin(), _children.end(), pred);
}

size_t File::number_of(File::ID id) const
{
	for (size_t i = 0; i < _children.size(); ++i)
	{
		if (_children[i] == id)
			return i;
	}

	return (size_t)-1;
}

bool File::has_child(File::ID id) const
{
	return number_of(id) != (size_t)-1;
}

File* File::parent()
{
	return File_system::i()->file_with_id(_parent);
}

const File* File::parent() const
{
	return File_system::i()->file_with_id(_parent);
}

File* File::i_child(size_t i)
{
	return File_system::i()->file_with_id(_children[i]);
}

const File* File::i_child(size_t i) const
{
	return File_system::i()->file_with_id(_children[i]);
}

void File::mark_for_delete(const QString* reason)
{ 
	if (reason != nullptr)
		_reason_delete = reason;
	else
		_reason_delete = &EMPTY_STR;

	if (parent())
		parent()->child_marked_for_delete(this);

	size_t sz = num_children();
	_to_delete.reserve(sz);
	for (size_t i = 0; i < sz; ++i)
	{
		File* child = i_child(i);
		child->_parent_marks_for_delete(*reason);
		_to_delete.push_back(child->Id());
	}
}

void File::_parent_marks_for_delete(const QString& reason)
{
	_reason_delete = &reason;
	_to_delete.clear();

	size_t sz = num_children();
	bool push = reason.length() > 0;
	if (push) 
		_to_delete.reserve(sz);
	for (size_t i = 0; i < sz; ++i)
	{
		File* child = i_child(i);
		if (push)
			_to_delete.push_back(child->Id());
		child->_parent_marks_for_delete(reason);
	}
}

bool File::for_delete() const 
{ 
	return _reason_delete->length() > 0; 
}

size_t File::number_of_deleted(const File* file) const
{
	size_t sz = _to_delete.size();
	for (size_t i = 0; i != sz; ++i)
	{
		if (_to_delete[i] == file->Id())
			return i;
	}
	return (size_t)-1;
}

bool File::has_to_delete(const File* file) const
{
	return number_of_deleted(file) != (size_t)-1;
}

size_t File::num_files_to_delete() const
{
	return _to_delete.size();
}

File* File::i_file_to_delete(size_t i) const
{
	XDD_ASSERT3(i < _to_delete.size(), "File item is out of bounds!", return nullptr);
	return File_system::i()->file_with_id(_to_delete[i]);
}

bool File::has_files_to_delete() const
{
	return !_to_delete.empty();
}

bool File::_has_cached_icon() const
{
	return !_icon_cache.isNull();
}

void File::_set_cached_icon(const QIcon& icon)
{
	_icon_cache = icon;
}

const QIcon& File::_cached_icon() const
{
	if (_icon_cache.isNull())
	{
		QString path;
		Scan_manager::i()->fs()->full_path_of(*this, path);
		QFileInfo info(path);
		_icon_cache = QFileIconProvider().icon(info);
	}

	return _icon_cache;
}

bool File::update_delete_cache_rec() const
{
	if (for_delete())
		return _has_for_delete_cache = true;

#ifdef XDD_CPP11
	return _has_for_delete_cache = children_if_any([] (const File* child) {
		return child->update_delete_cache_rec();
	});
#else
	struct Func
	{
		bool operator (const File* child) {
			return child->has_marked_for_delete_rec();
		}
	} func;

	return _has_for_delete_cache = children_if_any(func);

#endif//#ifdef XDD_CPP11
}

bool File::has_for_delete_cache() const
{
	return _has_for_delete_cache;
}

void File::child_marked_for_delete(const File* child)
{
	if (child->delete_reason() != EMPTY_STR) 
		_add_child_to_delete_list(child);
	else
		_remove_child_from_delete_list(child);	
}

File::Files::iterator File::child_to_delete_iter_by_id(File::ID id)
{
	Files::iterator child_iter = _to_delete.begin();
	for (; child_iter != _to_delete.end(); ++child_iter)
	{
		if (*child_iter == id)
			break;
	}
	return child_iter;
}

void File::_add_child_to_delete_list(const File* child)
{
	if (child_to_delete_iter_by_id(child->Id()) == _to_delete.end())
		_to_delete.push_back(child->Id());// add child if it doesn't present
}

void File::_remove_child_from_delete_list(const File* child)
{
	Files::iterator child_iter = child_to_delete_iter_by_id(child->Id());
	if (child_iter != _to_delete.end())
		_to_delete.erase(child_iter);// Remove from delete lists
}

void File::_remove_all_children_from_delete_list()
{
	_to_delete.clear();
}

void File::sort_marked_for_delete(Field field, Sort_order order)
{
	struct Field_comparator {
		Field_comparator(Sort_order order) : _order(order) {}
		Sort_order _order;
		File* file_with_id(File::ID id) { return  File_system::i()->file_with_id(id); }
	};

	switch (field)
	{
	case F_NAME:
	{
		struct Name_field_comparator : public Field_comparator {
			Name_field_comparator(Sort_order order) : Field_comparator(order) {}
			bool operator ()(File::ID a, File::ID b) {
				return relation(file_with_id(a)->name(), file_with_id(b)->name(), _order);
			}
		} pred(order);
		std::sort(_to_delete.begin(), _to_delete.end(), pred);
	}
		break;

	case F_SIZE:
	{
		struct size_field_comparator : public Field_comparator {
			size_field_comparator(Sort_order order) : Field_comparator(order) {}
			Sort_order _order;
			bool operator ()(File::ID a, File::ID b) {
				return relation(file_with_id(a)->size(), file_with_id(b)->size(), _order);
			}
		} pred(order);
		std::sort(_to_delete.begin(), _to_delete.end(), pred);
	}
		break;

	case F_DELETE_REASON:
	{
		struct Reason_field_comparator : public Field_comparator {
			Reason_field_comparator(Sort_order order) : Field_comparator(order) {}
			Sort_order _order;
			bool operator ()(File::ID a, File::ID b) {
				return relation(file_with_id(a)->delete_reason(), file_with_id(b)->delete_reason(), _order);
			}
		} pred(order);
		std::sort(_to_delete.begin(), _to_delete.end(), pred);
	}
		break;
	}
}

}// namespace xdd
