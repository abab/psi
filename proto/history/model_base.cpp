/*
 * model_base.cpp - base model for history
 * Copyright (C) 2008, 2009 Aleksey Palazhchenko
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "model_base.h"
using namespace History;

BaseHistoryModel::BaseHistoryModel(Storage *storage)
		: QAbstractItemModel(0), storage_(storage)
{
	Q_ASSERT(storage);
	root_ = new HistoryItem();
}

BaseHistoryModel::~BaseHistoryModel()
{
	delete root_;
}

HistoryItem* BaseHistoryModel::itemFromIndex(const QModelIndex& ind) const
{
	if (!ind.isValid()) {
		qDebug("itemFromIndex - invalid index");
		return root_;
	}
	return static_cast<HistoryItem*>(ind.internalPointer());
}

QModelIndex BaseHistoryModel::indexFromItem(HistoryItem* item, const int column) const
{
	Q_ASSERT(item);
	Q_ASSERT(column >= 0);
	if(item == root_) {
		qDebug("indexFromItem - item is root_");
		return QModelIndex();
	}
	return createIndex(item->row(), column, item);
}

QModelIndex BaseHistoryModel::userDataIndex(const QModelIndex& ind) const
{
	Q_ASSERT(ind.isValid());

	// column 0 contains this all
	return index(ind.row(), 0, ind.parent());
}

QModelIndex BaseHistoryModel::index(int row, int column, const QModelIndex& parent) const
{
	const HistoryItem* item = itemFromIndex(parent);
	Q_ASSERT(row < item->childCount());
	HistoryItem* childItem = item->child(row);
 	Q_ASSERT(childItem);
	return indexFromItem(childItem, column);
}

QModelIndex BaseHistoryModel::parent(const QModelIndex& ind) const
{
	return indexFromItem(itemFromIndex(ind)->parent(), 0);	// only 0 column can have child items
}

int BaseHistoryModel::rowCount(const QModelIndex& parent) const
{
	// only 0 column can have child items
	if (parent.column() > 0) {
		return 0;
	}

	return itemFromIndex(parent)->childCount();
}

int BaseHistoryModel::columnCount(const QModelIndex& parent) const
{
	return itemFromIndex(parent)->columnCount();
}

QVariant BaseHistoryModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid()) {
		qDebug("data - index is invalid");
		return QVariant();
	}
	Q_ASSERT_X((index.column() == 0) || (role < Qt::UserRole), "BaseHistoryModel::data", "all custom roles _should_ be in column 0");
	return itemFromIndex(index)->data(index.column(), role);
}

QVariant BaseHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Vertical) {
		return QVariant();
	}

	return root_->data(section, role);
}

QModelIndexList BaseHistoryModel::match(const QModelIndex& start, int role, const QVariant& value, int hits, Qt::MatchFlags flags) const
{
	Q_ASSERT((flags == Qt::MatchExactly) || (flags == Qt::MatchContains));
	Q_UNUSED(hits);

	QModelIndexList res;
	if((flags == Qt::MatchExactly) && (data(start, role) == value)) {
		res.append(start);
	} else if(flags == Qt::MatchContains) {
		Q_ASSERT(value.canConvert(QVariant::String));
		const QString valueStr = value.toString();
		if(data(start, role).toString().contains(valueStr)) {
			res.append(start);
		}
	}

	const HistoryItem* startItem = itemFromIndex(start);
	const int rows = startItem->childCount();
	for(int row=0; row<rows; ++row)
	{
		const HistoryItem* child = startItem->child(row);
		const int cols = child->columnCount();
		for(int col=0; col<cols; ++col)
		{
			if((col > 0) && (role >= Qt::UserRole)) {	// all our custom roles stored in column 0
				break;
			}
			res << match(index(row,col,start), role, value, hits, flags);
		}
	}

	return res;
}

#ifdef HISTORY_DEBUG_MODELS

QString BaseHistoryModel::indexToStr(const QModelIndex& index) const
{
	QString res("pointer: %1 column: %2 row: %3 parent: %4");
	return res.arg(reinterpret_cast<qptrdiff>(index.internalPointer()), 0, 16)
		.arg(index.column()).arg(index.row())
		.arg(reinterpret_cast<qptrdiff>(index.parent().internalPointer()), 0, 16);
}

QString BaseHistoryModel::flagsToStr(const Qt::ItemFlags& flags) const
{
	QString res;
	if(flags.testFlag(Qt::ItemIsSelectable)) {
		res += "Qt::ItemIsSelectable ";
	}
	if(flags.testFlag(Qt::ItemIsEditable)) {
		res += "Qt::ItemIsEditable ";
	}
	if(flags.testFlag(Qt::ItemIsEnabled)) {
		res += "Qt::ItemIsEnabled ";
	}
	return res;
}

void BaseHistoryModel::setDebugTooltips(HistoryItem* item)
{
	const QModelIndex index = indexFromItem(item, 0);
	QString toolTip = "<b>index</b>: " + indexToStr(index) + "<br>";
	toolTip += "<b>flags</b>: " + flagsToStr(index.flags()) + "<br>";
	toolTip += "<b>item</b>: " + QString::number(reinterpret_cast<qptrdiff>(item), 16) + "<br>";
	toolTip += "<b>parent</b>: " + QString::number(reinterpret_cast<qptrdiff>(item->parent()), 16) + "<br>";
	toolTip += "<b>child items</b>:[ ";
	for(int i=0; i<item->childCount(); ++i) {
		toolTip += QString::number(reinterpret_cast<qptrdiff>(item->child(i)), 16) + " ";
	}
	toolTip += "]<br>";

	for(int column=0; column<item->columnCount(); ++column) {
		toolTip += "<br>column " + QString::number(column) + ":<br>";
		const DataCell& allData = item->data(column);
		foreach(int role, allData.keys()) {
			toolTip += "<b>" + QString::number(role) + "</b> " + allData.value(role).toString() + "<br>";
		}
	}
	item->addDataToRow(Qt::ToolTipRole, toolTip);

	for(int i=0; i<item->childCount(); ++i) {
		setDebugTooltips(item->child(i));
	}
}

#endif
