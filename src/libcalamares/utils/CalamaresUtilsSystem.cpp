/* === This file is part of Calamares - <http://github.com/calamares> ===
 *
 *   Copyright 2014, Teo Mrnjavac <teo@kde.org>
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

#include "CalamaresUtilsSystem.h"

#include "utils/Logger.h"
#include "JobQueue.h"
#include "GlobalStorage.h"

#include <QDir>
#include <QProcess>

namespace CalamaresUtils
{

int mount( const QString& devicePath,
           const QString& mountPoint,
           const QString& filesystemName,
           const QString& options )
{
    if ( devicePath.isEmpty() || mountPoint.isEmpty() )
        return -3;

    QDir mountPointDir( mountPoint );
    if ( !mountPointDir.exists() )
    {
        bool ok = mountPointDir.mkpath( mountPoint );
        if ( !ok )
            return -3;
    }

    QString program( "mount" );
    QStringList args = { devicePath, mountPoint };

    if ( !filesystemName.isEmpty() )
        args << "-t" << filesystemName;

    if ( !options.isEmpty() )
        args << "-o" << options;

    return QProcess::execute( program, args );
}

int chrootCall( const QStringList& args,
                const QString& stdInput,
                int timeoutSec )
{
    if ( !Calamares::JobQueue::instance() )
        return -3;

    Calamares::GlobalStorage* gs = Calamares::JobQueue::instance()->globalStorage();
    if ( !gs || !gs->contains( "rootMountPoint" ) )
    {
        cLog() << "No rootMountPoint in global storage";
        return -3;
    }

    QString destDir = gs->value( "rootMountPoint" ).toString();
    if ( !QDir( destDir ).exists() )
    {
        cLog() << "rootMountPoint points to a dir which does not exist";
        return -3;
    }

    QString program( "chroot" );
    QStringList arguments = { destDir };
    arguments << args;

    QProcess process;
    process.setProgram( program );
    process.setArguments( arguments );
    process.setProcessChannelMode( QProcess::MergedChannels );

    cLog() << "Running" << program << arguments;
    process.start();
    if ( !process.waitForStarted() )
    {
        cLog() << "Process failed to start" << process.error();
        return -2;
    }

    if ( !stdInput.isEmpty() )
    {
        process.write( stdInput.toLocal8Bit() );
        process.closeWriteChannel();
    }

    if ( !process.waitForFinished( timeoutSec ? ( timeoutSec * 1000 ) : -1 ) )
    {
        cLog() << "Timed out. output so far:";
        cLog() << process.readAllStandardOutput();
        return -4;
    }

    cLog() << "Output:";
    cLog() << process.readAllStandardOutput();

    if ( process.exitStatus() == QProcess::CrashExit )
    {
        cLog() << "Process crashed";
        return -1;
    }

    cLog() << "Finished. Exit code:" << process.exitCode();
    return process.exitCode();
}


int chrootCall( const QString& command,
                const QString& stdInput,
                int timeoutSec )
{
    return chrootCall( QStringList() = { command },
                       stdInput,
                       timeoutSec );
}


}
