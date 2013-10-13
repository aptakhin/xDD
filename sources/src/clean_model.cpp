/** xDDTools */
#include "xdd/file.hpp"
#include "xdd/clean_model.hpp"
#include "xdd/manager.hpp"
#include <QFileInfo>
#include <QFileIconProvider>

namespace xdd {

Clean_model::Clean_model(QObject *parent)
:   QAbstractItemModel(parent),
	pseudo_root_((size_t)-1, QString("Pseudo root"), File::T_DIRECTORY),
	ready_(false),
	free_size_(0),
	free_size_valid_(false)
{
}

Clean_model::~Clean_model()
{
}

void Clean_model::notify_scan_started()
{
	ready_ = false;
}

void Clean_model::notify_scan_finished()
{
	ready_ = true;
	beginResetModel();
	endResetModel();
}

File* Clean_model::assoc_file(const QModelIndex& index)
{
	return reinterpret_cast<File*>(index.internalPointer());
}

const File* Clean_model::assoc_file(const QModelIndex& index) const
{
	return reinterpret_cast<const File*>(index.internalPointer());
}

const File* Clean_model::parent(const File* file) const
{
	if (pseudo_root_.has_to_delete(file))
		return &pseudo_root_;
	else
		return file->parent();
}

void Clean_model::flush(bool hint_do_rec_reset)
{
	pseudo_root_._remove_all_children_from_delete_list();
	if (hint_do_rec_reset)
		reset_node_rec(File_system::i()->root());
	free_size_valid_ = false;
}

void Clean_model::reset_node_rec(const File* node)
{
	if (node->for_delete())
		pseudo_root_.child_marked_for_delete(node);// Find child. Add to list and return.
	else
	{
		size_t sz = node->num_children();
		for (size_t i = 0; i < sz; ++i)
			reset_node_rec(node->i_child(i));
	}
}

uint64 Clean_model::calculate_free_size() const
{
	if (!free_size_valid_)
	{
		uint64 size = 0;
		files_each(pseudo_root_.files_to_delete(), [&size] (const File* file) {
			size += file->size();
		});
		free_size_ = size;

		free_size_valid_ = true;
	}
	return free_size_;
}

const File* Clean_model::locate(const QModelIndex& index, int role) const
{
	QModelIndex parent = index.parent();
	const File* file = nullptr;

	if (parent == QModelIndex())
		file = &pseudo_root_;
	else
		file = locate(parent, role);

	if (file != nullptr && (size_t)index.row() < file->num_files_to_delete())
		return file->i_file_to_delete((size_t)index.row());
	else
		return nullptr;
}

QVariant Clean_model::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	const File* file = locate(index, role);

	if (!file)
		return QVariant();

	if (role == Qt::DecorationRole && index.column() == C_NAME)
		return QVariant(file->cached_icon());

	if (role == Qt::DisplayRole)
	{
		switch (index.column())
		{
		case C_NAME:    return file->name();
		case C_SIZE:    return helper::format_size(file->size());
		case C_REASON:  return file->delete_reason();
		}
		return QVariant();
	}

	if (role == Qt::ToolTipRole && index.column() == C_NAME)
		return Scan_manager::i()->fs()->full_path_of(*file);

	return QVariant();
}

QModelIndex Clean_model::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	const File* child = assoc_file(index);
	const File* model_parent = parent(child);

	if (model_parent->is_root())
		 return QModelIndex();

	size_t row = 0;
	const File* parent_of_parent = model_parent->parent();
	row = parent_of_parent->number_of_deleted(model_parent);
	return createIndex((int)row, 0, (void*)model_parent);
}

QModelIndex	Clean_model::index(int row, int column, const QModelIndex& parent) const
{
	if (!ready_ || !hasIndex(row, column, parent))
		return QModelIndex();

	const File* parent_file = &pseudo_root_;
	if (parent.isValid())
		parent_file = assoc_file(parent);

	if ((size_t)row >= parent_file->num_files_to_delete())
		return QModelIndex();
		
	const File* file = parent_file->i_file_to_delete((size_t)row); 
	return createIndex(row, column, (void*)file);	
}

bool Clean_model::hasChildren(const QModelIndex& parent) const
{
	if (!ready_)
		return false;

	const File* item = &pseudo_root_;
	if (parent.isValid())
		item = assoc_file(parent);
	return item->num_files_to_delete();
}

Qt::ItemFlags Clean_model::flags(const QModelIndex &index) const
 {
	 if (!index.isValid())
		 return Qt::ItemIsEnabled;

	 return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
 }

QVariant Clean_model::headerData(int section, Qt::Orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	switch (section)
	{
	case C_NAME:    return QString("Name");
	case C_SIZE:    return QString("Size");
	case C_REASON:  return QString("Reason");
	}
	return QVariant();
 }

int	Clean_model::rowCount(const QModelIndex& parent) const
{
	if (!ready_)
		return 0;

	const File* item = &pseudo_root_;
	if (parent.isValid())
		item = assoc_file(parent);
	
	return item->num_files_to_delete();
}

int	Clean_model::columnCount(const QModelIndex&) const
{
	return ready_? 3 : 0;
}

bool Clean_model::removeRows(int row, int count, const QModelIndex& parent)
{
	int till = row + count;
	beginRemoveRows(parent, row, till - 1);
	for (; row < till; ++row)
		remove_item_at(row, parent);
	endRemoveRows();
	free_size_valid_ = false;
	return true;
}

bool Clean_model::remove_item_at(int row, const QModelIndex& parent)
{
	File* item = assoc_file(index(row, 0, parent));
	if (!item)
		return false;

	item->mark_for_delete(&EMPTY_STR);

	if (pseudo_root_.has_to_delete(item))
		pseudo_root_.remove_child_from_delete_list_impl(item);

	return true;
}

void Clean_model::sort(int column, Qt::SortOrder order)
{
	sort_rec(&pseudo_root_, column, order);
}

void Clean_model::sort_rec(File* node, int column, Qt::SortOrder order)
{
	File::Field field;

	switch (column)
	{
	case Clean_model::C_NAME:   field = File::F_NAME; break;
	case Clean_model::C_SIZE:   field = File::F_SIZE; break;
	case Clean_model::C_REASON: field = File::F_DELETE_REASON; break;
	}

	node->sort_marked_for_delete(field, from_qt(order));

	to_delete_each_rec(node, [column, order] (const File* child) {
		sort_rec(const_cast<File*>(child), column, order);
	});
}

void Clean_model::write_cleaning_files_str(const QString& separator, QString& cleaning_files) const
{
	cleaning_files.clear();
	write_cleaning_files_str(&pseudo_root_, separator, cleaning_files);
}

void Clean_model::write_cleaning_files_str(const File* node, const QString& separator, QString& cleaning_files) const
{
	QString acc;
	files_each(node->files_to_delete(), [&acc, &separator] (const File* file) {
		acc += File_system::i()->full_path_of(*file) + separator;
	});
	cleaning_files = acc;
}

} // namespace xdd
