/*
 * xep82datetime.h - functions for date/time conversions according to XEP-82
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

#ifndef XEPDATETIME_H
#define XEPDATETIME_H

#include <QDateTime>
#include <QString>

QDateTime xep82FormatToDateTime(const QString& xepString, bool* thereWasFractionalSeconds = 0);
QString dateTimeToXep82Format(const QDateTime& dt, const bool withFractionalSeconds);

bool containsFractionalSeconds(const QString& xepString);

#endif	// XEPDATETIME_H
