#ifndef VPNPROTOCOL_H
#define VPNPROTOCOL_H

#include <QObject>
#include <QString>
#include <QJsonObject>

#include "core/defs.h"
#include "containers/containers_defs.h"

#include "3rd/AdpInfo/netadpinfo.h"

using namespace amnezia;

class QTimer;

class VpnProtocol : public QObject
{
    Q_OBJECT

public:
    explicit VpnProtocol(const QJsonObject& configuration, QObject* parent = nullptr);
    virtual ~VpnProtocol() override = default;

    enum ConnectionState {Unknown, Disconnected, Preparing, Connecting, Connected, Disconnecting, Reconnecting, Error};
    Q_ENUM(ConnectionState)

    static QString textConnectionState(ConnectionState connectionState);

    virtual ErrorCode prepare() { return ErrorCode::NoError; }

    virtual bool isConnected() const;
    virtual bool isDisconnected() const;
    virtual ErrorCode start() = 0;
    virtual void stop() = 0;

    ConnectionState connectionState() const;
    ErrorCode lastError() const;
    QString textConnectionState() const;
    void setLastError(ErrorCode lastError);

    QString routeGateway() const;
    QString vpnGateway() const;

    static VpnProtocol* factory(amnezia::DockerContainer container, const QJsonObject &configuration);

signals:
    void bytesChanged(quint64 receivedBytes, quint64 sentBytes);
    void connectionStateChanged(VpnProtocol::ConnectionState state);
    void timeoutTimerEvent();
    void protocolError(amnezia::ErrorCode e);

    // This signal is emitted when the controller is initialized. Note that the
    // VPN tunnel can be already active. In this case, "connected" should be set
    // to true and the "connectionDate" should be set to the activation date if
    // known.
    // If "status" is set to false, the backend service is considered unavailable.
    void initialized(bool status, bool connected,
                     const QDateTime& connectionDate);
protected slots:
    virtual void onTimeout();

protected:
    void startTimeoutTimer();
    void stopTimeoutTimer();

    virtual void setBytesChanged(quint64 receivedBytes, quint64 sentBytes);
    virtual void setConnectionState(VpnProtocol::ConnectionState state);

    ConnectionState m_connectionState;
    QString m_routeGateway;
    QString m_vpnLocalAddress;
    QString m_vpnGateway;
    adpinfo::NetAdpInfo adpInfo;

    QJsonObject m_rawConfig;

private:
    QTimer* m_timeoutTimer;
    ErrorCode m_lastError;
    quint64 m_receivedBytes;
    quint64 m_sentBytes;
};

#endif // VPNPROTOCOL_H
