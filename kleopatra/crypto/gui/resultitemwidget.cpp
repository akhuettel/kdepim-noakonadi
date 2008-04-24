/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/resultitemwidget.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "resultitemwidget.h"

#include <KLocalizedString>
#include <KPushButton>
#include <KStandardGuiItem>

#include <QHBoxLayout>
#include <QLabel>
#include <QUrl>
#include <QVBoxLayout>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;
using namespace boost;
 
namespace {
    //### TODO move outta here, make colors configurable
    static QColor colorForVisualCode( Task::Result::VisualCode code ) {
        switch ( code ) {
            case Task::Result::AllGood:
                return Qt::green;
            case Task::Result::NeutralError:
            case Task::Result::Warning:
                return Qt::yellow;
            case Task::Result::Danger:
                return Qt::red;
            case Task::Result::NeutralSuccess:
            default:
                return Qt::blue;
        }
    }
}

class ResultItemWidget::Private {
    ResultItemWidget* const q;
public:
    explicit Private( const shared_ptr<const Task::Result> result, ResultItemWidget* qq ) : q( qq ), m_result( result ), m_detailsLabel( 0 ), m_showDetailsLabel( 0 ) {}
    
    void slotLinkActivated( const QString & );
    void updateShowDetailsLabel();

    const shared_ptr<const Task::Result> m_result;
    QLabel * m_detailsLabel;
    QLabel * m_showDetailsLabel;
    KPushButton * m_closeButton;
};

void ResultItemWidget::Private::updateShowDetailsLabel()
{
    if ( !m_showDetailsLabel || !m_detailsLabel )
        return;
    
    const bool show = !m_detailsLabel->isVisible();
    m_showDetailsLabel->setText( QString("<a href=\"kleoresultitem://toggledetails/\">%1</a>").arg( show ? i18n( "Show Details" ) : i18n( "Hide Details" ) ) );
}

ResultItemWidget::ResultItemWidget( const shared_ptr<const Task::Result> & result, const QString & label, QWidget * parent, Qt::WindowFlags flags) : QWidget( parent, flags ), d( new Private( result, this ) )
{
    assert( d->m_result );
    const QColor color = colorForVisualCode( d->m_result->code() );
    setStyleSheet( QString( "* { background-color: %1; margin: 0px; } QFrame#resultFrame{ border-color: %2; border-style: solid; border-radius: 3px; border-width: 2px } QLabel { padding: 5px; border-radius: 3px }" ).arg( color.lighter( 150 ).name(), color.name() ) );
    QVBoxLayout* topLayout = new QVBoxLayout( this );
    QFrame* frame = new QFrame;
    frame->setObjectName( "resultFrame" );
    topLayout->addWidget( frame );
    QVBoxLayout* layout = new QVBoxLayout( frame );
    layout->setMargin( 0 );
    layout->setSpacing( 0 );
    QWidget* hbox = new QWidget;
    QHBoxLayout* hlay = new QHBoxLayout( hbox );
    hlay->setMargin( 0 );
    hlay->setSpacing( 0 );
    QLabel* overview = new QLabel;
    overview->setWordWrap( true );
    overview->setTextFormat( Qt::RichText );
    overview->setText( i18nc( "%1: action %2: result; example: Decrypting foo.txt: Succeeded", "%1: %2", label, d->m_result->overview() ) );
    connect( overview, SIGNAL(linkActivated(QString)), this, SLOT(slotLinkActivated(QString)) );

    hlay->addWidget( overview, 1 );
    layout->addWidget( hbox );

    const QString details = d->m_result->details();
    
    d->m_showDetailsLabel = new QLabel;
    connect( d->m_showDetailsLabel, SIGNAL(linkActivated(QString)), this, SLOT(slotLinkActivated(QString)) );
    hlay->addWidget( d->m_showDetailsLabel );
    d->m_showDetailsLabel->setVisible( !details.isEmpty() );
    
    d->m_detailsLabel = new QLabel;
    d->m_detailsLabel->setWordWrap( true );
    d->m_detailsLabel->setTextFormat( Qt::RichText );
    d->m_detailsLabel->setText( details );
    connect( d->m_detailsLabel, SIGNAL(linkActivated(QString)), this, SLOT(slotLinkActivated(QString)) );
    layout->addWidget( d->m_detailsLabel );

    d->m_detailsLabel->setVisible( false );
    
    d->m_closeButton = new KPushButton;
    d->m_closeButton->setGuiItem( KStandardGuiItem::close() );
    connect( d->m_closeButton, SIGNAL(clicked()), this, SIGNAL(closeButtonClicked()) );
    layout->addWidget( d->m_closeButton );
    d->m_closeButton->setVisible( false );

    d->updateShowDetailsLabel();
}

ResultItemWidget::~ResultItemWidget()
{
}

void ResultItemWidget::showCloseButton( bool show )
{
    d->m_closeButton->setVisible( show );
}

bool ResultItemWidget::detailsVisible() const
{
    return d->m_detailsLabel && d->m_detailsLabel->isVisible();
}

bool ResultItemWidget::hasErrorResult() const
{
    return d->m_result->hasError();
}

void ResultItemWidget::Private::slotLinkActivated( const QString & link )
{
    const QUrl url( link );
    if ( url.scheme() != "kleoresultitem" ) {
        emit q->linkActivated( link );
        return;
    }
    if ( url.host() == "toggledetails" ) {
        q->showDetails( !q->detailsVisible() );
        return;
    }
}

void ResultItemWidget::showDetails( bool show )
{
    if ( d->m_detailsLabel )
        d->m_detailsLabel->setVisible( show );
    d->updateShowDetailsLabel();
}

#include "resultitemwidget.moc"
