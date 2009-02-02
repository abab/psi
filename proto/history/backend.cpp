/*
 * backend.cpp - backend for new event-logging system
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

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>

#include "backend.h"
#include "xep82datetime.h"
using namespace History;

SQLiteWrapper::SQLiteWrapper(const QString& databaseName)
{
	db_ = QSqlDatabase::addDatabase("QSQLITE", "PsiConnection_" + databaseName + "_" + QString::number(qrand()));
	db_.setDatabaseName(databaseName);
	const bool opened = db_.open();
	Q_ASSERT(opened);

	initConnection();
	createSchemaIfNeeded();
	tablesMaintainance();
}

SQLiteWrapper::~SQLiteWrapper()
{
	// destructor of QSqlDatabase will do everything for us
}

QSqlQuery SQLiteWrapper::exec(const QString& query, const BindedValues& values, const bool mayFail) const
{
	Q_ASSERT(db_.isOpen());
	Q_ASSERT(!query.isEmpty());
	QSqlQuery q(db_);

#ifdef HISTORY_DEBUG_BACKEND
	const QTime start = QTime::currentTime();
#endif

	q.prepare(query);
	BindedValues::const_iterator it = values.constBegin();
	while(it != values.constEnd()) {
		q.bindValue(it.key(), it.value());
		++it;
	}

	q.exec();
	if (! (q.isActive() || mayFail)) {
		const QSqlError err = q.lastError();
		qCritical("\n ------------- SQLiteWrapper::exec error ------------- ");
		qCritical() << q.lastQuery();
		qCritical() << values;
		qCritical() << q.executedQuery();
		qCritical("driverText = %s", qPrintable(err.driverText()));
		qCritical("databaseText = %s", qPrintable(err.databaseText()));
		qCritical(" ------------- ------------------------- ------------- \n");
		Q_ASSERT(false);
	}

#ifdef HISTORY_DEBUG_BACKEND
	const int secs = start.secsTo(QTime::currentTime());
	if(secs > 0) {
		qDebug("[%ds]\t%s", start.secsTo(QTime::currentTime()), qPrintable(query + ";"));
		if(!values.isEmpty()) {
			qDebug() << values;
		}
	}
#endif

	return q;
}

void SQLiteWrapper::initConnection() const
{
#ifdef HISTORY_DEBUG_BACKEND
	// we can use sqlite3 for deep view
	exec("PRAGMA locking_mode = NORMAL;");
#else
	exec("PRAGMA locking_mode = EXCLUSIVE;");
#endif
	exec("PRAGMA synchronous = OFF");
}

void SQLiteWrapper::createSchemaIfNeeded() const
{
	QSqlQuery q = exec("SELECT * FROM collections LIMIT 1", BindedValues(), true);
	if (! q.isActive()) {
		exec("PRAGMA encoding = \"UTF-8\"");
		exec("BEGIN");

		// collections
		exec("CREATE TABLE collections (\n"
				"collection_id	INTEGER		NOT NULL PRIMARY KEY AUTOINCREMENT,\n"
				"ownerjid		TEXT,\n"
				"contactjid		TEXT,\n"
				"type			INTEGER,\n"
				"start			TEXT,\n"
				"subject		TEXT\n"
			")");
		exec("CREATE INDEX owner_i		ON collections ( ownerjid )");
		exec("CREATE INDEX contactjid_i	ON collections ( contactjid )");
		exec("CREATE INDEX start_i		ON collections ( start )");

		// entries
		exec("CREATE TABLE entries (\n"
				"entry_id		INTEGER		NOT NULL PRIMARY KEY AUTOINCREMENT,\n"
				"collection_id	INTEGER		NOT NULL,\n"
				"type			INTEGER,\n"
				"jid			TEXT,\n"
				"nick			TEXT,\n"
				"utc			TEXT,\n"
				"body			TEXT\n"
			")");
		exec("CREATE INDEX collection_id_i	ON entries ( collection_id )");
		exec("CREATE INDEX utc_i			ON entries ( utc )");

		// SQLite does not create sequences for new (empty) tables. It's bad.
		// MAYBE better workaround?
		exec("INSERT INTO sqlite_sequence ( name, seq ) VALUES( 'entries',		0 )");
		exec("INSERT INTO sqlite_sequence ( name, seq ) VALUES( 'collections',	0 )");
		exec("CREATE VIEW next_entry_id			AS SELECT seq+1 AS id FROM sqlite_sequence WHERE name='entries'		LIMIT 1");
		exec("CREATE VIEW next_collection_id	AS SELECT seq+1 AS id FROM sqlite_sequence WHERE name='collections'	LIMIT 1");

		exec("COMMIT");
	} else {
		q.finish();
	}
}

void SQLiteWrapper::tablesMaintainance() const
{
	exec("VACUUM");
	exec("ANALYZE");
}


// --------------------------Entry---------------------------------------


EntryInfo::EntryInfo(const Id entryId, const Id collectionId, const EntryType type,
				const XMPP::Jid& jid, const QString& nickname,
				const QString& body, const QDateTime& utc)
: id_(entryId), collectionId_(collectionId), type_(type), jid_(jid), nickname_(nickname), body_(body), utc_(utc)
{
}

bool EntryInfo::operator==(const EntryInfo& right) const
{
	return (id()				== right.id() &&
			collectionId()		== right.collectionId() &&
			type()				== right.type() &&
			body()				== right.body() &&
			utc()				== right.utc() &&
			contactJid()		== right.contactJid() &&
			contactNickname()	== right.contactNickname());
}

Id EntryInfo::id() const
{
	return id_;
}

Id EntryInfo::collectionId() const
{
	return collectionId_;
}

EntryType EntryInfo::type() const
{
	return type_;
}

QString EntryInfo::body() const
{
	return body_;
}

QDateTime EntryInfo::utc() const
{
	return utc_;
}

XMPP::Jid EntryInfo::contactJid() const
{
	return jid_;
}

QString EntryInfo::contactNickname() const
{
	return nickname_;
}


// --------------------------Collection---------------------------------------


CollectionInfo::CollectionInfo(const Id collectionId, const CollectionType type,
						const XMPP::Jid& ownerJid, const XMPP::Jid& contactJid,
						const QString& subject, const QDateTime& start)
: id_(collectionId), type_(type), ownerJid_(ownerJid), contactJid_(contactJid),
	subject_(subject), start_(start)
{
}

bool CollectionInfo::operator==(const CollectionInfo& right) const
{
	return (id()				== right.id() &&
			contactJid()		== right.contactJid() &&
			ownerJid()			== right.ownerJid() &&
			type()				== right.type() &&
			start()				== right.start() &&
			subject()			== right.subject());
}

Id CollectionInfo::id() const
{
	return id_;
}

XMPP::Jid CollectionInfo::contactJid() const
{
	return contactJid_;
}

XMPP::Jid CollectionInfo::ownerJid() const
{
	return ownerJid_;
}

CollectionType CollectionInfo::type() const
{
	return type_;
}

QDateTime CollectionInfo::start() const
{
	return start_;
}

QString CollectionInfo::subject() const
{
	return subject_;
}


// --------------------------Storage------------------------------------------

QPointer<Storage> Storage::instance_ = 0;
SQLiteWrapper* Storage::wrapper_ = 0;

Storage* Storage::getStorage(const QString& databaseName)
{
	Q_ASSERT(!instance_);
	instance_ = new Storage(databaseName);
	return instance_;
}

Storage* Storage::getStorage()
{
	Q_ASSERT(instance_);
	return instance_;
}

Storage::Storage(const QString& databaseName)
{
	wrapper_ = new SQLiteWrapper(databaseName);
	Q_ASSERT(wrapper_);
}

Storage::~Storage()
{
	delete wrapper_;
	wrapper_ = 0;
}

EntryInfo Storage::entryFromRecord(const QSqlRecord& rec)
{
	const Id entryId		= rec.value("entry_id").toLongLong();
	const Id collectionId	= rec.value("collection_id").toLongLong();
	const EntryType type	= static_cast<EntryType>(rec.value("type").toInt());
	const XMPP::Jid jid		= rec.value("jid").toString();
	const QString nickname	= rec.value("nick").toString();
	const QString body		= rec.value("body").toString();
	QDateTime dt			= xep82FormatToDateTime(rec.value("utc").toString());
	dt.setTimeSpec(Qt::UTC);
	return EntryInfo(entryId, collectionId, type, jid, nickname, body, dt);
}

CollectionInfo Storage::collectionFromRecord(const QSqlRecord& rec)
{
	const Id collectionId		= rec.value("collection_id").toLongLong();
	const CollectionType type	= static_cast<CollectionType>(rec.value("type").toInt());
	const XMPP::Jid ownerJid	= rec.value("ownerjid").toString();
	const XMPP::Jid contactJid	= rec.value("contactjid").toString();
	const QString subject		= rec.value("subject").toString();
	QDateTime start				= xep82FormatToDateTime(rec.value("start").toString());
	start.setTimeSpec(Qt::UTC);
	return CollectionInfo(collectionId, type, ownerJid, contactJid, subject, start);
}

EntryInfo Storage::newEntry(const Id collectionId, const EntryType type, const XMPP::Jid& jid,
								const QString& nickname, const QString& body, const QDateTime& dt)
{
	QSqlQuery q = wrapper_->exec("SELECT * FROM next_entry_id");
	q.first();
	const Id entryId = q.value(0).toLongLong();

	QString query = "INSERT INTO entries (	 entry_id,	 collection_id,	 type,	 jid,	 nick,	 body,	 utc ) "
									"VALUES(:entry_id,	:collection_id,	:type,	:jid,	:nick,	:body,	:utc )";
	BindedValues values;
	values[":entry_id"]			= QString::number(entryId);
	values[":collection_id"]	= QString::number(collectionId);
	values[":type"]				= QString::number(type);
	values[":jid"]				= jid.full();
	values[":nick"]				= nickname;
	values[":body"]				= body;
	values[":utc"]				= dateTimeToXep82Format(dt.toUTC(), true);

	wrapper_->exec(query, values);
	return EntryInfo(entryId, collectionId, type, jid, nickname, body, dt.toUTC());
}

EntryInfo Storage::entryById(const Id entryId)
{
	const QString query("SELECT entry_id, collection_id, type, jid, nick, body, utc FROM entries "
					"WHERE ( entry_id = :entry_id ) LIMIT 1");

	BindedValues values;
	values[":entry_id"] = QString::number(entryId);

	QSqlQuery q = wrapper_->exec(query, values);
	if(q.first()) {
		return entryFromRecord(q.record());
	} else {
		qCritical() << entryId;
		Q_ASSERT_X(false, "Can't find entry", "Storage::entryById");
		return EntryInfo(-1, -1, SystemMessageEntry, XMPP::Jid(), QString(), QString(), QDateTime());	// we should die there
	}
}

EntriesInfo Storage::entriesByCollectionId(const Id collectionId)
{
	const QString query("SELECT entry_id, collection_id, type, jid, nick, body, utc FROM entries "
					"WHERE ( collection_id = :collection_id )");

	BindedValues values;
	values[":collection_id"] = QString::number(collectionId);

	QSqlQuery q = wrapper_->exec(query, values);
	EntriesInfo entries;
	if(q.first()) {
		while(q.isValid()) {
			entries.append(entryFromRecord(q.record()));
			q.next();
		}
	} else {
		qCritical() << collectionId;
		Q_ASSERT_X(false, "Can't find entries", "Storage::entriesByCollectionId");
	}
	return entries;
}

void Storage::setEntryBody(const Id entryId, const QString& body)
{
	BindedValues values;
	values[":entry_id"]	= QString::number(entryId);
	values[":body"]		= body;

	wrapper_->exec("UPDATE entries SET body = :body WHERE entry_id = :entry_id", values);
}

void Storage::removeEntry(const Id entryId)
{
	BindedValues values;
	values[":entry_id"]	= QString::number(entryId);

	wrapper_->exec("DELETE FROM entries WHERE entry_id = :entry_id", values);
}


CollectionInfo Storage::newCollection(const CollectionType type, const XMPP::Jid& ownerJid,
								const XMPP::Jid& contactJid, const QDateTime& start)
{
	QSqlQuery q = wrapper_->exec("SELECT * FROM next_collection_id");
	q.first();
	const Id collectionId = q.value(0).toLongLong();

	QString query = "INSERT INTO collections (	 collection_id,	 ownerjid,	 contactjid,	 type,	 start) "
										"VALUES(:collection_id,	:ownerjid,	:contactjid,	:type,	:start)";

	BindedValues values;
	values[":collection_id"]= QString::number(collectionId);
	values[":ownerjid"]		= ownerJid.full();
	values[":contactjid"]	= contactJid.full();
	values[":type"]			= QString::number(type);
	values[":start"]		= dateTimeToXep82Format(start.toUTC(), true);
	wrapper_->exec(query, values);

	return CollectionInfo(collectionId, type, ownerJid, contactJid, QString(), start.toUTC());
}

CollectionInfo Storage::collectionById(const Id collectionId)
{
	const QString query("SELECT collection_id, ownerjid, contactjid, type, start, subject FROM collections "
					"WHERE collection_id = :collection_id LIMIT 1");

	BindedValues values;
	values[":collection_id"]= QString::number(collectionId);

	QSqlQuery q = wrapper_->exec(query, values);
	if(q.first()) {
		return collectionFromRecord(q.record());
	} else {
		qCritical() << collectionId;
		Q_ASSERT_X(false, "Can't find collection", "Storage::collectionById");
		return CollectionInfo(-1, ChatCollection, XMPP::Jid(), XMPP::Jid(), QString(), QDateTime());	// we should die there
	}
}

CollectionsInfo Storage::collections(const XMPP::Jid& owner, const XMPP::Jid& contact)
{
	const QString query("SELECT collection_id, ownerjid, contactjid, type, start, subject FROM collections");
	QString where;
	BindedValues values;
	if(!owner.isNull()) {
		where += " WHERE (ownerjid = :ownerjid)";
		values[":ownerjid"] = owner.bare();
	}
	if(!contact.isNull()) {
		if(where.isEmpty()) {
			where += " WHERE";
		} else {
			where += " AND";
		}
		where += " (contactjid = :contactjid)";
		values[":contactjid"] = contact.bare();
	}

	QSqlQuery q = wrapper_->exec(query + where, values);
	q.first();
	CollectionsInfo cols;
	while(q.isValid()) {
		cols.append(collectionFromRecord(q.record()));
		q.next();
	}
	return cols;
}

void Storage::setCollectionSubject(const Id collectionId, const QString& subject)
{
	BindedValues values;
	values[":subject"]		= subject;
	values[":collection_id"]= QString::number(collectionId);

	wrapper_->exec("UPDATE collections SET subject = :subject WHERE collection_id = :collection_id", values);
}

void Storage::removeCollection(const Id collectionId)
{
	BindedValues values;
	values[":collection_id"]= QString::number(collectionId);

	wrapper_->exec("DELETE FROM entries WHERE collection_id = :collection_id", values);
	wrapper_->exec("DELETE FROM collections WHERE collection_id = :collection_id", values);
}

JidList Storage::owners()
{
	JidList list;
	QString query("SELECT DISTINCT ownerjid FROM collections");
	QSqlQuery q = wrapper_->exec(query);
	q.first();
	while(q.isValid()) {
		const QSqlRecord rec = q.record();
		list.append(XMPP::Jid(rec.value("ownerjid").toString()));
		q.next();
	}

	return list;
}
