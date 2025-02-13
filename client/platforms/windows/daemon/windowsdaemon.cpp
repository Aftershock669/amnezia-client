/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsdaemon.h"

#include <Windows.h>

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLocalSocket>
#include <QNetworkInterface>
#include <QTextStream>
#include <QtGlobal>

#include "dnsutilswindows.h"
#include "leakdetector.h"
#include "logger.h"
#include "core/networkUtilities.h"
#include "platforms/windows/windowscommons.h"
#include "platforms/windows/windowsservicemanager.h"
#include "windowsfirewall.h"

namespace {
Logger logger("WindowsDaemon");
}

WindowsDaemon::WindowsDaemon() : Daemon(nullptr), m_splitTunnelManager(this) {
  MZ_COUNT_CTOR(WindowsDaemon);

  m_wgutils = new WireguardUtilsWindows(this);
  m_dnsutils = new DnsUtilsWindows(this);

  connect(m_wgutils, &WireguardUtilsWindows::backendFailure, this,
          &WindowsDaemon::monitorBackendFailure);
  connect(this, &WindowsDaemon::activationFailure,
          []() { WindowsFirewall::instance()->disableKillSwitch(); });
}

WindowsDaemon::~WindowsDaemon() {
  MZ_COUNT_DTOR(WindowsDaemon);
  logger.debug() << "Daemon released";
}

void WindowsDaemon::prepareActivation(const InterfaceConfig& config, int inetAdapterIndex) {
  // Before creating the interface we need to check which adapter
  // routes to the server endpoint
  if (inetAdapterIndex == 0) {
      auto serveraddr = QHostAddress(config.m_serverIpv4AddrIn);
      m_inetAdapterIndex = NetworkUtilities::AdapterIndexTo(serveraddr);
  } else {
      m_inetAdapterIndex = inetAdapterIndex;
  }
}

void WindowsDaemon::activateSplitTunnel(const InterfaceConfig& config, int vpnAdapterIndex) {
  if (config.m_vpnDisabledApps.length() > 0) {
      m_splitTunnelManager.start(m_inetAdapterIndex, vpnAdapterIndex);
      m_splitTunnelManager.setRules(config.m_vpnDisabledApps);
  } else {
      m_splitTunnelManager.stop();
  }
}

bool WindowsDaemon::run(Op op, const InterfaceConfig& config) {
  if (op == Down) {
    m_splitTunnelManager.stop();
    return true;
  }

  if (op == Up) {
    logger.debug() << "Tunnel UP, Starting SplitTunneling";
    if (!WindowsSplitTunnel::isInstalled()) {
      logger.warning() << "Split Tunnel Driver not Installed yet, fixing this.";
      WindowsSplitTunnel::installDriver();
    }
  }

  activateSplitTunnel(config);

  return true;
}

void WindowsDaemon::monitorBackendFailure() {
  logger.warning() << "Tunnel service is down";

  emit backendFailure();
  deactivate();
}
