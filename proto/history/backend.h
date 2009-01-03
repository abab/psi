/*
 * backend.h - backend for new event-logging system
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

#ifndef _BACKEND_H_
#define _BACKEND_H_

#include <QSqlDatabase>
#include <QVariant>
#include <QDateTime>
#include <QStringList>
#include <QPointer>

#include <xmpp_jid.h>

/*! \namespace History
 *  \brief Namespace of new event-logging system.
 *  \author Aleksey Palazhchenko
 *  \version 1.0
 *  \date Google's Summer of Code 2008 and later
 */
namespace History
{

/*! Primary key in database. */
typedef qint64 Id;

/*! List of primary keys in database. */
typedef QList<Id> IdList;

typedef QMap<QString, QString> BindedValues;

struct QueryWithValues
{
	QString query;
	BindedValues values;
	QueryWithValues(const QString& q, const BindedValues& v = BindedValues()) :
		query(q), values(v) { }
};

class Storage;

//////////////////////////////////////////////////////////////////////////////////////////////

/*! \brief Wrapper for SQLite database.
 *
 *  No public interface. Class is used by friend: Storage.
 */
class SQLiteWrapper
{
	// no Q_OBJECT - pure C++ class
	Q_DISABLE_COPY(SQLiteWrapper)
	friend class Storage;

private:
	SQLiteWrapper(const QString& databaseName);
	~SQLiteWrapper();

	/*! Executes SQL query.
	 *  \param query - query with placeholders for values.
	 *  \param values - pairs placeholder/value.
	 *  \param mayFail - true, if it's ok for query to fail.
	 */
	QSqlQuery exec(const QString& query, const BindedValues& values=BindedValues(), const bool mayFail=false) const;
	QSqlQuery exec(const QueryWithValues& qwv, const bool mayFail=false) const;

	/*! Low-level stuff: locking_mode, synchronous, etc.*/
	void initConnection() const;
	/*! Creates tables, views, etc if needed.*/
	void createSchemaIfNeeded() const;
	/*! VACUUM and ANALYZE.*/
	void tablesMaintainance() const;

private:
	QSqlDatabase db_;
};

//////////////////////////////////////////////////////////////////////////////////////////////

/*! Type of entry. */
enum EntryType {
	SentMessageEntry		= 2,	/*!< Message from user (owner). */
	ReceivedMessageEntry	= 3,	/*!< Message to user (owner). */
	SystemMessageEntry		= 4,	/*!< System message. */
	NoteEntry				= 5,	/*!< Private note (may be added by user). */
	PubSubEntry				= 50	// LATER rename and give a description
};

/*! \brief Info about one entry. */
class EntryInfo
{
	// no Q_OBJECT - pure C++ class
	friend class Storage;

public:
	EntryInfo(const EntryInfo& other);
	EntryInfo& operator=(const EntryInfo& other);

	/*! Primary key at database. */
	Id id() const;

	/*! Returns a collection's id. */
	Id collectionId() const;

	/*! Returns type of message. */
	EntryType type() const;

	/*! Returns message text. */
	QString body() const;

	/*! Returns time (UTC) of message. */
	QDateTime utc() const;

	XMPP::Jid contactJid() const;

	QString contactNickname() const;

private:
	/*! Private constructor used by Storage. */
	EntryInfo(const Id entryId, const Id collectionId, const EntryType type,
			const XMPP::Jid& jid, const QString& nickname,
			const QString& body, const QDateTime& utc);

private:
	Id id_;
	Id collectionId_;
	EntryType type_;
	XMPP::Jid jid_;
	QString nickname_;
	QString body_;
	QDateTime utc_;
};

typedef QList<EntryInfo> EntriesInfo;


//////////////////////////////////////////////////////////////////////////////////////////////


/*! Type of Collection. */
enum CollectionType {
//	NormalCollection  = 1,
	ChatCollection    = 2,	/*!< Chat. */
	MucCollection     = 3,	/*!< Muc. */
	PubSubCollection  = 50 // LATER rename and give a description
};

/*! \brief Info about one collection. */
class CollectionInfo
{
	// no Q_OBJECT - pure C++ class
	friend class Storage;

public:
	CollectionInfo(const CollectionInfo& collection);
	CollectionInfo& operator=(const CollectionInfo& collection);

	/*! Primary key at database. */
	Id id() const;

	/*! Returns contacts's JID. */
	XMPP::Jid contactJid() const;

	/*! Returns collection ownerJid's JID. */
	XMPP::Jid ownerJid() const;

	/*! Returns type of conversation. */
	CollectionType type() const;

	/*! Returns start time (UTC) of conversation. */
	QDateTime start() const;

	/*! Returns subject of collection. */
	QString subject() const;

private:
	/*! Private constructor used by Storage. */
	CollectionInfo(const Id collectionId, const CollectionType type,
				const XMPP::Jid& ownerJid, const XMPP::Jid& contactJid,
				const QString& subject, const QDateTime& start);

private:
	Id id_;
	CollectionType type_;
	XMPP::Jid ownerJid_;
	XMPP::Jid contactJid_;
	QString subject_;
	QDateTime start_;
};

/*! QList of collections. */
typedef QList<CollectionInfo> CollectionsInfo;

// LATER move this to Iris?
typedef QList<XMPP::Jid> JidList;


/*! \brief Place to store collections, messages and bookmarks.*/
class Storage : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(Storage)

public:
	/*! Creates new storage.
	 *  \return Pointer to new storage.
	 */
	static Storage* getStorage(const QString& databaseName);
	/*! Returns existed storage. */
	static Storage* getStorage();
	virtual ~Storage();

	/*! Creates new entry.
	 *  \param collectionId - primary key of parent collection or -1.
	 *  \param type - type of message.
	 *  \param jid - jid of sender.
	 *  \param nickname - nickname of sender (useful in MUCs).
	 *  \param body - message text.
	 *  \param dt - time (UTC) of message.
	 *  \return New message.
	 */
	EntryInfo newEntry(const Id collectionId, const EntryType type, const XMPP::Jid& jid,
						const QString& nickname, const QString& body, const QDateTime& dt);

	/*! Returns entry with specified id.*/
	EntryInfo entryById(const Id entryId);

	/*! Returns all entries in collection. They are sorted by UTC.*/
	EntriesInfo entriesByCollectionId(const Id collectionId);

	void setEntryBody(const Id entryId, const QString& body);

	/*! Removes entry.*/
	void removeEntry(const Id entryId);


	/*! Creates new collection. */
	CollectionInfo newCollection(const CollectionType type, const XMPP::Jid& ownerJid,
								  const XMPP::Jid& contactJid, const QDateTime& start);

	/*! Returns collection with specified id.*/
	CollectionInfo collectionById(const Id collectionId);

	CollectionsInfo collections(const XMPP::Jid& ownerJid = XMPP::Jid(), const XMPP::Jid& contactJid = XMPP::Jid());

	void setCollectionSubject(const Id collectionId, const QString& subject);

	/*! Removes collection <b>with all entries</b>.*/
	void removeCollection(const Id collectionId);

	JidList owners();

private:
	/*! Private constructor. Used by getStorage(). */
	Storage(const QString& databaseName);

private:
	static QPointer<Storage> instance_;
	static SQLiteWrapper* wrapper_;
};

} // namespace

#endif // _BACKEND_H_
