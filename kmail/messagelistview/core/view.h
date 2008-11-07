/******************************************************************************
 *
 *  Copyright 2008 Szymon Tomasz Stefanek <pragma@kvirc.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#ifndef __KMAIL_MESSAGELISTVIEW_CORE_VIEW_H__
#define __KMAIL_MESSAGELISTVIEW_CORE_VIEW_H__

#include <QTreeView>
#include <QList>
#include <QPoint>

#include "messagelistview/core/enums.h"

namespace KPIM
{
  class MessageStatus;
}

namespace KMail
{

namespace MessageListView
{

namespace Core
{

typedef unsigned long int MessageItemSetReference;

class Aggregation;
class Delegate;
class Item;
class MessageItem;
class Model;
class Skin;
class StorageModel;
class Widget;

/**
 * The MessageListView::View is the real display of the message list. It is
 * based on QTreeView, has a Model that manipulates the underlying message storage
 * and a Delegate that is responsable of painting the items.
 */
class View : public QTreeView
{
  friend class Model;
  Q_OBJECT
public:
  View( Widget *parent );
  ~View();

private:
  Widget *mWidget;
  Model *mModel;
  Delegate *mDelegate;

  const Aggregation *mAggregation;          ///< The Aggregation we're using now, shallow pointer
  const Skin *mSkin;                        ///< The Skin we're using now, shallow pointer
  bool mNeedToApplySkinColumns;             ///< Flag signaling a skin change, we need it in modelHasBeenReset().
  Item *mLastCurrentItem;
  QPoint mMousePressPosition;
  bool mFirstShow;

public:

  /**
   * Returns the Model attacched to this View. You probably never need to manipulate
   * it directly.
   */
  Model * model() const
    { return mModel; };

  /**
   * Returns the Delegate attacched to this View. You probably never need to manipulate
   * it directly.
   */
  Delegate * delegate() const
    { return mDelegate; };

  /**
   * Sets the StorageModel to be displayed in this view. The StorageModel may be 0 (so no content is displayed).
   * Setting the StorageModel will obviously trigger a view reload.
   * Be sure to set the Aggregation and the Skin BEFORE calling this function.
   *
   * Pre-selection is the action of automatically selecting a message just after the folder
   * has finished loading. See Model::setStorageModel() for more informations.
   */
  void setStorageModel( const StorageModel * storageModel, PreSelectionMode preSelectionMode = PreSelectLastSelected );

  /**
   * Applies the specified pre-selection to the view.
   * This is used to apply the pre-selection to a folder that was in fact already opened.
   *
   * See Model::applyMessagePreSelection() for more informations.
   */
  void applyMessagePreSelection( PreSelectionMode preSelectionMode );

  /**
   * Returns the currently displayed StorageModel. May be 0.
   */
  const StorageModel * storageModel() const;

  /**
   * Sets the aggregation for this view.
   * Does not trigger a reload of the view: you *MUST* trigger it manually.
   */
  void setAggregation( const Aggregation * aggregation );

  /**
   * Sets the specified skin for this view.
   * Does not trigger a reload of the view: you *MUST* trigger it manually.
   */
  void setSkin( const Skin * skin );

  /**
   * Triggers a reload of the view in order to re-display the current folder.
   * Call this function after changing the Aggregation or the Skin.
   */
  void reload();

  /**
   * Returns the current MessageItem (that is bound to current StorageModel).
   * May return 0 if there is no current message or no current StorageModel.
   * If the current message item isn't currently selected (so is only focused)
   * then it's selected when this function is called, unless selectIfNeeded is false.
   */
  MessageItem * currentMessageItem( bool selectIfNeeded = true ) const;

  /**
   * Sets the current message item.
   */
  void setCurrentMessageItem( MessageItem * it );

  /**
   * Returns true if the specified item is currently viewable.
   * For 'viewable' here we mean not hidden and with parents expanded.
   */
  bool isCurrentlyViewable( Item * it ) const;

  /**
   * Makes sure that the specified is currently viewable.
   * For 'viewable' here we mean not hidden and with parents expanded.
   */
  void ensureCurrentlyViewable( Item * it );

  /**
   * Returns the currently selected MessageItems (bound to current StorageModel).
   * The list may be empty if there are no selected messages or no StorageModel.
   *
   * If includeCollapsedChildren is true then the children of the selected but
   * collapsed items are also added to the list.
   *
   * The returned list is guaranteed to be valid only until you return control
   * to the main even loop. Don't store it for any longer. If you need to reference
   * this set of messages at a later stage then take a look at createPersistentSet().
   */
  QList< MessageItem * > selectionAsMessageItemList( bool includeCollapsedChildren = true ) const;

  /**
   * Returns the MessageItems bound to the current StorageModel that
   * are part of the current thread. The current thread is the thread
   * that contains currentMessageItem().
   * The list may be empty if there is no currentMessageItem() or no StorageModel.
   *
   * The returned list is guaranteed to be valid only until you return control
   * to the main even loop. Don't store it for any longer. If you need to reference
   * this set of messages at a later stage then take a look at createPersistentSet().
   */
  QList< MessageItem * > currentThreadAsMessageItemList() const;

  /**
   * Fast function that determines if the selection is empty
   */
  bool selectionEmpty() const;

  /**
   * Selects the specified MessageItems. The current selection is NOT removed.
   * Use clearSelection() for that purpose.
   */
  void selectMessageItems( const QList< MessageItem * > &list );

  /**
   * Creates a persistent set for the specified MessageItems and
   * returns its reference. Later you can use this reference
   * to retrieve the list of MessageItems that are still valid.
   * See persistentSetCurrentMessageList() for that.
   *
   * Persistent sets consume resources (both memory and CPU time
   * while manipulating the view) so be sure to call deletePersistentSet()
   * when you no longer need it.
   */
  MessageItemSetReference createPersistentSet( const QList< MessageItem * > &items );

  /**
   * Returns the list of MessageItems that are still existing in the
   * set pointed by the specified reference. This list will contain
   * at most the messages that you have passed to createPersistentSet()
   * but may contain less (even 0) if these MessageItem object were removed
   * from the view for some reason.
   */
  QList< MessageItem * > persistentSetCurrentMessageItemList( MessageItemSetReference ref );

  /**
   * Deletes the persistent set pointed by the specified reference.
   * If the set does not exist anymore, nothing happens.
   */
  void deletePersistentSet( MessageItemSetReference ref );

  /**
   * If bMark is true this function marks the messages as "about to be removed"
   * so they appear dimmer and aren't selectable in the view.
   * If bMark is false then this function clears the "about to be removed" state
   * for the specified MessageItems.
   */
  void markMessageItemsAsAboutToBeRemoved( QList< MessageItem * > &items, bool bMark );

  /**
   * Returns true if the current Aggregation is threaded, false otherwise
   * (or if there is no current Aggregation).
   */
  bool isThreaded() const;

  /**
   * If expand is true then it expands the current thread, otherwise
   * collapses it.
   */
  void setCurrentThreadExpanded( bool expand );

  /**
   * If expand is true then it expands all the threads, otherwise
   * collapses them.
   */
  void setAllThreadsExpanded( bool expand );

  /**
   * Selects the next message item in the view. If unread is true
   * then selects the next unread message item. If expandSelection is
   * true then the previous selection is retained, otherwise it's cleared.
   * If centerItem is true then the specified item will be positioned
   * at the center of the view, if possible.
   * If loop is true then the "next" algorithm will restart from the beginning
   * of the list if the end is reached, otherwise it will just stop returning false.
   */
  bool selectNextMessageItem( bool unread, bool expandSelection, bool centerItem, bool loop );

  /**
   * Selects the previous message item in the view. If unread is true
   * then selects the previous unread message item. If expandSelection is
   * true then the previous selection is retained, otherwise it's cleared.
   * If centerItem is true then the specified item will be positioned
   * at the center of the view, if possible.
   * If loop is true then the "previous" algorithm will restart from the end
   * of the list if the beginning is reached, otherwise it will just stop returning false.
   */
  bool selectPreviousMessageItem( bool unread, bool expandSelection, bool centerItem, bool loop );

  /**
   * Focuses the next message item in the view without actually selecting it.
   * If unread is true then focuses the next unread message item.
   * If centerItem is true then the specified item will be positioned
   * at the center of the view, if possible.
   * If loop is true then the "next" algorithm will restart from the beginning
   * of the list if the end is reached, otherwise it will just stop returning false.
   */
  bool focusNextMessageItem( bool unread, bool centerItem, bool loop );

  /**
   * Focuses the previous message item in the view without actually selecting it.
   * If unread is true then focuses the previous unread message item.
   * If centerItem is true then the specified item will be positioned
   * at the center of the view, if possible.
   * If loop is true then the "previous" algorithm will restart from the end
   * of the list if the beginning is reached, otherwise it will just stop returning false.
   */
  bool focusPreviousMessageItem( bool unread, bool centerItem, bool loop );

  /**
   * Selects the currently focused message item. If the currently focused
   * message is already selected (which is very likely) nothing happens.
   * If centerItem is true then the specified item will be positioned
   * at the center of the view, if possible.
   */
  void selectFocusedMessageItem( bool centerItem );

  /**
   * Selects the first unread message item in the view.
   * If centerItem is true then the specified item will be positioned
   * at the center of the view, if possible.
   */
  void selectFirstUnreadMessageItem( bool centerItem );

protected:
  /**
   * Reimplemented in order to catch QHelpEvent
   */
  virtual bool event( QEvent *e );

  /**
   * Reimplemented in order to catch palette, font and style changes
   */
  virtual void changeEvent( QEvent *e );

  /**
   * Reimplemented in order to apply skin column widths on the first show
   */
  virtual void showEvent( QShowEvent *e );

  /**
   * Reimplemented in order to handle clicks with sub-item precision.
   */
  virtual void mousePressEvent( QMouseEvent * e );

  /**
   * Reimplemented in order to handle double clicks with sub-item precision.
   */
  virtual void mouseDoubleClickEvent( QMouseEvent * e );

  /**
   * Reimplemented in order to handle DnD
   */
  virtual void mouseMoveEvent( QMouseEvent * e );

  /**
   * Reimplemented in order to handle message DnD
   */
  virtual void dragEnterEvent( QDragEnterEvent * e );

  /**
   * Reimplemented in order to handle message DnD
   */
  virtual void dragMoveEvent( QDragMoveEvent * e );

  /**
   * Reimplemented in order to handle message DnD
   */
  virtual void dropEvent( QDropEvent * e );

  /**
   * Reimplemented in order to resize columns when header is not visible
   */
  virtual void resizeEvent( QResizeEvent * e );

  /**
   * Applies the skin columns to this view.
   * Columns visible by default are shown, the other are hidden.
   * Visible columns are assigned space inside the view by using the size hints and some heuristics.
   */
  void applySkinColumns();

  /**
   * This is called by the model from inside setFolder().
   * It's called just after the model has been reset but before
   * any row has been inserted. This allows us to call updateSkinColumns() as needed.
   */
  void modelHasBeenReset();

  /**
   * Recursive helper for currentThreadAsMessageItemList()
   */
  void appendMessageItemChildren( MessageItem * par, QList< MessageItem * > &list );

  /**
   * This is called by the model to insulate us from certain QTreeView signals
   * This is because they may be spurious (caused by Model item rearrangements).
   */
  void ignoreCurrentChanges( bool ignore );

  /**
   * Expands or collapses the children of the specified item, recursively.
   */
  void setChildrenExpanded( const Item * parent, bool expand );

  /**
   * Finds the next message item with respect to the current item.
   * If there is no current item then the search starts from the beginning.
   * Returns 0 if no next message could be found.
   *
   * If unread is true finds the next unread message.
   * If loop is true then restarts from the beginning if end is
   * reached, otherwise it just returns 0 in this case.
   */
  Item * nextMessageItem( bool unread, bool loop );

  /**
   * Finds message item that comes "after" the reference item.
   * If reference item is 0 then the search starts from the beginning.
   * Returns 0 if no next message could be found.
   *
   * If unread is true finds the next unread message.
   * If loop is true then restarts from the beginning if end is
   * reached, otherwise it just returns 0 in this case.
   */
  Item * messageItemAfter( Item * referenceItem, bool unread, bool loop );

  /**
   * Finds the first message item in the view.
   * If unread is true then the first unread message item is found,
   * otherwise just any message item will be returned.
   *
   * Returns 0 if the view is empty.
   */
  Item * firstMessageItem( bool unread )
    { return messageItemAfter( 0, unread, false ); };

  /**
   * Finds the previous message item with respect to the current item.
   * If there is no current item then the search starts from the end.
   * Returns 0 if no previous message could be found.
   *
   * If unread is true finds the previous unread message.
   * If loop is true then restarts from the end if beginning is
   * reached, otherwise it just return 0 in this case.
   */
  Item * previousMessageItem( bool unread, bool loop );

  /**
   * Finds message item that comes "before" the reference item.
   * If reference item is 0 then the search starts from the end.
   * Returns 0 if no next message could be found.
   *
   * If unread is true finds the next unread message.
   * If loop is true then restarts from the beginning if end is
   * reached, otherwise it just returns 0 in this case.
   */
  Item * messageItemBefore( Item * referenceItem, bool unread, bool loop );

  /**
   * Finds the last message item in the view.
   * If unread is true then the last unread message item is found,
   * otherwise just any message item will be returned.
   *
   * Returns 0 if the view is empty.
   */
  Item * lastMessageItem( bool unread )
    { return messageItemBefore( 0, unread, false ); };

  /**
   * This is called by Model to signal a start of a lengthy job batch.
   * Note that this is NOT called for jobs that can be completed in a single step.
   */
  void modelJobBatchStarted();

  /**
   * This is called by Model to signal the end of a lengthy job batch.
   * Note that this is NOT called for jobs that can be completed in a single step.
   */
  void modelJobBatchTerminated();

  /**
   * This is called by Model to signal that the initial loading stage of a newly
   * attached StorageModel is terminated.
   */
  void modelFinishedLoading();

  /**
   * Performs a change in the specified MessageItem status.
   * It first applies the change to the cached state in MessageItem and
   * then requests our parent widget to act on the storage.
   */
  void changeMessageStatus( MessageItem * it, const KPIM::MessageStatus &set, const KPIM::MessageStatus &unset );

public slots:
  /**
   * Collapses all the group headers (if present in the current Aggregation)
   */
  void slotCollapseAllGroups();

  /**
   * Expands all the group headers (if present in the current Aggregation)
   */
  void slotExpandAllGroups();

protected slots:

  /**
   * Handles context menu requests for the header.
   */
  void slotHeaderContextMenuRequested( const QPoint &pnt );

  /**
   * Handles header context menu selections. Shows or hides
   * the header columns.
   */
  void slotHeaderContextMenuTriggered( QAction * act );

  /**
   * Handles selection item management
   */
  void slotSelectionChanged( const QItemSelection &current, const QItemSelection & );

}; // class View

} // namespace Core

} // namespace MessageListView

} // namespace KMail


#endif //!__KMAIL_MESSAGELISTVIEW_CORE_VIEW_H__
