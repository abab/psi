/*
 * archivetask.cpp
 * Copyright (C) 2008 Aleksey Palazchenko
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <QDebug>

#include "xmpp_task.h"
#include "xmpp_jid.h"
#include "xmpp_xmlcommon.h"

#include "archivetask.h"

using namespace History;

const int ArchiveTask::RSM_MAX = 30;

ArchiveTask::ArchiveTask(Task* parent, const TaskType type, const QString& xmlns, const RsmSet& setForPrevPage)
: Task(parent), type_(type), xmlns_(xmlns)
{
	int rsmMax = RSM_MAX;
	switch (type)
	{
		case DetectSupportTask:
			rsmMax = 1;
			// no break
		case RetrListOfCollectionsTask:
			forSend_ = doc()->createElement("list");
			break;
		case RetrCollectionTask:
			forSend_ = doc()->createElement("retrieve");
			break;
		default:
			Q_ASSERT_X(false, "Unknown task", "ArchiveTask::ArchiveTask");
	}

	forSend_.setAttribute("xmlns", xmlns_);
	if(setForPrevPage.isValid()) {
		forSend_.appendChild(RsmSet::setForFirstPage(doc(), rsmMax));
	} else {
		forSend_.appendChild(setForPrevPage.setForNextPage(doc(), rsmMax));
	}
}

ArchiveTask::~ArchiveTask()
{
}

void ArchiveTask::setWith(const XMPP::Jid& with)
{
	forSend_.setAttribute("with", with.full());
}

void ArchiveTask::setStart(const QDateTime& start)
{
	forSend_.setAttribute("start", dateTimeToXep82Format(start, true));
}

void ArchiveTask::setEnd(const QDateTime& end)
{
	Q_ASSERT(type_ == RetrListOfCollectionsTask);
	forSend_.setAttribute("end", dateTimeToXep82Format(end, true));
}

int ArchiveTask::receivedCollectionsCount() const
{
	Q_ASSERT(type() == RetrListOfCollectionsTask);
	return received_.elementsByTagName("chat").count();
}

CollectionInfo ArchiveTask::receivedCollection(const int collectionNo) const
{
	Q_ASSERT(type() == RetrListOfCollectionsTask);
	Q_ASSERT(collectionNo>=0);
	Q_ASSERT(collectionNo<receivedCollectionsCount());

	const QDomElement chatElement = received_.elementsByTagName("chat").at(collectionNo).toElement();
	const CollectionType type = ChatCollection;	// TODO how to detect type?
	const XMPP::Jid ownerJid = "test@test.com";	// FIXME jid
	const XMPP::Jid contactJid = chatElement.attribute("with");
	const QString subject = chatElement.attribute("subject");
	const QDateTime start = xep82FormatToDateTime(chatElement.attribute("start"));
	return CollectionInfo(-1, type, ownerJid, contactJid, subject, start);
}

int ArchiveTask::receivedEntriesCount() const
{
	Q_ASSERT(type() == RetrCollectionTask);
	const int from = received_.elementsByTagName("from").count();
	const int to = received_.elementsByTagName("to").count();
	const int note = received_.elementsByTagName("note").count();
	Q_ASSERT(received_.lastChild().toElement().tagName() == "set");	// RSM's <set>
	Q_ASSERT((from+to+note+1) == received_.childNodes().count());	// +1 for RSM's <set>
	return from+to+note;
}

EntryInfo ArchiveTask::receivedEntry(const int entryNo) const
{
	Q_ASSERT(type() == RetrCollectionTask);
	Q_ASSERT(entryNo>=0);
	Q_ASSERT(entryNo<receivedEntriesCount());

	const QDomElement entryElement = received_.childNodes().at(entryNo).toElement();
	const QString entryTag = entryElement.tagName();

	EntryType type;
	if(entryTag == "to") {
		type = ReceivedMessageEntry;
	} else if(entryTag == "from") {
		type = SentMessageEntry;
	} else if(entryTag == "note") {
		type = NoteEntry;
	} else {
		qWarning() << entryTag;
		Q_ASSERT_X(false, "unknown entry type", "ArchiveTask::receivedEntry()");
	}

	const XMPP::Jid jid = entryElement.attribute("jid");	// FIXME jid
	const QString nickname = entryElement.attribute("name");
	const QString body = entryElement.firstChildElement("body").text();
	const QDateTime utc;	// FIXME utc
	return EntryInfo(-1, -1, type, jid, nickname, body, utc);
}

void ArchiveTask::onGo()
{
	// we want valid command there
	switch (type())
	{
		case DetectSupportTask:
			Q_ASSERT(forSend_.attributes().count() == 1);	// only xmlns
			break;
		case RetrListOfCollectionsTask:
			// FIXME no checks yet
			break;
		case RetrCollectionTask:
			Q_ASSERT(forSend_.attribute("end").isNull());
			Q_ASSERT(!forSend_.attribute("start").isEmpty());
			Q_ASSERT(!forSend_.attribute("with").isEmpty());
			break;
		default:
			Q_ASSERT_X(false, "Unchecked command", "ArchiveTask::onGo()");
	}

	QDomElement iq = createIQ(doc(), "get", QString(), id());
	iq.appendChild(forSend_);
	send(iq);
}

bool ArchiveTask::take(const QDomElement &x)
{
	if(!iqVerify(x, QString(), id())) {
		return false;
	}

	if(x.attribute("type") == "result") {
		rsmSet_ = RsmSet(x);
		if(type() == RetrListOfCollectionsTask) {
			received_ = x.firstChildElement("list");
		} else if(type() == RetrCollectionTask) {
			received_ = x.firstChildElement("chat");
		}
		setSuccess();
	} else {
		qWarning() << "error in ArchiveTask::take()";
		setError(x);
	}

	return true;
}
