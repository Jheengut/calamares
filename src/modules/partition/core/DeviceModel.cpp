/* === This file is part of Calamares - <http://github.com/calamares> ===
 *
 *   Copyright 2014, Aurélien Gâteau <agateau@kde.org>
 *
 *   Calamares is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Calamares is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Calamares. If not, see <http://www.gnu.org/licenses/>.
 */
#include <core/DeviceModel.h>

#include <core/PartitionModel.h>

// CalaPM
#include <core/device.h>

// KF5
#include <KFormat>

// STL
#include <algorithm>

DeviceModel::DeviceModel( QObject* parent )
    : QAbstractListModel( parent )
{
}

DeviceModel::~DeviceModel()
{
}

void
DeviceModel::init( const QList< Device* >& devices )
{
    beginResetModel();
    m_devices = devices;
    std::sort( m_devices.begin(), m_devices.end(), []( const Device* dev1, const Device* dev2 )
    {
        return dev1->deviceNode() < dev2->deviceNode();
    } );
    endResetModel();
}

int
DeviceModel::rowCount( const QModelIndex& parent ) const
{
    return parent.isValid() ? 0 : m_devices.count();
}

QVariant
DeviceModel::data( const QModelIndex& index, int role ) const
{
    int row = index.row();
    if ( row < 0 || row >= m_devices.count() )
        return QVariant();

    Device* device = m_devices.at( row );

    switch ( role )
    {
    case Qt::DisplayRole:
        if ( device->name().isEmpty() )
            return device->deviceNode();
        else
            return tr( "%1 - %2 (%3)" )
                   .arg( device->name() )
                   .arg( KFormat().formatByteSize( device->capacity() ) )
                   .arg( device->deviceNode() );
    default:
        return QVariant();
    }
}

Device*
DeviceModel::deviceForIndex( const QModelIndex& index ) const
{
    int row = index.row();
    if ( row < 0 || row >= m_devices.count() )
        return nullptr;
    return m_devices.at( row );
}
