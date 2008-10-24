#include <QApplication>

#include "history/backend.h"
#include "history/dialog.h"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	History::Storage *s = History::Storage::getStorage("demo.db");

	// fill storage
	const int contactsCount = 7;
	const int collectionsCount = 5;
	const int messagesCount = 5;
	QDateTime start = QDateTime::currentDateTime().toUTC().addYears(-1);
	for(int contactCo=1; contactCo<=contactsCount; ++contactCo) {
		QString contact = QString("node%1@doman.com").arg(contactCo);
		for(int collectionCo=1; collectionCo<=collectionsCount; ++collectionCo) {
			CollectionType type = ChatCollection;
			if(!(contactCo%3)) {
				type = MucCollection;
				contact = QString("room@conf%1.domain.com").arg(contactCo);
			}
			XMPP::Jid contactJid(contact);
			History::CollectionInfo c = s->newCollection(type, XMPP::Jid("test@owner.com"), contactJid, start);
			s->setCollectionSubject(c.id(), QString("Subject %1 - %2").arg(contactCo).arg(collectionCo));
			s->newEntry(c.id(), NoteEntry, contactJid, "me", "private note", start);

			for(int msgCo=1; msgCo<=messagesCount; ++msgCo) {
				const QString body = QString("Body %1 - %2 - %3").arg(contactCo).arg(collectionCo).arg(msgCo);
				s->newEntry(c.id(), ((msgCo%2) == 0) ? SentMessageEntry : ReceivedMessageEntry, contactJid, "node", body, start);
				start = start.addSecs(42);
			}

			start = start.addDays(1);
		}
	}

	HistoryDlg dlg(s);
	dlg.show();
//	dlg.setCurrentContact(XMPP::Jid("node4@domain.com"));
	int res = app.exec();
	delete s;
	return res;
}
