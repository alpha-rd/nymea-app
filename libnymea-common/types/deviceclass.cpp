/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Simon Stuerz <simon.stuerz@guh.io>                  *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea:app                                         *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "deviceclass.h"

#include <QDebug>

DeviceClass::DeviceClass(QObject *parent) :
    QObject(parent)
{
}

QUuid DeviceClass::id() const
{
    return m_id;
}

void DeviceClass::setId(const QUuid &id)
{
    m_id = id;
}

QUuid DeviceClass::vendorId() const
{
    return m_vendorId;
}

void DeviceClass::setVendorId(const QUuid &vendorId)
{
    m_vendorId = vendorId;
}

QUuid DeviceClass::pluginId() const
{
    return m_pluginId;
}

void DeviceClass::setPluginId(const QUuid &pluginId)
{
    m_pluginId = pluginId;
}

QString DeviceClass::name() const
{
    return m_name;
}

void DeviceClass::setName(const QString &name)
{
    m_name = name;
}

QString DeviceClass::displayName() const
{
    return m_displayName;
}

void DeviceClass::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
}

QStringList DeviceClass::createMethods() const
{
    return m_createMethods;
}

void DeviceClass::setCreateMethods(const QStringList &createMethods)
{
    m_createMethods = createMethods;
}

DeviceClass::SetupMethod DeviceClass::setupMethod() const
{
    return m_setupMethod;
}

void DeviceClass::setSetupMethod(DeviceClass::SetupMethod setupMethod)
{
    m_setupMethod = setupMethod;
}

QStringList DeviceClass::interfaces() const
{
    return m_interfaces;
}

void DeviceClass::setInterfaces(const QStringList &interfaces)
{
    m_interfaces = interfaces;
}

QString DeviceClass::baseInterface() const
{
    foreach (const QString &interface, m_interfaces) {
        if (interface == "gateway") {
            return "gateway";
        }
        if (interface == "shutter") {
            return "shutter";
        }
        if (interface == "blind") {
            return "blind";
        }
        if (interface == "garagegate") {
            return "garagegate";
        }
        if (interface == "inputtrigger") {
            return "inputtrigger";
        }
        if (interface == "awning") {
            return "awning";
        }
        if (interface == "outputtrigger") {
            return "outputtrigger";
        }
        if (interface == "light") {
            return "light";
        }
        if (interface == "sensor") {
            return "sensor";
        }
        if (interface == "weather") {
            return "weather";
        }
        if (interface == "media") {
            return "media";
        }
        if (interface == "button" || interface == "powerswitch") {
            return "button";
        }
        if (interface == "notifications") {
            return "notifications";
        }
        if (interface == "powersocket") {
            return "powersocket";
        }
        if (interface == "smartmeter") {
            return "smartmeter";
        }
        if (interface == "heating") {
            return "heating";
        }
        if (interface == "evcharger") {
            return "evcharger";
        }
    }
    return "uncategorized";
}

bool DeviceClass::browsable() const
{
    return m_browsable;
}

void DeviceClass::setBrowsable(bool browsable)
{
    m_browsable = browsable;
}

ParamTypes *DeviceClass::paramTypes() const
{
    return m_paramTypes;
}

void DeviceClass::setParamTypes(ParamTypes *paramTypes)
{
    if (m_paramTypes) {
        m_paramTypes->deleteLater();
    }
    m_paramTypes = paramTypes;
    emit paramTypesChanged();
}

ParamTypes *DeviceClass::settingsTypes() const
{
    return m_settingsTypes;
}

void DeviceClass::setSettingsTypes(ParamTypes *settingsTypes)
{
    if (m_settingsTypes) {
        m_settingsTypes->deleteLater();
    }
    m_settingsTypes = settingsTypes;
    emit settingsTypesChanged();
}

ParamTypes *DeviceClass::discoveryParamTypes() const
{
    return m_discoveryParamTypes;
}

void DeviceClass::setDiscoveryParamTypes(ParamTypes *paramTypes)
{
    if (m_discoveryParamTypes) {
        m_discoveryParamTypes->deleteLater();
    }
    m_discoveryParamTypes = paramTypes;
    emit discoveryParamTypesChanged();
}

StateTypes *DeviceClass::stateTypes() const
{
    return m_stateTypes;
}

void DeviceClass::setStateTypes(StateTypes *stateTypes)
{
    if (m_stateTypes) {
        m_stateTypes->deleteLater();
    }
    m_stateTypes = stateTypes;
    emit stateTypesChanged();
}

EventTypes *DeviceClass::eventTypes() const
{
    return m_eventTypes;
}

void DeviceClass::setEventTypes(EventTypes *eventTypes)
{
    if (m_eventTypes) {
        m_eventTypes->deleteLater();
    }
    m_eventTypes = eventTypes;
    emit eventTypesChanged();
}

ActionTypes *DeviceClass::actionTypes() const
{
    return m_actionTypes;
}

void DeviceClass::setActionTypes(ActionTypes *actionTypes)
{
    if (m_actionTypes) {
        m_actionTypes->deleteLater();
    }
    m_actionTypes = actionTypes;
    emit actionTypesChanged();
}

ActionTypes *DeviceClass::browserItemActionTypes() const
{
    return m_browserItemActionTypes;
}

void DeviceClass::setBrowserItemActionTypes(ActionTypes *browserActionTypes)
{
    if (m_browserItemActionTypes) {
        m_browserItemActionTypes->deleteLater();
    }
    m_browserItemActionTypes = browserActionTypes;
    emit browserItemActionTypesChanged();
}

bool DeviceClass::hasActionType(const QString &actionTypeId)
{
    foreach (ActionType *actionType, m_actionTypes->actionTypes()) {
        if (actionType->id() == actionTypeId) {
            return true;
        }
    }
    return false;
}
