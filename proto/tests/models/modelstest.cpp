#include <QtTest>

#include <backend.h>
#include <models.h>
using namespace History;

#ifdef FAKE_BACKEND
// HACK: Fake backend uses references.
// It allows us to save API.
#define Collection Collection&
#define Collections Collections&
#define Entry Entry&
#define Entries Entries&
#endif

class ModelsTest : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();
	void cleanupTestCase();

	void basicTests();

private:
	void fillStorage();

	Storage* storage_;
	CollectionsModel* model_;
	HistoryItem* root_;
};

void ModelsTest::fillStorage()
{
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
			History::CollectionInfo c = storage_->newCollection(type, XMPP::Jid("test@owner.com"), contactJid, start);
			storage_->setCollectionSubject(c.id(), QString("Subject %1 - %2").arg(contactCo).arg(collectionCo));
			storage_->newEntry(c.id(), NoteEntry, contactJid, "me", "private note", start);

			for(int msgCo=1; msgCo<=messagesCount; ++msgCo) {
				const QString body = QString("Body %1 - %2 - %3").arg(contactCo).arg(collectionCo).arg(msgCo);
				storage_->newEntry(c.id(), ((msgCo%2) == 0) ? SentMessageEntry : ReceivedMessageEntry, contactJid, "node", body, start);
				start = start.addSecs(42);
			}

			start = start.addDays(1);
		}
	}
}

void ModelsTest::initTestCase()
{
	storage_ = Storage::getStorage("test.db");
	QVERIFY2(storage_, "Can't create database test.db");
	fillStorage();
	model_ = new CollectionsModel(storage_);
	QVERIFY2(model_, "Can't create BaseHistoryModel");
	root_ = model_->itemFromIndex(QModelIndex());
	QVERIFY2(root_, "root item can't be NULL (but root's index must be invalid)");
}

void ModelsTest::cleanupTestCase()
{
	delete model_;
	delete storage_;
}

void ModelsTest::basicTests()
{
	QVERIFY2((root_ == root_->parent()) && (root_ == root_->parent()->parent()), "root's parent item should be root");
	QVERIFY2((model_->indexFromItem(root_, 0) == QModelIndex()), "root's index should be invalid");
	QVERIFY2((model_->indexFromItem(root_, 0).parent() == QModelIndex()), "root's parent index should be invalid");
}

QTEST_MAIN(ModelsTest)
#include "modelstest.moc"
