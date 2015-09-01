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
#include <QScrollBar>
#include <QTimeLine>
#include <QHelpEvent>
#include <QMouseEvent>
#include <QPainter>

#include "knthememanager.h"
#include "knconnectionhandler.h"

#include "knmusicsearchbase.h"
#include "knmusicproxymodel.h"
#include "knmusicmodel.h"
#include "knmusicnowplayingbase.h"
#include "knmusicsolomenubase.h"
#include "knmusicmultimenubase.h"
#include "knmusictreeviewheader.h"
#include "knmusicratingdelegate.h"
#include "knmusicdetailtooltipbase.h"

#include "knmusictreeviewbase.h"

#include <QDebug>

#define MaxOpacity 0x20
#define FontBase 0xBF

KNMusicTreeViewBase::KNMusicTreeViewBase(QWidget *parent, KNMusicTab *tab) :
    QTreeView(parent),
    m_musicTab(tab),
    m_mouseAnime(new QTimeLine(200, this)),
    m_animate(true),
    m_proxyModel(nullptr),
    m_initialLoad(true),
    m_pressed(false)
{
    //Set properties.
    setAllColumnsShowFocus(true);
    setAlternatingRowColors(false); //We will use our own alternating drawing.
    setContentsMargins(0, 0, 0, 0);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDropIndicatorShown(true);
    setFrameShape(QFrame::NoFrame);
    setIndentation(0);
    setMouseTracking(true);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setUniformRowHeights(true);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    //Set scroll bar policies.
    horizontalScrollBar()->setSingleStep(5);
    horizontalScrollBar()->setPageStep(5);
    verticalScrollBar()->setSingleStep(4);
    verticalScrollBar()->setPageStep(4);

    //Configure the time line.
    m_mouseAnime->setEasingCurve(QEasingCurve::OutCubic);
    m_mouseAnime->setUpdateInterval(10);
    //Link the time line.
    connect(m_mouseAnime, &QTimeLine::frameChanged,
            this, &KNMusicTreeViewBase::onActionMouseInOut);

    //Generate the music tree view animation header.
    KNMusicTreeViewHeader *header=new KNMusicTreeViewHeader(this);
    //Link the reqirement.
    connect(header, &KNMusicTreeViewHeader::requireResizeColumnToContents,
            this, &KNMusicTreeViewBase::resizeColumnToContents);
    //Set the header view.
    setHeader(header);

    //Set the rating delegate for Rating and AlbumRating row.
    setItemDelegateForColumn(Rating,
                             new KNMusicRatingDelegate(this));
    setItemDelegateForColumn(AlbumRating,
                             new KNMusicRatingDelegate(this));

    //Set the search shortcut.
    QAction *searchAction=new QAction(this);
    searchAction->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_F));
    connect(searchAction, &QAction::triggered,
            [=]
            {
                //Check whether the search plugin is loaded.
                if(knMusicGlobal->search())
                {
                    knMusicGlobal->search()->onActionSearchShortcut(this);
                }
            });
    addAction(searchAction);

    //Link the tree view signal and slot.
    connect(this, &KNMusicTreeViewBase::activated,
            this, &KNMusicTreeViewBase::onActionActivate);

    //Link with theme manager.
    connect(knTheme, &KNThemeManager::themeChange,
            this, &KNMusicTreeViewBase::onActionThemeUpdate);
}

KNMusicTreeViewBase::~KNMusicTreeViewBase()
{
    ;
}

void KNMusicTreeViewBase::enterEvent(QEvent *event)
{
    //Do original enter event.
    QTreeView::enterEvent(event);
    //Check if animate switch is on.
    if(m_animate)
    {
        //Start mouse in anime.
        startAnime(MaxOpacity);
    }
}

void KNMusicTreeViewBase::leaveEvent(QEvent *event)
{
    //Do original leave event.
    QTreeView::leaveEvent(event);
    //Check if animate switch is on.
    if(m_animate)
    {
        //Start mouse leave anime.
        startAnime(0);
    }
}

void KNMusicTreeViewBase::drawRow(QPainter *painter,
                                  const QStyleOptionViewItem &options,
                                  const QModelIndex &index) const
{
    //Check if this row is odd or oven, and then take one row to have
    //alternative background.
    if(index.row() & 1)
    {
        //Draw the alternative background.
        painter->fillRect(QRect(options.rect.x(),
                                options.rect.y(),
                                width(),
                                options.rect.height()),
                          palette().color(QPalette::AlternateBase));
    }
    //Paint the other parts of the row.
    QTreeView::drawRow(painter, options, index);
}

void KNMusicTreeViewBase::mousePressEvent(QMouseEvent *event)
{
    //Do the original pressed event.
    QTreeView::mousePressEvent(event);
    //Set pressed flag.
    m_pressed=true;
}

void KNMusicTreeViewBase::mouseReleaseEvent(QMouseEvent *event)
{
    //Do the original release event.
    QTreeView::mouseReleaseEvent(event);
    //Check we has been pressed on this widget before.
    if(m_pressed)
    {
        //Reset the pressed flag.
        m_pressed=false;
        //Check is button right button and position is in the treeview.
        if(event->button()==Qt::RightButton && rect().contains(event->pos()))
        {
            //Hide the detail tooltip first.
            if(knMusicGlobal->detailTooltip())
            {
                knMusicGlobal->detailTooltip()->hide();
            }
            //According to the selected rows, display different menu.
            switch(selectionModel()->selectedRows().size())
            {
            case 0:
                break;
            case 1:
                showSoloMenu(event->pos());
                break;
            default:
                showMultiMenu(event->pos());
                break;
            }
        }
    }
}

void KNMusicTreeViewBase::keyReleaseEvent(QKeyEvent *event)
{
    //Judge according to the released key.
    switch(event->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    {
        //Check the current index first.
        if(currentIndex().isValid())
        {
            //Hack this as the activate action.
            onActionActivate(currentIndex());
        }
        break;
    }
    default:
        //Do the orignal key release event.
        QTreeView::keyReleaseEvent(event);
    }
}

void KNMusicTreeViewBase::moveToFirst(const int &logicalIndex)
{
    //The overdriven function: move section.
    header()->moveSection(header()->visualIndex(logicalIndex), 0);
}

bool KNMusicTreeViewBase::event(QEvent *event)
{
    switch(event->type())
    {
    case QEvent::ToolTip:
    case QEvent::ToolTipChange:
    {
        //Cast the event as a help event.
        QHelpEvent *helpEvent=static_cast<QHelpEvent *>(event);
        //Get the position of the tooltip index.
        QPoint indexPosition=QPoint(helpEvent->pos().x(),
                                    helpEvent->pos().y()-header()->height());
        //Show the detail tooltip at the index position
        return showDetailTooltip(indexPosition);
    }
    default:
        //Process other kinds of events.
        return QTreeView::event(event);
    }
}

void KNMusicTreeViewBase::wheelEvent(QWheelEvent *event)
{
    //Do original wheel event.
    QTreeView::wheelEvent(event);
    //When user using wheel, means the user don't want to preview the current
    //thing.
    //Hide the preview tooltip.
    if(knMusicGlobal->detailTooltip() &&
            knMusicGlobal->detailTooltip()->isVisible())
    {
        knMusicGlobal->detailTooltip()->hide();
    }

}

void KNMusicTreeViewBase::onActionThemeUpdate()
{
    //Get the new palette from theme manager, and set it.
    setPalette(knTheme->getPalette(objectName()));
    //Update the palette.
    onActionMouseInOut(0);
}

void KNMusicTreeViewBase::onActionMouseInOut(const int &frame)
{
    //Get the palette.
    QPalette pal=palette();
    //Update the text palette color.
    QColor color=pal.color(QPalette::Text);
    color.setAlpha(FontBase+(frame<<1));
    pal.setColor(QPalette::Text, color);
    //Update the alternating base color.
    color=pal.color(QPalette::AlternateBase);
    color.setAlpha(frame);
    pal.setColor(QPalette::AlternateBase, color);
    //Update the button color.
    color=pal.color(QPalette::Button);
    color.setHsv(color.hue(), color.saturation(), frame<<1);
    pal.setColor(QPalette::Button, color);
    //Set the palette.
    setPalette(pal);
}

void KNMusicTreeViewBase::onActionActivate(const QModelIndex &index)
{
    //Hide the detail tooltip.
    if(knMusicGlobal->detailTooltip())
    {
        knMusicGlobal->detailTooltip()->hide();
    }
    //Play the activate index.
    playIndex(index);
}

void KNMusicTreeViewBase::playCurrent()
{
    //Play the current index.
    playIndex(currentIndex());
}

void KNMusicTreeViewBase::removeCurrent()
{
    ;
}

void KNMusicTreeViewBase::renameCurrent()
{
    ;
}

void KNMusicTreeViewBase::resetHeaderState()
{
    //Hide all the data column first.
    for(int i=Name+1; i<MusicDataCount; i++)
    {
        setColumnHidden(i, true);
    }
    //Show the default columns.
    setColumnHidden(Time, false);
    setColumnHidden(Artist, false);
    setColumnHidden(Album, false);
    setColumnHidden(Genre, false);
    setColumnHidden(Rating, false);
    //Reorder the columns.
    moveToFirst(Rating);
    moveToFirst(Genre);
    moveToFirst(Album);
    moveToFirst(Artist);
    moveToFirst(Time);
    moveToFirst(Name);
    //Set the default width.
    setColumnWidth(Genre, 81);
    setColumnWidth(Album, 126);
    setColumnWidth(Artist, 126);
    setColumnWidth(Time, 60);
    setColumnWidth(Name, 200);
}

inline void KNMusicTreeViewBase::startAnime(const int &endFrame)
{
    //Stop the mouse animations.
    m_mouseAnime->stop();
    //Set the parameter of the time line.
    m_mouseAnime->setFrameRange(
                palette().color(QPalette::AlternateBase).alpha(),
                endFrame);
    //Start the time line.
    m_mouseAnime->start();
}

void KNMusicTreeViewBase::playIndex(const QModelIndex &index)
{
    //Check the music row and the index is valid.
    if(musicModel()==nullptr || !index.isValid())
    {
        //Ignore those invalid request.
        return;
    }
    //Get the now playing from the music global.
    KNMusicNowPlayingBase *nowPlaying=knMusicGlobal->nowPlaying();
    //Check null.
    if(nowPlaying==nullptr)
    {
        return;
    }
    //Ask the now playing to play the index row.
    nowPlaying->playMusicRow(proxyModel(), index.row(), m_musicTab);
}

void KNMusicTreeViewBase::showSoloMenu(const QPoint &position)
{
    //Get the index of the position where mouse pressed.
    QModelIndex pressedIndex=indexAt(position);
    //Check the valid of the index.
    if(pressedIndex.isValid() && knMusicGlobal->soloMenu()!=nullptr)
    {
        //Get the solo menu.
        KNMusicSoloMenuBase *soloMenu=knMusicGlobal->soloMenu();
        //Generate the connection handler.
        KNConnectionHandler connections;
        //Link the menu require signal to this slot.
        connections.append(
                   connect(soloMenu, &KNMusicSoloMenuBase::requirePlayCurrent,
                           this, &KNMusicTreeViewBase::playCurrent));
        connections.append(
                   connect(soloMenu, &KNMusicSoloMenuBase::requireRemoveCurrent,
                           this, &KNMusicTreeViewBase::removeCurrent));
        connections.append(
                   connect(soloMenu, &KNMusicSoloMenuBase::requireRenameCurrent,
                           this, &KNMusicTreeViewBase::renameCurrent));
        //Set the information to the solo menu.
        soloMenu->setMusicRow(m_proxyModel, pressedIndex);
        //Get the menu position, fixed the bug which ignore the header's height.
        QPoint menuPosition=mapToGlobal(position);
        menuPosition.setY(menuPosition.y()+header()->height());
        //Set the position to menu.
        soloMenu->setMouseDownPos(menuPosition);
        //Launch the menu.
        soloMenu->exec(menuPosition);
        //Disconnect the links.
        connections.disconnectAll();
    }
}

void KNMusicTreeViewBase::showMultiMenu(const QPoint &position)
{
    //Get the selected indexes.
    QModelIndexList dumpIndexes=selectedIndexes();
    //Generate a temporary list.
    QList<int> rowList;
    //Check the indexes size.
    if(dumpIndexes.isEmpty())
    {
        return;
    }
    //Check all the indexes list, we will only save one row.
    for(auto i=dumpIndexes.constBegin(); i!=dumpIndexes.constEnd(); ++i)
    {
        //Check whether current row is in the row list.
        if(rowList.contains((*i).row()))
        {
            continue;
        }
        //Add the new row to row list.
        rowList.append((*i).row());
    }
    //Get the index of the position where mouse pressed.
    QModelIndex pressedIndex=indexAt(position);
    //Check the validation of the multi menu.
    if(knMusicGlobal->multiMenu())
    {
        //Get the multi menu.
        KNMusicMultiMenuBase *multiMenu=knMusicGlobal->multiMenu();
        //Generate the connection handler.
        KNConnectionHandler connections;
        //Link the menu require signal to this slot.
        ;
        //Set information to menu.
        multiMenu->setMusicRows(m_proxyModel, rowList, pressedIndex);
        //Get the menu position, fixed the bug which ignore the header's height.
        QPoint menuPosition=mapToGlobal(position);
        menuPosition.setY(menuPosition.y()+header()->height());
        //Set the position to menu.
        multiMenu->setMouseDownPos(menuPosition);
        //Launch the menu.
        multiMenu->exec(menuPosition);
        //Disconnect the links.
        connections.disconnectAll();
    }
}

inline bool KNMusicTreeViewBase::showDetailTooltip(const QPoint &indexPosition)
{
    //If the detail tooltip is not set, ignore the tooltip request.
    if(!knMusicGlobal->detailTooltip())
    {
        return false;
    }
    //If the position is in the header, hide the detail tooltip.
    if(indexPosition.y() < 0)
    {
        //Hide the detail tooltip widget.
        knMusicGlobal->detailTooltip()->hide();
        //Finished.
        return false;
    }
    //Locate index at the position.
    QModelIndex mouseIndex=indexAt(indexPosition);
    //Check the validation of the index.
    if(mouseIndex.isValid())
    {
        //If the position is on the vertical scrollbar, then hide the
        //tooltip widget.
        if(verticalScrollBar()->isVisible() &&
                indexPosition.x() > (viewport()->rect().right() -
                                     verticalScrollBar()->width()))
        {
            //Hide the tooltip.
            knMusicGlobal->detailTooltip()->hide();
            //Finished.
            return true;
        }
        //We don't need to check the proxy model. Because the indexAt() will
        //get a invalid index if the proxy model is null.
        //Set the index of the current music model to the detail tooltip.
        knMusicGlobal->detailTooltip()->setPreviewIndex(
                    musicModel(),
                    m_proxyModel->mapToSource(mouseIndex));
        //Show the tooltip at the specific position.
        QRect indexRect=visualRect(mouseIndex);
        knMusicGlobal->detailTooltip()->showTooltip(
                    mapToGlobal(
                        QPoint(indexPosition.x(),
                               indexRect.y()+(indexRect.height()>>1))));
        //Give the focus back to this tree view.
        setFocus();
    }
    return true;
}

bool KNMusicTreeViewBase::animate() const
{
    return m_animate;
}

KNMusicProxyModel *KNMusicTreeViewBase::proxyModel()
{
    //Check is the proxy model need to initial.
    if(m_proxyModel==nullptr)
    {
        //Initial the proxy model.
        m_proxyModel=new KNMusicProxyModel(this);
        //Set the search text.
        //!FIXME: Set search text here.
        //Set the proxy model.
        setModel(m_proxyModel);
    }
    return m_proxyModel;
}

KNMusicModel *KNMusicTreeViewBase::musicModel()
{
    //If the proxy model is nullptr, then it wil be nullptr.
    //Or else, get the music model of the proxy model.
    return m_proxyModel==nullptr?nullptr:m_proxyModel->musicModel();
}

void KNMusicTreeViewBase::setMusicModel(KNMusicModel *musicModel)
{
    //Hide the detail tooltip first.
    if(knMusicGlobal->detailTooltip())
    {
        knMusicGlobal->detailTooltip()->hide();
    }
    //Check before set the music model, if the current playing model is the
    //previous model, shadow the proxy model.
    if(knMusicGlobal->nowPlaying()!=nullptr)
    {
        //Get the now playing object.
        KNMusicNowPlayingBase *nowPlaying=knMusicGlobal->nowPlaying();
        //Check the current playing model is the current proxy and the proxy
        //model is copied from the treeview.
        if(nowPlaying->playingMusicModel()==proxyModel()->musicModel() &&
                nowPlaying->playingModel()==proxyModel())
        {
            //If so, shadow the proxy model.
            nowPlaying->shadowPlayingModel();
        }
    }
    //Set the source model.
    proxyModel()->setSourceModel(musicModel);
    //Check if the music model nullptr.
    if(musicModel==nullptr)
    {
        //Set initial load to become true.
        m_initialLoad=true;
        //All work has been done.
        return;
    }
    //Check if previous model is nullptr, if the initial load has been set to be
    //true, we have to reset the header state.
    if(m_initialLoad)
    {
        //Reset the header state for first load.
        resetHeaderState();
        m_initialLoad=false;
    }
}

void KNMusicTreeViewBase::setAnimate(bool animate)
{
    m_animate = animate;
}
