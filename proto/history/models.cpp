/*
 * models.cpp - models for event history
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
 *b
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <QIcon>

#include "models.h"
#include "meta.h"
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

HistoryItem::HistoryItem(int initialColumnCount)
: parentItem_(this)
{
	for(int i=0; i<initialColumnCount; ++i) {
		itemData_.append(DataCell());
	}
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
	return ( row < childCount() ? childItems_.value(row) : 0);
}

void HistoryItem::setParent(HistoryItem *parent)
{
	Q_ASSERT(parent);
	Q_ASSERT(parent->parent());
	parentItem_ = parent;
}

QVariant HistoryItem::data(const int column, const int role) const
{
	Q_ASSERT(column >= 0);
	Q_ASSERT((column == 0) || (role < Qt::UserRole));	// all custom roles _should_ be in column 0
	const QVariant d = (data(column)).value(role, QVariant());
#ifdef HISTORY_DEBUG_MODELS
	if(!d.isValid() && role >= Qt::UserRole) {
		const QString roleStr = roleToString(role);
		Q_ASSERT_X(!roleStr.isEmpty(), qPrintable(QString("no data for column %1 role %2").arg(column).arg(roleStr)),
				"HistoryItem::data()");
	}
#endif
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
	Q_ASSERT_X(parentItem_, "no parentItem_", "HistoryItem::row");
	return (parentItem_ ? parentItem_->childItems_.indexOf(const_cast<HistoryItem*>(this)) : 0);
}

void HistoryItem::addData(const int column, const int role, const QVariant& data)
{
	Q_ASSERT(column >= 0);
	if(columnCount() <= column) {
		// add empty columns
		for(int i=columnCount(); i<=column; ++i) {
			itemData_.append(DataCell());
		}
	}
	itemData_[column][role] = data;
}

void HistoryItem::addDataToRow(const int role, const QVariant& data)
{
	// to row = to all columns
	for(int i=0; i<columnCount(); ++i) {
		addData(i, role, data);
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
			if(!roleToString(role).isEmpty()) {
				qDebug() << "  " << roleToString(role) << "-" << dataMap.value(role);
			}
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


////////////////////////////////////////////////////////////////////////////////////////

BaseHistoryModel::BaseHistoryModel(Storage *storage)
		: QAbstractItemModel(0), storage_(storage)
{
	Q_ASSERT_X(storage, "BaseHistoryModel::BaseHistoryModel(Storage*)", "storage is NULL");
	root_ = new HistoryItem();
}

BaseHistoryModel::~BaseHistoryModel()
{
	delete root_;
}

void BaseHistoryModel::clearModel()
{
	delete root_;
	root_ = new HistoryItem();
}

HistoryItem* BaseHistoryModel::itemFromIndex(const QModelIndex& index) const
{
	if (!index.isValid()) {
		return root_;
	}
	return static_cast<HistoryItem*>(index.internalPointer());
}

QModelIndex BaseHistoryModel::indexFromItem(HistoryItem* item, int column) const
{
	Q_ASSERT_X(item, "BaseHistoryModel::indexFromItem(HistoryItem*)", "item is NULL");
	return (item == root_ ? QModelIndex() : createIndex(item->row(), column, item));
}

QModelIndex BaseHistoryModel::userDataIndex(const QModelIndex& index) const
{
	if(!index.isValid()) {
		return index;
	}

	// column 0 contains this all
	QModelIndex newIndex = this->index(index.row(), 0, index.parent());
	return newIndex;
}

QModelIndex BaseHistoryModel::index(int row, int column, const QModelIndex& parent) const
{
	HistoryItem* item = itemFromIndex(parent);
	Q_ASSERT(row < item->childCount());
	HistoryItem* childItem = item->child(row);
 	Q_ASSERT(childItem);
	return indexFromItem(childItem, column);
}

QModelIndex BaseHistoryModel::parent(const QModelIndex& index) const
{
	HistoryItem* parentItem = itemFromIndex(index)->parent();	// don't worry, root's parent is root

	if (parentItem == root_) {
		return QModelIndex();
	}

	// only first column can have child items
	return indexFromItem(parentItem, 0);
}

int BaseHistoryModel::rowCount(const QModelIndex& parent) const
{
	// only first column can have child items
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
		return QVariant();
	}
	Q_ASSERT((index.column() == 0) || (role < Qt::UserRole));	// all custom roles _should_ be in column 0
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

	QModelIndexList res;
	if((flags == Qt::MatchExactly) && (data(start, role) == value)) {
		res.append(start);
		if(hits >= 0) {
			++hits;
		}
	} else if(flags == Qt::MatchContains) {
		Q_ASSERT(value.canConvert(QVariant::String));
		QString valueStr = value.toString();
		if(data(start, role).toString().contains(valueStr)) {
			res.append(start);
			if(hits >= 0) {
				++hits;
			}
		}
	}

	HistoryItem* startItem = itemFromIndex(start);
	const int rows = startItem->childCount();
	for(int row=0; row<rows; ++row)
	{
		const HistoryItem* child = startItem->child(row);
		const int cols = child->columnCount();
		for(int col=0; col<cols; ++col)
		{
			if((col > 0) && (role >= Qt::UserRole)) {	// all out custom roles stored in column 0
				break;
			}
			if((hits >= 0) && (res.count() >= hits)) {
				return res;
			}
			res << match(index(row,col,start), role, value, hits, flags);
		}
	}

	return res;
}

void BaseHistoryModel::refreshModel()
{
	reset();
#ifdef HISTORY_DEBUG_MODELS
	setDebugTooltips(root_);
#endif
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
			QString r = roleToString(role);
			if(!r.isEmpty()) {
				toolTip += "<b>" + r + "</b> " + allData.value(role).toString() + "<br>";
			}
		}
	}
	item->addDataToRow(Qt::ToolTipRole, toolTip);

	for(int i=0; i<item->childCount(); ++i) {
		setDebugTooltips(item->child(i));
	}
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
