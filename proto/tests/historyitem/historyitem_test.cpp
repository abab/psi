#include <QtTest>
#include <QtCore>

#include <model_historyitem.h>
using namespace History;

class HistoryItemTest : public QObject
{
	Q_OBJECT

private slots:
	void init();
	void cleanup();

	void deleteTest();
	void deleteTime();

private:
	static HistoryItem* root_;
	static const int collectionsCount_;
	static const int entriesCount_;
};

HistoryItem* HistoryItemTest::root_ = 0;
const int HistoryItemTest::collectionsCount_ = 50;
const int HistoryItemTest::entriesCount_ = 100;

void HistoryItemTest::init()
{
	QVERIFY(HistoryItem::globalItemsCount() == 0);
	QVERIFY(root_ == 0);
	root_ = new HistoryItem;
}

void HistoryItemTest::cleanup()
{
	delete root_; root_ = 0;
	QVERIFY(HistoryItem::globalItemsCount() == 0);
}

void HistoryItemTest::deleteTest()
{
	QVERIFY(root_ != 0);
	QVERIFY(root_->parent() == 0);

	// auto
	{
		HistoryItem* child = new HistoryItem(root_);
		QVERIFY(child->parent() == root_);
		delete child;
		QVERIFY(root_->childCount() == 0);
	}

	// manual
	{
		HistoryItem* child = new HistoryItem;
		root_->appendChild(child);
		QVERIFY(root_->childCount() == 1);
		QVERIFY(root_->child(0) == child);
		QVERIFY(child->parent() == root_);

		root_->removeChild(child);
		QVERIFY(root_->childCount() == 0);
		QVERIFY(child->parent() == 0);

		delete child;
	}

	// scope
	{
		{
			HistoryItem child;
			root_->appendChild(&child);
			QVERIFY(root_->childCount() == 1);
			QVERIFY(root_->child(0) == &child);
			QVERIFY(child.parent() == root_);
		}
		QVERIFY(root_->childCount() == 0);
	}

	// delete
	{
		HistoryItem* child = new HistoryItem(root_);
		const int childCount = 10;
		for(int i=0; i<childCount; ++i) {
			new HistoryItem(child);
		}
		QVERIFY(child->childCount() == childCount);
		delete child;
		QVERIFY(root_->childCount() == 0);
	}

	delete root_;
	root_ = 0;
}

void HistoryItemTest::deleteTime()
{
	QDateTime start = QDateTime::currentDateTime();

	for(int i=0; i<collectionsCount_; ++i) {
		HistoryItem* collection = new HistoryItem(root_);	// w/ parent
		for(int j=0; j<entriesCount_; ++j) {
			collection->appendChild(new HistoryItem(0));	// w/o parent
		}
	}

	qDebug() << "create:" << start.secsTo(QDateTime::currentDateTime()) << "seconds";
	start = QDateTime::currentDateTime();
	delete root_;	root_ = 0;
	qDebug() << "delete:" << start.secsTo(QDateTime::currentDateTime()) << "seconds";
}

QTEST_MAIN(HistoryItemTest)
#include "historyitem_test.moc"
