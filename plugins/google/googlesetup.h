/*
 *   SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QObject>
#include <qqmlregistration.h>

#include <KGAPI/Account>
#include <KGAPI/AuthJob>

#include <QCoro/QCoroCore>

#include "accountbuilder.h"

class GoogleSetup : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(AccountBuilder *builder READ builder WRITE setBuilder NOTIFY builderChanged)
public:
    GoogleSetup(QObject *parent = nullptr);

    AccountBuilder *builder() const;
    void setBuilder(AccountBuilder *builder);
    Q_SIGNAL void builderChanged();

    QCoro::Task<void> doWork();

private:
    AccountBuilder *m_builder;
};
