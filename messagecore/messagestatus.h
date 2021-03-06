/*  -*- mode: C++ -*-
    This file is part of KDEPIM.
    Copyright (c) 2005 Andreas Gungl <a.gungl@gmx.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KDEPIM_MESSAGESTATUS_H
#define KDEPIM_MESSAGESTATUS_H

#include <QtCore/QSet>

#include "messagecore_export.h"

class QString;

namespace KPIM {

//---------------------------------------------------------------------------
/**
  @short KDEPIM Message Status.
  @author Andreas Gungl <a.gungl@gmx.de>

  The class encapsulates the handling of the different flags
  which describe the status of a message.
  The flags itself are not intended to be used outside this class.

  In the status pairs Watched/Ignored and Spam/Ham, there both
  values can't be set at the same time, however they can
  be unset at the same time.

  The stati New/Unread/Read/Old are mutually exclusive.
*/
class MESSAGECORE_EXPORT MessageStatus
{
  public:
    /** Constructor - sets status initially to unknown. */
    MessageStatus();

    /** Assign the status from another instance. The internal
        representation is identical afterwards, i.e. a comparison
        by operator == will return true.
    */
    MessageStatus &operator = ( const MessageStatus &other );

    /** Compare the status with that from another instance.
        @return true if the stati are equal, false if different.
    */
    bool operator == ( const MessageStatus &other ) const;

    /** Compare the status with that from another instance.
        @return true if the stati are equal, false if different.
    */
    bool operator != ( const MessageStatus &other ) const;

    /** Check, if some of the flags in the status match
        with those flags from another instance.
        @return true if at least one flag is set in both stati.
    */
    bool operator & ( const MessageStatus &other ) const;

    /** Clear all status flags, this resets to unknown. */
    void clear();

    /** Set / add stati described by another MessageStatus object.
        This can be used to merge in multiple stati at once without
        using the single setter methods.
        However, internally the setters are used anyway to ensure the
        integrity of the resulting status.
    */
    void set( const MessageStatus &other );

    /** Toggle one or more stati described by another MessageStatus object.
        Internally the setters are used to ensure the integrity of the
        resulting status.
        Toggling of the stati New, Unread, Read and Old is not supported.
        These stati are completely ignored.
    */
    void toggle( const MessageStatus &other );

    /* ----- getters ----------------------------------------------------- */

    /** Check for Unknown status.
        @return true if status is unknown.
    */
    bool isOfUnknownStatus() const;

    /** Check for New status. Ignored messages are not new.
        @return true if status is new.
    */
    bool isNew() const;

    /** Check for Unread status. Note that new messages are not unread.
        Ignored messages are not unread as well.
        @return true if status is unread.
    */
    bool isUnread() const;

    /** Check for Read status. Note that ignored messages are read.
        @return true if status is read.
    */
    bool isRead() const;

    /** Check for Old status.
        @return true if status is old.
    */
    bool isOld() const;

    /** Check for Deleted status.
        @return true if status is deleted.
    */
    bool isDeleted() const;

    /** Check for Replied status.
        @return true if status is replied.
    */
    bool isReplied() const;

    /** Check for Forwarded status.
        @return true if status is forwarded.
    */
    bool isForwarded() const;

    /** Check for Queued status.
        @return true if status is queued.
    */
    bool isQueued() const;

    /** Check for Sent status.
        @return true if status is sent.
    */
    bool isSent() const;

    /** Check for Important status.
        @return true if status is important.
    */
    bool isImportant() const;

    /** Check for Watched status.
        @return true if status is watched.
    */
    bool isWatched() const;

    /** Check for Ignored status.
        @return true if status is ignored.
    */
    bool isIgnored() const;

    /** Check for ToAct status.
        @return true if status is action item.
    */
    bool isToAct() const;

    /** Check for Spam status.
        @return true if status is spam.
    */
    bool isSpam() const;

    /** Check for Ham status.
        @return true if status is not spam.
    */
    bool isHam() const;

    /** Check for Attachment status.
        @return true if status indicates an attachment.
    */
    bool hasAttachment() const;

    /* ----- setters ----------------------------------------------------- */

    /** Set the status to new. */
    void setNew();

    /** Set the status to unread. */
    void setUnread();

    /** Set the status to read. */
    void setRead();

    /** Set the status to old. */
    void setOld();

    /** Set the status for deleted.
        @param deleted Set (true) or unset (false) this status flag.
    */
    void setDeleted( bool deleted = true );

    /** Set the status for replied.
        @param replied Set (true) or unset (false) this status flag.
    */
    void setReplied( bool replied = true );

    /** Set the status for forwarded.
        @param forwarded Set (true) or unset (false) this status flag.
    */
    void setForwarded( bool forwarded = true );

    /** Set the status for queued.
        @param queued Set (true) or unset (false) this status flag.
    */
    void setQueued( bool queued = true );

    /** Set the status for sent.
        @param sent Set (true) or unset (false) this status flag.
    */
    void setSent( bool sent = true );

    /** Set the status for important.
        @param important Set (true) or unset (false) this status flag.
    */
    void setImportant( bool important = true );

    /** Set the status to watched.
        @param watched Set (true) or unset (false) this status flag.
    */
    void setWatched( bool watched = true );

    /** Set the status to ignored.
        @param ignored Set (true) or unset (false) this status flag.
    */
    void setIgnored( bool ignored = true );

    /** Set the status to action item.
        @param toAct Set (true) or unset (false) this status flag.
    */
    void setToAct( bool toAct = true );

    /** Set the status to spam.
        @param spam Set (true) or unset (false) this status flag.
    */
    void setSpam( bool spam = true );

    /** Set the status to not spam.
        @param ham Set (true) or unset (false) this status flag.
    */
    void setHam( bool ham = true );

    /** Set the status for an attachment.
        @param withAttechment Set (true) or unset (false) this status flag.
    */
    void setHasAttachment( bool withAttachment = true );

    /* ----- state representation  --------------------------------------- */

    /** Get the status as a whole e.g. for storage in an index.
        Don't manipulte the index via this value, this bypasses
        all integrity checks in the setter methods.
        @return The status encoded in bits.
    */
    qint32 toQInt32() const;

    /** Set the status as a whole e.g. for reading from an index.
        Don't manipulte the index via this value, this bypasses
        all integrity checks in the setter methods.
        @param status The status encoded in bits to be set in this instance.
    */
    void fromQInt32( qint32 status );

    /** Convert the status to a string representation.
        @return A string containing coded uppercase letters
                which describe the status.
    */
    QString getStatusStr() const;

    /** Set the status based on a string representation.
        @param aStr The status string to be analyzed.
                    Normally it is a string obtained using
                    getStatusStr().
    */
    void setStatusFromStr( const QString &aStr );

    /** Convert the status to a string for sorting.
        @return A string containing coded lowercase letters
                which allows a predefined sorting by status.
    */
    QString getSortRank() const;

    /** Get the status as a whole e.g. for storage as IMAP flags.
        @return The status encoded in flags.
    */
    QSet<QByteArray> getStatusFlags() const;

    /** Set the status as a whole e.g. for reading from IMAP flags.
        @param status The status encoded in bits to be set in this instance.
    */
    void setStatusFromFlags( const QSet<QByteArray> &flags );

    /* ----- static accessors to simple states --------------------------- */

    /** Return a predefined status initialized as New as is useful
        e.g. when providing a state for comparison.
        @return A reference to a status instance initialized as New.
    */
    static MessageStatus statusNew();

    /** Return a predefined status initialized as Read as is useful
        e.g. when providing a state for comparison.
        @return A reference to a status instance initialized as Read.
    */
    static MessageStatus statusRead();

    /** Return a predefined status initialized as Unread as is useful
        e.g. when providing a state for comparison.
        @return A reference to a status instance initialized as Unread.
    */
    static MessageStatus statusUnread();

    /** Return a predefined status initialized as New and Unread as is
        useful e.g. when searching for unread messages.
        @return A reference to a status instance initialized as New | Unread.
    */
    static MessageStatus statusNewAndUnread();

    /** Return a predefined status initialized as Old as is useful
        e.g. when providing a state for comparison.
        @return A reference to a status instance initialized as Old.
    */
    static MessageStatus statusOld();

    /** Return a predefined status initialized as Deleted as is useful
        e.g. when providing a state for comparison.
        @return A reference to a status instance initialized as Deleted.
    */
    static MessageStatus statusDeleted();

    /** Return a predefined status initialized as Replied as is useful
        e.g. when providing a state for comparison.
        @return A reference to a status instance initialized as Replied.
    */
    static MessageStatus statusReplied();

    /** Return a predefined status initialized as Forwarded as is useful
        e.g. when providing a state for comparison.
        @return A reference to a status instance initialized as Forwarded.
    */
    static MessageStatus statusForwarded();

    /** Return a predefined status initialized as Queued as is useful
        e.g. when providing a state for comparison.
        @return A reference to a status instance initialized as Queued.
    */
    static MessageStatus statusQueued();

    /** Return a predefined status initialized as Sent as is useful
        e.g. when providing a state for comparison.
        @return A reference to a status instance initialized as Sent.
    */
    static MessageStatus statusSent();

    /** Return a predefined status initialized as Important as is useful
        e.g. when providing a state for comparison.
        @return A reference to a status instance initialized as Important.
    */
    static MessageStatus statusImportant();

    /** Return a predefined status initialized as Watched as is useful
        e.g. when providing a state for comparison.
        @return A reference to a status instance initialized as Watched.
    */
    static MessageStatus statusWatched();

    /** Return a predefined status initialized as Ignored as is useful
        e.g. when providing a state for comparison.
        @return A reference to a status instance initialized as Ignored.
    */
    static MessageStatus statusIgnored();

    /** Return a predefined status initialized as Action Item as is useful
        e.g. when providing a state for comparison.
        @return A reference to a status instance initialized as ToAct.
    */
    static MessageStatus statusToAct();

    /** Return a predefined status initialized as Spam as is useful
        e.g. when providing a state for comparison.
        @return A reference to a status instance initialized as Spam.
    */
    static MessageStatus statusSpam();

    /** Return a predefined status initialized as Ham as is useful
        e.g. when providing a state for comparison.
        @return A reference to a status instance initialized as Ham.
    */
    static MessageStatus statusHam();

    /** Return a predefined status initialized as Attachment as is useful
        e.g. when providing a state for comparison.
        @return A reference to a status instance initialized as Attachment.
    */
    static MessageStatus statusHasAttachment();

  private:
    qint32 mStatus;
};

} // namespace KPIM

#endif /*KMAIL_MESSAGESTATUS_H*/
