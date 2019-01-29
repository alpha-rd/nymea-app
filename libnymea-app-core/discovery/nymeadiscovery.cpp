#include "nymeadiscovery.h"
#include "upnpdiscovery.h"
#include "zeroconfdiscovery.h"
#include "bluetoothservicediscovery.h"
#include "connection/awsclient.h"

#include <QUuid>
#include <QBluetoothUuid>
#include <QUrlQuery>
#include <QNetworkConfigurationManager>
#include <QNetworkSession>

NymeaDiscovery::NymeaDiscovery(QObject *parent) : QObject(parent)
{
    m_discoveryModel = new DiscoveryModel(this);
    connect(m_discoveryModel, &DiscoveryModel::deviceAdded, this, [this](DiscoveryDevice *device) {
        if (!m_pendingHostResolutions.contains(device->uuid())) {
            return;
        }
        Connection *c = device->connections()->bestMatch();
        if (!c) {
            qDebug() << "Host found but there isn't a valid candidate yet?";
            connect(device->connections(), &Connections::connectionAdded, this, [this, device](Connection *connection) {
                if (m_pendingHostResolutions.contains(device->uuid())) {
                    qDebug() << "Host" << device->uuid() << "resolved to" << connection->url().toString();
                    m_pendingHostResolutions.removeAll(device->uuid());
                    emit serverUuidResolved(device->uuid(), connection->url().toString());
                }
            });
            return;
        }
        qDebug() << "Host" << device->uuid() << "appeared! Best match is" << c->url();
        m_pendingHostResolutions.removeAll(device->uuid());
        emit serverUuidResolved(device->uuid(), c->url().toString());
    });

    m_upnp = new UpnpDiscovery(m_discoveryModel, this);
    m_zeroConf = new ZeroconfDiscovery(m_discoveryModel, this);

#ifndef Q_OS_IOS
    m_bluetooth = new BluetoothServiceDiscovery(m_discoveryModel, this);
#endif

    m_cloudPollTimer.setInterval(5000);
    connect(&m_cloudPollTimer, &QTimer::timeout, this, [this](){
        if (m_awsClient && m_awsClient->isLoggedIn()) {
            m_awsClient->fetchDevices();
        }
    });


    QNetworkConfigurationManager manager;
    QList<QNetworkConfiguration> configs = manager.allConfigurations(QNetworkConfiguration::Active);

    foreach (const QNetworkConfiguration &config, configs) {
        if (config.purpose() != QNetworkConfiguration::PublicPurpose) {
            continue;
        }
        if (config.bearerType() != QNetworkConfiguration::BearerWLAN && config.bearerType() != QNetworkConfiguration::BearerEthernet) {
            continue;
        }
        qDebug() << "Have Network configuration:" << config.name() << config.bearerTypeName() << config.purpose() << config.type();
    }
}

bool NymeaDiscovery::discovering() const
{
    return m_discovering;
}

void NymeaDiscovery::setDiscovering(bool discovering)
{
    if (m_discovering == discovering)
        return;

    m_discovering = discovering;
    // If we have zeroconf skip upnp. ZeroConf will not do an active discovery and if it's available it'll always have good data
    if (!m_zeroConf->available()) {
        if (discovering) {
            m_upnp->discover();
        } else {
            m_upnp->stopDiscovery();
        }
    }
    if (discovering) {
        // If there's no Zeroconf, use UPnP instead
        if (!m_zeroConf->available()) {
            m_upnp->discover();
        }

        // Always start Bluetooth discovery if HW is available
        if (m_bluetooth) {
            m_bluetooth->discover();
        }

        // start polling cloud
        m_cloudPollTimer.start();
        // If we're logged in, poll right away
        if (m_awsClient && m_awsClient->isLoggedIn()) {
            syncCloudDevices();
            m_awsClient->fetchDevices();
        }
    } else {
        if (!m_zeroConf->available()) {
            m_upnp->stopDiscovery();
        }

        if (m_bluetooth) {
            m_bluetooth->stopDiscovery();
        }

        m_cloudPollTimer.stop();
    }

    emit discoveringChanged();
}

DiscoveryModel *NymeaDiscovery::discoveryModel() const
{
    return m_discoveryModel;
}

AWSClient *NymeaDiscovery::awsClient() const
{
    return m_awsClient;
}

void NymeaDiscovery::setAwsClient(AWSClient *awsClient)
{
    if (m_awsClient != awsClient) {
        m_awsClient = awsClient;
        emit awsClientChanged();
    }

    if (m_awsClient) {
        m_awsClient->fetchDevices();
        connect(m_awsClient, &AWSClient::devicesFetched, this, &NymeaDiscovery::syncCloudDevices);
        syncCloudDevices();
    }
}

void NymeaDiscovery::resolveServerUuid(const QUuid &uuid)
{
    // Do we already know this host?
    DiscoveryDevice *dev = m_discoveryModel->find(uuid);
    if (!dev) {
        qDebug() << "Host" << uuid << "not known yet...";
        m_pendingHostResolutions.append(uuid);
        return;
    }
    Connection *c = dev->connections()->bestMatch();
    if (!c) {
        qDebug() << "Host" << uuid << "is known but doesn't have a usable connection option yet.";
        m_pendingHostResolutions.append(uuid);
        return;
    }
    qDebug() << "Host" << uuid << "is known. Best match is" << c->url();
    emit serverUuidResolved(uuid, c->url().toString());
}

void NymeaDiscovery::syncCloudDevices()
{
    for (int i = 0; i < m_awsClient->awsDevices()->rowCount(); i++) {
        AWSDevice *d = m_awsClient->awsDevices()->get(i);
        DiscoveryDevice *device = m_discoveryModel->find(d->id());
        if (!device) {
            device = new DiscoveryDevice();
            device->setUuid(d->id());
            device->setName(d->name());
            qDebug() << "CloudDiscovery: Adding new host:" << device->name() << device->uuid().toString();
            m_discoveryModel->addDevice(device);
        }
        QUrl url;
        url.setScheme("cloud");
        url.setHost(d->id());
        Connection *conn = device->connections()->find(url);
        if (!conn) {
            conn = new Connection(url, Connection::BearerTypeCloud, true, d->id());
            qDebug() << "CloudDiscovery: Adding new connection to host:" << device->name() << conn->url().toString();
            device->connections()->addConnection(conn);
        }
        conn->setOnline(d->online());
    }

    QList<DiscoveryDevice*> devicesToRemove;
    for (int i = 0; i < m_discoveryModel->rowCount(); i++) {
        DiscoveryDevice *device = m_discoveryModel->get(i);
        for (int j = 0; j < device->connections()->rowCount(); j++) {
            if (device->connections()->get(j)->bearerType() == Connection::BearerTypeCloud) {
                if (m_awsClient->awsDevices()->getDevice(device->uuid().toString()) == nullptr) {
                    device->connections()->removeConnection(j);
                    break;
                }
            }
        }
        if (device->connections()->rowCount() == 0) {
            devicesToRemove.append(device);
        }
    }
    while (!devicesToRemove.isEmpty()) {
        m_discoveryModel->removeDevice(devicesToRemove.takeFirst());
    }
}

