/*
 * Copyright (C) Kreogist Dev Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "knmusicstorebackend.h"

#include "knmusicstorepage.h"

KNMusicStorePage::KNMusicStorePage(QWidget *parent) :
    KNMouseSenseWidget(parent),
    m_backendName(QString())
{
    //Set properties.
    setChangeOpacity(true);
    setSenseRange(0x09, 0x19);
}

void KNMusicStorePage::setBackend(KNMusicStoreBackend *backend)
{
    //Save the backend object name.
    m_backendName=backend->objectName();
}

QString KNMusicStorePage::metadata() const
{
    return m_metadata;
}

void KNMusicStorePage::setMetadata(const QString &metadata)
{
    m_metadata = metadata;
}

QString KNMusicStorePage::backendName() const
{
    //Give the backend name.
    return m_backendName;
}
