#include "ui_configurationdialogadd.h"
#include "configurationdialogadd.h"

#include <kdebug.h>
#include <QErrorMessage>
#include <QTimer>


namespace feedsync
{

ConfigurationDialogAdd::ConfigurationDialogAdd( QWidget *parent)
{
    kDebug();

   // UI setup
    QWidget *widget = new QWidget( this );
    ui = new Ui::ConfigurationDialogAdd();
    ui->setupUi(widget);
    setMainWidget( widget );

    setCaption( i18n("Online reader Add") );

    QStringList m_AggregatorType;
        m_AggregatorType.append( i18n("GoogleReader") );
        m_AggregatorType.append( i18n("Opml") );
    ui->cb_AggregatorType->addItems(m_AggregatorType);

    connect( this, SIGNAL( finished() ), this, SLOT( slotFinished() ) );

    QTimer::singleShot( 0, this, SLOT(slotDelayedInit()) );
}

ConfigurationDialogAdd::~ConfigurationDialogAdd()
{
    kDebug();
    delete ui;
}

void ConfigurationDialogAdd::check()
{
    kDebug();
    kDebug() << ui->cb_AggregatorType->itemText( ui->cb_AggregatorType->currentIndex() ) ;
    kDebug() << ui->le_login->text();
    kDebug() << ui->le_passwd->text();
    kDebug() << ui->le_filename->text();

    if (ui->cb_AggregatorType->itemText( ui->cb_AggregatorType->currentIndex() ) == "GoogleReader") {

        if (ui->le_login->text()=="") {
            return;
        } else if (ui->le_passwd->text()=="") {
            return;
        } else {
            // Remove old
            if (_baseconfigname != "") {
                KConfig config("akregator_feedsyncrc");
                config.deleteGroup(_baseconfigname);
            }
            // Insert new
            KConfig config("akregator_feedsyncrc");
            KConfigGroup generalGroup( &config, "FeedSyncSource_GoogleReader" + ui->le_login->text() );
            generalGroup.writeEntry( "AggregatorType", ui->cb_AggregatorType->itemText( ui->cb_AggregatorType->currentIndex() ) );
            generalGroup.writeEntry( "Login", ui->le_login->text() );
            generalGroup.writeEntry( "Password", ui->le_passwd->text() );
            generalGroup.writeEntry( "Identifier", ui->le_login->text() );
            generalGroup.config()->sync();
            // Valid
            accept();
        }

    } else if (ui->cb_AggregatorType->itemText( ui->cb_AggregatorType->currentIndex() ) == "Opml") {

        if (ui->le_filename->text()=="") {
            return;
        } else {
            KConfig config("akregator_feedsyncrc");
            KConfigGroup generalGroup( &config, "FeedSyncSource_Opml" + ui->le_filename->text() );
            generalGroup.writeEntry( "AggregatorType", ui->cb_AggregatorType->itemText( ui->cb_AggregatorType->currentIndex() ) );
            generalGroup.writeEntry( "Filename", ui->le_filename->text() );
            generalGroup.writeEntry( "Identifier", ui->le_filename->text() );
            generalGroup.config()->sync();
            // Valid
            accept();
        }

    } else {
    }
}

void ConfigurationDialogAdd::load( const KConfigGroup group )
{
    kDebug();
    _baseconfigname = group.name();
    if ( group.readEntry( "AggregatorType", QString() ) == "GoogleReader") {
        ui->le_login->setText( group.readEntry( "Login", QString() ) );
        ui->le_passwd->setText( group.readEntry( "Password", QString() ) );
    } else {
    }
}

// SLOT

void ConfigurationDialogAdd::slotDelayedInit()
{
    kDebug();
 }

void ConfigurationDialogAdd::slotButtonClicked(int button) {
    if (button == KDialog::Ok) {
        check();
    } else {
        KDialog::slotButtonClicked(button);
    }
}

void ConfigurationDialogAdd::slotFinished()
{
    kDebug();

    deleteLater();
}

}
