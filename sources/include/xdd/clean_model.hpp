/** xDDTools */
#pragma once

#include "xdd/proto.hpp"
#include "xdd/manager.hpp"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QStringList>
#include <QBrush>

namespace xdd {

class Clean_model : public QAbstractItemModel
{
	Q_OBJECT
public:
	explicit Clean_model(QObject *parent = 0);

	~Clean_model();

	enum Columns
	{
		C_NAME,
		C_SIZE,
		C_REASON,
	};

	void reset();

	void notify_scan_started();
	void notify_scan_finished();

	uint64 calculate_free_size() const;

	bool empty() const { return !_pseudo_root.has_files_to_delete(); }

	void write_cleaning_files_str(const QString& separator, QString& cleaning_files) const;
	

protected:
	void reset_node_rec(const File* node);

	const File* locate(const QModelIndex& index, int role) const;

	File* assoc_file(const QModelIndex& index);
	const File* assoc_file(const QModelIndex& index) const;

	const File* parent(const File* file) const;

	bool remove_item_at(int row, const QModelIndex& index);

	static void sort_rec(File* node, int column, Qt::SortOrder order);

	void write_cleaning_files_str(const File* node, const QString& separator, QString& cleaning_files) const;

	
public /*overriden*/:

	int	columnCount(const QModelIndex& parent) const;
	QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags(const QModelIndex& index) const;
	bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QModelIndex	index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex& index) const;
	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	bool removeRow(int row, const QModelIndex& parent = QModelIndex());
	bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

private:
	
	File _pseudo_root;

	bool _ready;

	mutable uint64 _free_size;
	mutable bool _free_size_valid;
};

} // namespace xdd
