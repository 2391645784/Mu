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

#ifndef KNMUSICSTOREHEADER_H
#define KNMUSICSTOREHEADER_H

#include "knmusicstoreutil.h"

#include "knmousesensewidget.h"

using namespace MusicStoreUtil;

class QBoxLayout;
class QLabel;
class QTimeLine;
class KNAnimeLabelButton;
/*!
 * \brief The KNMusicStoreHeader class provides the header of the music store.
 * This widget shows the current position of the widget, and provides icons for
 * users to access some functions.
 */
class KNMusicStoreHeader : public KNMouseSenseWidget
{
    Q_OBJECT
public:
    /*!
     * \brief Construct a KNMusicStoreHeader widget.
     * \param parent The parent widget.
     */
    explicit KNMusicStoreHeader(QWidget *parent = 0);

    /*!
     * \brief Add a widget to header.
     * \param widget The widget pointer.
     * \param stretch The widget stretch parameter.
     * \param alignment The widget prefer alignment.
     */
    void addHeaderWidget(QWidget *widget,
                         int stretch=0,
                         Qt::Alignment alignment=0);

    /*!
     * \brief Add widget to the state widget layout area. This allows to add
     * icon to the header bar.
     * \param widget The icon widget pointer.
     */
    void addStateWidget(QWidget *widget);

signals:
    /*!
     * \brief Require to show one page.
     * \param index The page index.
     */
    void requireShowPage(int index);

public slots:
    /*!
     * \brief Set the navigator label text.
     * \param itemIndex The item index.
     * \param text The navigator text.
     */
    void setNavigatorText(int itemIndex, const QString &text);

protected:
    /*!
     * \brief Reimplemented from KNMouseSenseWidget::paintEvent().
     */
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

protected slots:
    /*!
     * \brief Reimplemented from KNMouseSenseWidget::onPaletteChanged().
     */
    void onPaletteChanged() Q_DECL_OVERRIDE;

private slots:
    void retranslate();
    void onNavigatorClick(int index);

private:
    inline void updateSearchLabel();
    //Linear gradient.
    QLinearGradient m_borderGradient;
    //Header text.
    QString m_searchText, m_searchKeyword;
    //Animation.
    QTimeLine *m_mouseAnime;
    //Plugin tray.
    QBoxLayout *m_mainLayout, *m_pluginTray;
    //Widgets
    KNAnimeLabelButton *m_navigatorItem[PagesCount];
    QLabel *m_indicator[PagesCount-1];
};

#endif // KNMUSICSTOREHEADER_H
