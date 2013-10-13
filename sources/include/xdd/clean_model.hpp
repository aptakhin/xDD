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
		NAME,
		SIZE,
		REASON,
	};

	void flush(bool hint_do_rec_reset);

	void notify_scan_started();
	void notify_scan_finished();

	uint64 calculate_free_size() const;

	bool empty() const { return !pseudo_root_.has_files_to_delete(); }

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

	int	columnCount(const QModelIndex& parent) const override;
	QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QModelIndex	index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& index) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

private:
	
	File pseudo_root_;

	bool ready_;

	mutable uint64 free_size_;
	mutable bool free_size_valid_;
};

} // namespace xdd
