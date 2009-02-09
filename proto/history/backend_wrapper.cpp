/*
 * backend_wrapper.cpp - wrapper for SQLite database
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

#include "backend_wrapper.h"
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
