/*
 * xep82datetime.cpp - functions for date/time conversions according to XEP-82
 * Copyright (C) 2008 Aleksey Palazhchenko
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

#include "xep82datetime.h"

#define XEP82_FORMAT_WITHOUT	"yyyy-MM-ddThh:mm:ssZ"
#define XEP82_FORMAT_WITH		"yyyy-MM-ddThh:mm:ss.zzzZ"

bool containsFractionalSeconds(const QString& xepString)
{
	const bool without	= QDateTime::fromString(xepString, XEP82_FORMAT_WITHOUT).isValid();
	const bool with		= QDateTime::fromString(xepString, XEP82_FORMAT_WITH   ).isValid();
	Q_ASSERT(with != without);
	return with;
}

QDateTime xep82FormatToDateTime(const QString& xepString, bool* thereWasFractionalSeconds)
{
	bool unused;
	Q_UNUSED(unused);
	bool& was = (thereWasFractionalSeconds ? *thereWasFractionalSeconds : unused);

	QString format;
	if(containsFractionalSeconds(xepString)) {
		was = true;
		format = XEP82_FORMAT_WITH;
	} else {
		was = false;
		format = XEP82_FORMAT_WITHOUT;
	}

	QDateTime res = QDateTime::fromString(xepString, format);
	if(!res.isValid()) {
		Q_ASSERT_X(false, qPrintable("can't convert " + xepString + " to QDateTime"), "xep82FormatToDateTime");
	}
	res.setTimeSpec(Qt::UTC);
	return res;
}

QString dateTimeToXep82Format(const QDateTime& dt, const bool withFractionalSeconds)
{
	QString format;
	if(withFractionalSeconds) {
		format = XEP82_FORMAT_WITH;
	} else {
		format = XEP82_FORMAT_WITHOUT;
	}

	const QString res = dt.toString(format);
	if(dt.timeSpec() != Qt::UTC) {
		// huh? Another timezone? is it needed?
		Q_ASSERT_X(false, "No support for timezones yet.", "dateTimeToXep82Format");
	}
	return res;
}
