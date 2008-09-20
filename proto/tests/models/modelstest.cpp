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
	BaseHistoryModel* base_;
	HistoryItem* root_;
};

void ModelsTest::fillStorage()
{
	const int contactsCount = 5;
	const int collectionsCount = 5;
	const int entriesCount = 5;
	QDateTime start = QDateTime::currentDateTime().addYears(-1);
	for(int contactCo=1; contactCo<=contactsCount; ++contactCo) {
		XMPP::Jid contact(QString("contact%1@contact.com").arg(contactCo));
		for(int collectionCo=1; collectionCo<=collectionsCount; ++collectionCo) {
			CollectionInfo c = storage_->newCollection(ChatCollection, XMPP::Jid("test@owner.com"), contact, start);
			storage_->setCollectionSubject(c.id(), QString("Subject %1 - %2").arg(contactCo).arg(collectionCo));

			for(int entryCo=1; entryCo<=entriesCount; ++entryCo) {
				const QString body = QString("Body %1 - %2 - %3").arg(contactCo).arg(collectionCo).arg(entryCo);
				storage_->newEntry(c.id(), NoteEntry, contact, "node", body, start);
				start = start.addSecs(10);
			}

			start = start.addDays(15);
		}
	}
}

void ModelsTest::initTestCase()
{
	storage_ = Storage::getStorage("test.db");
	QVERIFY2(storage_, "Can't create database test.db");
	fillStorage();
	base_ = new BaseHistoryModel(storage_);
	QVERIFY2(base_, "Can't create BaseHistoryModel");
	root_ = base_->itemFromIndex(QModelIndex());
	QVERIFY2(root_, "root item can't be NULL (but root's index must be invalid)");
}

void ModelsTest::cleanupTestCase()
{
	delete base_;
	delete storage_;
}

void ModelsTest::basicTests()
{
	QVERIFY2((root_ == root_->parent()) && (root_ == root_->parent()->parent()), "root's parent item should be root");
	QVERIFY2((base_->indexFromItem(root_, 0) == QModelIndex()), "root's index should be invalid");
	QVERIFY2((base_->indexFromItem(root_, 0).parent() == QModelIndex()), "root's parent index should be invalid");
}

QTEST_MAIN(ModelsTest);
#include "modelstest.moc"
