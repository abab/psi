/*
 * dialog.h - a dialog to show event history
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

#ifndef _DIALOG_H_
#define _DIALOG_H_

#include <QPointer>
#include <QMenu>
#include <QSortFilterProxyModel>

#include <xmpp_jid.h>

#include "models.h"
#include "archivetask.h"
using namespace History;

#include "ui_dialog.h"

/*! History dialog. */
class HistoryDlg : public QWidget
{
	Q_OBJECT
public:
	HistoryDlg(Storage* storage, XMPP::Task* rootTask);
	~HistoryDlg();

	void setCurrentContact(const XMPP::Jid& contact);

private slots:
	void showEntriesFromCollections(const IdList&);

	void onCollectionsViewClicked();

	void onEntryClicked(const QModelIndex& proxyIndex);
	void onEntriesViewDblClicked();

	void onRenameCurrentCollection();
	void onRemoveSelectedCollections();

	void onAddNote();
	void onRemoveNote();

	void onEntrySearch(const QString& text);

	void onGetCollectionsBtnClicked();
	void onArchiveTaskFinished();

	void refreshCollectionsModel();	// FIXME HACK

private:
	void createActions();
	void menuForSelectedCollections(const QModelIndexList& sourceIndexes, QMenu*);
	void menuForEntry(const QModelIndex& sourceIndex, QMenu* menu);

	void detectXepSupport();

	HistoryDlg(const HistoryDlg &);
	HistoryDlg& operator=(const HistoryDlg &);

	Ui::HistoryDlgBase				ui_;
	QPointer<Storage>				storage_;
	QPointer<CollectionsModel>		collectionsUnfiltered_;
	QPointer<QSortFilterProxyModel> collections_;
	QPointer<EntriesModel>			entriesUnfiltered_;
	QPointer<QSortFilterProxyModel>	entries_;

	QPointer<QMenu>					menu_;

	QPointer<QAction>				renameCollectionAction_;
	QPointer<QAction>				removeCollectionAction_;
	QPointer<QAction>				addNoteAction_;
	QPointer<QAction>				removeNoteAction_;

	QPointer<XMPP::Task>			rootTask_;
	QString							xepNamespace_;

	Id								collectionId_;	// FIXME remove completely
};

#endif	// _DIALOG_H_
