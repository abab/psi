/*
 * models.h - models for event history
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

#ifndef _MODELS_H_
#define _MODELS_H_

#include <QAbstractItemModel>
#include <QDateTime>
#include <QPointer>
#include <QColor>
#include <QBrush>
#include <QDebug>

#include "backend.h"

namespace History
{

const QString NO_SUBJECT = QT_TR_NOOP("<no subject>");

//LATER move this to config
const QBrush ReceivedMessageBrush	= QBrush(QColor(255,255,255));
const QBrush SentMessageBrush		= QBrush(QColor(230,230,230));
const QBrush NoteBrush				= QBrush(QColor(230,230,255));
const QBrush CollectionHeaderBrush	= QBrush(QColor(230,255,230));

//LATER change this
const QString OUTPUT_DATE_FORMAT = "d MMMM, yyyy";
const QString OUTPUT_TIME_FORMAT = "h:mm:ss";

/*! User role of item at collection.*/
enum HistoryItemRole {
	CollectionOwnerJidRole		= Qt::UserRole + 2, /*!< Owner's JID role.*/
	CollectionContactJidRole	= Qt::UserRole + 3, /*!< Contact's JID role.*/
	CollectionIdRole			= Qt::UserRole + 4,	/*!< Primary key of collection in storage.*/

	EntryIdRole					= Qt::UserRole + 42, /*!< Primary key of entry in storage.*/
	EntryTypeRole				= Qt::UserRole + 43
};

#ifdef HISTORY_DEBUG_MODELS
QString roleToString(int role);
#endif

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
	HistoryItem(int initialColumnCount = 0);
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

bool indexLessThan(const QModelIndex& a, const QModelIndex& b);

/*! \brief Base class for all history models. */
class BaseHistoryModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	BaseHistoryModel(Storage *storage);
	virtual ~BaseHistoryModel();

	HistoryItem* itemFromIndex(const QModelIndex& index) const;
	QModelIndex indexFromItem(HistoryItem* item, int column) const;
	QModelIndex userDataIndex(const QModelIndex& index) const;

	virtual QModelIndex index(int row, int column, const QModelIndex& parent) const;
	virtual QModelIndex parent(const QModelIndex& index) const;
	virtual int rowCount(const QModelIndex& parent) const;
	virtual int columnCount(const QModelIndex& parent) const;
	virtual QVariant data(const QModelIndex& index, int role) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;
 	virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

	/*! Recursive search for items.
	 *  \param start - parent
	 *  \param flags Allowed flags: Qt::MatchExactly, Qt::MatchContains. Only one flag
	 *  		is allowed, not more, not less.
	 */
	QModelIndexList match(const QModelIndex& start, int role, const QVariant& value, int& hits,
			Qt::MatchFlags flags = Qt::MatchExactly) const;

#ifdef HISTORY_DEBUG_MODELS
	QString indexToStr(const QModelIndex&) const;
	QString flagsToStr(const Qt::ItemFlags&) const;
	void setDebugTooltips(HistoryItem* parent);
#endif

public slots:
	/*! Removes all items.*/
	virtual void clearModel();
	/*! Send signal to view to redraw.*/
	void refreshModel();

protected:
	QPointer<Storage> storage_;
	HistoryItem* root_;

private:
	// disabled
	BaseHistoryModel(const BaseHistoryModel &);
	BaseHistoryModel& operator=(const BaseHistoryModel &);
};

/*! \brief Model for collections. */
class CollectionsModel : public BaseHistoryModel
{
	Q_OBJECT
public:
	CollectionsModel(Storage *storage);
	virtual ~CollectionsModel();

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;
 	virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

 	/*! Is \e index collection or group of collections? */
	bool isCollection(const QModelIndex& index) const;

	/*! Removes collections with entries from model and backend. */
	void removeItems(const QModelIndexList& indexes);

 	void fillModel();

public slots:
	/*! Slot for view. */
	void onItemsSelected(const QModelIndexList& indexes);

signals:
	/*! Signal for dialog. */
	void collectionsForLoad(const IdList& collectionsId);

private:
	CollectionsModel(const CollectionsModel &);
	CollectionsModel& operator=(const CollectionsModel &);

	QModelIndex addOwner(const XMPP::Jid& owner);
 	void addContacts(const XMPP::Jid& owner, const QModelIndex& parent);
 	void addCollections(const XMPP::Jid& owner, const XMPP::Jid& contact, const QModelIndex& parent);

 	IdList collectCollectionsIds(const QModelIndexList& indexes);
};


enum EntriesModelColumn
{
//	StarColumn		= 0,
	TypeColumn		= 0,
	TimeColumn		= 1,
	FromColumn		= 2,
	MessageColumn	= 3,
	LastColumn		= 3
};

/*! \brief Model for entries. */
class EntriesModel : public BaseHistoryModel
{
	Q_OBJECT
public:
	EntriesModel(Storage *storage);
	virtual ~EntriesModel();

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;
 	virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

 	bool isNote(const QModelIndex& index) const;

	const IdList& loadedCollections() const { return loadedCollections_; }

public slots:
	void addEntriesFromCollection(const Id collectionId);
	virtual void clearModel();

private:
	EntriesModel(const EntriesModel &);
	EntriesModel& operator=(const EntriesModel &);

	IdList loadedCollections_;
};

} // namespace

#endif	// _MODELS_H_
