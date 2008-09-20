/*
 * xep82datetime.cpp - functions for date/time conversions according to XEP-82
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

#ifndef XEPDATETIME_H
#define XEPDATETIME_H

#include <QDateTime>
#include <QString>
#include <QDebug>

#include "xep82datetime.h"

bool containsFractionalSeconds(const QString& xepString)
{
	bool without	= QDateTime::fromString(xepString, "yyyy-MM-ddThh:mm:ssZ").isValid();
	bool with		= QDateTime::fromString(xepString, "yyyy-MM-ddThh:mm:ss.zzzZ").isValid();
	Q_ASSERT(with != without);
	return with;
}

QDateTime xep82FormatToDateTime(const QString& xepString, bool* thereWasFractionalSeconds = 0)
{
//	if(xepString.isEmpty()) {
//		return QDateTime();	// MAYBE remove this?
//	}

	if(thereWasFractionalSeconds != 0) {
		*thereWasFractionalSeconds = false;
	}

	QString format("yyyy-MM-ddThh:mm:ss");
	if(containsFractionalSeconds(xepString)) {
		if(thereWasFractionalSeconds != 0) {
			*thereWasFractionalSeconds = true;
		}
		format += ".zzz";
	}
	format += "Z";

	QDateTime res = QDateTime::fromString(xepString, format);
	if(!res.isValid()) {
		Q_ASSERT_X(false, qPrintable("can't convert " + xepString + " to QDateTime"), "xep82FormatToDateTime");
	}
	res.setTimeSpec(Qt::UTC);
	return res;
}

QString dateTimeToXep82Format(const QDateTime& dt, const bool withFractionalSeconds)
{
//	if(!dt.isValid()) {
//		return QString();	// MAYBE remove this?
//	}

	QString format = "yyyy-MM-ddThh:mm:ss";
	if(withFractionalSeconds) {
		format += ".zzz";
	}
	QString res = dt.toString(format);
	if(dt.timeSpec() == Qt::UTC) {
		res += "Z";
	} else {
		// huh? Another timezone? is it needed?
		Q_ASSERT_X(false, "No support for timezones yet.", "dateTimeToXep82Format");
	}
	return res;
}

#endif	// XEPDATETIME_H
