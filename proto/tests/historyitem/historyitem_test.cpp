#include <QtTest>

#include <model_historyitem.h>
using namespace History;

class HistoryItemTest : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();
	void cleanupTestCase();

	void test();

private:
	HistoryItem* root_;
};

void HistoryItemTest::initTestCase()
{
	QVERIFY(HistoryItem::globalItemsCount() == 0);
}

void HistoryItemTest::cleanupTestCase()
{
	QVERIFY(HistoryItem::globalItemsCount() == 0);
}

void HistoryItemTest::test()
{
	root_ = new HistoryItem;
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

	// data

	delete root_;
}

QTEST_MAIN(HistoryItemTest)
#include "historyitem_test.moc"
