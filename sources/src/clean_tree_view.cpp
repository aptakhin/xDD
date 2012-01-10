#include "xdd/Clean_tree_view.hpp"
#include <QKeyEvent>
#include "xdd/Clean_model.hpp"
#include <QHeaderView>

namespace xdd {

Clean_tree_view::Clean_tree_view(QObject *parent)
:	QTreeView((QWidget*)parent)
{
	QObject::connect(this->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
		this, SLOT(sort_indicator_changed(int, Qt::SortOrder)));
}

Clean_tree_view::~Clean_tree_view()
{
}

void Clean_tree_view::keyPressEvent(QKeyEvent* ev)
{
	if (ev->key() == Qt::Key_Delete)
	{
		QModelIndexList indexes = selectedIndexes();
		int row = -1;
		int first_row = -1;
		for (QModelIndexList::iterator i = indexes.begin(); i != indexes.end(); ++i)
		{
			if (row != i->row())
			{
				row = i->row();

				if (first_row == -1)
					first_row = i->row();

				model()->removeRow(i->row(), i->parent());
			}
		}
		// All rows removed

		// Now select the new one selected row for fast massive deleting
		if (first_row != -1)
		{
			selectionModel()->select(model()->index(first_row, 0, QModelIndex()), 
				QItemSelectionModel::Select);
		}

		emit clean_updated();
	}
}

void Clean_tree_view::sort_indicator_changed(int column, Qt::SortOrder order)
{
	model()->sort(column, order);
	reset();
}

} // namespace xdd
