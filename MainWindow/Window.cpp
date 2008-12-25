/* Copyright (C) 2007 - 2008 Jan Kundrát <jkt@gentoo.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QDebug>
#include <QDockWidget>
#include <QMenuBar>
#include <QTreeView>

#include "Window.h"
#include "Imap/Model/Model.h"
#include "Imap/Model/MailboxModel.h"
#include "Imap/Model/MsgListModel.h"
#include "Imap/Streams/SocketFactory.h"

namespace Gui {

MainWindow::MainWindow(): QMainWindow()
{
    createMenus();
    createDockWindows();
    setupModels();
}

void MainWindow::createMenus()
{
    reloadMboxList = new QAction( "Rescan Child Mailboxes", this );
    connect( reloadMboxList, SIGNAL( triggered() ), this, SLOT( slotReloadMboxList() ) );
    /*QMenu* mailboxMenu = menuBar()->addMenu( "Mailbox" );
    mailboxMenu->addAction( reloadMboxList );*/
}

void MainWindow::createDockWindows()
{
    QDockWidget* dock = new QDockWidget( "Mailbox Folders", this );
    mboxTree = new QTreeView( dock );
    mboxTree->setUniformRowHeights( true );
    mboxTree->setHeaderHidden( true );
    mboxTree->setContextMenuPolicy(Qt::CustomContextMenu);
    mboxTree->setSelectionMode( QAbstractItemView::ExtendedSelection );
    connect( mboxTree, SIGNAL( customContextMenuRequested( const QPoint & ) ),
            this, SLOT( showContextMenuMboxTree( const QPoint& ) ) );
    dock->setWidget( mboxTree );
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    dock = new QDockWidget( "Messages", this );
    msgListTree = new QTreeView( dock );
    msgListTree->setUniformRowHeights( true );
    msgListTree->setHeaderHidden( true );
    dock->setWidget( msgListTree );
    addDockWidget(Qt::RightDockWidgetArea, dock);

}

void MainWindow::setupModels()
{
    Imap::Mailbox::SocketFactoryPtr factory(
            new Imap::Mailbox::UnixProcessSocketFactory( "/usr/sbin/dovecot",
                QStringList() << "--exec-mail" << "imap" )
            /*new Imap::Mailbox::ProcessSocketFactory( "ssh",
                QStringList() << "sosna.fzu.cz" << "/usr/sbin/imapd" )*/
            );

    cache.reset( new Imap::Mailbox::NoCache() );
    model = new Imap::Mailbox::Model( this, cache, authenticator, factory );
    mboxModel = new Imap::Mailbox::MailboxModel( this, model );
    msgListModel = new Imap::Mailbox::MsgListModel( this, model );

    QObject::connect( mboxTree, SIGNAL( clicked(const QModelIndex&) ), msgListModel, SLOT( setMailbox(const QModelIndex&) ) );
    QObject::connect( mboxTree, SIGNAL( activated(const QModelIndex&) ), msgListModel, SLOT( setMailbox(const QModelIndex&) ) );

    mboxTree->setModel( mboxModel );
    msgListTree->setModel( msgListModel );
}

void MainWindow::showContextMenuMboxTree( const QPoint& position )
{
    QList<QAction*> actionList;
    if ( mboxTree->indexAt( position ).isValid() ) {
        actionList.append( reloadMboxList );
    }
    if ( ! actionList.isEmpty() )
        QMenu::exec( actionList, mboxTree->mapToGlobal( position ) );
}

void MainWindow::slotReloadMboxList()
{
    qDebug() << "slotReloadMboxList:" << mboxTree->selectionModel()->selectedIndexes().count();
}

}

#include "Window.moc"
