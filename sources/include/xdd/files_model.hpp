/** xDDTools */
#pragma once

#include "xdd/file.hpp"
#include "xdd/manager.hpp"
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QStringList>
#include <QBrush>

namespace xdd {

class Files_model : public QAbstractItemModel
{
	Q_OBJECT
public:
	explicit Files_model(QObject* parent = 0);

	~Files_model();

	enum Columns
	{
		NAME,
		SIZE
	};

	const File* locate(const QModelIndex& index, int role) const;

	void notify_scan_started();
	void notify_scan_finished();

	void flush();

	void remove_deleted();

signals:
	/// Notify selected files to clean changed
	void update_clean();

public:

	int	columnCount(const QModelIndex& parent) const override;
	QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QModelIndex	index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& index) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	
private:
	File* assoc_file(const QModelIndex& index);
	const File* assoc_file(const QModelIndex& index) const;

private:
	std::unique_ptr<const File_system> fs_;
	bool ready_;
};

} // namespace xdd
