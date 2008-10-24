/*
 * archivetask.h
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

#ifndef ARCHIVETASK_H
#define ARCHIVETASK_H

#include <QDomElement>
#include <QString>
#include <QDateTime>

#include "xmpp_task.h"
#include "xmpp_jid.h"

#include "rsm.h"
#include "backend.h"
#include "xep82datetime.h"

namespace History
{

const QString XEP136_NAMESPACE = "urn:xmpp:archive";

enum TaskType {
	DetectSupportTask			= 1,
	RetrListOfCollectionsTask	= 10,
	RetrCollectionTask			= 11
};

class ArchiveTask : public XMPP::Task
{
	Q_OBJECT
public:
	ArchiveTask(Task* parent, const TaskType type, const QString& xmlns, const RsmSet& setForPrevPage = RsmSet());
	~ArchiveTask();

	TaskType type() const { return type_; }
	QString xmlns() const { return xmlns_; }
	RsmSet rsmSet() const { return rsmSet_; }

	void setWith(const XMPP::Jid& with);
	void setStart(const QDateTime& start);
	void setEnd(const QDateTime& end);

	QString with() const { return received_.attribute("with"); }
	QDateTime start() const { return xep82FormatToDateTime(received_.attribute("start")); }

	int receivedCollectionsCount() const;
	CollectionInfo receivedCollection(const int collectionNo) const;

	int receivedEntriesCount() const;
	EntryInfo receivedEntry(const int entryNo) const;

	void onGo();
	bool take(const QDomElement &);

private:
	QDomElement forSend_;
	QDomElement received_;
	TaskType type_;
	QString xmlns_;
	RsmSet rsmSet_;

	static const int RSM_MAX;
};

}	// namespace

#endif	// ARCHIVETASK_H


static QDebug operator<<(QDebug d, const History::ArchiveTask& task)
{
	QString out = "task " + task.id();
	out += task.success() ? " success " : " failed";
	switch(task.type())
	{
		case History::DetectSupportTask:
			out += "DetectSupportTask ";
			out += "xmlns: " + task.xmlns();
			break;
		case History::RetrListOfCollectionsTask:
			out += "RetrListOfCollectionsTask";
			break;
		case History::RetrCollectionTask:
			out += "RetrCollectionTask ";
			out += "with: " + task.with();
			out += " start:" + task.start().toString();
			break;
		default:
			qWarning() << task.type();
			Q_ASSERT_X(false, "Unhandled task type", "operator<< for ArchiveTask");
	}
	return d << out;
}
