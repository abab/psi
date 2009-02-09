/*
 * backend_common.h
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

#ifndef _BACKEND_COMMON_H_
#define _BACKEND_COMMON_H_

#include <QList>

/*! \namespace History
 * \brief Namespace of new event-logging system.
 * \author Aleksey Palazhchenko
 * \version 0.5
 * \date Google's Summer of Code 2008 and later
 */
namespace History
{

/*! Primary key in database. */
typedef qint64 Id;

/*! List of primary keys in database. */
typedef QList<Id> IdList;

} // namespace

#endif // _BACKEND_COMMON_H_
