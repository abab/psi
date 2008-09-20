#ifndef _META_H_
#define _META_H_

#include <QDebug>

#include <xmpp_jid.h>

//LATER move this all to Iris
static QDebug operator<<(QDebug d, const XMPP::Jid& jid) { return d << jid.full(); }
static bool operator==(const XMPP::Jid& a, const XMPP::Jid& b) { return a.compare(b); }

// sadly, bool operator==(const QVariant& v1, const QVariant& v2) does not support custom types :(
// We are forced to create QVariants from XMPP::Jid::bare()
/*
Q_DECLARE_METATYPE(XMPP::Jid)
*/

#endif /* META_H_ */
