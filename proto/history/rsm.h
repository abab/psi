/*
 * rsm.h - support for XEP-0059: Result Set Management
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

#ifndef RSM_H
#define RSM_H

#include <QDomElement>
#include <QDebug>

/*! \brief Class for work with Resource Set Management (XEP-0059). */
class RsmSet
{
	// no Q_OBJECT - pure C++ class
public:
	/*! Creates empty invalid set. */
	RsmSet();
	/*! Extracts RSM set from inputStanza.*/
	RsmSet(const QDomElement& inputStanza);
	/*! Returns true for set created by default constructor. */
	bool isValid() const { return inputSet_.isNull(); }

	/*! Returns UID for first element in this page. */
	QString first() const;
	/*! Returns UID for last element in this page. */
	QString last() const;
	/*! Returns index of the first element. */
	int firstIndex() const;
	/*! Returns count of elements in whole result set (not in this page). */
	int count() const;
	/*! Returns true if this page is the last. Also this page is empty. */
	bool lastPage() const;

	/*! Returns 'set' XML tag for receive first page of set.
	 *	\param doc - XMPP::Task::doc() is common value.
	 *	\param max - max count of elements per page.
	 */
	static QDomElement setForFirstPage(QDomDocument* doc, const int max);
	/*! Returns 'set' XML tag for receive next page of set. */
	QDomElement setForNextPage(QDomDocument* doc, const int max) const;

private:
	QDomElement inputSet_;

	static const QString RSM_NAMESPACE;
};

#endif	// RSM_H
