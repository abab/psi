/*
 * model_historyitem.h - basic element of model
 * Copyright (C) 2008 Aleksey Palazchenko
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

#ifndef _MODEL_HISTORYITEM_H_
#define _MODEL_HISTORYITEM_H_

#include <QMap>
#include <QList>
#include <QVariant>

namespace History
{

/*! QMap of role and data - one cell. */
typedef QMap<int, QVariant> DataCell;

/*! QList of DataCell - data for one row: all columns and all roles.*/
typedef QList<DataCell> DataRow;

/*! \brief One item of any history model. */
class HistoryItem
{
	// no Q_OBJECT - pure C++ class
public:
	/*! Creates new item. Default parent is \e this.*/
	HistoryItem(const int initialColumnCount = 0);
	/*! Deletes item and all child items. */
	~HistoryItem() { qDeleteAll(childItems_); }

	/*! Sets parent for item. \e Parent should be existed item. */
	void setParent(HistoryItem *parent);
	/*! Returns parent item. */
	HistoryItem* parent() const { return parentItem_; }

	/*! Appends existed child to item.*/
	void appendChild(HistoryItem* child) { childItems_.append(child); }
	/*! Removes child item.*/
	void removeChild(HistoryItem* child);
	/*! Returns child item by row number.*/
	HistoryItem* child(const int row) const;

	/*! Returns child count.*/
	int childCount() const { return childItems_.count(); }
	/*! Returns columns count for this item.*/
	int columnCount() const { return itemData_.count(); }

	/*! Returns data by column and role.*/
	QVariant data(const int column, const int role) const;
	/*! Return data for all roles in this column. */
	const DataCell& data(const int column) const;

	/*! Adds data with role to column.*/
	void addData(const int column, const int role, const QVariant &data);
	/*! Add data to all columns.*/
	void addDataToRow(const int role, const QVariant &data);

	/*! Returns row of this item in parent.*/
	int row() const;

#ifdef HISTORY_DEBUG_MODELS
	void dump() const;
	void dumpAll() const;
#endif

private:
	HistoryItem* parentItem_;
	QList<HistoryItem*> childItems_;
	DataRow itemData_;
};

}	// namespace

#endif	// _MODEL_HISTORYITEM_H_
