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

#include "model_historyitem.h"
#include "model_basehistorymodel.h"

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

bool indexLessThan(const QModelIndex& a, const QModelIndex& b);


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
