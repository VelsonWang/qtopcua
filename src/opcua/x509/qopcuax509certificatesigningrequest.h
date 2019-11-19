/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt OPC UA module.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QOPCUAX509CERTIFICATESIGNINGREQUEST_H
#define QOPCUAX509CERTIFICATESIGNINGREQUEST_H

#include "QtOpcUa/qopcuakeypair.h"
#include "QtOpcUa/qopcuax509extension.h"
#include "QtOpcUa/qopcuax509distinguishedname.h"
#include <QVector>
#include <QtOpcUa/qopcuaglobal.h>
#include <QtOpcUa/qopcuax509certificatesigningrequest.h>

QT_BEGIN_NAMESPACE

class QOpcUaX509CertificateSigningRequestPrivate;

class Q_OPCUA_EXPORT QOpcUaX509CertificateSigningRequest
{
    Q_DECLARE_PRIVATE(QOpcUaX509CertificateSigningRequest)

public:
    enum class MessageDigest {
        SHA256
    };
    enum class Encoding {
        PEM,
        DER
    };

    QOpcUaX509CertificateSigningRequest();
    ~QOpcUaX509CertificateSigningRequest();

    void setMessageDigest(MessageDigest digest);
    MessageDigest messageDigest() const;

    void setEncoding(Encoding encoding);
    Encoding encoding() const;

    void setSubject(const QOpcUaX509DistinguishedName &subject);
    const QOpcUaX509DistinguishedName &subject() const;

    void addExtension(QOpcUaX509Extension *extension);
    QByteArray createRequest(const QOpcUaKeyPair &privateKey);
    QByteArray createSelfSignedCertificate(const QOpcUaKeyPair &privateKey, int validityInDays = 365);

private:
    QOpcUaX509CertificateSigningRequestPrivate *d_ptr = nullptr;
};

QT_END_NAMESPACE

#endif // QOPCUAX509CERTIFICATESIGNINGREQUEST_H
