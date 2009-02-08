/*
 * model_base.h - base model for history
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

#ifndef _MODEL_BASE_H_
#define _MODEL_BASE_H_

#include "model_historyitem.h"
#include "backend.h"

#include <QAbstractItemModel>

namespace History
{

/*! \brief Base class for all history models.
 *	Basic rule: invalid index <-> root item.
 */
class BaseHistoryModel : public QAbstractItemModel
{
	Q_OBJECT
	Q_DISABLE_COPY(BaseHistoryModel)

public:
	explicit BaseHistoryModel(Storage *storage);
	virtual ~BaseHistoryModel();

	HistoryItem* itemFromIndex(const QModelIndex& index) const;
	QModelIndex indexFromItem(HistoryItem* item, const int column) const;
	/*! Column 0 contains user (developer) data - ids, etc. */
	QModelIndex userDataIndex(const QModelIndex& index) const;

	// virtuals from QAbstractItemModel
	virtual QModelIndex index(int row, int column, const QModelIndex& parent) const;
	virtual QModelIndex parent(const QModelIndex& index) const;
	virtual int rowCount(const QModelIndex& parent) const;
	virtual int columnCount(const QModelIndex& parent) const;
	virtual QVariant data(const QModelIndex& index, int role) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

	virtual Qt::ItemFlags flags(const QModelIndex &index) const = 0;
 	virtual bool setData(const QModelIndex &index, const QVariant &value, int role) = 0;

	/*! Recursive search for items.
	 *  \param start - parent
	 *  \param flags Allowed flags: Qt::MatchExactly, Qt::MatchContains. Only one flag
	 *  		is allowed, not more, not less.
	 */
	virtual QModelIndexList match(const QModelIndex& start, int role, const QVariant& value, int hits,
			Qt::MatchFlags flags) const;

#ifdef HISTORY_DEBUG_MODELS
	QString indexToStr(const QModelIndex&) const;
	QString flagsToStr(const Qt::ItemFlags&) const;
	void setDebugTooltips(HistoryItem* parent);
#endif

protected:
	Storage* storage_;
	HistoryItem* root_;
};

}	// namespace

#endif	// _MODEL_BASE_H_
