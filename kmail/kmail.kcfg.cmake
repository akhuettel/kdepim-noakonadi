<?xml version="1.0" encoding="UTF-8"?>
<kcfg xmlns="http://www.kde.org/standards/kcfg/1.0"
      xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
      xsi:schemaLocation="http://www.kde.org/standards/kcfg/1.0
      http://www.kde.org/standards/kcfg/1.0/kcfg.xsd" >
  <include>kmglobal.h</include>
  <include>qtextcodec.h</include>
  <include>templatesconfiguration.h</include>
  <include>kglobalsettings.h</include>
  <include>kcolorscheme.h</include>
  <include>util.h</include>
  <kcfgfile name="kmailrc"/>
    <group name="Behaviour">
      <entry name="DelayedMarkAsRead"  type="Bool">
        <label></label>
        <whatsthis></whatsthis>
        <default>true</default>
      </entry>
      <entry name="DelayedMarkTime"  type="UInt">
        <label></label>
        <whatsthis></whatsthis>
        <default>0</default>
      </entry>
      <entry name="ActionEnterFolder"  type="Enum">
        <label></label>
        <whatsthis></whatsthis>
        <choices>
          <choice name="SelectFirstNew"/>
          <choice name="SelectFirstUnreadNew"/>
          <choice name="SelectLastSelected"/>
        </choices>
        <default>SelectLastSelected</default>
      </entry>
      <entry name="NetworkState"  type="Enum">
        <choices>
          <choice name="Online"/>
          <choice name="Offline"/>
        </choices>
        <default>Online</default>
      </entry>
      <entry name="LoopOnGotoUnread"  type="Enum">
        <label></label>
        <whatsthis></whatsthis>
        <choices>
          <choice name="DontLoop"/>
          <choice name="LoopInCurrentFolder"/>
          <choice name="LoopInAllFolders"/>
          <choice name="LoopInAllMarkedFolders"/>
        </choices>
        <default>DontLoop</default>
      </entry>
      <entry name="ShowPopupAfterDnD"  type="Bool">
        <label></label>
        <whatsthis></whatsthis>
        <default>true</default>
      </entry>
      <entry name="ExcludeImportantMailFromExpiry"  type="Bool">
        <label></label>
        <whatsthis>This prevents the automatic expiry of old messages in a folder from deleting (or moving to an archive folder) the messages that are marked 'Important' or 'Action Item'</whatsthis>
        <default>true</default>
      </entry>

      <entry name="SendOnCheck"  type="Enum">
        <label>Send queued mail on mail check</label>
        <whatsthis>&lt;qt&gt;&lt;p&gt;Select whether you want KMail to send all messages in the outbox on manual or all mail checks, or whether you do not want messages to be sent automatically at all. &lt;/p&gt;&lt;/qt&gt;</whatsthis>
        <choices>
          <choice name="DontSendOnCheck"/>
          <choice name="SendOnManualChecks"/>
          <choice name="SendOnAllChecks"/>
        </choices>
        <default>DontSendOnCheck</default>
      </entry>

      <entry name="AutoLostFoundMove" type="Bool">
        <label>Automatically move non-synced mails from folders with insufficient access rights</label>
        <whatsthis>If there are new messages in a folder, which have not been uploaded to the server yet, but you do not have sufficient access rights on the folder now to upload them, these messages will automatically be moved into a lost and found folder.</whatsthis>
        <default>false</default>
      </entry>

      <entry name="AllowLocalFlags" type="Bool">
        <label>Allow local flags in read-only folders</label>
        <default>false</default>
      </entry>
    </group>

    <group name="ConfigurationDialogRestrictions">
      <entry name="MinimumCheckInterval" type="Int">
        <default>1</default>
        <whatsthis>This setting allows administrators to set a minimum delay between two mail checks. The user will not be able to choose a value smaller than the value set here.</whatsthis>
      </entry>
    </group>

    <group name="FolderSelectionDialog">
      <entry name="LastSelectedFolder" type="String">
        <whatsthis>The most recently selected folder in the folder selection dialog.</whatsthis>
        <default>inbox</default>
      </entry>

    </group>

    <group name="General">
      <entry name="disregardUmask" type="Bool">
        <label>Disregard the users umask setting and use "read-write for the user only" instead</label>
        <default>false</default>
      </entry>
      <entry name="SystemTrayEnabled" type="Bool">
        <label>Enable system tray icon</label>
        <default>false</default>
      </entry>
      <entry name="SystemTrayPolicy" type="Enum">
        <label>Policy for showing the system tray icon</label>
        <choices>
          <choice name="ShowAlways"/>
          <choice name="ShowOnUnread"/>
        </choices>
        <default>ShowOnUnread</default>
      </entry>
      <entry name="CloseDespiteSystemTray" type="Bool">
          <label>Close the application when the main window is closed, even if there is a system tray icon active.</label>
        <default>false</default>
      </entry>
      <entry name="VerboseNewMailNotification" type="Bool">
        <label>Verbose new mail notification</label>
        <whatsthis>If this option is enabled then for each folder the number of newly arrived messages is shown in the new mail notification; otherwise, you will only get a simple 'New mail arrived' message.</whatsthis>
        <default>true</default>
      </entry>
      <entry name="UseMessageIndicator" type="Bool">
        <label>Use message indicator</label>
        <whatsthis>If this option is enabled and a message indicator is installed on the desktop, then indicators will be displayed for folders containing unread messages.</whatsthis>
        <default>false</default>
      </entry>
      <entry name="ExternalEditor" type="String" key="external-editor">
        <label>Specify e&amp;ditor:</label>
        <default>kwrite %f</default>
      </entry>
      <entry name="UseExternalEditor" type="Bool" key="use-external-editor">
        <label>Use e&amp;xternal editor instead of composer</label>
        <default>false</default>
      </entry>
      <entry name="CustHeaderCount" type="Int" key="mime-header-count" />
      <entry name="FolderLoadingTimeout" type="Int" hidden="true">
         <default>1000</default>
      </entry>
      <entry name="CloseToQuotaThreshold" type="Int">
        <label>The threshold for when to warn the user that a folder is nearing its quota limit.</label>
         <default>85</default>
      </entry>
      <entry name="QuotaUnit"  type="Enum">
        <label></label>
        <whatsthis></whatsthis>
        <choices>
          <choice name="KB"/>
          <choice name="MB"/>
          <choice name="GB"/>
        </choices>
        <default>MB</default>
      </entry>
      <entry name="MaildirFilenameSeparator" type="String">
        <label>
          The filename separator for maildir files "uniq:info" - see the original maildir specification at http://cr.yp.to/proto/maildir.html
          The default depends on the current operating system. WIN='!', all others ':'.
        </label>
        <default>${MAILDIR_FILENAME_SEPARATOR}</default>
      </entry>
    </group>
<!-- General -->

    <group name="Groupware">
      <entry name="GroupwareEnabled" type="Bool">
        <label>Enable groupware functionality</label>
        <whatsthis></whatsthis>
        <default>true</default>
      </entry>

      <entry name="LegacyMangleFromToHeaders" type="Bool">
        <label>Mangle From:/To: headers in replies to replies</label>
        <whatsthis>Microsoft Outlook has a number of shortcomings in its implementation of the iCalendar standard; this option works around one of them. If you have problems with Outlook users not being able to get your replies, try setting this option.</whatsthis>
        <default>false</default>
      </entry>

      <entry name="LegacyBodyInvites" type="Bool">
        <label>Send groupware invitations in the mail body</label>
        <whatsthis>Microsoft Outlook has a number of shortcomings in its implementation of the iCalendar standard; this option works around one of them. If you have problems with Outlook users not being able to get your invitations, try setting this option.</whatsthis>
        <default>false</default>
      </entry>

     <entry name="ExchangeCompatibleInvitations" type="Bool">
        <label>Exchange-compatible invitation naming</label>
        <whatsthis>Microsoft Outlook, when used in combination with a Microsoft Exchange server, has a problem understanding standards-compliant groupware email. Turn this option on to send groupware invitations in a way that Microsoft Exchange understands.</whatsthis>
        <default>false</default>
      </entry>

      <entry name="OutlookCompatibleInvitationReplyComments" type="Bool">
        <label>Outlook compatible invitation reply comments</label>
        <whatsthis>When replying to invitations, send the reply comment in way that Microsoft Outlook understands.</whatsthis>
        <default>false</default>
      </entry>

      <entry name="AutomaticSending" type="Bool">
        <label>Automatic invitation sending</label>
        <whatsthis>When this is checked, you will not see the mail composer window. Instead, all invitation mails are sent automatically. If you want to see the mail before sending it, you can uncheck this option. However, be aware that the text in the composer window is in iCalendar syntax, and you should not try modifying it by hand.</whatsthis>
        <default>true</default>
      </entry>

      <entry name="DeleteInvitationEmailsAfterSendingReply" type="Bool">
        <label>Delete invitation emails after the reply to them has been sent</label>
        <whatsthis>When this is checked, received invitation emails that have been replied to will be moved to the Trash folder, once the reply has been successfully sent.</whatsthis>
        <default>${DELETE_INVITATIONS_AFTER_REPLY_DEFAULT}</default>
      </entry>

      <entry name="AskForCommentWhenReactingToInvitation"  type="Enum">
        <label></label>
        <whatsthis></whatsthis>
        <choices>
          <choice name="NeverAsk"/>
          <choice name="AskForAllButAcceptance"/>
          <choice name="AlwaysAsk"/>
        </choices>
        <default>AskForAllButAcceptance</default>
      </entry>

    </group>

    <group name="IMAP Resource">
      <entry name="TheIMAPResourceEnabled" type="Bool">
        <whatsthis>&lt;p&gt;Enabling this makes it possible to store the entries from the Kontact applications (KOrganizer, KAddressBook, and KNotes.)&lt;/p&gt;&lt;p&gt;If you want to set this option you must also set the applications to use the IMAP resource; this is done in the KDE System Settings.&lt;/p&gt;</whatsthis>
        <default>false</default>
      </entry>

      <entry name="HideGroupwareFolders" type="Bool">
        <whatsthis>&lt;p&gt;Usually you will not have any reason to see the folders that hold the IMAP resources. But if you need to see them, you can set that here.&lt;/p&gt;</whatsthis>
        <default>true</default>
      </entry>

      <entry name="ShowOnlyGroupwareFoldersForGroupwareAccount" type="Bool">
        <default>false</default>
        <whatsthis>&lt;p&gt;If the account used for storing groupware information is not used to manage normal mail, set this option to make KMail only show groupware folders in it. This is useful if you are handling regular mail via an additional online IMAP account.&lt;/p&gt;</whatsthis>
      </entry>

      <entry name="TheIMAPResourceStorageFormat" type="Enum">
        <whatsthis>&lt;p&gt;Choose the storage format of the groupware folders. &lt;ul&gt;&lt;li&gt;The default format is to use the ical (for calendar folders) and vcard (for address book folders) standards. This format makes all Kontact features available.&lt;/li&gt;&lt;li&gt;The Kolab XML format uses a custom model that matches more closely the one used in Outlook. This format gives better Outlook compatibility, when using a Kolab server or a compatible solution.&lt;/li&gt;&lt;/ul&gt;&lt;/p&gt;</whatsthis>
        <choices>
          <choice name="IcalVcard"/>
          <choice name="XML"/>
        </choices>
        <default>IcalVcard</default>
      </entry>

      <entry name="TheIMAPResourceFolderParent" type="String">
        <whatsthis>&lt;p&gt;This chooses the parent of the IMAP resource folders.&lt;/p&gt;&lt;p&gt;By default, the Kolab server sets the IMAP inbox to be the parent.&lt;/p&gt;</whatsthis>
        <default>inbox</default>
      </entry>

      <entry name="TheIMAPResourceAccount" type="Int">
        <whatsthis>&lt;p&gt;This is the ID of the account holding the IMAP resource folders.&lt;/p&gt;</whatsthis>
        <default></default>
      </entry>

      <entry name="TheIMAPResourceFolderLanguage" type="Int">
        <whatsthis>&lt;p&gt;If you want to set the folder names of the IMAP storage to your local language, you can choose between these available languages.&lt;/p&gt;&lt;p&gt; Please note, that the only reason to do so is for compatibility with Microsoft Outlook. It is considered a bad idea to set this, since it makes changing languages impossible. &lt;/p&gt;&lt;p&gt;So do not set this unless you have to.&lt;/p&gt;</whatsthis>
        <default>0</default>
      </entry>

      <entry name="FilterOnlyDIMAPInbox" type="Bool">
        <default>true</default>
        <label>Only filter mails received in disconnected IMAP inbox.</label>
      </entry>
      <entry name="FilterGroupwareFolders" type="Bool">
        <default>false</default>
        <label>Also filter new mails received in groupware folders.</label>
      </entry>

      <entry name="ImmediatlySyncDIMAPOnGroupwareChanges" type="Bool">
        <default>true</default>
        <label>Synchronize groupware changes in DIMAP folders immediately when being online.</label>
      </entry>
    </group>

    <group name="Internal">
      <entry name="MsgDictSizeHint" type="Int" hidden="true">
        <default>9973</default>
      </entry>
      <entry name="PreviousNewFeaturesMD5" type="String" hidden="true">
        <whatsthis>This value is used to decide whether the KMail Introduction should be displayed.</whatsthis>
        <default></default>
      </entry>
    </group>

    <group name="Network">
      <entry name="MaxConnectionsPerHost" type="Int">
        <label>Maximal number of connections per host</label>
        <whatsthis>This can be used to restrict the number of connections per host while checking for new mail. By default the number of connections is unlimited (0).</whatsthis>
        <default>0</default>
        <min>0</min>
      </entry>
    </group>

    <group name="UserInterface">
      <entry name="EnableFolderQuickSearch" type="Bool">
        <label>Show folder quick search line edit</label>
        <default>false</default>
      </entry>
      <entry name="HideLocalInbox" type="Bool">
        <label>Hide local inbox if unused</label>
        <default>true</default>
      </entry>
    </group>

    <group name="Composer">
      <entry name="ForwardingInlineByDefault" type="Bool">
        <default>false</default>
        <label>Forward Inline As Default.</label>
      </entry>
      <entry name="AllowSemicolonAsAddressSeparator" type="Bool">
        <default>${ALLOW_SEMICOLON_AS_ADDRESS_SEPARATOR_DEFAULT}</default>
        <label>Allow the semicolon character (';') to be used as separator in the message composer.</label>
      </entry>
      <entry name="ForceReplyCharset" type="Bool" key="force-reply-charset">
        <label>Keep original charset when replying or forwarding if possible</label>
        <default>false</default>
      </entry>
      <entry name="AutoTextSignature" type="String" key="signature">
        <label>A&amp;utomatically insert signature</label>
        <default>auto</default>
      </entry>
      <entry name="StickyIdentity" type="Bool" key="sticky-identity">
        <whatsthis>Remember this identity, so that it will be used in future composer windows as well.
        </whatsthis>
        <default>false</default>
      </entry>
      <entry name="StickyFcc" type="Bool" key="sticky-fcc">
        <whatsthis>Remember this folder for sent items, so that it will be used in future composer windows as well.</whatsthis>
        <default>false</default>
      </entry>
      <entry name="StickyTransport" type="Bool" key="sticky-transport">
        <whatsthis>Remember this mail transport, so that it will be used in future composer windows as well.</whatsthis>
        <default>false</default>
      </entry>
      <entry name="WordWrap" type="Bool" key="word-wrap">
        <label>Word &amp;wrap at column:</label>
        <default>true</default>
      </entry>
      <entry name="UseFixedFont" type="Bool" key="use-fixed-font">
        <label>Use Fi&amp;xed Font</label>
        <default>false</default>
      </entry>
      <entry name="LineWrapWidth" type="Int" key="break-at">
        <label></label>
        <default>78</default>
        <min>30</min>
        <max>998</max>
      </entry>
       <entry name="TooManyRecipients" type="Bool" key="too-many-recipients">
        <label>Warn if the number of recipients is larger than</label>
        <default>${WARN_TOOMANY_RECIPIENTS_DEFAULT}</default>
        <whatsthis>If the number of recipients is larger than this value, KMail will warn and ask for a confirmation before sending the mail. The warning can be turned off.</whatsthis>
      </entry>
      <entry name="RecipientThreshold" type="Int" key="recipient-threshold">
        <label></label>
        <default>5</default>
        <min>1</min>
        <max>100</max>
        <whatsthis>If the number of recipients is larger than this value, KMail will warn and ask for a confirmation before sending the mail. The warning can be turned off.</whatsthis>
     </entry>
     <entry name="PreviousIdentity" type="UInt" key="previous-identity" />
      <entry name="PreviousFcc" type="String" key="previous-fcc" />
      <entry name="CurrentTransport" type="String" key="current-transport" />
      <entry name="MaxTransportEntries" type="Int" key="max-transport-items">
        <default>10</default>
      </entry>
      <entry name="OutlookCompatibleAttachments" type="Bool" key="outlook-compatible-attachments">
        <label>Outlook-compatible attachment naming</label>
        <whatsthis>Turn this option on to make Outlook &#8482; understand attachment names containing non-English characters</whatsthis>
        <default>false</default>
      </entry>
      <entry name="UseHtmlMarkup" type="Bool" key="html-markup">
        <default>false</default>
      </entry>
      <entry name="PgpAutoSign" type="Bool" key="pgp-auto-sign">
        <default>false</default>
      </entry>
      <entry name="PgpAutoEncrypt" type="Bool" key="pgp-auto-encrypt">
        <default>false</default>
      </entry>
      <entry name="NeverEncryptDrafts" type="Bool" key="never-encrypt-drafts">
        <default>true</default>
      </entry>

      <entry name="ChiasmusKey" type="String" key="chiasmus-key">
      </entry>
      <entry name="ChiasmusOptions" type="String" key="chiasmus-options">
      </entry>

      <entry name="ConfirmBeforeSend" type="Bool" key="confirm-before-send">
        <label>Confirm &amp;before send</label>
        <default>false</default>
      </entry>
      <entry name="RequestMDN" type="Bool" key="request-mdn">
        <label>Automatically request &amp;message disposition notifications</label>
        <whatsthis>&lt;qt&gt;&lt;p&gt;Enable this option if you want KMail to request Message Disposition Notifications (MDNs) for each of your outgoing messages.&lt;/p&gt;&lt;p&gt;This option only affects the default; you can still enable or disable MDN requesting on a per-message basis in the composer, menu item &lt;em&gt;Options&lt;/em&gt;-&gt;&lt;em&gt;Request Disposition Notification&lt;/em&gt;.&lt;/p&gt;&lt;/qt&gt;</whatsthis>
        <default>false</default>
      </entry>
      <entry name="ShowRecentAddressesInComposer" type="Bool" key="showRecentAddressesInComposer">
        <label>Use recent addresses for autocompletion</label>
        <whatsthis>Disable this option if you do not want recently used addresses to appear in the autocompletion list in the composer's address fields.</whatsthis>
        <default>true</default>
      </entry>
      <entry name="Headers" type="Int" key="headers">
			  <default>HDR_STANDARD</default>
      </entry>
      <entry name="CompletionMode" type="Int" key="Completion Mode">
        <default code="true">KGlobalSettings::completionMode()</default>
      </entry>
      <entry name="AutoSpellChecking" type="Bool" key="autoSpellChecking">
        <default>true</default>
      </entry>
      <entry name="ShowForgottenAttachmentWarning" type="Bool" key="showForgottenAttachmentWarning">
        <default>true</default>
      </entry>
      <entry name="AttachmentKeywords" type="StringList" key="attachment-keywords">
        <default code="true">
          KMail::Util::AttachmentKeywords()
        </default>
      </entry>
      <entry name="ShowMessagePartDialogOnAttach" type="Bool" key="showMessagePartDialogOnAttach">
        <default>false</default>
      </entry>
      <entry name="AutosaveInterval" type="Int" key="autosave">
        <label>Autosave interval:</label>
        <whatsthis>A backup copy of the text in the composer window can be created regularly. The interval used to create the backups is set here. You can disable autosaving by setting it to the value 0.</whatsthis>
        <default>2</default>
      </entry>
      <entry name="PrependSignature" type="Bool" key="prepend-signature">
        <label>Insert signature above quoted text</label>
        <default>false</default>
      </entry>
      <entry name="DashDashSignature" type="Bool" key="dash-dash-signature">
        <label>Prepend separator to signature</label>
        <default>true</default>
      </entry>
      <entry name="ReplyPrefixes" type="StringList" key="reply-prefixes">
        <default>Re\\s*:,Re\\[\\d+\\]:,Re\\d+:</default>
      </entry>
      <entry name="ReplaceReplyPrefix" type="Bool" key="replace-reply-prefix">
        <label>Replace recognized prefi&amp;x with "Re:"</label>
	<default>true</default>
      </entry>
      <entry name="ForwardPrefixes" type="StringList" key="forward-prefixes">
        <default>Fwd:,FW:</default>
      </entry>
      <entry name="ReplaceForwardPrefix" type="Bool" key="replace-forward-prefix">
        <label>Replace recognized prefix with "&amp;Fwd:"</label>
        <default>true</default>
      </entry>
      <entry name="SmartQuote" type="Bool" key="smart-quote">
        <label>Use smart &amp;quoting</label>
        <default>true</default>
      </entry>

      <entry name="SecondRecipientTypeDefault" type="Enum">
        <choices>
          <choice name="To"/>
          <choice name="Cc"/>
        </choices>
        <default>To</default>
      </entry>
      <entry name="MaximumRecipients" type="Int">
        <label>Maximum number of recipient editor lines.</label>
        <default>200</default>
      </entry>
      <entry name="CustomTemplates" type="StringList" key="CustomTemplates" />

      <entry name="MimetypesToStripWhenInlineForwarding" type="StringList">
          <label>List of message part types to strip off mails that are being forwarded inline.</label>
      </entry>

      <entry name="ShowSnippetManager" type="Bool">
          <label>Show the Text Snippet Management and Insertion Panel in the composer.</label>
          <default>false</default>
      </entry>
      <entry name="SnippetSplitterPosition" type="IntList"/>

      <entry name="MaximumAttachmentSize" type="Int">
          <label>The maximum size in MB that email attachments are allowed to have (-1 for no limit).</label>
          <default>-1</default>
      </entry>

      <entry name="ShowGnuPGAuditLogAfterSuccessfulSignEncrypt" type="Bool">
          <label>Show the GnuPG Audit Log even after crypto operations that completed successfully.</label>
          <default>false</default>
      </entry>

    </group>
<!-- Composer -->

    <group name="Fonts">
      <entry name="UseDefaultFonts" type="Bool" key="defaultFonts">
        <default>true</default>
      </entry>
      <entry name="ComposerFont" type="Font" key="composer-font">
        <default code="true">KGlobalSettings::generalFont()</default>
      </entry>
      <entry name="FixedFont" type="Font" key="fixed-font">
        <default code="true">KGlobalSettings::fixedFont()</default>
      </entry>

    </group>

    <group name="Geometry">
      <entry name="ComposerSize" type="Size" key="composer">
        <default>QSize(480,510)</default>
      </entry>
      <entry name="FolderViewWidth" type="Int">
        <default>250</default>
      </entry>
      <entry name="FolderViewHeight" type="Int">
        <default>500</default>
      </entry>
      <entry name="FolderTreeHeight" type="Int">
        <default>400</default>
      </entry>
      <entry name="SearchAndHeaderHeight" type="Int">
        <default>180</default>
      </entry>
      <entry name="SearchAndHeaderWidth" type="Int">
        <default>450</default>
      </entry>
      <entry name="ReaderWindowWidth" type="Int">
        <default>200</default>
      </entry>
      <entry name="ReaderWindowHeight" type="Int">
        <default>320</default>
      </entry>

      <entry name="readerWindowMode" type="Enum">
       <label>Message Preview Pane</label>
       <choices>
         <choice name="hide">
           <label>Do not show a message preview pane</label>
         </choice>
         <choice name="below">
           <label>Show the message preview pane below the message list</label>
         </choice>
         <choice name="right">
           <label>Show the message preview pane next to the message list</label>
         </choice>
       </choices>
       <default>below</default>
      </entry>

      <entry name="FolderList" type="Enum">
       <label>Folder List</label>
       <choices>
         <choice name="longlist">
           <label>Long folder list</label>
         </choice>
         <choice name="shortlist">
           <label>Short folder list</label>
         </choice>
       </choices>
       <default>longlist</default>
      </entry>
    </group>

    <group name="Reader">
     <entry name="MimePaneHeight" type="Int">
      <default>100</default>
     </entry>
     <entry name="MessagePaneHeight" type="Int">
      <default>180</default>
     </entry>
      <entry name="headerStyle" type="String" key="header-style">
        <label>What style of headers should be displayed</label>
        <default>fancy</default>
      </entry>
      <entry name="headerSetDisplayed" type="String" key="header-set-displayed">
        <label>How much of headers should be displayed</label>
        <default>rich</default>
      </entry>
     <entry name="UseDefaultColors" type="Bool" key="defaultColors">
       <default>true</default>
     </entry>
     <entry name="FallbackCharacterEncoding" type="String" key="FallbackCharacterEncoding">
       <default code="true"> QString( QTextCodec::codecForLocale()->name() ).toLower() == "eucjp" ? QString("jis7") : QString( QTextCodec::codecForLocale()->name() ).toLower()</default>
       <label></label>
       <whatsthis>Some emails, especially those generated automatically, do not specify the character encoding which needs to be used to properly display them. In such cases a fallback character encoding will be used, which you can configure here. Set it to the character encoding most commonly used in your part of the world. As a default the encoding configured for the whole system is used.</whatsthis>
     </entry>

     <entry name="OverrideCharacterEncoding" type="String" key="encoding">
       <default></default>
       <label></label>
       <whatsthis>Changing this from its default 'Auto' will force the use of the specified encoding for all emails, regardless of what they specify themselves.</whatsthis>
     </entry>

     <entry name="ShowEmoticons" type="Bool">
       <default>true</default>
       <label>Replace smileys by emoticons</label>
       <whatsthis>Enable this if you want smileys like :-) appearing in the message text to be replaced by emoticons (small pictures).</whatsthis>
     </entry>
     <entry name="ShowExpandQuotesMark" type="Bool">
       <default>false</default>
       <label>Show expand/collapse quote marks</label>
       <whatsthis>Enable this option to show different levels of quoted text. Disable to hide the levels of quoted text.</whatsthis>
     </entry>
      <entry name="CollapseQuoteLevelSpin" type="Int" >
        <label>Automatic collapse level:</label>
        <default>3</default>
        <min>0</min>
        <max>10</max>
      </entry>

      <entry name="ShrinkQuotes" type="Bool">
        <default>false</default>
        <label>Reduce font size for quoted text</label>
        <whatsthis>Enable this option to show quoted text with a smaller font.</whatsthis>
     </entry>

     <entry name="ChiasmusDecryptionKey" type="String">
     </entry>

     <entry name="ChiasmusDecryptionOptions" type="String">
     </entry>

     <entry name="ShowUserAgent" type="Bool">
       <default>false</default>
       <label>Show user agent in fancy headers</label>
       <whatsthis>Enable this option to get the User-Agent and X-Mailer header lines displayed when using fancy headers.</whatsthis>
     </entry>

     <entry name="AllowAttachmentDeletion" type="Bool">
       <default>true</default>
       <label>Allow to delete attachments of existing mails.</label>
     </entry>
     <entry name="AllowAttachmentEditing" type="Bool">
       <default>false</default>
       <label>Allow to edit attachments of existing mails.</label>
     </entry>

     <entry name="AlwaysDecrypt" type="Bool">
      <default>false</default>
      <label>Always decrypt messages when viewing or ask before decrypting</label>
     </entry>

     <entry name="MimeTreeLocation" type="Enum">
       <label>Message Structure Viewer Placement</label>
       <choices>
         <choice name="top">
           <label>Above the message pane</label>
         </choice>
         <choice name="bottom">
           <label>Below the message pane</label>
         </choice>
       </choices>
       <default>bottom</default>
     </entry>

     <entry name="MimeTreeMode" type="Enum">
      <label>Message Structure Viewer</label>
      <choices>
        <choice name="Never">
          <label>Show never</label>
        </choice>
        <choice name="Always">
          <label>Show always</label>
        </choice>
      </choices>
      <default>Never</default>
     </entry>

     <entry name="showColorbar" type="Bool">
       <label>Show HTML status bar</label>
       <default>false</default>
     </entry>

     <entry name="showSpamStatus" type="Bool">
       <label>Show spam status in fancy headers</label>
       <default>true</default>
     </entry>

     <entry name="numberOfAddressesToShow" type="Int">
       <label>Number of addresses to show before collapsing</label>
       <default>4</default>
       <min>1</min>
     </entry>

      <entry name="htmlMail" type="Bool">
        <label>Prefer HTML to plain text</label>
        <default>false</default>
      </entry>
      <entry name="htmlLoadExternal" type="Bool">
        <label>Allow messages to load external references from the Internet</label>
        <default>false</default>
      </entry>

      <entry name="attachmentStrategy" type="String" key="attachment-strategy">
        <label>How attachments are shown</label>
        <default>Smart</default>
      </entry>
    </group>

    <group name="TextIndex">
      <entry name="automaticDecrypt" type="Bool" key="automaticDecrypt">
        <default>true</default>
	<label></label>
      </entry>
    </group>

    <group name="MDN">
      <entry name="notSendWhenEncrypted" type="Bool" key="not-send-when-encrypted">
        <label>Do not send MDNs in response to encrypted messages</label>
        <default>true</default>
      </entry>
      <entry name="SendMDNsWithEmptySender" type="Bool">
        <default>false</default>
        <label>Send Message Disposition Notifications with an empty sender.</label>
        <whatsthis>Send Message Disposition Notifications with an empty sender string. Some servers might be configure to reject such messages, so if you are experiencing problems sending MDNs, uncheck this option.</whatsthis>
      </entry>
    </group>

  <group name="GlobalTemplates">
    <entry name="TemplateNewMessage" type="String" key="TemplateNewMessage">
      <label>Message template for new message</label>
      <whatsthis></whatsthis>
      <default code="true">TemplatesConfiguration::defaultNewMessage()</default>
    </entry>
    <entry name="TemplateReply" type="String" key="TemplateReply">
      <label>Message template for reply</label>
      <whatsthis></whatsthis>
      <default code="true">TemplatesConfiguration::defaultReply()</default>
    </entry>
    <entry name="TemplateReplyAll" type="String" key="TemplateReplyAll">
      <label>Message template for reply to all</label>
      <whatsthis></whatsthis>
      <default code="true">TemplatesConfiguration::defaultReplyAll()</default>
    </entry>
    <entry name="TemplateForward" type="String" key="TemplateForward">
      <label>Message template for forward</label>
      <whatsthis></whatsthis>
      <default code="true">TemplatesConfiguration::defaultForward()</default>
    </entry>
    <entry name="QuoteString" type="String" key="QuoteString">
      <label>Quote characters</label>
      <whatsthis></whatsthis>
      <default code="true">TemplatesConfiguration::defaultQuoteString()</default>
    </entry>
  </group>

    <group name="OutOfOffice">
        <entry name="AllowOutOfOfficeSettings" type="Bool">
            <default>true</default>
            <label>Allow out-of-office settings to be changeable by the user.</label>
        </entry>
        <entry name="AllowOutOfOfficeUploadButNoSettings" type="Bool">
            <default>false</default>
            <label>Allow users to upload out-of-office sieve scripts, but prevent them from changing any settings, such as the domain to react to or the spam reaction switch.</label>
        </entry>
        <entry name="OutOfOfficeDomain" type="String">
            <default></default>
            <label>Send out-of-office replies to mails coming from this domain only.</label>
        </entry>
        <entry name="OutOfOfficeReactToSpam" type="Bool">
            <default>false</default>
            <label>Allow out-of-office replies to be sent to messages marked as SPAM.</label>
        </entry>
        <entry name="CheckOutOfOfficeOnStartup" type="Bool">
              <default>true</default>
              <label>Check if there is still an active out-of-office reply configured when starting KMail.</label>
        </entry>
    </group>

  <group name="FavoriteFolderView">
    <entry name="EnableFavoriteFolderView" type="Bool">
      <default>true</default>
    </entry>
    <entry name="FavoriteFolderViewHeight" type="Int">
      <default>100</default>
    </entry>
    <entry name="FavoriteFolderIds" type="IntList"/>
    <entry name="FavoriteFolderNames" type="StringList"/>
    <entry name="FavoriteFolderViewSeenInboxes" type="IntList"/>
  </group>


  <group name="MessageListView">
    <entry name="DisplayMessageToolTips" type="Bool">
      <default>true</default>
      <label>Display tooltips for messages and group headers</label>
      <whatsthis>Enable this option to display tooltips when hovering over an item in the message list.</whatsthis>
    </entry>
    <entry name="HideTabBarWithSingleTab" type="Bool">
      <default>false</default>
      <label>Hide tab bar when only one tab is open</label>
      <whatsthis>With this option enabled the tab bar will be displayed only when there are two or more tabs. With this option disabled the tab bar will be always shown. When the tab bar is hidden you can always open a folder in a new tab by middle-clicking it.</whatsthis>
    </entry>
  </group>

</kcfg>
