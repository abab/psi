/*
 * dialog.cpp - a dialog to show event history
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

#include <QTimer>	// FIXME remove this!

#include "xmpp_xmlcommon.h"

#include "backend.h"
#include "xep82datetime.h"
#include "dialog.h"
#include "meta.h"

HistoryDlg::HistoryDlg(Storage* storage, XMPP::Task* rootTask)
	: QWidget(0), storage_(storage), rootTask_(rootTask), collectionId_(-1)
{
	detectXepSupport();

	ui_.setupUi(this);

	collectionsUnfiltered_	= new CollectionsModel(storage);
	entriesUnfiltered_		= new     EntriesModel(storage);

	collections_ = new QSortFilterProxyModel(this);
	collections_->setSourceModel(collectionsUnfiltered_);
	collections_->setDynamicSortFilter(true);
	collections_->setFilterKeyColumn(0);
	collections_->setFilterCaseSensitivity(Qt::CaseInsensitive);

	entries_ = new QSortFilterProxyModel(this);
	entries_->setSourceModel(entriesUnfiltered_);
	entries_->setDynamicSortFilter(true);
	entries_->setFilterKeyColumn(MessageColumn);
	entries_->setFilterCaseSensitivity(Qt::CaseInsensitive);

	ui_.collectionsView->setModel(collections_);
	ui_.entriesView->setModel(entries_);

	ui_.collectionsView->setUniformRowHeights(true);

	ui_.collectionsView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui_.collectionsView->setSelectionBehavior(QAbstractItemView::SelectItems);
	ui_.collectionsView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);
	ui_.entriesView->setSelectionMode(QAbstractItemView::SingleSelection);
	ui_.entriesView->setSelectionBehavior(QAbstractItemView::SelectItems);
	ui_.entriesView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);

	connect(ui_.collectionsView, SIGNAL(pressed(QModelIndex)), this, SLOT(onCollectionsViewClicked()));
	connect(ui_.entriesView, SIGNAL(pressed(QModelIndex)), this, SLOT(onEntryClicked(QModelIndex)));
	connect(ui_.entriesView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEntriesViewDblClicked()));

	connect(collectionsUnfiltered_,	SIGNAL(collectionsForLoad(IdList)),	this,	SLOT(showEntriesFromCollections(IdList)));

	connect(ui_.collectionsSearchEdit,	SIGNAL(textChanged(QString)),	collections_,	SLOT(setFilterWildcard(QString)));
	connect(ui_.entriesSearchEdit,		SIGNAL(textChanged(QString)),	this,			SLOT(onEntrySearch(QString)));

	for(int col=0; col<collections_->columnCount(QModelIndex()); ++col) {
		ui_.collectionsView->resizeColumnToContents(col);
	}

	createActions();

	connect(ui_.getCollectionsBtn, SIGNAL(clicked()), this, SLOT(onGetCollectionsBtnClicked()));
}

HistoryDlg::~HistoryDlg()
{
	delete menu_;

	delete entries_;
	delete entriesUnfiltered_;
	delete collections_;
	delete collectionsUnfiltered_;
}

void HistoryDlg::setCurrentContact(const XMPP::Jid& contact)
{
	Q_ASSERT(contact.isValid());
	int hits = 1;
	const QModelIndexList contactIndexes = collectionsUnfiltered_->match(QModelIndex(), CollectionContactJidRole, contact.bare(), hits);
	QModelIndex proxyIndex = collections_->mapFromSource(contactIndexes.first());
	ui_.collectionsView->setCurrentIndex(proxyIndex);
	onCollectionsViewClicked();
}

void HistoryDlg::showEntriesFromCollections(const IdList& list)
{
#ifdef HISTORY_DEBUG_MODELS
	qDebug() << "HistoryDlg::showEntriesFromCollections" << list;
//	qDebug() << "loaded: " << entriesUnfiltered_->loadedCollections();
#endif

	// TODO do we need it?
//	if(listSubtraction(list, entriesUnfiltered_->loadedCollections()).isEmpty()) {
//		// no need to reload, just move view
//		const QModelIndex sourceIndex = entriesUnfiltered_->match(QModelIndex(), CollectionIdRole, list.first(), 1).first();
//		const QModelIndex proxyIndex = entries_->mapFromSource(sourceIndex);
//		ui_.entriesView->setCurrentIndex(proxyIndex);
//		ui_.entriesView->scrollTo(proxyIndex, QAbstractItemView::PositionAtCenter);
//	} else {

	entriesUnfiltered_->clearModel();
	foreach(qint64 id, list) {
		entriesUnfiltered_->addEntriesFromCollection(id);
	}
	entriesUnfiltered_->refreshModel();

	ui_.entriesView->resizeColumnsToContents();
	ui_.entriesView->resizeRowsToContents();
	ui_.entriesView->scrollToBottom();

	// TODO do we need it?
//	int hits = 1;
//	const QModelIndex sourceIndex = entriesUnfiltered_->match(QModelIndex(), CollectionIdRole, list.first(), hits).first();
//	const QModelIndex proxyIndex = entries_->mapFromSource(sourceIndex);
//	ui_.entriesView->setCurrentIndex(proxyIndex);
//	ui_.entriesView->scrollTo(proxyIndex, QAbstractItemView::PositionAtCenter);

//	}
}

void HistoryDlg::onEntrySearch(const QString& text)
{
	entries_->setFilterWildcard(text);
	ui_.entriesView->scrollTo(ui_.entriesView->currentIndex(), QAbstractItemView::PositionAtCenter);
}

void HistoryDlg::onCollectionsViewClicked()
{
	QModelIndexList proxyIndexes = ui_.collectionsView->selectionModel()->selectedIndexes();
	if(proxyIndexes.isEmpty()) {
		return;
	}

	QModelIndexList sourceIndexes;
	foreach(QModelIndex proxyIndex, proxyIndexes) {
		sourceIndexes.append(collections_->mapToSource(proxyIndex));
	}
	collectionsUnfiltered_->onItemsSelected(sourceIndexes);

	const Qt::MouseButtons buttons = qApp->mouseButtons();
	if(buttons.testFlag(Qt::RightButton)) {
		menuForSelectedCollections(sourceIndexes, menu_);
		if(!menu_->isEmpty()) {
			menu_->exec(QCursor::pos());
		}
	}
}

void HistoryDlg::onEntryClicked(const QModelIndex& proxyIndex)
{
	QModelIndex sourceIndex = entriesUnfiltered_->userDataIndex(entries_->mapToSource(proxyIndex));
	QVariant rawData = entriesUnfiltered_->data(sourceIndex, CollectionIdRole);
	const Id collectionId = rawData.toLongLong();
	int hits = 1;
	const QModelIndex collectionSourceIndex = collectionsUnfiltered_->match(QModelIndex(), CollectionIdRole, collectionId, hits).first();
	ui_.collectionsView->setCurrentIndex(collections_->mapFromSource(collectionSourceIndex));

	const Qt::MouseButtons buttons = qApp->mouseButtons();
	if(buttons.testFlag(Qt::RightButton)) {
		menuForEntry(sourceIndex, menu_);
		if(!menu_->isEmpty()) {
			menu_->exec(QCursor::pos());
		}
	}
}

void HistoryDlg::onEntriesViewDblClicked()
{
	ui_.entriesSearchEdit->clear();
}

void HistoryDlg::createActions()
{
	renameCollectionAction_ = new QAction(tr("Rename collection"), this);
	connect(renameCollectionAction_, SIGNAL(triggered()), this, SLOT(onRenameCurrentCollection()));
	removeCollectionAction_ = new QAction(tr("Remove"), this);
	connect(removeCollectionAction_, SIGNAL(triggered()), this, SLOT(onRemoveSelectedCollections()));

	addNoteAction_		= new QAction(tr("Add note"), this);
	connect(addNoteAction_,		SIGNAL(triggered()), this, SLOT(onAddNote()));
	removeNoteAction_	= new QAction(tr("Remove note"), this);
	connect(removeNoteAction_,	SIGNAL(triggered()), this, SLOT(onRemoveNote()));

	menu_ = new QMenu(this);
}

void HistoryDlg::menuForSelectedCollections(const QModelIndexList& sourceIndexes, QMenu* menu)
{
	menu->clear();
	if(sourceIndexes.count() == 0) {
		return;
	}
	if((sourceIndexes.count() == 1) && collectionsUnfiltered_->isCollection(sourceIndexes.first())) {
		menu->addAction(renameCollectionAction_);
	}
	menu->addAction(removeCollectionAction_);
}

void HistoryDlg::menuForEntry(const QModelIndex& sourceIndex, QMenu* menu)
{
	menu->clear();
	if(!entriesUnfiltered_->isNote(sourceIndex)) {
		menu->addAction(addNoteAction_);
	} else {
		menu->addAction(removeNoteAction_);
	}
}

void HistoryDlg::onRenameCurrentCollection()
{
	ui_.collectionsView->edit(ui_.collectionsView->currentIndex());
}

void HistoryDlg::onRemoveSelectedCollections()
{
	QModelIndexList sourceIndexes;
	foreach(QModelIndex proxyIndex, ui_.collectionsView->selectionModel()->selectedIndexes()) {
		sourceIndexes.append(collections_->mapToSource(proxyIndex));
	}
	collectionsUnfiltered_->removeItems(sourceIndexes);

	entriesUnfiltered_->clearModel();
	entriesUnfiltered_->refreshModel();
}

void HistoryDlg::onAddNote()
{
	qDebug() << "will be soon :)";
}

void HistoryDlg::onRemoveNote()
{
	qDebug() << "will be soon :)";
}

void HistoryDlg::detectXepSupport()
{
	if(rootTask_) {
		QList<QString> namespaces;
		namespaces << XEP136_NAMESPACE << XEP136_OLD_NAMESPACE << XEP136_OLDEST_NAMESPACE;
		foreach(QString xmlns, namespaces) {
			ArchiveTask* task = new ArchiveTask(rootTask_, DetectSupportTask, xmlns);
			connect(task, SIGNAL(finished()), this, SLOT(onArchiveTaskFinished()));
			task->go(true);
		}
	} else {
		qWarning() << "No root task!";
	}
}

void HistoryDlg::onGetCollectionsBtnClicked()
{
	Q_ASSERT(rootTask_);
	ui_.getCollectionsBtn->setEnabled(false);
	ArchiveTask* task = new ArchiveTask(rootTask_, RetrListOfCollectionsTask, xepNamespace_);
	connect(task, SIGNAL(finished()), this, SLOT(onArchiveTaskFinished()));
	task->go(true);

	// HACK !!
	QTimer::singleShot(2000, this, SLOT(refreshCollectionsModel()));
}

void HistoryDlg::onArchiveTaskFinished()
{
	ArchiveTask* finishedTask = qobject_cast<ArchiveTask*>(sender());
	const bool success = finishedTask->success();
	qDebug() << *finishedTask;

	switch (finishedTask->type())
	{
		case DetectSupportTask:
			if(success) {
				ui_.getCollectionsBtn->setEnabled(true);
				if(xepNamespace_.isEmpty()) {
					xepNamespace_ = finishedTask->xmlns();
					qDebug() << "using" << xepNamespace_;
				}
			}
			break;
		case RetrListOfCollectionsTask:
		{
			ui_.getCollectionsBtn->setEnabled(true);
			if(!success) {
				qWarning() << "RetrListOfCollectionsTask failed!";
				break;
			}


			const int collectionsCount = finishedTask->receivedCollectionsCount();
			for(int collectionNo=0; collectionNo<collectionsCount; ++collectionNo) {
				const CollectionInfo& col = finishedTask->receivedCollection(collectionNo);

				// add to backend
				// FIXME it's unsafe, and I know it
				collectionId_ = storage_->newCollection(col).id();

				// request entries
				ArchiveTask* entriesTask = new ArchiveTask(rootTask_, RetrCollectionTask, xepNamespace_);
				entriesTask->setWith(col.contactJid());
				entriesTask->setStart(col.start());
				connect(entriesTask, SIGNAL(finished()), this, SLOT(onArchiveTaskFinished()));
				entriesTask->go(true);
			}

			// request next page
			const RsmSet prev = finishedTask->rsmSet();
			if(!prev.lastPage()) {
				ArchiveTask* nextPageTask = new ArchiveTask(rootTask_, RetrListOfCollectionsTask, xepNamespace_, prev);
				connect(nextPageTask, SIGNAL(finished()), this, SLOT(onArchiveTaskFinished()));
				nextPageTask->go(true);
			}
			break;
		}
		case RetrCollectionTask:
		{
			if(!success) {
				qWarning() << "RetrCollectionTask failed!";
				break;
			}

			// add to backend
			const int entriesCount = finishedTask->receivedEntriesCount();
			qDebug() << "count: " << entriesCount;
			for(int entryNo=0; entryNo<entriesCount; ++entryNo) {
				const EntryInfo& entry = finishedTask->receivedEntry(entryNo);
				qDebug() << collectionId_;
				storage_->newEntry(collectionId_, entry); // FIXME unsafe
			}

			// request next page
			const RsmSet prev = finishedTask->rsmSet();
			if(!prev.lastPage()) {
				ArchiveTask* nextPageTask = new ArchiveTask(rootTask_, RetrCollectionTask, xepNamespace_, prev);
				nextPageTask->setWith(finishedTask->with());
				nextPageTask->setStart(finishedTask->start());
				connect(nextPageTask, SIGNAL(finished()), this, SLOT(onArchiveTaskFinished()));
//				nextPageTask->go(true);	//FIXME WTF !!!!
			}
			break;
		}
		default:
			Q_ASSERT_X(false, "Unhandled task", "HistoryDlg::onArchiveTaskFinished()");
	}
}

// FIXME one big hack
void HistoryDlg::refreshCollectionsModel()
{
	// FIXME this is simply ugly!
	collectionsUnfiltered_->fillModel();
	collectionsUnfiltered_->refreshModel();
	collectionsUnfiltered_->setDebugTooltips(collectionsUnfiltered_->itemFromIndex(QModelIndex()));
}

