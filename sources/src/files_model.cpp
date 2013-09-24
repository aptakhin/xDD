#include "xdd/files_model.hpp"
#include "xdd/manager.hpp"
#include <QFileInfo>
#include <QFileIconProvider>

namespace xdd {

QColor red_brush(255, 0, 0);
QColor black_brush(0, 0, 0);

Files_model::Files_model(QObject *parent)
:   QAbstractItemModel(parent), 
	fs_(nullptr),
	ready_(false)
{
}

Files_model::~Files_model()
{
	delete fs_;
}

File* Files_model::assoc_file(const QModelIndex& index) 
{
	return reinterpret_cast<File*>(index.internalPointer());
}

const File* Files_model::assoc_file(const QModelIndex& index) const
{
	return reinterpret_cast<const File*>(index.internalPointer());
}

void Files_model::flush()
{
	File_system::i()->root()->update_delete_cache_rec();
}

void Files_model::remove_deleted()
{
	File_system::i()->root()->update_delete_cache_rec();
}

void Files_model::notify_scan_started()
{
	fs_ = nullptr;
	ready_ = false;
}

void Files_model::notify_scan_finished()
{
	fs_ = Scan_manager::i()->fs();
	ready_ = true;
	beginResetModel();
	endResetModel();
}

const File* Files_model::locate(const QModelIndex& index, int role) const
{
	QModelIndex parent = index.parent();
	const File* file = fs_->root();

	if (parent != QModelIndex())
		file = locate(parent, role);

	if (file == nullptr || (size_t)index.row() >= file->num_children())
		return nullptr;

	return file->i_child((size_t)index.row());
}

bool Files_model::setData(const QModelIndex& index, const QVariant& value, int role)
{
	const File* file = locate(index, role);
	if (role == Qt::CheckStateRole && file != nullptr && index.column() == C_NAME)
	{
		const QString& reason = value.toInt() == Qt::Checked? USER_WANTS_STR : EMPTY_STR;

		class Delete_thread : public QThread
		{
		public:
			Delete_thread(File* file, const QString& reason, Files_model* model) 
			:	file_(file), 
				reason_(reason), 
				model_(model) 
			{
			}

			void run()
			{
				file_->mark_for_delete(&reason_);
				emit model_->update_clean();

				const File* file = last_file(file_);

				emit model_->dataChanged(model_->createIndex(0, 0, (void*) file_), 
					model_->createIndex(0, 0, (void*) file));
			}

		protected:

			const File* last_file(const File* file)
			{
				if (!file->has_children())
					return file;
				else
					return file->i_child(file->num_children() - 1);
			}

		protected:
			File* file_;
			const QString& reason_;
			Files_model* model_;
		};

		Delete_thread* del = new Delete_thread(const_cast<File*>(file), reason, this);
		del->start(); // Autoremoves when done
		return true;
	}
	return false;
}

QVariant Files_model::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	const File* file = locate(index, role);
	if (file == nullptr)
		return QVariant();

	if (role == Qt::DecorationRole && index.column() == C_NAME)
		return QVariant(file->cached_icon());

	if ((role == Qt::EditRole || role == Qt::CheckStateRole) && index.column() == C_NAME)
	{
		if (file->for_delete())
			return QVariant(Qt::Checked);
		else if (file->has_for_delete_cache())
			return QVariant(Qt::PartiallyChecked);
		else
			return QVariant(Qt::Unchecked);
	}

	if (role == Qt::ForegroundRole)
		return QVariant(file->for_delete()? red_brush : black_brush);

	if (role == Qt::DisplayRole || role == Qt::ForegroundRole)
	{
		switch (index.column())
		{
			case C_NAME: return file->name();
			case C_SIZE: return helper::format_size(file->size());
		}
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole && index.column() == C_SIZE)
		return QVariant(Qt::AlignRight);

	return QVariant();
}

QModelIndex Files_model::parent(const QModelIndex& index) const
{
	 if (!index.isValid())
		 return QModelIndex();

	 const File* child = assoc_file(index);
	 const File* parent = child->parent();

	 if (parent->is_root())
		 return QModelIndex();

	 size_t row = 0;
	 const File* parent_of_parent = parent->parent();
	 row = parent_of_parent->number_of(parent->Id());
	 return createIndex((int)row, 0, (void*)parent);
}

QModelIndex	Files_model::index(int row, int column, const QModelIndex& parent) const
{
	if (!ready_ || !hasIndex(row, column, parent))
		 return QModelIndex();

	const File* parent_file = parent.isValid()? assoc_file(parent) : fs_->root();

	if ((size_t)row < parent_file->num_children())
	{
		const File* file = parent_file->i_child(row); 
		return createIndex(row, column, (void*)file);
	}
	return QModelIndex();
}

bool Files_model::hasChildren(const QModelIndex& parent) const
{
	if (!ready_)
		return false;

	const File* parent_file = fs_->root();
	if (parent.isValid())
		parent_file = assoc_file(parent);
	return parent_file->has_children();
}

Qt::ItemFlags Files_model::flags(const QModelIndex &index) const
 {
	 if (!index.isValid())
		 return Qt::ItemIsEnabled;

	 if (index.column() == C_NAME)
		 return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;

	 return Qt::ItemIsEnabled;
 }

QVariant Files_model::headerData(int section, Qt::Orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	switch (section)
	{
		case C_NAME: return QString("Name");
		case C_SIZE: return QString("Size");
	}
	return QVariant();
 }

int	Files_model::rowCount(const QModelIndex& parent) const
{
	if (!ready_)
		return 0;

	const File* parent_file = fs_->root();
	if (parent.isValid())
		parent_file = assoc_file(parent);
	return parent_file->num_children();
}

int	Files_model::columnCount(const QModelIndex&) const
{
	return ready_? 2 : 0;
}

} // namespace xdd
