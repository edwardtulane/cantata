/*
 * Cantata
 *
 * Copyright (c) 2011-2014 Craig Drummond <craig.p.drummond@gmail.com>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "titlewidget.h"
#include "support/squeezedtextlabel.h"
#include "support/localize.h"
#include "support/utils.h"
#include "support/icon.h"
#include "gui/covers.h"
#include <QAction>
#include <QImage>
#include <QPixmap>
#include <QGridLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>

TitleWidget::TitleWidget(QWidget *p)
    : QWidget(p)
{
    QGridLayout *layout=new QGridLayout(this);
    image=new QLabel(this);
    mainText=new SqueezedTextLabel(this);
    subText=new SqueezedTextLabel(this);
    QLabel *chevron=new QLabel(QChar(Qt::RightToLeft==layoutDirection() ? 0x203A : 0x2039), this);
    QFont f=mainText->font();
    subText->setFont(Utils::smallFont(f));
    f.setBold(true);
    mainText->setFont(f);
    if (f.pixelSize()>0) {
        f.setPixelSize(f.pixelSize()*2.25);
    } else {
        f.setPointSizeF(f.pointSizeF()*2.25);
    }
    f.setBold(false);
    chevron->setFont(f);
    int size=mainText->sizeHint().height()+subText->sizeHint().height()+Utils::layoutSpacing(this);
    size+=12;
    image->setFixedSize(size, size);
    setToolTip(i18n("Click to go back"));
    int spacing=layout->spacing()-1;
    layout->addItem(new QSpacerItem(spacing, spacing), 0, 0, 2, 1);
    layout->addWidget(chevron, 0, 1, 2, 1);
    layout->addWidget(image, 0, 2, 2, 1);
    layout->addWidget(mainText, 0, 3, 1, 1);
    layout->addWidget(subText, 1, 3, 1, 1);
    mainText->installEventFilter(this);
    subText->installEventFilter(this);
    image->installEventFilter(this);
    installEventFilter(this);
    setAttribute(Qt::WA_Hover);
    connect(Covers::self(), SIGNAL(cover(Song,QImage,QString)), this, SLOT(coverRetrieved(Song,QImage,QString)));
    connect(Covers::self(), SIGNAL(coverUpdated(Song,QImage,QString)), this, SLOT(coverRetrieved(Song,QImage,QString)));
    connect(Covers::self(), SIGNAL(artistImage(Song,QImage,QString)), this, SLOT(coverRetrieved(Song,QImage,QString)));
    layout->setContentsMargins(1, spacing, 1, spacing);
    mainText->setAlignment(Qt::AlignBottom);
    subText->setAlignment(Qt::AlignTop);
    image->setAlignment(Qt::AlignCenter);
    chevron->setAlignment(Qt::AlignCenter);
}

void TitleWidget::update(const Song &sng, const QIcon &icon, const QString &text, const QString &sub)
{
    song=sng;
    image->setVisible(true);
    mainText->setText(text);
    subText->setText(sub);
    if (!sng.isEmpty()) {
        Covers::Image cImg=Covers::self()->requestImage(sng, true);
        if (!cImg.img.isNull()) {
            image->setPixmap(QPixmap::fromImage(cImg.img.scaled(image->width()-2, image->height()-2, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
            return;
        }
    }
    if (icon.isNull()) {
        image->setVisible(false);
    } else {
        image->setPixmap(icon.pixmap(64, 64).scaled(image->width()-2, image->height()-2, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

bool TitleWidget::eventFilter(QObject *o, QEvent *event)
{
    switch(event->type()) {
    case QEvent::HoverEnter:
        if (isEnabled() && o==this) {
            #ifdef Q_OS_MAC
            setStyleSheet(QString("QLabel{color:%1;}").arg(OSXStyle::self()->viewPalette().highlight().color().name()));
            #else
            setStyleSheet(QLatin1String("QLabel{color:palette(highlight);}"));
            #endif
        }
        break;
    case QEvent::HoverLeave:
        if (isEnabled() && o==this) {
            setStyleSheet(QString());
        }
        break;
    case QEvent::MouseButtonPress:
        if (Qt::LeftButton==static_cast<QMouseEvent *>(event)->button() && Qt::NoModifier==static_cast<QMouseEvent *>(event)->modifiers()) {
            pressed=true;
        }
        break;
    case QEvent::MouseButtonRelease:
        if (pressed && Qt::LeftButton==static_cast<QMouseEvent *>(event)->button() && !QApplication::overrideCursor()) {
            actions().first()->trigger();
        }
        pressed=false;
        break;
    default:
        break;
    }
    return QWidget::eventFilter(o, event);
}

void TitleWidget::coverRetrieved(const Song &s, const QImage &img, const QString &file)
{
    Q_UNUSED(file);
    if (song.isEmpty() || img.isNull()) {
        return;
    }
    if (song.artistOrComposer()!=s.artistOrComposer()) {
        return;
    }
    if (s.isArtistImageRequest()!=song.isArtistImageRequest()) {
        return;
    }
    if (s.isComposerImageRequest()!=song.isComposerImageRequest()) {
        return;
    }
    if (!s.isComposerImageRequest() && !s.isArtistImageRequest() && s.album!=song.album) {
        return;
    }
    image->setPixmap(QPixmap::fromImage(img.scaled(image->width()-6, image->height()-6, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
}