/*
 * models.cpp - models for event history
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
 *b
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <QIcon>

#include "models.h"
using namespace History;

#ifdef HISTORY_DEBUG_MODELS

QString History::roleToString(int role)
{
	QString str;
	switch(role) {
		case Qt::DisplayRole:
			str = "Qt::DisplayRole";
			break;
		case Qt::EditRole:
			str = "Qt::EditRole";
			break;

		case CollectionIdRole:
			str = "CollectionIdRole";
			break;
		case CollectionOwnerJidRole:
			str = "CollectionOwnerJidRole";
			break;
		case CollectionContactJidRole:
			str = "CollectionContactJidRole";
			break;

		case EntryIdRole:
			str = "EntryIdRole";
			break;
		case EntryTypeRole:
			str = "EntryTypeRole";
			break;

		// stuff
		case Qt::DecorationRole:
		case Qt::ToolTipRole:
		case Qt::FontRole:
		case Qt::TextAlignmentRole:
		case Qt::BackgroundRole:
		case Qt::ForegroundRole:
		case Qt::CheckStateRole:
		case Qt::SizeHintRole:
			break;

		default:
			str = QString("role #%1").arg(role);
	}
	return str;
}

#endif

////////////////////////////////////////////////////////////////////////////////////////

CollectionsModel::CollectionsModel(Storage *storage)
		: BaseHistoryModel(storage)
{
	root_->addData(0, Qt::DisplayRole, tr("Collections"));
	fillModel();

#ifdef HISTORY_DEBUG_MODELS
	setDebugTooltips(root_);
#endif
}

CollectionsModel::~CollectionsModel()
{
}

QModelIndex CollectionsModel::addOwner(const XMPP::Jid& owner)
{
	HistoryItem* item = new HistoryItem();
	item->addData(0, Qt::DisplayRole, owner.bare());
	item->addData(0, Qt::DecorationRole, QIcon(":/history/owner.png"));
	item->addData(0, CollectionOwnerJidRole, owner.bare());

	item->setParent(root_);
	root_->appendChild(item);
	return indexFromItem(item, 0);
}

void CollectionsModel::fillModel()
{
	// accounts (owners) - 1 level
	JidList owners = storage_->owners();
	foreach(XMPP::Jid owner, owners) {
		const QModelIndex ownerIndex = addOwner(owner);
		// contacts - 2 level
		addContacts(owner, ownerIndex);
	}
}

void CollectionsModel::addContacts(const XMPP::Jid& owner, const QModelIndex& parent)
{
	CollectionsInfo cols = storage_->collections(owner);
	JidList addedContacts;
	foreach(CollectionInfo col, cols) {
		const XMPP::Jid contact = col.contactJid();
		if(!addedContacts.contains(contact)) {
			addedContacts.append(contact);

			HistoryItem* item = new HistoryItem();
			item->addData(0, Qt::DisplayRole, contact.bare());
			if(col.type() != MucCollection) {
		    	item->addData(0, Qt::DecorationRole, QIcon(":/history/contact_chat.png"));
			} else {
		    	item->addData(0, Qt::DecorationRole, QIcon(":/history/contact_muc.png"));
			}
			item->addData(0, CollectionContactJidRole, contact.bare());

			HistoryItem* p = itemFromIndex(parent);
			item->setParent(p);
			p->appendChild(item);

			// collections - 3 level
			addCollections(owner, contact, indexFromItem(item, 0));
		}
	}
}

void CollectionsModel::addCollections(const XMPP::Jid& owner, const XMPP::Jid& contact, const QModelIndex& parent)
{
	CollectionsInfo cols = storage_->collections(owner, contact);
	foreach(CollectionInfo col, cols) {
		HistoryItem* item = new HistoryItem();
		QString subject = col.subject();
		item->addData(0, Qt::DisplayRole, subject.isEmpty() ? NO_SUBJECT : subject);
		item->addData(0, Qt::EditRole, subject);
    	item->addData(0, Qt::DecorationRole, QIcon(":/history/collection.png"));
		item->addData(0, CollectionIdRole, col.id());

		HistoryItem* p = itemFromIndex(parent);
		item->setParent(p);
		p->appendChild(item);
	}
}

Qt::ItemFlags CollectionsModel::flags(const QModelIndex& index) const
{
	if(!index.isValid()) {
		return Qt::ItemIsEnabled;
	}

	if(!isCollection(index)) {
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool CollectionsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if(isCollection(index) && (role == Qt::EditRole)) {
		const Id collectionId = data(index, CollectionIdRole).toLongLong();
		const QString subject = value.toString();
		storage_->setCollectionSubject(collectionId, subject);
		HistoryItem* item = itemFromIndex(index);
		item->addData(index.column(), Qt::DisplayRole, subject.isEmpty() ? NO_SUBJECT : subject);
		item->addData(index.column(), Qt::EditRole, subject);
		emit dataChanged(index, index);
#ifdef HISTORY_DEBUG_MODELS
		setDebugTooltips(root_);
#endif
		return true;
	}
	return false;
}

bool CollectionsModel::isCollection(const QModelIndex& index) const
{
	if(!index.isValid()) {
		return false;
	}
	return (itemFromIndex(index)->childCount() == 0);
}

bool History::indexLessThan(const QModelIndex& a, const QModelIndex& b)
{
	int levelA = 0, levelB = 0;
	QModelIndex A = a, B = b;
	while(A.isValid()) { ++levelA; A = A.parent(); }
	while(B.isValid()) { ++levelB; B = B.parent(); }
	if(levelA < levelB) {
		return false;
	} else if(levelA > levelB) {
		return true;
	} else if(a.parent() != b.parent()) {
		return (a.parent().row() < b.parent().row());
	} else {
		return (a.row() < b.row());
	}
}

void CollectionsModel::removeItems(const QModelIndexList& indexes)
{
	// groups will be at the end of list
	QList<QPersistentModelIndex> persIndexes;
	foreach(QModelIndex index, indexes) {
		persIndexes.append(QPersistentModelIndex(index));
	}
	qSort(persIndexes.begin(), persIndexes.end(), indexLessThan);

	foreach(QPersistentModelIndex index, persIndexes) {
		if(!index.isValid()) continue; // ok there

		if(!isCollection(index)) {
			HistoryItem* item = itemFromIndex(index);
			QModelIndexList newIndexes;
			const int childCount = item->childCount();
			for(int row=0; row<childCount; ++row) {
				newIndexes.append(indexFromItem(item->child(row), 0));
			}
			removeItems(newIndexes);
		} else {
			// removing from backend
			const Id collectionId = data(index, CollectionIdRole).toLongLong();
			storage_->removeCollection(collectionId);

			// removing from model (with empty groups)
			HistoryItem* item = itemFromIndex(index);
			do
			{
				HistoryItem* parent = item->parent();
				emit beginRemoveRows(indexFromItem(parent, 0), item->row(), item->row());
				parent->removeChild(item);
				emit endRemoveRows();
				delete item;

				if(parent == root_) {
					break;
				}
				item = parent;
			} while(item->childCount() == 0);
		}
	}

#ifdef HISTORY_DEBUG_MODELS
	setDebugTooltips(root_);
#endif
}

IdList CollectionsModel::collectCollectionsIds(const QModelIndexList& indexes)
{
	Q_ASSERT(!indexes.isEmpty());
	IdList ids;
	QModelIndexList collectAfter;
	foreach(QModelIndex index, indexes) {
		HistoryItem* item = itemFromIndex(index);
		const int childCount = item->childCount();

		if(childCount == 0) {
			// collections level
			ids += item->data(0, CollectionIdRole).toLongLong();
		} else {
			for(int row=0; row<childCount; ++row) {
				collectAfter.append(indexFromItem(item->child(row), 0));
			}
		}
	}

	if(!collectAfter.isEmpty()) {
		ids += collectCollectionsIds(collectAfter);
	}
	return ids;
}


void CollectionsModel::onItemsSelected(const QModelIndexList& indexes)
{
	const IdList ids = collectCollectionsIds(indexes);
	emit collectionsForLoad(ids);
}

////////////////////////////////////////////////////////////////////////////////////////

EntriesModel::EntriesModel(Storage *storage)
		: BaseHistoryModel(storage)
{
	clearModel();
}

EntriesModel::~EntriesModel()
{
}

bool EntriesModel::isNote(const QModelIndex& index) const
{
	if(!index.isValid()) {
		return false;
	}
	Q_ASSERT(index.model() == this);
	const QModelIndex dataIndex = userDataIndex(index);
	return (data(dataIndex, EntryTypeRole) == NoteEntry);
}

Qt::ItemFlags EntriesModel::flags(const QModelIndex& index) const
{
	if(!index.isValid()) {
		return Qt::ItemIsEnabled;
	}

	if(!isNote(index) || index.column() != MessageColumn) {
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool EntriesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if(isNote(index) && (role == Qt::EditRole)) {
		const QModelIndex dataIndex = userDataIndex(index);
		const Id entryId = data(dataIndex, EntryIdRole).toLongLong();
		const QString body = value.toString();
		storage_->setEntryBody(entryId, body);
//		const QDateTime utc = QDateTime::currentDateTime().toUTC(); 	<-- this will break sorting
//		entry.setUtc(utc);

		HistoryItem* item = itemFromIndex(index);
		item->addData(MessageColumn,  Qt::DisplayRole, body);
		item->addData(MessageColumn,  Qt::EditRole,    body);
//		item->addData(timeColumn, Qt::DisplayRole, utc.toString(OUTPUT_TIME_FORMAT));	<-- this will break sorting
		QModelIndex begin = this->index(index.row(), 0,          index.parent());
		QModelIndex end   = this->index(index.row(), LastColumn, index.parent());
		emit dataChanged(begin, end);
#ifdef HISTORY_DEBUG_MODELS
		setDebugTooltips(root_);
#endif
		return true;
	}
	return false;
}

void EntriesModel::clearModel()
{
	BaseHistoryModel::clearModel();
//	root_->addData(StarColumn, Qt::DisplayRole, "");	// bookmark's star
	root_->addData(TimeColumn,		Qt::DisplayRole, tr("Time"));
	root_->addData(FromColumn,		Qt::DisplayRole, tr("From"));
	root_->addData(MessageColumn,	Qt::DisplayRole, tr("Message"));
	loadedCollections_.clear();

#ifdef HISTORY_DEBUG_MODELS
	setDebugTooltips(root_);
#endif
}

void EntriesModel::addEntriesFromCollection(const Id collectionId)
{
	CollectionInfo col = storage_->collectionById(collectionId);

	// header
	HistoryItem *header = new HistoryItem(root_->columnCount());
	header->addData(TimeColumn,		Qt::DisplayRole,	col.start());
	header->addData(FromColumn,		Qt::DisplayRole,	col.contactJid().full());
	header->addData(MessageColumn,	Qt::DisplayRole,	col.subject());
	header->addData(0,			CollectionIdRole,	collectionId);
	if (col.type() != MucCollection) {
		header->addData(TypeColumn, Qt::DecorationRole,	QIcon(":/history/contact_chat.png"));
	} else {
		header->addData(TypeColumn, Qt::DecorationRole,	QIcon(":/history/contact_muc.png"));
	}
	header->addDataToRow(Qt::BackgroundRole, CollectionHeaderBrush);
	header->setParent(root_);
	root_->appendChild(header);

	EntriesInfo entries = storage_->entriesByCollectionId(col.id());
	foreach(EntryInfo entry, entries) {
		HistoryItem *item = new HistoryItem();
		item->addData(TimeColumn,		Qt::DisplayRole,    entry.utc().time().toString(OUTPUT_TIME_FORMAT));
		item->addData(FromColumn,		Qt::DisplayRole,    entry.contactNickname());
		item->addData(MessageColumn,	Qt::DisplayRole,    entry.body());
		item->addData(0,			EntryIdRole,        entry.id());
		item->addData(0,			CollectionIdRole,   collectionId);

		const EntryType type = entry.type();
		item->addData(0,			EntryTypeRole,      type);
		if(type == SentMessageEntry) {
			item->addData(TypeColumn, Qt::DecorationRole, QIcon(":/history/msg_sent.png"));
			item->addDataToRow(Qt::BackgroundRole, SentMessageBrush);
		} else if(type == ReceivedMessageEntry) {
			item->addData(TypeColumn, Qt::DecorationRole, QIcon(":/history/msg_recv.png"));
			item->addDataToRow(Qt::BackgroundRole, ReceivedMessageBrush);
		} else if(type == NoteEntry) {
			item->addData(TypeColumn, Qt::DecorationRole, QIcon(":/history/msg_note.png"));
			item->addDataToRow(Qt::BackgroundRole, NoteBrush);
			item->addData(MessageColumn, Qt::EditRole, entry.body());
		}

		item->setParent(root_);
		root_->appendChild(item);
	}
	loadedCollections_.append(collectionId);
}
