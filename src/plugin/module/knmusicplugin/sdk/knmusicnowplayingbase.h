/*
 * Copyright (C) Kreogist Dev Team <kreogistdevteam@126.com>
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */
#ifndef KNMUSICNOWPLAYINGBASE_H
#define KNMUSICNOWPLAYINGBASE_H

#include "knmusicheaderplayerbase.h"

#include "knmusicglobal.h"

#include <QObject>

using namespace KNMusic;

class KNMusicModel;
class KNMusicProxyModel;
class KNMusicNowPlayingBase : public QObject
{
    Q_OBJECT
public:
    KNMusicNowPlayingBase(QObject *parent = 0):QObject(parent){}
    virtual void setHeaderPlayer(KNMusicHeaderPlayerBase *headerPlayer)=0;

signals:
    void loopStateChanged(int state);

public slots:
    virtual void playNext()=0;
    virtual void playPrevious()=0;
    virtual void onActionPlayingFinished()=0;
    virtual void changeLoopState()=0;
    virtual void playTemporaryFiles(const QStringList &filePaths)=0;
    virtual void setPlayingModel(KNMusicProxyModel *model)=0;
    virtual void playMusic(const int &row)=0;
    virtual void playMusic(const QModelIndex &index)=0;
    virtual void checkRemovedModel(KNMusicModel *model)=0;
};

#endif // KNMUSICNOWPLAYINGBASE_H
