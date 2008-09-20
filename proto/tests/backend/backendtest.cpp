#include <QtTest>

#include <QFile>

#include <backend.h>
using namespace History;

#ifdef FAKE_BACKEND
// HACK: Fake backend uses references.
// It allows us to save API.
#define CollectionInfo CollectionInfo&
#define CollectionsInfo CollectionsInfo&
#define Entry Entry&
#define Entries Entries&
#endif

class BackendTest : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();
	void cleanupTestCase();

	void fillDatabase();
	void basicIntegrity();

private:
	Storage* storage_;

	static const int collectionsCount_;
	static const int entriesCount_;
};

const int BackendTest::collectionsCount_ = 50;
const int BackendTest::entriesCount_ = 100;

void BackendTest::initTestCase()
{
	QFile::remove("test.db");
	storage_ = Storage::getStorage("test.db");
	QVERIFY2(storage_, "Can't create database test.db");
}

void BackendTest::cleanupTestCase()
{
	delete storage_;
}

void BackendTest::fillDatabase()
{
	qDebug() << "Filling database with" << collectionsCount_ << "collections with" << entriesCount_ << "entries each...";

	QDateTime t = QDateTime::currentDateTime();
	QDateTime start(QDateTime::currentDateTime());
	for(int i=0; i<collectionsCount_; ++i) {
		XMPP::Jid contact("nodeX@contact.com");
		contact.setNode(QString("node")+QString::number(i));
		CollectionInfo collection = storage_->newCollection(ChatCollection, XMPP::Jid("test@owner.com"), contact, t);
		storage_->setCollectionSubject(collection.id(), QString("Subject-") + QString::number(i));
		for(int j=0; j<entriesCount_; ++j) {
			const QString body = QString("Body %1-%2").arg(i).arg(j);
			storage_->newEntry(collection.id(), NoteEntry, contact, "node", body, t);
			t.addSecs(1);
		}
		t.addSecs(1);
	}
	qDebug() << start.secsTo(QDateTime::currentDateTime()) << "seconds";
}

void BackendTest::basicIntegrity()
{
	CollectionsInfo cols = storage_->collections();
	QVERIFY(cols.count() == collectionsCount_);

	for(int i=0; i<cols.count(); ++i) {
		CollectionInfo col = cols[i];
		QVERIFY(col.id() > 0);

		QVERIFY(col.contactJid().bare().contains("@contact.com"));
		QVERIFY(col.ownerJid().bare().contains("@owner.com"));
		QVERIFY(col.start().isValid());
		QVERIFY(col.subject().contains("Subject"));

		QVERIFY(col.id() == (storage_->collectionById(col.id())).id());

		EntriesInfo entries = storage_->entriesByCollectionId(col.id());
		QVERIFY(entries.count() == entriesCount_);
		for(int j=0; j<entries.count(); ++j) {
			EntryInfo entry = entries[i];
			QVERIFY(entry.id() > 0);

			QVERIFY(entry.collectionId() == col.id());
			QVERIFY(entry.type() == NoteEntry);
			QVERIFY(entry.body().contains("Body"));
			QVERIFY(entry.utc().isValid());

			QVERIFY(entry.id() == (storage_->entryById(entry.id())).id());
		}
	}
}

QTEST_MAIN(BackendTest);
#include "backendtest.moc"
