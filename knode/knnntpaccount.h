/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNNNTPACCOUNT_H
#define KNNNTPACCOUNT_H


#include "kncollection.h"
#include "knserverinfo.h"
#include <QObject>
#include <QTimer>
#include <QDate>

class KNNntpAccount;

namespace KNode {
  class Identity;
  class Cleanup;
}


/** Handles the interval checking of an news server account. */
class KNNntpAccountIntervalChecking : public QObject  {

  Q_OBJECT

  public:
    KNNntpAccountIntervalChecking(KNNntpAccount *account);
    ~KNNntpAccountIntervalChecking();
    void installTimer();
    void deinstallTimer();

  protected:
    QTimer *t_imer;
    KNNntpAccount *a_ccount;

  protected slots:
    void slotCheckNews();

};


/** Represents an account on a news server. */
class KNNntpAccount : public KNCollection , public KNServerInfo {

  public:
    KNNntpAccount();
    ~KNNntpAccount();

    collectionType type()             { return CTnntpAccount; }

    /** tries to read information, returns false if it fails to do so */
    bool readInfo(const QString &confPath);
    void saveInfo();
    //void syncInfo();
    QString path();
    /** returns true when the user accepted */
    bool editProperties(QWidget *parent);

    // news interval checking
    void startTimer();

    //get
    bool fetchDescriptions() const         { return f_etchDescriptions; }
    QDate lastNewFetch() const             { return l_astNewFetch; }
    bool wasOpen() const                   { return w_asOpen; }
    bool useDiskCache() const              { return u_seDiskCache; }
    KNode::Identity* identity() const   { return i_dentity; }
    bool intervalChecking() const          { return i_ntervalChecking; }
    int checkInterval() const              { return c_heckInterval; }
    KNode::Cleanup *cleanupConfig() const { return mCleanupConf; }

    /** Returns the cleanup configuration that should be used for this account */
    KNode::Cleanup *activeCleanupConfig() const;

    //set
    void setFetchDescriptions(bool b) { f_etchDescriptions = b; }
    void setLastNewFetch(QDate date)  { l_astNewFetch = date; }
    void setUseDiskCache(bool b)      { u_seDiskCache=b; }
    void setCheckInterval(int c);
    void setIntervalChecking(bool b)  { i_ntervalChecking=b; }

  protected:
    /** server specific identity */
    KNode::Identity *i_dentity;
    /** account specific cleanup configuration */
    KNode::Cleanup *mCleanupConf;
    /** use an additional "list newsgroups" command to fetch the newsgroup descriptions */
    bool f_etchDescriptions;
    /** last use of "newgroups" */
    QDate l_astNewFetch;
    /** was the server open in the listview on the last shutdown? */
    bool w_asOpen;
    /** cache fetched articles on disk */
    bool u_seDiskCache;
    /** is interval checking enabled */
    bool i_ntervalChecking;
    int c_heckInterval;

    /** helper class for news interval checking, manages the QTimer */
    KNNntpAccountIntervalChecking *a_ccountIntervalChecking;

};

#endif
