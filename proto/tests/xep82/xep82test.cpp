#include <QtTest>

#include <xep82datetime.h>

class Xep82Test : public QObject
{
	Q_OBJECT

private slots:
	void basicTests();
};

void Xep82Test::basicTests()
{
	QVERIFY(!containsFractionalSeconds("2008-08-02T09:03:05Z"));
	QVERIFY(containsFractionalSeconds("2008-08-02T09:03:05.023Z"));

	QDateTime dt;
	bool thereWasFractionalSeconds;

	dt = xep82FormatToDateTime("2008-08-02T09:03:05Z", &thereWasFractionalSeconds);
	QVERIFY(dt.isValid());
	QVERIFY(dt.date().year() == 2008);
	QVERIFY(dt.date().month() == 8);
	QVERIFY(dt.date().day() == 2);
	QVERIFY(dt.time().hour() == 9);
	QVERIFY(dt.time().minute() == 3);
	QVERIFY(dt.time().second() == 5);
	QVERIFY(dt.time().msec() == 0);
	QVERIFY(dt.timeSpec() == Qt::UTC);
	QVERIFY(thereWasFractionalSeconds == false);


	dt = xep82FormatToDateTime("2008-08-02T09:03:05.023Z", &thereWasFractionalSeconds);
	QVERIFY(dt.isValid());
	QVERIFY(dt.date().year() == 2008);
	QVERIFY(dt.date().month() == 8);
	QVERIFY(dt.date().day() == 2);
	QVERIFY(dt.time().hour() == 9);
	QVERIFY(dt.time().minute() == 3);
	QVERIFY(dt.time().second() == 5);
	QVERIFY(dt.time().msec() == 23);
	QVERIFY(dt.timeSpec() == Qt::UTC);
	QVERIFY(thereWasFractionalSeconds == true);

	QDate d(2008, 8, 2);
	QTime t(9, 3, 5, 23);
	dt = QDateTime(d, t, Qt::UTC);
	QVERIFY(dateTimeToXep82Format(dt, false) == "2008-08-02T09:03:05Z");
	QVERIFY(dateTimeToXep82Format(dt, true)  == "2008-08-02T09:03:05.023Z");
}

QTEST_MAIN(Xep82Test);
#include "xep82test.moc"
