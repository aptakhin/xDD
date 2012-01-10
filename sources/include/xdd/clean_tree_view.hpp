/** xDDTools */
#pragma once

#include "xdd/proto.hpp"
#include <QTreeView>

namespace xdd {

class Clean_tree_view : public QTreeView
{
	Q_OBJECT
public:
	explicit Clean_tree_view(QObject *parent = 0);

	~Clean_tree_view();


protected/*overriden*/:
	void keyPressEvent(QKeyEvent* ev);
	

public /*overriden*/:
	

public slots:
	void sort_indicator_changed(int column, Qt::SortOrder order);

signals:
	void clean_updated();

};

} // namespace xdd
