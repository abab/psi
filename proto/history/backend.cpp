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

#include "backend.h"
#include "xep82datetime.h"
using namespace History;

const char SQLiteWrapper::initConnection[] = ""
        //"PRAGMA locking_mode = EXCLUSIVE;"	// requires attention - only one connection can exists
        "PRAGMA locking_mode = NORMAL;"			// LATER Useful for debug, remove later.
        "PRAGMA synchronous = OFF"				// _MUCH_ faster, but little bit unsafe
        ;

const char SQLiteWrapper::createTables[] =
	"PRAGMA encoding = \"UTF-8\";"	// smaller
	"BEGIN;"

	"CREATE TABLE collections (\n"
	"	collection_id	INTEGER		NOT NULL PRIMARY KEY AUTOINCREMENT,\n"
	"	ownerjid		TEXT,\n"
	"	contactjid		TEXT,\n"
	"	type			INTEGER,\n"
	"	start			TEXT,\n"
	"	subject			TEXT\n"
	");"

	"CREATE INDEX owner_i		ON collections ( ownerjid );"
	"CREATE INDEX contactjid_i	ON collections ( contactjid );"
	"CREATE INDEX start_i		ON collections ( start );"

	"CREATE TABLE entries (\n"
	"	entry_id		INTEGER		NOT NULL PRIMARY KEY AUTOINCREMENT,\n"
	"	collection_id	INTEGER		NOT NULL,\n"
	"	type			INTEGER,\n"
	"	jid				TEXT,\n"
	"	nick			TEXT,\n"
	"	utc				TEXT,\n"
	"	body			TEXT\n"
	");"

	"CREATE INDEX collection_id_i ON entries ( collection_id );"
	"CREATE INDEX utc_i           ON entries ( utc );"

	// SQLite does not create sequences for new (empty) tables. It's bad.
	"INSERT INTO sqlite_sequence ( name, seq ) VALUES( 'entries'    , 0 );"
	"INSERT INTO sqlite_sequence ( name, seq ) VALUES( 'collections', 0 );"

	"CREATE VIEW next_entry_id      AS SELECT seq+1 AS id FROM sqlite_sequence WHERE name='entries'     LIMIT 1;"
	"CREATE VIEW next_collection_id AS SELECT seq+1 AS id FROM sqlite_sequence WHERE name='collections' LIMIT 1;"

	"COMMIT;";

const char SQLiteWrapper::maintanceTables[] = "VACUUM;ANALYZE;";
QStringList SQLiteWrapper::usedDatabases = QStringList();
int SQLiteWrapper::connectionNumber = 0;

SQLiteWrapper* SQLiteWrapper::getNewConnection(const QString& databaseName)
{
	if(usedDatabases.contains(databaseName)) {
		Q_ASSERT_X(false, "SQLiteWrapper::getNewConnection()",
			qPrintable(QString("%1 - already used!").arg(databaseName)));
		return 0;
	}
	usedDatabases.append(databaseName);

	// new wrapper, new connection
	SQLiteWrapper *wr = new SQLiteWrapper;
	wr->connectionName_ = "PsiConnection_" + databaseName + "_" + QString::number(++connectionNumber);
	wr->db_ = QSqlDatabase::addDatabase("QSQLITE", wr->connectionName_);
	wr->db_.setDatabaseName(databaseName);
	wr->databaseName_ = databaseName;

	// init connection
	if(!wr->db_.open()) {
		closeConnection(wr);
		Q_ASSERT_X(false, "SQLiteWrapper::getNewConnection()",
			qPrintable(QString("%1 - can't open database!").arg(databaseName)));
		return 0;
	}

	QStringList list = QString(initConnection).split(";");
	foreach(QString str, list) {
		wr->exec(str);
	}

	wr->createSchemaIfNeeded();
	wr->tablesMaintainance();
	return wr;
}

void SQLiteWrapper::closeConnection(SQLiteWrapper* wrapper)
{
	if (usedDatabases.contains(wrapper->databaseName_)) {

		if (wrapper->connected()) {
			wrapper->db_.close();
		}

		QString connectionName = wrapper->connectionName_;
		usedDatabases.removeAll(wrapper->databaseName_);
		delete wrapper;
		QSqlDatabase::removeDatabase(connectionName);
	}
}

QSqlQuery SQLiteWrapper::exec(const QString& query, const BindedValues& values, const bool mayFail) const
{
	QSqlQuery q(db_);
	if (query.isEmpty() || !connected()) {
		return q;
	}

#ifdef HISTORY_DEBUG_BACKEND
	QTime start = QTime::currentTime();
#endif

	q.prepare(query);
	BindedValues::const_iterator it = values.constBegin();
	while(it != values.constEnd()) {
		q.bindValue(it.key(), it.value());
		++it;
	}

	q.exec();
	if (q.lastError().isValid() && !mayFail) {
		QSqlError err = q.lastError();
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

QSqlQuery SQLiteWrapper::exec(const QueryWithValues& qwv, const bool mayFail) const
{
	return exec(qwv.query, qwv.values, mayFail);
}

void SQLiteWrapper::createSchemaIfNeeded() const
{
	// needed?
	QSqlQuery q = exec("SELECT * FROM collections LIMIT 1", BindedValues(), true);
	if (! q.isActive()) {
		QStringList list = QString(createTables).split(";");
		foreach(QString str, list) {
			exec(str);
		}
	} else {
		// fetch row
		q.first();
		q.next();
	}
}

void SQLiteWrapper::tablesMaintainance() const
{
	QStringList list = QString(maintanceTables).split(";");
	foreach(QString str, list) {
		exec(str);
	}
}


// --------------------------Entry---------------------------------------


EntryInfo::EntryInfo(const Id entryId, const Id collectionId, const EntryType type,
				const XMPP::Jid& jid, const QString& nickname,
				const QString& body, const QDateTime& utc)
: id_(entryId), collectionId_(collectionId), type_(type), jid_(jid), nickname_(nickname), body_(body), utc_(utc)
{
}

EntryInfo::EntryInfo(const EntryInfo& other)
{
	operator=(other);
}

EntryInfo& EntryInfo::operator=(const EntryInfo& entry)
{
	this->id_           = entry.id_;
	this->collectionId_ = entry.collectionId_;
	this->type_         = entry.type_;
	this->jid_          = entry.jid_;
	this->nickname_     = entry.nickname_;
	this->body_         = entry.body_;
	this->utc_          = entry.utc_;
	return *this;
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

CollectionInfo::CollectionInfo(const CollectionInfo& other)
{
	operator=(other);
}

CollectionInfo& CollectionInfo::operator=(const CollectionInfo& other)
{
	this->id_			= other.id_;
	this->type_			= other.type_;
	this->contactJid_	= other.contactJid_;
	this->ownerJid_		= other.ownerJid_;
	this->subject_		= other.subject_;
	this->start_		= other.start_;
	return *this;
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


Storage* Storage::getStorage(const QString& filename)
{
	SQLiteWrapper *wrapper = SQLiteWrapper::getNewConnection(filename);
	if(!wrapper) {
		Q_ASSERT_X(false, "Storage::getStorage()",
			qPrintable(QString("%1 - NO new storage").arg(filename)));
		return 0;
	}
	return (new Storage(wrapper));
}

Storage::Storage(SQLiteWrapper* wrapper) : wrapper_(wrapper)
{
	Q_ASSERT_X(wrapper, "Storage::Storage()", "wrapper is NULL");
}

Storage::~Storage()
{
	SQLiteWrapper::closeConnection(wrapper_);
}

EntryInfo Storage::newEntry(const Id collectionId, const EntryType type, const XMPP::Jid& jid,
								const QString& nickname, const QString& body, const QDateTime& dt)
{
	QString query("SELECT * FROM next_entry_id");
	QSqlQuery q = wrapper_->exec(query);
	q.first();

	EntryInfo entry(q.value(0).toLongLong(), collectionId, type, jid, nickname, body, dt.toUTC());

	query = "INSERT INTO entries ( entry_id,  collection_id,  type,  jid,  nick,  body,  utc ) "
						 "VALUES( :entry_id, :collection_id, :type, :jid, :nick, :body, :utc )";
	BindedValues values;
	values[":entry_id"]			= QString::number(entry.id());
	values[":collection_id"]	= QString::number(entry.collectionId());
	values[":type"]				= QString::number(entry.type());
	values[":jid"]				= entry.contactJid().bare();
	values[":nick"]				= entry.contactNickname();
	values[":body"]				= entry.body();
	values[":utc"]				= dateTimeToXep82Format(entry.utc(), true);

	wrapper_->exec(query, values);
	return entry;
}

EntryInfo Storage::newEntry(const Id collectionId, const EntryInfo& entry)
{
	Q_ASSERT(entry.id() == -1);
	Q_ASSERT(entry.collectionId() == -1);
	Q_ASSERT(collectionId > 0);
	return newEntry(collectionId, entry.type(), entry.contactJid(), entry.contactNickname(),
			entry.body(), entry.utc());
}

EntryInfo Storage::entryById(const Id entryId)
{
	QString query("SELECT entry_id, collection_id, type, jid, nick, body, utc FROM entries "
					"WHERE ( entry_id = :entry_id ) LIMIT 1");

	BindedValues values;
	values[":entry_id"] = QString::number(entryId);

	QSqlQuery q = wrapper_->exec(query, values);
	if(q.first()) {
		const QSqlRecord rec = q.record();
		const Id collectionId = rec.value("collection_id").toLongLong();
		const EntryType type = static_cast<EntryType>(rec.value("type").toInt());
		const XMPP::Jid jid = rec.value("jid").toString();
		const QString nickname = rec.value("nick").toString();
		const QString body = rec.value("body").toString();
		QDateTime dt = xep82FormatToDateTime(rec.value("utc").toString());
		dt.setTimeSpec(Qt::UTC);
		EntryInfo entry(entryId, collectionId, type, jid, nickname, body, dt);
		return entry;
	} else {
		qCritical() << entryId;
		Q_ASSERT_X(false, "Can't find entry", "Storage::entryById");
		return EntryInfo();
	}
}

EntriesInfo Storage::entriesByCollectionId(const Id collectionId)
{
	QString query("SELECT entry_id, collection_id, type, jid, nick, body, utc FROM entries "
					"WHERE ( collection_id = :collection_id )");

	BindedValues values;
	values[":collection_id"] = QString::number(collectionId);

	QSqlQuery q = wrapper_->exec(query, values);
	EntriesInfo entries;
	if(q.first()) {
		while(q.isValid()) {
			const QSqlRecord rec = q.record();
			const Id entryId = rec.value("entry_id").toLongLong();
			const Id collectionId = rec.value("collection_id").toLongLong();
			const EntryType type = static_cast<EntryType>(rec.value("type").toInt());
			const XMPP::Jid jid = rec.value("jid").toString();
			const QString nickname = rec.value("nick").toString();
			const QString body = rec.value("body").toString();
			QDateTime dt = xep82FormatToDateTime(rec.value("utc").toString());
			dt.setTimeSpec(Qt::UTC);
			EntryInfo entry(entryId, collectionId, type, jid, nickname, body, dt);
			entries.append(entry);
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
	QString query("UPDATE entries SET body = :body WHERE entry_id = :entry_id");

	BindedValues values;
	values[":entry_id"]	= QString::number(entryId);
	values[":body"]		= body;

	wrapper_->exec(query, values);
}

void Storage::removeEntry(const Id entryId)
{
	QString query("DELETE FROM entries WHERE entry_id = :entry_id");

	BindedValues values;
	values[":entry_id"]	= QString::number(entryId);

	wrapper_->exec(query, values);
}


CollectionInfo Storage::newCollection(const CollectionType type, const XMPP::Jid& ownerJid,
								const XMPP::Jid& contactJid, const QDateTime& start)
{
	QString query("SELECT * FROM next_collection_id");
	QSqlQuery q = wrapper_->exec(query);
	q.first();

	CollectionInfo col(q.value(0).toLongLong(), type, ownerJid, contactJid, "", start);

	query = "INSERT INTO collections (   collection_id,  ownerjid,  contactjid,  type,  start,  subject) "
								"VALUES(:collection_id, :ownerjid, :contactjid, :type, :start, '')";

	BindedValues values;
	values[":collection_id"]= QString::number(q.value(0).toLongLong());
	values[":ownerjid"]		= ownerJid.bare();
	values[":contactjid"]	= contactJid.bare();
	values[":type"]			= QString::number(type);
	values[":start"]		= dateTimeToXep82Format(start.toUTC(), true);
	wrapper_->exec(query, values);

	return col;
}

CollectionInfo Storage::newCollection(const CollectionInfo& col)
{
	Q_ASSERT(col.id() == -1);
	return newCollection(col.type(), col.ownerJid(), col.contactJid(), col.start());
}

CollectionInfo Storage::collectionById(const Id collectionId)
{
	QString query("SELECT collection_id, ownerjid, contactjid, type, start, subject FROM collections "
					"WHERE collection_id = :collection_id LIMIT 1");

	BindedValues values;
	values[":collection_id"]= QString::number(collectionId);

	QSqlQuery q = wrapper_->exec(query, values);
	if(q.first()) {
		const QSqlRecord rec = q.record();
		const CollectionType type = static_cast<CollectionType>(rec.value("type").toInt());
		const XMPP::Jid ownerJid = rec.value("ownerjid").toString();
		const XMPP::Jid contactJid = rec.value("contactjid").toString();
		const QString subject = rec.value("subject").toString();
		QDateTime start = xep82FormatToDateTime(rec.value("start").toString());
		start.setTimeSpec(Qt::UTC);

		CollectionInfo col(collectionId, type, ownerJid, contactJid, subject, start);
		return col;
	} else {
		qCritical() << collectionId;
		Q_ASSERT_X(false, "Can't find collection", "Storage::collectionById");
		return CollectionInfo();
	}
}

CollectionsInfo Storage::collections(const XMPP::Jid& owner, const XMPP::Jid& contact)
{
	QString query("SELECT collection_id, ownerjid, contactjid, type, start, subject FROM collections");
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
		const QSqlRecord rec = q.record();
		const Id collectionId = rec.value("collection_id").toLongLong();
		const CollectionType type = static_cast<CollectionType>(rec.value("type").toInt());
		const XMPP::Jid ownerJid = rec.value("ownerjid").toString();
		const XMPP::Jid contactJid = rec.value("contactjid").toString();
		const QString subject = rec.value("subject").toString();
		QDateTime start = xep82FormatToDateTime(rec.value("start").toString());
		start.setTimeSpec(Qt::UTC);

		CollectionInfo col(collectionId, type, ownerJid, contactJid, subject, start);
		cols.append(col);
		q.next();
	}
	return cols;
}

void Storage::setCollectionSubject(const Id collectionId, const QString& subject)
{
	QString query("UPDATE collections SET subject = :subject WHERE collection_id = :collection_id");

	BindedValues values;
	values[":subject"]		= subject;
	values[":collection_id"]= QString::number(collectionId);

	wrapper_->exec(query, values);
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
