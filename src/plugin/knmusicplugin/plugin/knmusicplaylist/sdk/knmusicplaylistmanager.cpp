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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QThread>

#include "knutil.h"

#include "knmusicplaylistutil.h"
#include "knmusicplaylistengine.h"
#include "knmusicplaylistmodel.h"
#include "knmusicplaylistlistmodel.h"

#include "knmusicplaylistmanager.h"

#include <QDebug>

#define PlaylistListFileName "/Playlist.mudb"
#define PlaylistListVersion 3

KNMusicPlaylistManager::KNMusicPlaylistManager(QObject *parent) :
    QObject(parent),
    m_playlistList(new KNMusicPlaylistListModel(this)),
    m_playlistDirPath(QString()),
    m_isPlaylistListLoaded(false),
    m_playlistEngine(new KNMusicPlaylistEngine()),
    m_workingThread(new QThread())
{
    //Move the playlist engine to working thread.
    m_playlistEngine->moveToThread(m_workingThread);

    //Link the playlist list model with the manager.
    connect(m_playlistList, &KNMusicPlaylistListModel::requireShowContent,
            this, &KNMusicPlaylistManager::requireShowContent);
    connect(m_playlistList, &KNMusicPlaylistListModel::requireHideContent,
            this, &KNMusicPlaylistManager::requireHideContent);
    //Start working thread.
    m_workingThread->start();
}

KNMusicPlaylistManager::~KNMusicPlaylistManager()
{
    //Quit and wait the working thread.
    m_workingThread->quit();
    m_workingThread->wait();
    //Remove the working thread.
    m_workingThread->deleteLater();
}

KNMusicPlaylistListModel *KNMusicPlaylistManager::playlistList()
{
    return m_playlistList;
}

KNMusicPlaylistModel *KNMusicPlaylistManager::playlist(const QModelIndex &index)
{
    return m_playlistList->playlist(index);
}

QModelIndex KNMusicPlaylistManager::createPlaylist()
{
    //Generate the playlist.
    KNMusicPlaylistModel *model=new KNMusicPlaylistModel(m_workingThread,
                                                         m_playlistList);
    //Set the generate the title.
    model->setTitle(generateTitle());
    //Add the model to the playlist list, and give back the model index in the
    //model list model.
    return m_playlistList->append(model);
}

void KNMusicPlaylistManager::installPlaylistParser(
        KNMusicPlaylistParser *parser)
{
    //Add the parser to playlist engine.
    m_playlistEngine->installPlaylistParser(parser);
}

QStringList KNMusicPlaylistManager::playlistFilter()
{
    //Generate the filter list.
    QStringList filter;
    //Initial the all support filter.
    QString allSupportFilter=tr("All Support Playlist") + "(*.mplst";
    //Add mu playlist to the first one.
    filter.append(tr("Mu playlist") + " (*.mplst)");
    //Get the types and suffixs.
    QStringList types, suffixs;
    m_playlistEngine->getTypeAndSuffix(types, suffixs);
    //Add all types and the suffixs to the all support filter and the filter.
    for(int i=0; i<types.size(); i++)
    {
        //Add to all support filter.
        allSupportFilter.append(" " + suffixs.at(i));
        //Add to filter list.
        filter.append(types.at(i) + " (" + suffixs.at(i) + ")");
    }
    //Add the left half bracket to the all support filter.
    allSupportFilter.append(")");
    //Insert the all support filter to the first one.
    filter.prepend(allSupportFilter);
    //Give back the filter.
    return filter;
}

QString KNMusicPlaylistManager::generateTitle(const QString &preferName)
{
    //Check whether the prefer name won't need to be used to change.
    if(!preferName.isEmpty() &&
            !m_playlistList->findTitle(preferName))
    {
        return preferName;
    }
    //Now we need to generate a name. Start counting at 2, because you have
    //already get the 'Playlist', and the new one should be name as 'Playlist 2'
    quint32 counter=2;
    //Treat the prefer name as the base name.
    QString baseName=preferName;
    //Generate the base name of the file.
    if(preferName.isEmpty())
    {
        //Then it should be a 'New playlist'.
        baseName=tr("New Playlist");
        //If we are here, there may no other playlist named this, check this
        //name first.
        if(!m_playlistList->findTitle(baseName))
        {
            return baseName;
        }
    }
    else
    {
        //Check is the caption's format is like "Title 2"
        int lastSpacePos=preferName.lastIndexOf(' ');
        //If we cannot find the space, means this cannot be a format like that.
        if(lastSpacePos!=-1)
        {
            //Tried to translate the last part to a number.
            //Get the content.
            QString numerousContent=preferName.mid(lastSpacePos+1);
            //Set a bool varible to save the translate result.
            bool translateResult=false;
            //Tried to do the translation.
            int lastIndex=numerousContent.toInt(&translateResult);
            //If it DOES have a number, replace the base name and the counter.
            if(translateResult)
            {
                //Save the new base name.
                baseName=preferName.left(lastSpacePos);
                //Save the new counter.
                counter=lastIndex+1;
            }
        }
    }
    //Link the base name and the counter.
    QString countedName=baseName + " " + QString::number(counter);
    //Check if the counted name can be found in the list, if it cannot be find,
    //then add the counter, relink the name, and check it again.
    while(!m_playlistList->findTitle(countedName))
    {
        //Add counter.
        counter++;
        //Relink the name.
        countedName=baseName + " " + QString::number(counter);
    }
    //Now the counted name should be the valid name.
    return countedName;
}

bool KNMusicPlaylistManager::isPlaylistListLoaded() const
{
    return m_isPlaylistListLoaded;
}

void KNMusicPlaylistManager::setPlaylistListLoaded(bool isPlaylistListLoaded)
{
    m_isPlaylistListLoaded = isPlaylistListLoaded;
}

QString KNMusicPlaylistManager::playlistDirPath() const
{
    return m_playlistDirPath;
}

void KNMusicPlaylistManager::setPlaylistDirPath(const QString &playlistDirPath)
{
    m_playlistDirPath = playlistDirPath;
}

void KNMusicPlaylistManager::loadPlaylistList()
{
    //Get the playlist list file.
    QFile playlistListFile(m_playlistDirPath + PlaylistListFileName);
    //Check the existance and try to open the file in read only mode.
    if(!playlistListFile.exists() ||
            !playlistListFile.open(QIODevice::ReadOnly))
    {
        return;
    }
    //Read and parse the json object from the playlist list file.
    QJsonObject playlistListObject=
            QJsonDocument::fromJson(playlistListFile.readAll()).object();
    //Close the playlist list file.
    playlistListFile.close();
    //Check the valid of the json object, then check the playlist list version,
    //we can only load one version.
    if(playlistListObject.isEmpty() ||
            playlistListObject.value("Version").toInt()!=PlaylistListVersion)
    {
        return;
    }
    //Get the playlist file path content.
    QJsonArray playlistPaths=playlistListObject.value("Playlists").toArray();
    //A string list used to storage the failed loaded file path.
    QStringList failedPaths;
    //Generate those models.
    for(auto i=playlistPaths.constBegin(); i!=playlistPaths.constEnd(); ++i)
    {
        //Get the file path.
        QString playlistPath=(*i).toString();
        //Load the playlist file to a playlist model, but don't need to parse
        //it. If it's failed to load the playlist, add it to failed path list.
        if(!loadPlaylist(playlistPath))
        {
            failedPaths.append(playlistPath);
        }
    }
    //Check whether the failedPaths is empty, if it's not empty, a message box
    //should be display to hint the user there's invalid playlist.
    if(!failedPaths.isEmpty())
    {
        //!FIXME: Raise message box to hint the user there's invalid playlist.
        ;
    }
}

bool KNMusicPlaylistManager::loadPlaylist(const QString &filePath)
{
    //Get the playlist file.
    QFile playlistFile(filePath);
    //Tried to open it as read only mode.
    if(!playlistFile.open(QIODevice::ReadOnly))
    {
        return false;
    }
    //Get playlist content object from the playlist file.
    QJsonObject contentObject=
            QJsonDocument::fromJson(playlistFile.readAll()).object();
    //Close the playlist file.
    playlistFile.close();
    //Check the content object, the following case will just make it failed to
    //load the playlist.
    // * The content object is empty.
    // * It doesn't contain Version, Name or Songs value.
    if(contentObject.isEmpty() ||
            contentObject.value("Version").toInt()!=PlaylistListVersion ||
            !contentObject.contains("Name") ||
            !contentObject.contains("Songs"))
    {
        return false;
    }
    //Generate the playlist.
    KNMusicPlaylistModel *model=new KNMusicPlaylistModel(m_workingThread,
                                                         m_playlistList);
    //Set the file path of the playlist.
    model->setFilePath(filePath);
    //Set the playlist meta data from the json object.
    model->setTitle(contentObject.value("Name").toString());
    model->setContentData(contentObject.value("Songs").toArray());
    //Reset the changed state.
    model->setChanged(false);
    //Add this playlist model to playlist list.
    m_playlistList->append(model);
    //Load success.
    return true;
}

bool KNMusicPlaylistManager::savePlaylist(KNMusicPlaylistModel *model,
                                          QString filePath)
{
    //Check te model first.
    if(!model)
    {
        return false;
    }
    //Check if the file path is empty.
    if(filePath.isEmpty())
    {
        //We will use the file path from the model.
        filePath=model->filePath();
    }
    //Get the playlist file.
    QFile playlistFile(filePath);
    //Try to open the file as write mode.
    if(!playlistFile.open(QIODevice::WriteOnly))
    {
        return false;
    }
    //Create the playlist content.
    QJsonArray songs;
    //Write all the data to the content.
    for(int row=0; row<model->rowCount(); ++row)
    {
        //Translate the detail info from the model to a QJsonObject, and add it
        //to the playlist content.
        songs.append(
                    KNMusicPlaylistUtil::detailInfoToObject(
                        model->rowDetailInfo(row)));
    }
    //Create the playlist object.
    QJsonObject playlistObject;
    //Set the data of the playlist.
    playlistObject["Version"]=PlaylistListVersion;
    playlistObject["Name"]=model->title();
    playlistObject["Songs"]=songs;
    //Create playlist document
    QJsonDocument playlistDocument;
    //Set the playlist object to document object.
    playlistDocument.setObject(playlistObject);
    //Write document data to the file.
    playlistFile.write(playlistDocument.toJson(QJsonDocument::Indented));
    //Close the file.
    playlistFile.close();
    //Import complete.
    return true;
}

QModelIndex KNMusicPlaylistManager::importPlaylist(const QString &filePath)
{
    //Use the playlist engine to parse the file.
    KNMusicPlaylistModel *model=m_playlistEngine->read(filePath);
    //Check the model.
    if(model)
    {
        //If it's not null, add to playlist list, Import successfully.
        return m_playlistList->append(model);
    }
    //Or else failed.
    return QModelIndex();
}

QModelIndex KNMusicPlaylistManager::importPlaylists(
        const QStringList &filePaths)
{
    //Generate the first index.
    QModelIndex firstImport=QModelIndex();
    //Import all the files.
    for(auto i=filePaths.constBegin(); i!=filePaths.end(); ++i)
    {
        //Get the model index via import the playlist.
        QModelIndex playlistIndex(importPlaylist(*i));
        //Check whether the index is valid.
        if(playlistIndex.isValid() && (!firstImport.isValid()))
        {
            //Save the current index.
            firstImport=playlistIndex;
        }
    }
    //Give back the first import index.
    return firstImport;
}

bool KNMusicPlaylistManager::exportPlaylist(KNMusicPlaylistModel *model,
                                            const QString &filePath,
                                            const int &parserIndex)
{
    //Check the parser index.
    if(parserIndex==0)
    {
        //When the parser is 0, we will export the playlist using mu playlist.
        return savePlaylist(model, filePath);
    }
    //Or else, we will use the engine to export the playlist.
    //Notice that the parser index should -1. Because the index start inside the
    //playlist engine is 0. Here we start at 1.
    //The 0 of here is mu playlist.
    m_playlistEngine->write(model, filePath, parserIndex-1);
}
