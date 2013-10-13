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
:	parent_(0),
	id_(0),
	name_(),
	type_(T_FILE),
	size_(0),
	reason_delete_(&EMPTY_STR),
	has_for_delete_cache_(false)
{
}

File::File(const File& cpy)
:	parent_(cpy.parent_),
	id_(cpy.id_),
	name_(cpy.name_),
	type_(cpy.type_),
	size_(cpy.size_),
	reason_delete_(&EMPTY_STR),
	has_for_delete_cache_(false)
{
}

File::File(File::ID parent, const QString& name, Type type)
:   parent_(parent),
	id_(0),
	name_(name),
	type_(type),
	size_(0),
	reason_delete_(&EMPTY_STR),
	has_for_delete_cache_(false)
{
}

File::File(File::ID parent, const wchar_t* name, size_t len, Type type)
:	parent_(parent),
	id_(0),
	name_(QString::fromUtf16((const ushort*) name, len)),
	type_(type),
	size_(0),
	reason_delete_(&EMPTY_STR),
	has_for_delete_cache_(false)
{
}

File::~File()
{
}

bool File::add_child(const File::ID& file_id)
{
	children_.push_back(file_id);
	return true;
}

void File::sort_size_desc()
{
	if (!children_.empty())
	{
		std::sort(children_.begin(), children_.end(), [] (File::ID a, File::ID b) {
			File_system* fs = File_system::i();
			return fs->file_with_id(a)->size() > fs->file_with_id(b)->size();
		});
	}
}

size_t File::number_of(File::ID id) const
{
	for (size_t i = 0; i < children_.size(); ++i)
	{
		if (children_[i] == id)
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
	return File_system::i()->file_with_id(parent_);
}

const File* File::parent() const
{
	return File_system::i()->file_with_id(parent_);
}

File* File::i_child(size_t i)
{
	return File_system::i()->file_with_id(children_[i]);
}

const File* File::i_child(size_t i) const
{
	return File_system::i()->file_with_id(children_[i]);
}

void File::mark_for_delete(const QString* reason)
{ 
	reason_delete_ = (reason != nullptr)? reason : &EMPTY_STR;

	if (parent())
		parent()->child_marked_for_delete(this);

	size_t sz = num_children();
	to_delete_.reserve(sz);
	for (size_t i = 0; i < sz; ++i)
	{
		File* child = i_child(i);
		child->_parent_marks_for_delete(*reason);
		to_delete_.push_back(child->Id());
	}
}

void File::_parent_marks_for_delete(const QString& reason)
{
	reason_delete_ = &reason;
	to_delete_.clear();

	size_t sz = num_children();
	bool push = reason.length() > 0;
	if (push) 
		to_delete_.reserve(sz);
	for (size_t i = 0; i < sz; ++i)
	{
		File* child = i_child(i);
		if (push)
			to_delete_.push_back(child->Id());
		child->_parent_marks_for_delete(reason);
	}
}

bool File::for_delete() const 
{ 
	return reason_delete_->length() > 0; 
}

size_t File::number_of_deleted(const File* file) const
{
	size_t sz = to_delete_.size();
	for (size_t i = 0; i != sz; ++i)
	{
		if (to_delete_[i] == file->Id())
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
	return to_delete_.size();
}

File* File::i_file_to_delete(size_t i) const
{
	XDD_ASSERT3(i < to_delete_.size(), "File item is out of bounds!", return nullptr);
	return File_system::i()->file_with_id(to_delete_[i]);
}

bool File::has_files_to_delete() const
{
	return !to_delete_.empty();
}

bool File::has_cached_icon() const
{
	return !icon_cache_.isNull();
}

void File::set_cached_icon(const QIcon& icon)
{
	icon_cache_ = icon;
}

const QIcon& File::cached_icon() const
{
	if (icon_cache_.isNull())
	{
		QString path;
		Scan_manager::i()->fs()->full_path_of(*this, path);
		QFileInfo info(path);
		icon_cache_ = QFileIconProvider().icon(info);
	}

	return icon_cache_;
}

bool File::update_delete_cache_rec() const
{
	if (for_delete())
		return has_for_delete_cache_ = true;

	return has_for_delete_cache_ = children_if_any([] (const File* child) {
		return child->update_delete_cache_rec();
	});
}

bool File::has_for_delete_cache() const
{
	return has_for_delete_cache_;
}

void File::child_marked_for_delete(const File* child)
{
	if (child->delete_reason() != EMPTY_STR) 
		add_child_to_delete_list_impl(child);
	else
		remove_child_from_delete_list_impl(child);	
}

File::Files::iterator File::child_to_delete_iter_by_id(File::ID id)
{
	Files::iterator child_iter = to_delete_.begin();
	for (; child_iter != to_delete_.end(); ++child_iter)
	{
		if (*child_iter == id)
			break;
	}
	return child_iter;
}

void File::add_child_to_delete_list_impl(const File* child)
{
	if (child_to_delete_iter_by_id(child->Id()) == to_delete_.end())
		to_delete_.push_back(child->Id());// add child if it doesn't present
}

void File::remove_child_from_delete_list_impl(const File* child)
{
	Files::iterator child_iter = child_to_delete_iter_by_id(child->Id());
	if (child_iter != to_delete_.end())
		to_delete_.erase(child_iter);// Remove from delete lists
}

void File::_remove_all_children_from_delete_list()
{
	to_delete_.clear();
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
		std::stable_sort(to_delete_.begin(), to_delete_.end(), pred);
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
		std::stable_sort(to_delete_.begin(), to_delete_.end(), pred);
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
		std::stable_sort(to_delete_.begin(), to_delete_.end(), pred);
	}
		break;
	}
}

}// namespace xdd
