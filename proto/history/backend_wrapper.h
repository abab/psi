/*
 * backend_wrapper.h - wrapper for SQLite database
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

#ifndef _BACKEND_WRAPPER_H_
#define _BACKEND_WRAPPER_H_

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMap>

namespace History
{

typedef QMap<QString, QString> BindedValues;

class Storage;

//////////////////////////////////////////////////////////////////////////////////////////////

/*! \brief Wrapper for SQLite database.
 *	No public interface. Class is used by friend: Storage.
 */
class SQLiteWrapper
{
	// no Q_OBJECT - pure C++ class
	Q_DISABLE_COPY(SQLiteWrapper)
	friend class Storage;

private:
	explicit SQLiteWrapper(const QString& databaseName);
	~SQLiteWrapper();

	/*! Executes SQL query.
	 * \param query - query with placeholders for values.
	 * \param values - pairs placeholder/value.
	 * \param mayFail - true, if it's ok for query to fail.
	 */
	QSqlQuery exec(const QString& query, const BindedValues& values=BindedValues(), const bool mayFail=false) const;

	/*! Low-level stuff: locking_mode, synchronous, etc.*/
	void initConnection() const;
	/*! Creates tables, views, etc if needed.*/
	void createSchemaIfNeeded() const;
	/*! VACUUM and ANALYZE.*/
	void tablesMaintainance() const;

private:
	QSqlDatabase db_;
};

} // namespace

#endif // _BACKEND_WRAPPER_H_
