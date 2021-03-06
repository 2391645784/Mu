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

#ifndef KNMUSICSTORENETEASEBACKEND_H
#define KNMUSICSTORENETEASEBACKEND_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QScopedPointer>
#include <QMap>
#include <QDomElement>
#include <QLinkedList>

#include "knconnectionhandler.h"

#include "../../sdk/knmusicstoreutil.h"

#include "../../sdk/knmusicstorebackend.h"

using namespace MusicStoreUtil;

class QTimer;
/*!
 * \brief The KNMusicStoreNeteaseBackend class provides the music store backend
 * plugin for Netease cloud music. The API is not provided by the offcial
 * documentation. Those APIs come from the following projects:
 *   1. wu-nerd/dmusic-plugin-NeteaseCloudMusic
 */
class KNMusicStoreNeteaseBackend : public KNMusicStoreBackend
{
    Q_OBJECT
public:
    /*!
     * \brief Construct a KNMusicStoreNeteaseBackend object.
     * \param parent The parent object.
     */
    explicit KNMusicStoreNeteaseBackend(QObject *parent = 0);
    ~KNMusicStoreNeteaseBackend();

    /*!
     * \brief Reimplmented from KNMusicStoreBackend::setWorkingThread().
     */
    void setWorkingThread(QThread *thread) Q_DECL_OVERRIDE;

signals:

public slots:
    /*!
     * \brief Reimplmented from KNMusicStoreBackend::showHome().
     */
    void showHome() Q_DECL_OVERRIDE;

    /*!
     * \brief Reimplmented from KNMusicStoreBackend::showArtist().
     */
    void showArtist(const QString &artistInfo) Q_DECL_OVERRIDE;

    /*!
     * \brief Reimplmented from KNMusicStoreBackend::showAlbum().
     */
    void showAlbum(const QString &albumInfo) Q_DECL_OVERRIDE;

    /*!
     * \brief Reimplmented from KNMusicStoreBackend::showSingleSong().
     */
    void showSingleSong(const QString &songInfo) Q_DECL_OVERRIDE;

    /*!
     * \brief Reimplmented from KNMusicStoreBackend::setTimeout().
     */
    void setTimeout(int seconds) Q_DECL_OVERRIDE;

private slots:
    void onReplyFinished(QNetworkReply *reply);
    void onHomeNewArtworkReply(QNetworkReply *reply,
                               QList<uint> &urlMap,
                               int albumArtRequestType);
    void onArtistDetailReply(QNetworkReply *reply);
    void onAlbumDetailReply(QNetworkReply *reply);
    void onSingleDetailReply(QNetworkReply *reply);
    void onSingleLyricsReply(QNetworkReply *reply);
    void onTimeoutTick();

private:
    enum NeteaseReplyTypes
    {
        NeteaseHomeListNewAlbum,
        NeteaseHomeListNewSongs,
        NeteaseHomeListBillboard,
        NeteaseHomeListOricon,
        NeteaseHomeListItunes,
        NeteaseHomeListTopSongs,
        NeteaseHomeListNewAlbumArt,
        NeteaseHomeListNewSongArt,
        NeteaseArtistDetails,
        NeteaseAlbumDetails,
        NeteaseAlbumArt,
        NeteaseSingleDetails,
        NeteaseSingleLyricsText,
        NeteaseSingleAlbumArt
    };

    enum NeteaseRequestType
    {
        NeteaseGet
    };

    struct NetworkRequestItem
    {
        QNetworkRequest request;
        int requestType;
        int replyType;
    };

    inline void insertRequest(const QString &url,
                              int requestType,
                              int replyType,
                              bool useHeader=true);
    inline void launchRequest(const QNetworkRequest &request,
                              int requestType,
                              int replyType);
    inline QJsonArray getSongDataList(QNetworkReply *reply,
                                      int maximumItem=10,
                                      bool fetchAlbum=false,
                                      QList<uint> *artworkList=nullptr);
    inline QNetworkRequest generateRequest();
    inline void decreaseHomeCounter();
    inline void resetManager();
    inline void startTimeoutTick();
    inline void stopTimeoutTick();
    QString m_listUrls[HomeSongListCount];
    QLinkedList<NetworkRequestItem> m_queueRequest;
    QList<uint> m_albumArtworkList, m_songArtworkList;
    KNConnectionHandler m_accessManagerHandler;
    QScopedPointer<QNetworkAccessManager> m_accessManager;
    QMap<QNetworkReply *, int> m_replyMap;
    QMap<QNetworkReply *, int> m_replyTimeout;
    QTimer *m_timeout;
    int m_timeoutLimit, m_pipelineLimit, m_homeListCounter;
};

#endif // KNMUSICSTORENETEASEBACKEND_H
