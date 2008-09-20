/*
 * rsm.cpp - support for XEP-0059: Result Set Management
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

#include "rsm.h"

const QString RsmSet::RSM_NAMESPACE = "http://jabber.org/protocol/rsm";

RsmSet::RsmSet()
{
}

RsmSet::RsmSet(const QDomElement& inputStanza)
{
	if(inputStanza.tagName() == "set") {
		inputSet_ = inputStanza;
	} else {
		inputSet_ = inputStanza.elementsByTagName("set").item(0).toElement();
	}
	Q_ASSERT(!inputSet_.isNull());
	Q_ASSERT(inputSet_.attribute("xmlns") == RSM_NAMESPACE);
	Q_ASSERT(inputSet_.attributes().count() == 1);
}

QString RsmSet::first() const
{
	Q_ASSERT(!inputSet_.firstChildElement("first").isNull());
	Q_ASSERT(!inputSet_.firstChildElement("first").text().isEmpty());
	return inputSet_.firstChildElement("first").text();
}

int RsmSet::firstIndex() const
{
	Q_ASSERT(!inputSet_.firstChildElement("first").isNull());
	Q_ASSERT(!inputSet_.firstChildElement("first").attribute("index").isNull());
	return inputSet_.firstChildElement("first").attribute("index").toInt();
}

QString RsmSet::last() const
{
	Q_ASSERT(!inputSet_.firstChildElement("last").isNull());
	Q_ASSERT(!inputSet_.firstChildElement("last").text().isEmpty());
	return inputSet_.firstChildElement("last").text();
}

int RsmSet::count() const
{
	Q_ASSERT(!inputSet_.firstChildElement("count").isNull());
	Q_ASSERT(!inputSet_.firstChildElement("count").text().isEmpty());
	return inputSet_.firstChildElement("count").text().toInt();
}

bool RsmSet::lastPage() const
{
	// only 'count', no 'first' or 'last'
	bool countNull = inputSet_.firstChildElement("count").isNull();
	bool firstNull = inputSet_.firstChildElement("first").isNull();
	bool lastNull  = inputSet_.firstChildElement("last").isNull();
	return (!countNull && firstNull && lastNull && !isValid());
}

QDomElement RsmSet::setForFirstPage(QDomDocument* doc, const int max)
{
	QDomElement set = doc->createElement("set");
	set.setTagName("set");
	set.setAttribute("xmlns", RSM_NAMESPACE);

	QDomElement maxEl = doc->createElement("max");
	maxEl.appendChild(doc->createTextNode(QString::number(max)));
	set.appendChild(maxEl);

	return set;
}

QDomElement RsmSet::setForNextPage(QDomDocument* doc, const int max) const
{
	QDomElement set = setForFirstPage(doc, max);

	QDomElement afterEl = doc->createElement("after");
	afterEl.appendChild(doc->createTextNode(last()));
	set.appendChild(afterEl);

	return set;
}
