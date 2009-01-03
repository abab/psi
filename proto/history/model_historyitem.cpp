/*
 * model_historyitem.cpp - basic element of model
 * Copyright (C) 2008 Aleksey Palazhchenko
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

#include "model_historyitem.h"
using namespace History;

#ifdef HISTORY_DEBUG_MODELS
#include <QDebug>
#endif

HistoryItem::HistoryItem(const int initialColumnCount)
	: parentItem_(this)
{
	for(int colNo=0; colNo<initialColumnCount; ++colNo) {
		itemData_.append(DataCell());
	}
}

void HistoryItem::setParent(HistoryItem *parent)
{
	Q_ASSERT(parent);
	Q_ASSERT(parent->parent());
	parentItem_ = parent;
}

void HistoryItem::removeChild(HistoryItem* child)
{
	Q_ASSERT(child);
	const int count = childItems_.removeAll(child);
	Q_ASSERT(count == 1);
}

HistoryItem* HistoryItem::child(const int row) const
{
	Q_ASSERT(row >= 0);
	Q_ASSERT_X(row < childCount(), qPrintable(QString("row (%1) should be < childCount (%2)")
			.arg(row).arg(childCount())), "HistoryItem::child");
	return childItems_.value(row);
}

QVariant HistoryItem::data(const int column, const int role) const
{
	Q_ASSERT(column >= 0);
	Q_ASSERT((column == 0) || (role < Qt::UserRole));	// all custom roles _should_ be in column 0
	const QVariant d = (data(column)).value(role, QVariant());
	Q_ASSERT_X(d.isValid() || role < Qt::UserRole,
			qPrintable(QString("HistoryItem::date(%1,%2)").arg(column).arg(role)),
			"invalid data");
	return d;
}

const DataCell& HistoryItem::data(const int column) const
{
	Q_ASSERT(column >= 0);
	Q_ASSERT(column<columnCount());
	return itemData_.at(column);
}

int HistoryItem::row() const
{
	Q_ASSERT(parentItem_);
	return parentItem_->childItems_.indexOf(const_cast<HistoryItem*>(this));
}

void HistoryItem::addData(const int column, const int role, const QVariant& data)
{
	Q_ASSERT(column >= 0);
	if(columnCount() <= column) {
		// add empty columns
		for(int colNo=columnCount(); colNo<=column; ++colNo) {
			itemData_.append(DataCell());
		}
	}
	itemData_[column][role] = data;
}

void HistoryItem::addDataToRow(const int role, const QVariant& data)
{
	// to row = to all columns
	for(int colNo=0; colNo<columnCount(); ++colNo) {
		addData(colNo, role, data);
	}
}

#ifdef HISTORY_DEBUG_MODELS

void HistoryItem::dump() const
{
	qDebug() << "item:" << this;
	qDebug() << "parent:" << parentItem_;
	qDebug() << "child items:" << childItems_;
	for(int i=0; i<itemData_.count(); ++i) {
		qDebug() << "column" << i << ":";
		DataCell dataMap = itemData_[i];
		foreach(int role, dataMap.keys()) {
			qDebug() << "  " << role << "-" << dataMap.value(role);
		}
	}
	qDebug() << "-------------------------------------------------";
}

void HistoryItem::dumpAll() const
{
	dump();
	foreach(HistoryItem* item, childItems_) {
		item->dumpAll();
	}
}

#endif // HISTORY_DEBUG_MODELS
