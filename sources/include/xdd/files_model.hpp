/** xDDTools */
#pragma once

#include "xdd/File.hpp"
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
		C_NAME,
		C_SIZE
	};

    const File* locate(const QModelIndex& index, int role) const;

	void notify_scan_started();
	void notify_scan_finished();

	void reset();

signals:
	/// Notify selected files to clean changed
	void update_clean();

public /*overriden*/:

    int	columnCount(const QModelIndex& parent) const;
    QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex	index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex& index) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    
private:
	File* assoc_file(const QModelIndex& index);
	const File* assoc_file(const QModelIndex& index) const;

private:
	const File_system* _fs;
	bool _ready;

	QBrush _red_brush;
	QBrush _black_brush;
};

} // namespace xdd
