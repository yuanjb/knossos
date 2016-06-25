/*
 *  This file is a part of KNOSSOS.
 *
 *  (C) Copyright 2007-2016
 *  Max-Planck-Gesellschaft zur Foerderung der Wissenschaften e.V.
 *
 *  KNOSSOS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 of
 *  the License as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *  For further information, visit http://www.knossostool.org
 *  or contact knossos-team@mpimf-heidelberg.mpg.de
 */

#ifndef NETWORK_H
#define NETWORK_H

#include "loader.h"

#include <QNetworkCookieJar>
#include <QNetworkAccessManager>
#include <QThread>

class Network {
    QNetworkAccessManager manager;
    QNetworkCookieJar cookieJar;

public:
    explicit Network(const QObject *parent = nullptr);
    void setCookies(const QVariantList & setting);
    QVariantList getCookiesForHost(const QString & host);

    static Network & singleton() {
        static Network network;
        return network;
    }

    QPair<bool, QString> login(const QUrl & url, const QString & username, const QString & password);
    QPair<bool, QString> refresh(const QUrl & url);
    QPair<bool, QPair<QString, QByteArray> > getFile(const QUrl & url);
    QPair<bool, QPair<QString, QByteArray> > getPost(const QUrl & url);
    QPair<bool, QString> submitHeidelbrain(const QUrl & url, const QString & filePath, const QString & comment, const bool final);
    void submitSegmentationJob(const QString &filename);
    std::pair<int, int> checkOnlineMags(const QUrl & url);
};

#endif // NETWORK_H
