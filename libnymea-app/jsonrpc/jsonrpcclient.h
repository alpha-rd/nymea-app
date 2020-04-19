/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef NYMEAJSONRPCCLIENT_H
#define NYMEAJSONRPCCLIENT_H

#include <QObject>
#include <QVariantMap>
#include <QPointer>
#include <QVersionNumber>

#include "connection/nymeaconnection.h"
#include "jsonhandler.h"

class JsonRpcReply;
class Param;
class Params;

class JsonRpcClient : public JsonHandler
{
    Q_OBJECT
    Q_PROPERTY(NymeaConnection::BearerTypes availableBearerTypes READ availableBearerTypes NOTIFY availableBearerTypesChanged)
    Q_PROPERTY(NymeaConnection::ConnectionStatus connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(NymeaHost* currentHost READ currentHost NOTIFY currentHostChanged)
    Q_PROPERTY(Connection* currentConnection READ currentConnection NOTIFY currentConnectionChanged)
    Q_PROPERTY(bool initialSetupRequired READ initialSetupRequired NOTIFY initialSetupRequiredChanged)
    Q_PROPERTY(bool authenticationRequired READ authenticationRequired NOTIFY authenticationRequiredChanged)
    Q_PROPERTY(bool pushButtonAuthAvailable READ pushButtonAuthAvailable NOTIFY pushButtonAuthAvailableChanged)
    Q_PROPERTY(bool authenticated READ authenticated NOTIFY authenticatedChanged)
    Q_PROPERTY(CloudConnectionState cloudConnectionState READ cloudConnectionState NOTIFY cloudConnectionStateChanged)
    Q_PROPERTY(QString serverVersion READ serverVersion NOTIFY handshakeReceived)
    Q_PROPERTY(QString jsonRpcVersion READ jsonRpcVersion NOTIFY handshakeReceived)
    Q_PROPERTY(QString serverUuid READ serverUuid NOTIFY handshakeReceived)
    Q_PROPERTY(QString serverQtVersion READ serverQtVersion NOTIFY serverQtVersionChanged)
    Q_PROPERTY(QString serverQtBuildVersion READ serverQtBuildVersion NOTIFY serverQtVersionChanged)
    Q_PROPERTY(QVariantMap certificateIssuerInfo READ certificateIssuerInfo NOTIFY currentConnectionChanged)

public:
    enum CloudConnectionState {
        CloudConnectionStateDisabled,
        CloudConnectionStateUnconfigured,
        CloudConnectionStateConnecting,
        CloudConnectionStateConnected
    };
    Q_ENUM(CloudConnectionState)

    explicit JsonRpcClient(QObject *parent = nullptr);

    QString nameSpace() const override;

    void registerNotificationHandler(JsonHandler *handler, const QString &method);
    void unregisterNotificationHandler(JsonHandler *handler);

    int sendCommand(const QString &method, const QVariantMap &params, QObject *caller = nullptr, const QString &callbackMethod = QString());
    int sendCommand(const QString &method, QObject *caller = nullptr, const QString &callbackMethod = QString());

    NymeaConnection::BearerTypes availableBearerTypes() const;
    NymeaConnection::ConnectionStatus connectionStatus() const;
    bool connected() const;
    NymeaHost* currentHost() const;
    Connection* currentConnection() const;
    QVariantMap certificateIssuerInfo() const;
    bool initialSetupRequired() const;
    bool authenticationRequired() const;
    bool pushButtonAuthAvailable() const;
    bool authenticated() const;
    CloudConnectionState cloudConnectionState() const;
    void deployCertificate(const QByteArray &rootCA, const QByteArray &certificate, const QByteArray &publicKey, const QByteArray &privateKey, const QString &endpoint);

    QString serverVersion() const;
    QString jsonRpcVersion() const;
    QString serverUuid() const;
    QString serverQtVersion();
    QString serverQtBuildVersion();

    // ui methods
    Q_INVOKABLE void connectToHost(NymeaHost *host, Connection *connection = nullptr);
    Q_INVOKABLE void disconnectFromHost();
    Q_INVOKABLE void acceptCertificate(const QString &serverUuid, const QByteArray &pem);

    Q_INVOKABLE bool ensureServerVersion(const QString &jsonRpcVersion);

    Q_INVOKABLE int createUser(const QString &username, const QString &password);
    Q_INVOKABLE int authenticate(const QString &username, const QString &password, const QString &deviceName);
    Q_INVOKABLE int requestPushButtonAuth(const QString &deviceName);
    Q_INVOKABLE void setupRemoteAccess(const QString &idToken, const QString &userId);


signals:
    void availableBearerTypesChanged();
    void connectionStatusChanged();
    void connectedChanged(bool connected);
    void currentHostChanged();
    void currentConnectionChanged();
    void handshakeReceived();
    void newSslCertificate();
    void verifyConnectionCertificate(const QString &serverUuid, const QVariantMap &issuerInfo, const QByteArray &pem);
    void initialSetupRequiredChanged();
    void authenticationRequiredChanged();
    void pushButtonAuthAvailableChanged();
    void authenticatedChanged();
    void tokenChanged();
    void invalidProtocolVersion(const QString &actualVersion, const QString &minimumVersion);
    void authenticationFailed();
    void pushButtonAuthFailed();
    void createUserSucceeded();
    void createUserFailed(const QString &error);
    void cloudConnectionStateChanged();
    void serverQtVersionChanged();

    void responseReceived(const int &commandId, const QVariantMap &response);

private slots:
    void onInterfaceConnectedChanged(bool connected);
    void dataReceived(const QByteArray &data);

    void helloReply(const QVariantMap &params);

private:
    int m_id;
    // < namespace, method> >
    QHash<JsonHandler*, QString> m_notificationHandlerMethods;
    QMultiHash<QString, JsonHandler*> m_notificationHandlers;
    QHash<int, JsonRpcReply *> m_replies;
    NymeaConnection *m_connection = nullptr;

    JsonRpcReply *createReply(const QString &method, const QVariantMap &params, QObject *caller, const QString &callback);

    bool m_connected = false;
    bool m_initialSetupRequired = false;
    bool m_authenticationRequired = false;
    bool m_pushButtonAuthAvailable = false;
    bool m_authenticated = false;
    CloudConnectionState m_cloudConnectionState = CloudConnectionStateDisabled;
    int m_pendingPushButtonTransaction = -1;
    QString m_serverUuid;
    QVersionNumber m_jsonRpcVersion;
    QString m_serverVersion;
    QString m_serverQtVersion;
    QString m_serverQtBuildVersion;
    QByteArray m_token;
    QByteArray m_receiveBuffer;

    void setNotificationsEnabled();
    void getCloudConnectionStatus();

    // json handler
    Q_INVOKABLE void processAuthenticate(const QVariantMap &data);
    Q_INVOKABLE void processCreateUser(const QVariantMap &data);
    Q_INVOKABLE void processRequestPushButtonAuth(const QVariantMap &data);

    Q_INVOKABLE void setNotificationsEnabledResponse(const QVariantMap &params);
    Q_INVOKABLE void notificationReceived(const QVariantMap &data);
    Q_INVOKABLE void isCloudConnectedReply(const QVariantMap &data);
    Q_INVOKABLE void setupRemoteAccessReply(const QVariantMap &data);
    Q_INVOKABLE void deployCertificateReply(const QVariantMap &data);
    Q_INVOKABLE void getVersionsReply(const QVariantMap &data);

    void sendRequest(const QVariantMap &request);

    bool loadPem(const QUuid &serverUud, QByteArray &pem);
    bool storePem(const QUuid &serverUuid, const QByteArray &pem);

};


class JsonRpcReply : public QObject
{
    Q_OBJECT
public:
    explicit JsonRpcReply(int commandId, QString nameSpace, QString method, QVariantMap params = QVariantMap(), QPointer<QObject> caller = QPointer<QObject>(), const QString &callback = QString());
    ~JsonRpcReply();

    int commandId() const;
    QString nameSpace() const;
    QString method() const;
    QVariantMap params() const;
    QVariantMap requestMap();

    QPointer<QObject> caller() const;
    QString callback() const;

private:
    int m_commandId;
    QString m_nameSpace;
    QString m_method;
    QVariantMap m_params;

    QPointer<QObject> m_caller;
    QString m_callback;
};


#endif // NYMEAJSONRPCCLIENT_H
