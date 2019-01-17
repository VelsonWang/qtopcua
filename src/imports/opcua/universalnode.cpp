/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "universalnode.h"
#include "qopcuaclient.h"
#include "opcuanodeidtype.h"
#include <QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_OPCUA_PLUGINS_QML)

UniversalNode::UniversalNode(QObject *parent)
    : QObject(parent)
{
}

UniversalNode::UniversalNode()
    :QObject(nullptr)
{
}

UniversalNode::UniversalNode(const QString &nodeIdentifier, QObject *parent)
    : QObject(parent)
{
    setNodeIdentifier(nodeIdentifier);
}

UniversalNode::UniversalNode(const QString &namespaceName, const QString &nodeIdentifier, QObject *parent)
    : QObject(parent)
{
    setMembers(false, 0, true, namespaceName, true, nodeIdentifier);
}

UniversalNode::UniversalNode(quint16 namespaceIndex, const QString &nodeIdentifier, QObject *parent)
    : QObject(parent)
{
    setMembers(true, namespaceIndex, false, QString(), true, nodeIdentifier);
}

UniversalNode::UniversalNode(const UniversalNode &other, QObject *parent)
    : QObject(parent)
{
    setMembers(other.isNamespaceNameValid(), other.namespaceIndex(),
               !other.namespaceName().isEmpty(), other.namespaceName(),
               !other.nodeIdentifier().isEmpty(), other.nodeIdentifier());
}

UniversalNode::UniversalNode(const OpcUaNodeIdType *other, QObject *parent)
    : QObject(parent)
{
    if (other)
        from(*other);
}

const QString &UniversalNode::namespaceName() const
{
    return m_namespaceName;
}

void UniversalNode::setNamespace(const QString &namespaceName)
{
    bool ok;
    int index = namespaceName.toUInt(&ok);

    setMembers(ok, index, !ok, namespaceName, false, QString());
}

quint16 UniversalNode::namespaceIndex() const
{
    return m_namespaceIndex;
}

void UniversalNode::setNamespace(quint16 namespaceIndex)
{
    setMembers(true, namespaceIndex, false, QString(), false, QString());
}

const QString &UniversalNode::nodeIdentifier() const
{
    return m_nodeIdentifier;
}

void UniversalNode::setNodeIdentifier(const QString &nodeIdentifier)
{
    int index = 0;
    QString name;

    if (splitNodeIdAndNamespace(nodeIdentifier, &index, &name)) {
        setMembers(true, index, false, QString(), true, name);
    } else {
        setMembers(false, 0, false, QString(), true, nodeIdentifier);
    }
}

void UniversalNode::resolveNamespace(QOpcUaClient *client)
{
    if (!m_namespaceIndexValid)
        resolveNamespaceNameToIndex(client);
    else if (m_namespaceName.isEmpty())
        resolveNamespaceIndexToName(client);
}

void UniversalNode::resolveNamespaceIndexToName(QOpcUaClient *client)
{
    if (!m_namespaceIndexValid) {
        qCWarning(QT_OPCUA_PLUGINS_QML) << "Could not resolve namespace: Namespace index is not valid";
        return;
    }

    const auto namespaceArray = client->namespaceArray();

    if (!namespaceArray.size()) {
        qCWarning(QT_OPCUA_PLUGINS_QML) << "Namespaces table missing, unable to resolve namespace name.";
        return;
    }

    if (m_namespaceIndex >= namespaceArray.size()) {
        qCWarning(QT_OPCUA_PLUGINS_QML) << "Namespace index not in a valid range";
        return;
    }

    setMembers(true, m_namespaceIndex, true, namespaceArray.at(m_namespaceIndex), false, QString());
}

void UniversalNode::resolveNamespaceNameToIndex(QOpcUaClient *client)
{
    if (m_namespaceIndexValid)
        return; // Namespace index already resolved, nothing to do

    const auto namespaceArray = client->namespaceArray();

    if (!namespaceArray.size()) {
        qCWarning(QT_OPCUA_PLUGINS_QML) << "Namespaces table missing, unable to resolve namespace name.";
        return;
    }

    if (m_namespaceName.isEmpty()) {
        qCWarning(QT_OPCUA_PLUGINS_QML) << "Could not resolve namespace: Namespace name is empty" << (m_nodeIdentifier.isEmpty() ? QString() : (QString("(") + m_nodeIdentifier + ")"));
        return;
    }

    int index = namespaceArray.indexOf(m_namespaceName);
    if (index < 0) {
        qCWarning(QT_OPCUA_PLUGINS_QML) << "Could not resolve namespace: Namespace" << m_namespaceName << "not found in" << namespaceArray;
        return;
    }

    setMembers(true, index, true, m_namespaceName, false, QString());
}

bool UniversalNode::isNamespaceNameValid() const
{
    return !m_namespaceName.isEmpty();
}

bool UniversalNode::isNamespaceIndexValid() const
{
    return m_namespaceIndexValid;
}

QOpcUaQualifiedName UniversalNode::toQualifiedName() const
{
    QOpcUaQualifiedName qualifiedName;

    if (!m_namespaceIndexValid || m_nodeIdentifier.isEmpty()) {
        qCWarning(QT_OPCUA_PLUGINS_QML) << "Insufficient information to create a QOpcUaQualifiedName";
        return qualifiedName;
    }

    qualifiedName.setNamespaceIndex(m_namespaceIndex);
    qualifiedName.setName(m_nodeIdentifier);
    return qualifiedName;
}

QOpcUaExpandedNodeId UniversalNode::toExpandedNodeId() const
{
    QOpcUaExpandedNodeId expandedNodeId;

    if (m_namespaceName.isEmpty() || m_nodeIdentifier.isEmpty()) {
        qCWarning(QT_OPCUA_PLUGINS_QML) << "Insufficient information to create a QOpcUaExpandedNodeId";
        return expandedNodeId;
    }

    expandedNodeId.setServerIndex(0);
    expandedNodeId.setNamespaceUri(m_namespaceName);
    expandedNodeId.setNodeId(m_nodeIdentifier);
    return expandedNodeId;
}

void UniversalNode::from(const QOpcUaQualifiedName &qualifiedName)
{
    setMembers(true, qualifiedName.namespaceIndex(), false, QString(), true, qualifiedName.name());
}

void UniversalNode::from(const QOpcUaExpandedNodeId &expandedNodeId)
{
    setMembers(false, 0, true, expandedNodeId.namespaceUri(), true, expandedNodeId.nodeId());
}

void UniversalNode::from(const QOpcUaBrowsePathTarget &browsePathTarget)
{
    // QExpandedNodeId is too unreliable and needs some casehandling around it to get a common information
    int index = 0;
    QString namespaceName = browsePathTarget.targetId().namespaceUri();
    QString identifier;
    bool namespaceIndexValid =  splitNodeIdAndNamespace(browsePathTarget.targetId().nodeId(), &index, &identifier);

    setMembers(namespaceIndexValid, index, !namespaceName.isEmpty(), namespaceName, true, identifier);
}

void UniversalNode::from(const OpcUaNodeIdType *other) {
    from(*other);
}

void UniversalNode::from(const OpcUaNodeIdType &other)
{
    setMembers(other.m_universalNode.m_namespaceIndexValid, other.m_universalNode.m_namespaceIndex,
               !other.nodeNamespace().isEmpty(), other.nodeNamespace(),
               !other.identifier().isEmpty(), other.identifier());
}

void UniversalNode::from(const UniversalNode &other)
{
    setMembers(other.isNamespaceIndexValid(), other.namespaceIndex(),
               true, other.namespaceName(), true, other.nodeIdentifier());
}

QString UniversalNode::fullNodeId() const
{
    if (!m_namespaceIndexValid || m_nodeIdentifier.isEmpty()) {
        QString message("Unable to construct a full node id");
        if (!m_nodeIdentifier.isEmpty())
            message += " for node " + m_nodeIdentifier;
        else
            message += " because node id string is empty.";

        if (!m_namespaceIndexValid)
            message += "; namespace index is not valid.";
        qCWarning(QT_OPCUA_PLUGINS_QML) << message;
        return QString();
    }

    return QString("ns=%1;%2").arg(m_namespaceIndex).arg(m_nodeIdentifier);
}

QOpcUaNode *UniversalNode::createNode(QOpcUaClient *client)
{
    return client->node(fullNodeId());
}

UniversalNode &UniversalNode::operator=(const UniversalNode &rhs)
{
    m_namespaceName = rhs.m_namespaceName;
    m_nodeIdentifier = rhs.m_nodeIdentifier;
    m_namespaceIndex = rhs.m_namespaceIndex;
    m_namespaceIndexValid = rhs.m_namespaceIndexValid;
    return *this;
}

/* This function sets the members to the desired values and emits changes signal only once after all variables
 * have already been set.
 * If the namespace index or name changes without setting the counterpart as well it will invalidate
 * not part not being set.
 */
void UniversalNode::setMembers(bool setNamespaceIndex, quint16 namespaceIndex,
                               bool setNamespaceName, const QString &namespaceName,
                               bool setNodeIdentifier, const QString &nodeIdentifier)
{
    // qCDebug(QT_OPCUA_PLUGINS_QML) << Q_FUNC_INFO << setNamespaceIndex << namespaceIndex << setNamespaceName << namespaceName << setNodeIdentifier << nodeIdentifier;
    bool emitNamespaceIndexChanged = false;
    bool emitNamespaceNameChanged = false;
    bool emitNodeIdentifierChanged = false;

    if (setNamespaceIndex && (namespaceIndex != m_namespaceIndex || !m_namespaceIndexValid)) {
        m_namespaceIndex = namespaceIndex;
        m_namespaceIndexValid = true;
        emitNamespaceIndexChanged = true;

        if (!setNamespaceName) // Index changed without name given: invalidate name
            m_namespaceName.clear();
    }

    if (setNamespaceName && namespaceName != m_namespaceName) {
        m_namespaceName = namespaceName;
        emitNamespaceNameChanged = true;

        if (!setNamespaceIndex) // Name changed without index given: invalidate index
            m_namespaceIndexValid = false;
    }

    if (setNodeIdentifier && nodeIdentifier != m_nodeIdentifier) {
        if (nodeIdentifier.startsWith(QLatin1String("ns=")))
            qCWarning(QT_OPCUA_PLUGINS_QML) << "Setting node identifier with namespace internally is not allowed.";

        m_nodeIdentifier = nodeIdentifier;
        emitNodeIdentifierChanged = true;
    }

    if (emitNamespaceIndexChanged)
        emit namespaceIndexChanged(m_namespaceIndex);
    if (emitNamespaceNameChanged)
        emit namespaceNameChanged(m_namespaceName);
    if (emitNodeIdentifierChanged)
        emit nodeIdentifierChanged(m_nodeIdentifier);
    if (emitNamespaceIndexChanged || emitNamespaceNameChanged)
        emit namespaceChanged();
    if (emitNamespaceIndexChanged || emitNamespaceNameChanged || emitNodeIdentifierChanged) {
        emit nodeChanged();
    }
}

/*
    This function splits up a node identifier into namespace index and node name.
    Returns true if successful, otherwise false.
 */
bool UniversalNode::splitNodeIdAndNamespace(const QString nodeIdentifier, int *namespaceIndex, QString *identifier)
{
    if (nodeIdentifier.startsWith(QLatin1String("ns="))) {
        const auto token = nodeIdentifier.split(QLatin1Char(';'));
        if (token.size() != 2) {
            qCWarning(QT_OPCUA_PLUGINS_QML) << "Invalid node identifier:" << nodeIdentifier;
            return false;
        }

        const QString ns = token[0].mid(3);
        bool ok;
        *namespaceIndex = ns.toUInt(&ok);
        if (!ok) {
            qCWarning(QT_OPCUA_PLUGINS_QML) << "Namespace index is not a number:" << nodeIdentifier;
            return false;
        }
        *identifier = token[1];
        return true;
    }
    return false;
}

bool UniversalNode::operator==(const UniversalNode &rhs) const
{
    return this->m_namespaceName == rhs.m_namespaceName &&
            this->m_nodeIdentifier == rhs.m_nodeIdentifier &&
            this->m_namespaceIndex == rhs.m_namespaceIndex &&
            this->m_namespaceIndexValid == rhs.m_namespaceIndexValid;
}

QT_END_NAMESPACE
