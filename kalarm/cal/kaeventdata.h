/*
 *  kaeventdata.h  -  represents calendar alarm and event data
 *  Program:  kalarm
 *  Copyright © 2001-2011 by David Jarvie <djarvie@kde.org>
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KAEVENTDATA_H
#define KAEVENTDATA_H

#include "kalarm_cal_export.h"

#include "datetime.h"
#include "karecurrence.h"
#include "kcalendar.h"
#include "repetition.h"

#include <kcal/person.h>
#include <kcal/event.h>

#include <QColor>
#include <QFont>
#include <QList>

namespace KCal { class CalendarLocal; }
class AlarmResource;
class KARecurrence;
struct AlarmData;


typedef KCal::Person  EmailAddress;
class KALARM_CAL_EXPORT EmailAddressList : public QList<KCal::Person>
{
	public:
		EmailAddressList() : QList<KCal::Person>() { }
		EmailAddressList(const QList<KCal::Person>& list)  { operator=(list); }
		EmailAddressList& operator=(const QList<KCal::Person>&);
		operator QStringList() const;
		QString     join(const QString& separator) const;
		QStringList pureAddresses() const;
		QString     pureAddresses(const QString& separator) const;
	private:
		QString     address(int index) const;
};


// Base class containing data common to KAAlarm and KAEventData
class KALARM_CAL_EXPORT KAAlarmEventBase
{
	public:
		~KAAlarmEventBase()  { }
		int                lateCancel() const          { return mLateCancel; }
		bool               repeatAtLogin() const       { return mRepeatAtLogin; }

	protected:
		enum Type  { T_MESSAGE, T_FILE, T_COMMAND, T_EMAIL, T_AUDIO };

		KAAlarmEventBase() : mNextRepeat(0), mLateCancel(0), mAutoClose(false), mRepeatAtLogin(false)  {}
		KAAlarmEventBase(const KAAlarmEventBase& rhs)             { copy(rhs); }
		KAAlarmEventBase& operator=(const KAAlarmEventBase& rhs)  { copy(rhs);  return *this; }
		void               copy(const KAAlarmEventBase&);
		void               set(int flags);
		int                baseFlags() const;
#ifdef KDE_NO_DEBUG_OUTPUT
		void               baseDumpDebug() const  { }
#else
		void               baseDumpDebug() const;
#endif

		QString            mEventID;          // UID: KCal::Event unique ID
		DateTime           mNextMainDateTime; // next time to display the alarm, excluding repetitions
		QString            mText;             // message text, file URL, command, email body [or audio file for KAAlarm]
		QColor             mBgColour;         // background colour of alarm message
		QColor             mFgColour;         // foreground colour of alarm message, or invalid for default
		QFont              mFont;             // font of alarm message (ignored if mUseDefaultFont true)
		Type               mActionType;       // alarm action type
		Repetition         mRepetition;       // sub-repetition count and interval
		int                mNextRepeat;       // repetition count of next due sub-repetition
		int                mLateCancel;       // how many minutes late will cancel the alarm, or 0 for no cancellation
		bool               mAutoClose;        // whether to close the alarm window after the late-cancel period
		bool               mCommandScript;    // the command text is a script, not a shell command line
		bool               mRepeatAtLogin;    // whether to repeat the alarm at every login
		bool               mUseDefaultFont;   // use default message font, not mFont

	friend struct AlarmData;
};


// KAAlarm corresponds to a single KCal::Alarm instance.
// A KAEventData may contain multiple KAAlarm's.
class KALARM_CAL_EXPORT KAAlarm : public KAAlarmEventBase
{
	public:
		// Define the basic KAAlarm action types
		enum Action
		{
			MESSAGE = T_MESSAGE,   // KCal::Alarm::Display type: display a text message
			FILE    = T_FILE,      // KCal::Alarm::Display type: display a file (URL given by the alarm text)
			COMMAND = T_COMMAND,   // KCal::Alarm::Procedure type: execute a shell command
			EMAIL   = T_EMAIL,     // KCal::Alarm::Email type: send an email
			AUDIO   = T_AUDIO      // KCal::Alarm::Audio type: play a sound file
		};
		// Define the KAAlarm types.
		// KAAlarm's of different types may be contained in a KAEventData,
		// each defining a different component of the overall alarm.
		enum Type
		{
			INVALID_ALARM       = 0,     // not an alarm
			MAIN_ALARM          = 1,     // THE real alarm. Must be the first in the enumeration.
			// The following values may be used in combination as a bitmask 0x0E
			REMINDER_ALARM      = 0x02,  // reminder in advance of main alarm
			DEFERRED_ALARM      = 0x04,  // deferred alarm
			DEFERRED_REMINDER_ALARM = REMINDER_ALARM | DEFERRED_ALARM,  // deferred early warning
			// The following values must be greater than the preceding ones, to
			// ensure that in ordered processing they are processed afterwards.
			AT_LOGIN_ALARM      = 0x10,  // additional repeat-at-login trigger
			DISPLAYING_ALARM    = 0x20,  // copy of the alarm currently being displayed
			// The following values are for internal KAEventData use only
			AUDIO_ALARM         = 0x30,  // sound to play when displaying the alarm
			PRE_ACTION_ALARM    = 0x40,  // command to execute before displaying the alarm
			POST_ACTION_ALARM   = 0x50   // command to execute after the alarm window is closed
		};
		enum SubType
		{
			INVALID__ALARM                = INVALID_ALARM,
			MAIN__ALARM                   = MAIN_ALARM,
			// The following values may be used in combination as a bitmask 0x0E
			REMINDER__ALARM               = REMINDER_ALARM,
			TIMED_DEFERRAL_FLAG           = 0x08,  // deferral has a time; if clear, it is date-only
			DEFERRED_DATE__ALARM          = DEFERRED_ALARM,  // deferred alarm - date-only
			DEFERRED_TIME__ALARM          = DEFERRED_ALARM | TIMED_DEFERRAL_FLAG,
			DEFERRED_REMINDER_DATE__ALARM = REMINDER_ALARM | DEFERRED_ALARM,  // deferred early warning (date-only)
			DEFERRED_REMINDER_TIME__ALARM = REMINDER_ALARM | DEFERRED_ALARM | TIMED_DEFERRAL_FLAG,  // deferred early warning (date/time)
			// The following values must be greater than the preceding ones, to
			// ensure that in ordered processing they are processed afterwards.
			AT_LOGIN__ALARM               = AT_LOGIN_ALARM,
			DISPLAYING__ALARM             = DISPLAYING_ALARM,
			// The following values are for internal KAEventData use only
			AUDIO__ALARM                  = AUDIO_ALARM,   // audio alarm for main display alarm
			PRE_ACTION__ALARM             = PRE_ACTION_ALARM,
			POST_ACTION__ALARM            = POST_ACTION_ALARM
		};

		KAAlarm()          : mType(INVALID__ALARM), mDeferred(false) { }
		KAAlarm(const KAAlarm&);
		~KAAlarm()  { }
		Action             action() const               { return (Action)mActionType; }
		bool               valid() const                { return mType != INVALID__ALARM; }
		Type               type() const                 { return static_cast<Type>(mType & ~TIMED_DEFERRAL_FLAG); }
		SubType            subType() const              { return mType; }
		const QString&     eventId() const              { return mEventID; }
		DateTime           dateTime(bool withRepeats = false) const
		                                                { return (withRepeats && mNextRepeat && mRepetition)
		                                                    ? mRepetition.duration(mNextRepeat).end(mNextMainDateTime.kDateTime()) : mNextMainDateTime; }
		QDate              date() const                 { return mNextMainDateTime.date(); }
		QTime              time() const                 { return mNextMainDateTime.effectiveTime(); }
		bool               reminder() const             { return mType == REMINDER__ALARM; }
		bool               deferred() const             { return mDeferred; }
		void               setTime(const DateTime& dt)  { mNextMainDateTime = dt; }
		void               setTime(const KDateTime& dt) { mNextMainDateTime = dt; }
#ifdef KDE_NO_DEBUG_OUTPUT
		void               dumpDebug() const  { }
		static const char* debugType(Type)   { return ""; }
#else
		void               dumpDebug() const;
		static const char* debugType(Type);
#endif

	private:
		SubType            mType;             // alarm type
		bool               mRecurs;           // there is a recurrence rule for the alarm
		bool               mDeferred;         // whether the alarm is an extra deferred/deferred-reminder alarm

	friend class KAEventData;
};


/** KAEventData corresponds to a KCal::Event instance */
class KALARM_CAL_EXPORT KAEventData : public KAAlarmEventBase
{
	public:
		class Observer
		{
		    public:
			virtual ~Observer() {}
			virtual void eventUpdated(const KAEventData*) = 0;
		};

		// Flags for use in D-Bus calls, etc.
		// Note that these are copied in class KAEvent, where the D-Bus
		// values are actually defined.
		enum
		{
			BEEP            = 0x02,    // sound audible beep when alarm is displayed
			REPEAT_AT_LOGIN = 0x04,    // repeat alarm at every login
			ANY_TIME        = 0x08,    // only a date is specified for the alarm, not a time
			CONFIRM_ACK     = 0x10,    // closing the alarm message window requires confirmation prompt
			EMAIL_BCC       = 0x20,    // blind copy the email to the user
			DEFAULT_FONT    = 0x40,    // use default alarm message font
			REPEAT_SOUND    = 0x80,    // repeat sound file while alarm is displayed
			DISABLED        = 0x100,   // alarm is currently disabled
			AUTO_CLOSE      = 0x200,   // auto-close alarm window after late-cancel period
			SCRIPT          = 0x400,   // command is a script, not a shell command line
			EXEC_IN_XTERM   = 0x800,   // execute command in terminal window
			SPEAK           = 0x1000,  // speak the message when the alarm is displayed
			COPY_KORGANIZER = 0x2000,  // KOrganizer should hold a copy of the event
			EXCL_HOLIDAYS   = 0x4000,  // don't trigger alarm on holidays
			WORK_TIME_ONLY  = 0x8000,  // trigger alarm only during working hours
			DISPLAY_COMMAND = 0x10000, // display command output in alarm window
			// The following are read-only internal values
			REMINDER        = 0x100000,
			DEFERRAL        = 0x200000,
			TIMED_FLAG      = 0x400000,
			DATE_DEFERRAL   = DEFERRAL,
			TIME_DEFERRAL   = DEFERRAL | TIMED_FLAG,
			DISPLAYING_     = 0x800000,
			READ_ONLY_FLAGS = 0xF00000  // mask for all read-only internal values
		};
		enum Action
		{
			MESSAGE = T_MESSAGE,
			FILE    = T_FILE,
			COMMAND = T_COMMAND,
			EMAIL   = T_EMAIL,
			AUDIO   = T_AUDIO
		};
		enum OccurType     // what type of occurrence is due
		{
			NO_OCCURRENCE               = 0,      // no occurrence is due
			FIRST_OR_ONLY_OCCURRENCE    = 0x01,   // the first occurrence (takes precedence over LAST_RECURRENCE)
			RECURRENCE_DATE             = 0x02,   // a recurrence with only a date, not a time
			RECURRENCE_DATE_TIME        = 0x03,   // a recurrence with a date and time
			LAST_RECURRENCE             = 0x04,   // the last recurrence
			OCCURRENCE_REPEAT = 0x10,    // (bitmask for a repetition of an occurrence)
			FIRST_OR_ONLY_OCCURRENCE_REPEAT = OCCURRENCE_REPEAT | FIRST_OR_ONLY_OCCURRENCE,     // a repetition of the first occurrence
			RECURRENCE_DATE_REPEAT          = OCCURRENCE_REPEAT | RECURRENCE_DATE,      // a repetition of a date-only recurrence
			RECURRENCE_DATE_TIME_REPEAT     = OCCURRENCE_REPEAT | RECURRENCE_DATE_TIME, // a repetition of a date/time recurrence
			LAST_RECURRENCE_REPEAT          = OCCURRENCE_REPEAT | LAST_RECURRENCE       // a repetition of the last recurrence
		};
		enum OccurOption     // options for nextOccurrence()
		{
			IGNORE_REPETITION,    // check for recurrences only, ignore repetitions
			RETURN_REPETITION,    // return repetition if it's the next occurrence
			ALLOW_FOR_REPETITION  // check for repetition being the next occurrence, but return recurrence
		};
		enum DeferLimitType    // what type of occurrence currently limits a deferral
		{
			LIMIT_NONE,
			LIMIT_MAIN,
			LIMIT_RECURRENCE,
			LIMIT_REPETITION,
			LIMIT_REMINDER
		};

		KAEventData(Observer*);
		KAEventData(Observer*, const KDateTime& dt, const QString& message, const QColor& bg, const QColor& fg, const QFont& f, Action action, int lateCancel, int flags, bool changesPending = false);
		explicit KAEventData(Observer*, const KCal::Event*);
		KAEventData(Observer*, const KAEventData&);
private:
		KAEventData(const KAEventData&);
public:
		~KAEventData()     { delete mRecurrence; }
		KAEventData&       operator=(const KAEventData& e)       { if (&e != this) copy(e);  return *this; }
		void               set(const KCal::Event*);
		void               set(const KDateTime&, const QString& message, const QColor& bg, const QColor& fg, const QFont&, Action, int lateCancel, int flags, bool changesPending = false);
		void               setEmail(uint from, const EmailAddressList&, const QString& subject, const QStringList& attachments);
		void               setResource(AlarmResource* r)                    { mResource = r; }
		void               setAudioFile(const QString& filename, float volume, float fadeVolume, int fadeSeconds, bool allowEmptyFile = false);
		void               setTemplate(const QString& name, int afterTime = -1);
		void               setActions(const QString& pre, const QString& post, bool cancelOnError)
		                                                                    { mPreAction = pre;  mPostAction = post;  mCancelOnPreActErr = cancelOnError;  mUpdated = true; }
		OccurType          setNextOccurrence(const KDateTime& preDateTime, const QTime& startOfDay);
		void               setFirstRecurrence(const QTime& startOfDay);
		void               setCategory(KCalEvent::Status);
		void               setUid(KCalEvent::Status s)                       { mEventID = KCalEvent::uid(mEventID, s);  mUpdated = true; }
		void               setEventId(const QString& id)                     { mEventID = id;  mUpdated = true; }
		void               setTime(const KDateTime& dt)                      { mNextMainDateTime = dt;  mUpdated = true; }
		void               setSaveDateTime(const KDateTime& dt)              { mSaveDateTime = dt;  mUpdated = true; }
		void               setLateCancel(int lc)                             { mLateCancel = lc;  mUpdated = true; }
		void               setAutoClose(bool ac)                             { mAutoClose = ac;  mUpdated = true; }
		void               setRepeatAtLogin(bool rl);
		void               setExcludeHolidays(bool ex)                       { mExcludeHolidays = ex;  mUpdated = true; }
		void               setWorkTimeOnly(bool wto)                         { mWorkTimeOnly = wto; }
		void               setKMailSerialNumber(unsigned long n)             { mKMailSerialNumber = n; }
		void               setLogFile(const QString& logfile);
		void               setReminder(int minutes, bool onceOnly);
		bool               defer(const DateTime&, bool reminder, const QTime& startOfDay, bool adjustRecurrence = false);
		void               cancelDefer();
		void               setDeferDefaultMinutes(int minutes, bool dateOnly = false)
 		                                                                     { mDeferDefaultMinutes = minutes;  mDeferDefaultDateOnly = dateOnly;  mUpdated = true; }
		bool               setDisplaying(const KAEventData&, KAAlarm::Type, const QString& resourceID, const KDateTime&, bool showEdit, bool showDefer);
		void               reinstateFromDisplaying(const KCal::Event*, QString& resourceID, bool& showEdit, bool& showDefer);
		void               setArchive()                                      { mArchive = true;  mUpdated = true; }
		void               setEnabled(bool enable)                           { mEnabled = enable;  mUpdated = true; }
		void               startChanges()                                    { ++mChangeCount; }
		void               endChanges();
		void               setUpdated()                                      { mUpdated = true; }
		void               clearUpdated() const                              { mUpdated = false; }
		void               registerObserver(Observer*);
		void               unregisterObserver(Observer*);
		void               clearResourceId()                                 { mResourceId.clear(); }
		void               updateWorkHours() const                           { if (mWorkTimeOnly) notifyChanges(); }
		void               updateHolidays() const                            { if (mExcludeHolidays) notifyChanges(); }
		void               removeExpiredAlarm(KAAlarm::Type);
		void               incrementRevision()                               { ++mRevision;  mUpdated = true; }

		const QString&     cleanText() const              { return mText; }
		QString            message() const                { return (mActionType == T_MESSAGE || mActionType == T_EMAIL) ? mText : QString(); }
		QString            fileName() const               { return (mActionType == T_FILE) ? mText : QString(); }
		QString            command() const                { return (mActionType == T_COMMAND) ? mText : QString(); }
		uint               emailFromId() const            { return mEmailFromIdentity; }
		const EmailAddressList& emailAddresses() const    { return mEmailAddresses; }
		QString            emailAddresses(const QString& sep) const  { return mEmailAddresses.join(sep); }
		QStringList        emailPureAddresses() const     { return mEmailAddresses.pureAddresses(); }
		QString            emailPureAddresses(const QString& sep) const  { return mEmailAddresses.pureAddresses(sep); }
		const QString&     emailSubject() const           { return mEmailSubject; }
		const QStringList& emailAttachments() const       { return mEmailAttachments; }
		bool               emailBcc() const               { return mEmailBcc; }
		const QColor&      bgColour() const               { return mBgColour; }
		const QColor&      fgColour() const               { return mFgColour; }
		bool               useDefaultFont() const         { return mUseDefaultFont; }
		QFont              font() const                   { return mFont; }
		bool               autoClose() const              { return mAutoClose; }
		bool               confirmAck() const             { return mConfirmAck; }
		bool               commandScript() const          { return mCommandScript; }
		const Repetition&  repetition() const             { return mRepetition; }
		int                nextRepetition() const         { return mNextRepeat; }
		bool               displaying() const             { return mDisplaying; }
		bool               beep() const                   { return mBeep; }
		bool               isTemplate() const             { return !mTemplateName.isEmpty(); }
		const QString&     templateName() const           { return mTemplateName; }
		bool               usingDefaultTime() const       { return mTemplateAfterTime == 0; }
		int                templateAfterTime() const      { return mTemplateAfterTime; }
		KAAlarm            alarm(KAAlarm::Type) const;
		KAAlarm            firstAlarm() const;
		KAAlarm            nextAlarm(const KAAlarm& al) const  { return nextAlarm(al.type()); }
		KAAlarm            nextAlarm(KAAlarm::Type) const;
		KAAlarm            convertDisplayingAlarm() const;
		bool               updateKCalEvent(KCal::Event*, bool checkUid = true, bool original = false) const;
		Action             action() const                 { return (Action)mActionType; }
		bool               displayAction() const          { return mActionType == T_MESSAGE || mActionType == T_FILE || (mActionType == T_COMMAND && mCommandDisplay); }
		const QString&     id() const                     { return mEventID; }
		bool               valid() const                  { return mAlarmCount  &&  (mAlarmCount != 1 || !mRepeatAtLogin); }
		int                alarmCount() const             { return mAlarmCount; }
		const DateTime&    startDateTime() const          { return mStartDateTime; }
		DateTime           mainDateTime(bool withRepeats = false) const
		                                                  { return (withRepeats && mNextRepeat && mRepetition)
		                                                    ? mRepetition.duration(mNextRepeat).end(mNextMainDateTime.kDateTime()) : mNextMainDateTime; }
		QDate              mainDate() const               { return mNextMainDateTime.date(); }
		QTime              mainTime() const               { return mNextMainDateTime.effectiveTime(); }
		DateTime           mainEndRepeatTime() const      { return mRepetition ? mRepetition.duration().end(mNextMainDateTime.kDateTime()) : mNextMainDateTime; }
		int                reminder() const               { return mReminderMinutes; }
		bool               reminderOnceOnly() const       { return mReminderOnceOnly; }
		bool               reminderDeferral() const       { return mDeferral == REMINDER_DEFERRAL; }
		int                reminderArchived() const       { return mArchiveReminderMinutes; }
		DateTime           deferDateTime() const          { return mDeferralTime; }
		DateTime           deferralLimit(const QTime& startOfDay, DeferLimitType* = 0) const;
		int                deferDefaultMinutes() const    { return mDeferDefaultMinutes; }
		bool               deferDefaultDateOnly() const   { return mDeferDefaultDateOnly; }
		const QString&     messageFileOrCommand() const   { return mText; }
		QString            logFile() const                { return mLogFile; }
		bool               commandXterm() const           { return mCommandXterm; }
		bool               commandDisplay() const         { return mCommandDisplay; }
		unsigned long      kmailSerialNumber() const      { return mKMailSerialNumber; }
		bool               copyToKOrganizer() const       { return mCopyToKOrganizer; }
		bool               holidaysExcluded() const       { return mExcludeHolidays; }
		bool               workTimeOnly() const           { return mWorkTimeOnly; }
		bool               speak() const                  { return (mActionType == T_MESSAGE  ||  (mActionType == T_COMMAND && mCommandDisplay)) && mSpeak; }
		const QString&     audioFile() const              { return mAudioFile; }
		float              soundVolume() const            { return mSoundVolume; }
		float              fadeVolume() const             { return mSoundVolume >= 0 && mFadeSeconds ? mFadeVolume : -1; }
		int                fadeSeconds() const            { return mSoundVolume >= 0 && mFadeVolume >= 0 ? mFadeSeconds : 0; }
		bool               repeatSound() const            { return mRepeatSound; }
		const QString&     preAction() const              { return mPreAction; }
		const QString&     postAction() const             { return mPostAction; }
		bool               cancelOnPreActionError() const { return mCancelOnPreActErr; }
		bool               recurs() const                 { return checkRecur() != KARecurrence::NO_RECUR; }
		KARecurrence::Type recurType() const              { return checkRecur(); }
		KARecurrence*      recurrence() const             { return mRecurrence; }
		int                recurInterval() const;    // recurrence period in units of the recurrence period type (minutes, days, etc)
		KCal::Duration     longestRecurrenceInterval() const    { return mRecurrence ? mRecurrence->longestInterval() : KCal::Duration(0); }
		QString            recurrenceText(bool brief = false) const;
		QString            repetitionText(bool brief = false) const;
		bool               occursAfter(const KDateTime& preDateTime, const QTime& startOfDay, bool includeRepetitions) const;
		OccurType          nextOccurrence(const KDateTime& preDateTime, DateTime& result, const QTime& startOfDay, OccurOption = IGNORE_REPETITION) const;
		OccurType          previousOccurrence(const KDateTime& afterDateTime, DateTime& result, const QTime& startOfDay, bool includeRepetitions = false) const;
		int                flags() const;
		bool               deferred() const               { return mDeferral > 0; }
		bool               toBeArchived() const           { return mArchive; }
		bool               enabled() const                { return mEnabled; }
		bool               updated() const                { return mUpdated; }
		bool               mainExpired() const            { return mMainExpired; }
		bool               expired() const                { return (mDisplaying && mMainExpired)  ||  mCategory == KCalEvent::ARCHIVED; }
		KCalEvent::Status  category() const               { return mCategory; }
		AlarmResource*     resource() const               { return mResource; }
		QString            resourceId() const             { return mResourceId; }

		struct MonthPos
		{
			MonthPos() : days(7) { }
			int        weeknum;     // week in month, or < 0 to count from end of month
			QBitArray  days;        // days in week
		};
		bool               setRepetition(const Repetition&);
		void               setNoRecur()                   { clearRecur(); notifyChanges(); }
		void               setRecurrence(const KARecurrence&);
		bool               setRecurMinutely(int freq, int count, const KDateTime& end);
		bool               setRecurDaily(int freq, const QBitArray& days, int count, const QDate& end);
		bool               setRecurWeekly(int freq, const QBitArray& days, int count, const QDate& end);
		bool               setRecurMonthlyByDate(int freq, const QList<int>& days, int count, const QDate& end);
		bool               setRecurMonthlyByPos(int freq, const QList<MonthPos>& pos, int count, const QDate& end);
		bool               setRecurAnnualByDate(int freq, const QList<int>& months, int day, KARecurrence::Feb29Type, int count, const QDate& end);
		bool               setRecurAnnualByPos(int freq, const QList<MonthPos>& pos, const QList<int>& months, int count, const QDate& end);
//		static QValueList<MonthPos> convRecurPos(const QValueList<KCal::RecurrenceRule::WDayPos>&);
		void               adjustRecurrenceStartOfDay();
#ifdef KDE_NO_DEBUG_OUTPUT
		void               dumpDebug() const  { }
#else
		void               dumpDebug() const;
#endif
		static QByteArray  icalProductId();
		static int         currentCalendarVersion();
		static QByteArray  currentCalendarVersionString();
		static void        setStartOfDay(const QTime& startOfDay);
		static bool        convertKCalEvents(KCal::CalendarLocal&, int calendarVersion, bool adjustSummerTime, const QTime& startOfDay);
//		static bool        convertRepetitions(KCal::CalendarLocal&);
		KARecurrence::Type checkRecur() const;

	private:
		enum DeferType {
			NO_DEFERRAL = 0,        // there is no deferred alarm
			NORMAL_DEFERRAL,        // the main alarm, a recurrence or a repeat is deferred
			REMINDER_DEFERRAL       // a reminder alarm is deferred
		};

		void               copy(const KAEventData&);
		bool               setRecur(KCal::RecurrenceRule::PeriodType, int freq, int count, const QDate& end, KARecurrence::Feb29Type = KARecurrence::Feb29_None);
		bool               setRecur(KCal::RecurrenceRule::PeriodType, int freq, int count, const KDateTime& end, KARecurrence::Feb29Type = KARecurrence::Feb29_None);
		void               clearRecur();
		OccurType          nextRecurrence(const KDateTime& preDateTime, DateTime& result, const QTime& startOfDay) const;
		void               setAudioAlarm(KCal::Alarm*) const;
		void               notifyChanges() const;
		static bool        convertStartOfDay(KCal::Event*);
		static bool        convertRepetition(KCal::Event*);
		KCal::Alarm*       initKCalAlarm(KCal::Event*, const DateTime&, const QStringList& types, KAAlarm::Type = KAAlarm::INVALID_ALARM) const;
		KCal::Alarm*       initKCalAlarm(KCal::Event*, int startOffsetSecs, const QStringList& types, KAAlarm::Type = KAAlarm::INVALID_ALARM) const;
		static DateTime    readDateTime(const KCal::Event*, bool dateOnly, DateTime& start);
		static void        readAlarms(const KCal::Event*, void* alarmMap, bool cmdDisplay = false);
		static void        readAlarm(const KCal::Alarm*, AlarmData&, bool audioMain, bool cmdDisplay = false);
		inline void        set_deferral(DeferType);
		inline void        set_reminder(int minutes);
		inline void        set_archiveReminder();

		QList<Observer*>   mObservers;        // objects to notify when the event is updated
		QString            mTemplateName;     // alarm template's name, or null if normal event
		AlarmResource*     mResource;         // resource which owns the event
		QString            mResourceId;       // saved resource ID (not the resource the event is in)
		QString            mAudioFile;        // ATTACH: audio file to play
		QString            mPreAction;        // command to execute before alarm is displayed
		QString            mPostAction;       // command to execute after alarm window is closed
		DateTime           mStartDateTime;    // DTSTART and DTEND: start and end time for event
		KDateTime          mSaveDateTime;     // CREATED: date event was created, or saved in archive calendar
		KDateTime          mAtLoginDateTime;  // repeat-at-login time
		DateTime           mDeferralTime;     // extra time to trigger alarm (if alarm or reminder deferred)
		DateTime           mDisplayingTime;   // date/time shown in the alarm currently being displayed
		int                mDisplayingFlags;  // type of alarm which is currently being displayed
		int                mReminderMinutes;  // how long in advance reminder is to be, or 0 if none
		int                mArchiveReminderMinutes; // original reminder period if now expired, or for restoration after next recurrence, or 0 if none
		int                mDeferDefaultMinutes; // default number of minutes for deferral dialog, or 0 to select time control
		bool               mDeferDefaultDateOnly;// select date-only by default in deferral dialog
		int                mRevision;         // SEQUENCE: revision number of the original alarm, or 0
		KARecurrence*      mRecurrence;       // RECUR: recurrence specification, or 0 if none
		int                mAlarmCount;       // number of alarms: count of !mMainExpired, mRepeatAtLogin, mDeferral, mReminderMinutes, mDisplaying
		DeferType          mDeferral;         // whether the alarm is an extra deferred/deferred-reminder alarm
		unsigned long      mKMailSerialNumber;// if email text, message's KMail serial number
		int                mTemplateAfterTime;// time not specified: use n minutes after default time, or -1 (applies to templates only)
		uint               mEmailFromIdentity;// standard email identity uoid for 'From' field, or empty
		EmailAddressList   mEmailAddresses;   // ATTENDEE: addresses to send email to
		QString            mEmailSubject;     // SUMMARY: subject line of email
		QStringList        mEmailAttachments; // ATTACH: email attachment file names
		mutable int        mChangeCount;      // >0 = inhibit calling notifyChanges()
		mutable bool       mChanged;          // true if need to recalculate trigger times while mChangeCount > 0
		QString            mLogFile;          // alarm output is to be logged to this URL
		float              mSoundVolume;      // volume for sound file (range 0 - 1), or < 0 for unspecified
		float              mFadeVolume;       // initial volume for sound file (range 0 - 1), or < 0 for no fade
		int                mFadeSeconds;      // fade time for sound file, or 0 if none
		KCalEvent::Status  mCategory;         // event category (active, archived, template, ...)
		bool               mCancelOnPreActErr;// cancel alarm if pre-alarm action fails
		bool               mConfirmAck;       // alarm acknowledgement requires confirmation by user
		bool               mCommandXterm;     // command alarm is to be executed in a terminal window
		bool               mCommandDisplay;   // command output is to be displayed in an alarm window
		bool               mEmailBcc;         // blind copy the email to the user
		bool               mBeep;             // whether to beep when the alarm is displayed
		bool               mRepeatSound;      // whether to repeat the sound file while the alarm is displayed
		bool               mSpeak;            // whether to speak the message when the alarm is displayed
		bool               mCopyToKOrganizer; // KOrganizer should hold a copy of the event
		bool               mExcludeHolidays;  // don't trigger alarms on holidays
		bool               mWorkTimeOnly;     // trigger alarm only during working hours
		bool               mReminderOnceOnly; // the reminder is output only for the first recurrence
		bool               mMainExpired;      // main alarm has expired (in which case a deferral alarm will exist)
		bool               mArchiveRepeatAtLogin; // if now archived, original event was repeat-at-login
		bool               mArchive;          // event has triggered in the past, so archive it when closed
		bool               mDisplaying;       // whether the alarm is currently being displayed (i.e. in displaying calendar)
		bool               mDisplayingDefer;  // show Defer button (applies to displaying calendar only)
		bool               mDisplayingEdit;   // show Edit button (applies to displaying calendar only)
		bool               mEnabled;          // false if event is disabled
		mutable bool       mUpdated;          // event has been updated but not written to calendar file

		static const QByteArray FLAGS_PROPERTY;
		static const QString DATE_ONLY_FLAG;
		static const QString EMAIL_BCC_FLAG;
		static const QString CONFIRM_ACK_FLAG;
		static const QString KORGANIZER_FLAG;
		static const QString EXCLUDE_HOLIDAYS_FLAG;
		static const QString WORK_TIME_ONLY_FLAG;
		static const QString DEFER_FLAG;
		static const QString LATE_CANCEL_FLAG;
		static const QString AUTO_CLOSE_FLAG;
		static const QString TEMPL_AFTER_TIME_FLAG;
		static const QString KMAIL_SERNUM_FLAG;
		static const QByteArray NEXT_RECUR_PROPERTY;
		static const QByteArray REPEAT_PROPERTY;
		static const QByteArray ARCHIVE_PROPERTY;
		static const QString ARCHIVE_REMINDER_ONCE_TYPE;
		static const QByteArray LOG_PROPERTY;
		static const QString xtermURL;
		static const QString displayURL;
		static const QByteArray TYPE_PROPERTY;
		static const QString FILE_TYPE;
		static const QString AT_LOGIN_TYPE;
		static const QString REMINDER_TYPE;
		static const QString REMINDER_ONCE_TYPE;
		static const QString TIME_DEFERRAL_TYPE;
		static const QString DATE_DEFERRAL_TYPE;
		static const QString DISPLAYING_TYPE;
		static const QString PRE_ACTION_TYPE;
		static const QString POST_ACTION_TYPE;
		static const QString SOUND_REPEAT_TYPE;
		static const QByteArray NEXT_REPEAT_PROPERTY;
		static const QByteArray FONT_COLOUR_PROPERTY;
		static const QByteArray EMAIL_ID_PROPERTY;
		static const QByteArray VOLUME_PROPERTY;
		static const QByteArray SPEAK_PROPERTY;
		static const QByteArray CANCEL_ON_ERROR_PROPERTY;
		static const QString DISABLED_STATUS;
		static const QString DISP_DEFER;
		static const QString DISP_EDIT;
		static const QString SC;
};

#endif // KAEVENTDATA_H
