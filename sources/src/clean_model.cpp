/** xDDTools */
#include "xdd/file.hpp"
#include "xdd/Clean_model.hpp"
#include "xdd/manager.hpp"
#include <QFileInfo>
#include <QFileIconProvider>

namespace xdd {

Clean_model::Clean_model(QObject *parent)
:   QAbstractItemModel(parent),
	_pseudo_root((size_t)-1, QString("Pseudo root"), File::T_DIRECTORY),
	_ready(false),
	_free_size(0),
	_free_size_valid(false)
{
}

Clean_model::~Clean_model()
{
}

void Clean_model::notify_scan_started()
{
	_ready = false;
}

void Clean_model::notify_scan_finished(const File_system& fs)
{
	_ready = true;
	reset(fs);
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
	if (_pseudo_root.has_to_delete(file))
		return &_pseudo_root;
	else
		return file->parent();
}

void Clean_model::reset(const File_system& fs)
{
	_pseudo_root._remove_all_children_from_delete_list();
	reset_node_rec(fs.root());
	_free_size_valid = false;
}

void Clean_model::reset_node_rec(const File* node)
{
	if (node->for_delete())
		_pseudo_root.child_marked_for_delete(node);// Find child. add to list and return.
	else
	{
		size_t sz = node->num_children();
		for (size_t i = 0; i < sz; ++i)
			reset_node_rec(node->i_child(i));
	}
}

uint64 Clean_model::Calculate_free_size() const
{
	if (!_free_size_valid)
	{
	#ifdef XDD_CPP11
		uint64 size = 0;
		files_each(_pseudo_root.children(), [&size] (const File* file) {
			size += file->size();
		});
		_free_size = size;
	#else
		_free_size = 0;
		size_t i = 0, sz = _pseudo_root.num_files_to_delete();
		for (; i < sz; ++i)
			_free_size += _pseudo_root.i_file_to_delete(i)->size();
	#endif//#ifdef XDD_CPP11

		_free_size_valid = true;
	}
	return _free_size;
}

const File* Clean_model::locate(const QModelIndex& index, int role) const
{
    QModelIndex parent = index.parent();
    const File* file = nullptr;

    if (parent == QModelIndex())
        file = &_pseudo_root;
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
    {
		return QVariant(file->_cached_icon());
	}

	if (role == Qt::DisplayRole)
    {
		switch (index.column())
		{
		case C_NAME:	return file->name();
		case C_SIZE:	return helper::format_size(file->size());
		case C_REASON:	return file->delete_reason();
		}
		return QVariant();
    }

	if (role == Qt::ToolTipRole && index.column() == C_NAME)
    {
		return Scan_manager::i()->fs()->full_path_of(*file);
    }

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
	if (!_ready || !hasIndex(row, column, parent))
		return QModelIndex();

	const File* parent_file = nullptr;
	if (parent.isValid())
		parent_file = assoc_file(parent);
	else
		parent_file = &_pseudo_root;

	if ((size_t)row < parent_file->num_files_to_delete())
	{
		const File* file = parent_file->i_file_to_delete((size_t)row); 
		return createIndex(row, column, (void*)file);
	}
	return QModelIndex();
}

bool Clean_model::hasChildren(const QModelIndex& parent) const
{
	if (!_ready)
		return false;

	const File* item = &_pseudo_root;
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
	case C_NAME:	return QString("Name");
	case C_SIZE:	return QString("size");
	case C_REASON:	return QString("Reason");
	}
	return QVariant();
 }

int	Clean_model::rowCount(const QModelIndex& parent) const
{
	if (!_ready)
		return 0;

	const File* item = &_pseudo_root;
	if (parent.isValid())
		item = assoc_file(parent);
	
	return item->num_files_to_delete();
}

int	Clean_model::columnCount(const QModelIndex&) const
{
	return _ready? 3 : 0;
}

bool Clean_model::removeRows(int row, int count, const QModelIndex& parent)
{
	int till = row + count;
	beginRemoveRows(parent, row, till - 1);
	for (; row < till; ++row)
		remove_item_at(row, parent);
	endRemoveRows();
	_free_size_valid = false;
	return true;
}

bool Clean_model::removeRow(int row, const QModelIndex& parent)
{
	beginRemoveRows(parent, row, row);
	bool result = remove_item_at(row, parent);
	endRemoveRows();
	_free_size_valid = false;
	return result;
}

bool Clean_model::remove_item_at(int row, const QModelIndex& parent)
{
	File* item = assoc_file(index(row, 0, parent));
	if (!item)
		return false;

	item->mark_for_delete(&EMPTY_STR);

	if (_pseudo_root.has_to_delete(item))
		_pseudo_root._remove_child_from_delete_list(item);

	return true;
}

void Clean_model::sort(int column, Qt::SortOrder order)
{
	sort_rec(&_pseudo_root, column, order);
}

void Clean_model::sort_rec(File* node, int column, Qt::SortOrder order)
{
	File::Field field;

	switch (column)
	{
	case Clean_model::C_NAME:	field = File::F_NAME; break;
	case Clean_model::C_SIZE:	field = File::F_SIZE; break;
	case Clean_model::C_REASON:	field = File::F_DELETE_REASON; break;
	}

	node->sort_marked_for_delete(field, from_qt(order));

#ifdef XDD_CPP11
	to_delete_each_rec(node, [column, order] (const File* child) {
		sort_rec(const_cast<File*>(child), column, order);
	});
#else
	size_t i = 0, sz = node->num_files_to_delete();
	for (; i < sz; ++i)
	{
		sort_rec(node->i_file_to_delete(i), column, order);
	}
#endif
}

void Clean_model::write_cleaning_files_qt_str(const QString& separator, QString& cleaning_files) const
{
	cleaning_files.clear();
	write_cleaning_files_qt_str(&_pseudo_root, separator, cleaning_files);
}

void Clean_model::write_cleaning_files_qt_str(const File* node, const QString& separator, QString& cleaning_files) const
{
#ifdef XDD_CPP11
	QString acc;
	files_each(node->files_to_delete(), [&acc, &separator] (const File* file) {
		acc += File_system::i()->full_path_of(*file) + separator;
	});
	cleaning_files = acc;
#else
#	error "I'm too lazy"
#endif//#ifdef XDD_CPP11
}

} // namespace xdd
