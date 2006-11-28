/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "q2dviewerextension.h"

#include "volume.h"
#include "logging.h"
#include "qwindowlevelcombobox.h"
#include "toolsactionfactory.h"
#include <QAction>
#include <QSettings>
// VTK
#include <vtkRenderer.h>
#include "slicing2dtool.h"

namespace udg {

Q2DViewerExtension::Q2DViewerExtension( QWidget *parent )
 : QWidget( parent )
{
    setupUi( this );
    m_mainVolume = 0;
    m_secondaryVolume = 0;

    readSettings();
    createActions();
    createConnections();
    changeViewToSingle();
}

Q2DViewerExtension::~Q2DViewerExtension()
{
    writeSettings();
}

void Q2DViewerExtension::createActions()
{
    m_axialViewAction = new QAction( 0 );
    m_axialViewAction->setText( tr("&Axial View") );
    m_axialViewAction->setShortcut( tr("Ctrl+A") );
    m_axialViewAction->setStatusTip( tr("Change Current View To Axial") );
    m_axialViewAction->setIcon( QIcon(":/images/axial.png") );
    m_axialViewToolButton->setDefaultAction( m_axialViewAction );

    m_sagitalViewAction = new QAction( 0 );
    m_sagitalViewAction->setText( tr("&Sagital View") );
    m_sagitalViewAction->setShortcut( tr("Ctrl+S") );
    m_sagitalViewAction->setStatusTip( tr("Change Current View To Sagital") );
    m_sagitalViewAction->setIcon( QIcon(":/images/sagital.png") );
    m_sagitalViewToolButton->setDefaultAction( m_sagitalViewAction );

    m_coronalViewAction = new QAction( 0 );
    m_coronalViewAction->setText( tr("&Coronal View") );
    m_coronalViewAction->setShortcut( tr("Ctrl+C") );
    m_coronalViewAction->setStatusTip( tr("Change Current View To Coronal") );
    m_coronalViewAction->setIcon( QIcon(":/images/coronal.png") );
    m_coronalViewToolButton->setDefaultAction( m_coronalViewAction );

    m_singleViewAction = new QAction( 0 );
    m_singleViewAction->setText( tr("&Single View") );
    m_singleViewAction->setShortcut( tr("Ctrl+S") );
    m_singleViewAction->setStatusTip( tr("Change To Single View Mode") );
    m_singleViewAction->setIcon( QIcon(":/images/singleView.png") );
    m_singleViewAction->setCheckable( true );
    m_singleViewToolButton->setDefaultAction( m_singleViewAction );

    m_doubleViewAction = new QAction( 0 );
    m_doubleViewAction->setText( tr("&Double View") );
    m_doubleViewAction->setShortcut( tr("Ctrl+D") );
    m_doubleViewAction->setStatusTip( tr("Change To Double View Mode") );
    m_doubleViewAction->setIcon( QIcon(":/images/addViewRight.png") );
    m_doubleViewAction->setCheckable( true );
    m_doubleViewToolButton->setDefaultAction( m_doubleViewAction );

    //afegim un action group pel switcher de # de vistes
    QActionGroup *viewActionGroup = new QActionGroup( 0 );
    viewActionGroup->setExclusive( true );
    viewActionGroup->addAction( m_singleViewAction );
    viewActionGroup->addAction( m_doubleViewAction );
    m_singleViewAction->setChecked( true );

    // Pseudo-tool \TODO ara mateix no ho integrem dins del framework de tools, però potser que més endavant sí
    m_voxelInformationAction = new QAction( 0 );
    m_voxelInformationAction->setText( tr("Voxel Information") );
    m_voxelInformationAction->setShortcut( tr("Ctrl+I") );
    m_voxelInformationAction->setStatusTip( tr("Enable voxel information over cursor") );
    m_voxelInformationAction->setIcon( QIcon(":/images/voxelInformation.png") );
    m_voxelInformationAction->setCheckable( true );
    m_voxelInformationToolButton->setDefaultAction( m_voxelInformationAction );

    connect( m_voxelInformationAction , SIGNAL( triggered(bool) ) , m_2DView2_1 , SLOT( setVoxelInformationCaptionEnabled(bool) ) );
    connect( m_voxelInformationAction , SIGNAL( triggered(bool) ) , m_2DView2_2 , SLOT( setVoxelInformationCaptionEnabled(bool) ) );

    m_rotateClockWiseAction = new QAction( 0 );
    m_rotateClockWiseAction->setText( tr("Rotate Clockwise") );
    m_rotateClockWiseAction->setShortcut( Qt::CTRL + Qt::Key_Plus );
    m_rotateClockWiseAction->setStatusTip( tr("Rotate the image in clockwise direction") );
    m_rotateClockWiseAction->setIcon( QIcon(":/images/rotateClockWise.png") );
    m_rotateClockWiseToolButton->setDefaultAction( m_rotateClockWiseAction );

    connect( m_rotateClockWiseAction , SIGNAL( triggered() ) , m_2DView2_1 , SLOT( rotateClockWise() ) );
    connect( m_rotateClockWiseAction , SIGNAL( triggered() ) , m_2DView2_2 , SLOT( rotateClockWise() ) );

    m_rotateCounterClockWiseAction = new QAction( 0 );
    m_rotateCounterClockWiseAction->setText( tr("Rotate Counter Clockwise") );
    m_rotateCounterClockWiseAction->setShortcut( Qt::CTRL + Qt::Key_Minus );
    m_rotateCounterClockWiseAction->setStatusTip( tr("Rotate the image in counter clockwise direction") );
    m_rotateCounterClockWiseAction->setIcon( QIcon(":/images/rotateCounterClockWise.png") );
    m_rotateCounterClockWiseToolButton->setDefaultAction( m_rotateCounterClockWiseAction );

    connect( m_rotateCounterClockWiseAction , SIGNAL( triggered() ) , m_2DView2_1 , SLOT( rotateCounterClockWise() ) );
    connect( m_rotateCounterClockWiseAction , SIGNAL( triggered() ) , m_2DView2_2 , SLOT( rotateCounterClockWise() ) );

    // Tools
    m_actionFactory = new ToolsActionFactory( 0 );
    m_slicingAction = m_actionFactory->getActionFrom( "SlicingTool" );
    m_slicingToolButton->setDefaultAction( m_slicingAction );

    m_windowLevelAction = m_actionFactory->getActionFrom( "WindowLevelTool" );
    m_windowLevelToolButton->setDefaultAction( m_windowLevelAction );

    m_zoomAction = m_actionFactory->getActionFrom( "ZoomTool" );
    m_zoomToolButton->setDefaultAction( m_zoomAction );

    m_moveAction = m_actionFactory->getActionFrom( "TranslateTool" );
    m_moveToolButton->setDefaultAction( m_moveAction );

    m_screenShotAction = m_actionFactory->getActionFrom( "ScreenShotTool" );
    m_screenShotToolButton->setDefaultAction( m_screenShotAction );

    connect( m_actionFactory , SIGNAL( triggeredTool(QString) ) , m_2DView2_1, SLOT( setTool(QString) ) );
    connect( m_actionFactory , SIGNAL( triggeredTool(QString) ) , m_2DView2_2 , SLOT( setTool(QString) ) );

    m_toolsActionGroup = new QActionGroup( 0 );
    m_toolsActionGroup->setExclusive( true );
    m_toolsActionGroup->addAction( m_slicingAction );
    m_toolsActionGroup->addAction( m_windowLevelAction );
    m_toolsActionGroup->addAction( m_zoomAction );
    m_toolsActionGroup->addAction( m_moveAction );
    m_toolsActionGroup->addAction( m_screenShotAction );
    //activem per defecte una tool. \TODO podríem posar algun mecanisme especial per escollir la tool per defecte?
    m_slicingAction->trigger();
}

void Q2DViewerExtension::createConnections()
{
    // adicionals, \TODO ara es fa "a saco" però s'ha de millorar
    connect( m_slider2_1 , SIGNAL( valueChanged(int) ) , m_spinBox2_1 , SLOT( setValue(int) ) );
    connect( m_spinBox2_1 , SIGNAL( valueChanged(int) ) , m_2DView2_1 , SLOT( setSlice(int) ) );
    connect( m_2DView2_1 , SIGNAL( sliceChanged(int) ) , m_slider2_1 , SLOT( setValue(int) ) );

    connect( m_slider2_2 , SIGNAL( valueChanged(int) ) , m_spinBox2_2 , SLOT( setValue(int) ) );
    connect( m_spinBox2_2 , SIGNAL( valueChanged(int) ) , m_2DView2_2 , SLOT( setSlice(int) ) );
    connect( m_2DView2_2 , SIGNAL( sliceChanged(int) ) , m_slider2_2 , SLOT( setValue(int) ) );

    // sincronisme window level
    connect( m_2DView2_1 , SIGNAL( windowLevelChanged( double , double ) ) , m_2DView2_2 , SLOT( setWindowLevel( double , double ) ) );
    connect( m_2DView2_2 , SIGNAL( windowLevelChanged( double , double ) ) , m_2DView2_1 , SLOT( setWindowLevel( double , double ) ) );

    connect( m_axialViewAction , SIGNAL( triggered() ) , this , SLOT( changeViewToAxial() ) );
    connect( m_sagitalViewAction , SIGNAL( triggered() ) , this , SLOT( changeViewToSagital() ) );
    connect( m_coronalViewAction , SIGNAL( triggered() ) , this , SLOT( changeViewToCoronal() ) );

    connect( m_singleViewAction , SIGNAL( triggered() ) , this , SLOT( changeViewToSingle() ) );
    connect( m_doubleViewAction , SIGNAL( triggered() ) , this , SLOT( changeViewToDouble() ) );

    connect( m_synchroCheckBox , SIGNAL( clicked(bool) ) , this , SLOT( synchronizeSlices(bool) ) );

    connect( m_chooseSeriePushButton , SIGNAL( clicked() ) , this , SLOT( chooseNewSerie() ) );

    // window level combo box
    connect( m_windowLevelComboBox , SIGNAL( windowLevel(double,double) ) , m_2DView2_1 , SLOT( setWindowLevel(double,double) ) );
    connect( m_windowLevelComboBox , SIGNAL( windowLevel(double,double) ) , m_2DView2_2 , SLOT( setWindowLevel(double,double) ) );

    connect( m_windowLevelComboBox , SIGNAL( defaultValue() ) , m_2DView2_1 , SLOT( resetWindowLevelToDefault() ) );
    connect( m_windowLevelComboBox , SIGNAL( defaultValue() ) , m_2DView2_2 , SLOT( resetWindowLevelToDefault() ) );
}

void Q2DViewerExtension::setInput( Volume *input )
{
    m_mainVolume = input;
    // \TODO ara ho fem "a saco" però s'hauria de millorar
    m_2DView2_1->setInput( m_mainVolume );
    m_2DView2_2->setInput( m_mainVolume );
    double wl[2];
    m_2DView2_1->getDefaultWindowLevel( wl );
    m_windowLevelComboBox->updateWindowLevel( wl[0] , wl[1] );
    INFO_LOG("Q2DViewerExtension: Donem l'input principal")
    changeViewToAxial();
}

void Q2DViewerExtension::setSecondInput( Volume *input )
{
    m_secondaryVolume = input;
    // \TODO ara ho fem "a saco" però s'hauria de millorar
    m_2DView2_2->setInput( m_secondaryVolume );
    INFO_LOG("Afegim un segon volum per comparar")
    changeViewToAxial();
    changeViewToDouble();
}

void Q2DViewerExtension::changeViewToAxial()
{
    m_currentView = Axial;
    int extent[6];
    m_mainVolume->getWholeExtent( extent );
    int secondExtent[6];
    if( m_secondaryVolume )
        m_secondaryVolume->getWholeExtent( secondExtent );
    else
        m_mainVolume->getWholeExtent( secondExtent );

    m_spinBox2_1->setMinimum( extent[4] );
    m_spinBox2_1->setMaximum( extent[5] );
    m_slider2_1->setMaximum( extent[5] );
    m_viewText2_1->setText( tr("XY : Axial") );
    m_2DView2_1->setViewToAxial();
    INFO_LOG("Visor per defecte: Canviem a vista axial (Vista 2.1)")
    m_2DView2_1->render();

    m_spinBox2_2->setMinimum( secondExtent[4] );
    m_spinBox2_2->setMaximum( secondExtent[5] );
    m_slider2_2->setMaximum( secondExtent[5] );
    m_viewText2_2->setText( tr("XY : Axial") );
    m_2DView2_2->setViewToAxial();
    INFO_LOG("Visor per defecte: Canviem a vista axial (Vista 2.2)")
    m_2DView2_2->render();
}

void Q2DViewerExtension::changeViewToSagital()
{
    m_currentView = Sagital;
    int extent[6];
    m_mainVolume->getWholeExtent( extent );
    int secondExtent[6];
    if( m_secondaryVolume )
        m_secondaryVolume->getWholeExtent( secondExtent );
    else
        m_mainVolume->getWholeExtent( secondExtent );

    m_spinBox2_1->setMinimum( extent[0] );
    m_spinBox2_1->setMaximum( extent[1] );
    m_slider2_1->setMaximum( extent[1] );
    m_viewText2_1->setText( tr("YZ : Sagital") );
    m_2DView2_1->setViewToSagittal();
    INFO_LOG("Visor per defecte: Canviem a vista sagital (Vista 2.1)")
    m_2DView2_1->render();

    m_spinBox2_2->setMinimum( secondExtent[0] );
    m_spinBox2_2->setMaximum( secondExtent[1] );
    m_slider2_2->setMaximum( secondExtent[1] );
    m_viewText2_2->setText( tr("YZ : Sagital") );
    m_2DView2_2->setViewToSagittal();
    INFO_LOG("Visor per defecte: Canviem a vista sagital (Vista 2.2)")
    m_2DView2_2->render();
}

void Q2DViewerExtension::changeViewToCoronal()
{
    m_currentView = Coronal;
    int extent[6];
    m_mainVolume->getWholeExtent( extent );
    int secondExtent[6];
    if( m_secondaryVolume )
        m_secondaryVolume->getWholeExtent( secondExtent );
    else
        m_mainVolume->getWholeExtent( secondExtent );

    m_spinBox2_1->setMinimum( extent[2] );
    m_spinBox2_1->setMaximum( extent[3] );
    m_slider2_1->setMaximum( extent[3] );
    m_viewText2_1->setText( tr("XZ : Coronal") );
    m_2DView2_1->setViewToCoronal();
    INFO_LOG("Visor per defecte: Canviem a vista coronal (Vista 2.1)")
    m_2DView2_1->render();

    m_spinBox2_2->setMinimum( secondExtent[2] );
    m_spinBox2_2->setMaximum( secondExtent[3] );
    m_slider2_2->setMaximum( secondExtent[3] );
    m_viewText2_2->setText( tr("XZ : Coronal") );
    m_2DView2_2->setViewToCoronal();
    INFO_LOG("Visor per defecte: Canviem a vista coronal (Vista 2.2)")
    m_2DView2_2->render();
}

void Q2DViewerExtension::setView( ViewType view )
{
    switch( view )
    {
    case Axial:
        changeViewToAxial();
    break;
    case Sagital:
        changeViewToSagital();
    break;
    case Coronal:
        changeViewToCoronal();
    break;
    }
}

void Q2DViewerExtension::changeViewToSingle()
{
    m_splitter->widget( 1 )->hide();
}

void Q2DViewerExtension::changeViewToDouble()
{
    m_splitter->widget( 1 )->show();
}

void Q2DViewerExtension::pageChange( int index )
{
    setView( m_currentView );
}

void Q2DViewerExtension::synchronizeSlices( bool ok )
{
    if( ok )
    {
        INFO_LOG("Visor per defecte: Sincronitzem llesques");
        connect( m_slider2_1 , SIGNAL( valueChanged(int) ) , m_slider2_2 , SLOT( setValue(int) ) );
        connect( m_slider2_2 , SIGNAL( valueChanged(int) ) , m_slider2_1 , SLOT( setValue(int) ) );
    }
    else
    {
        INFO_LOG("Visor per defecte: Desincronitzem llesques");
        disconnect( m_slider2_1 , SIGNAL( valueChanged(int) ) , m_slider2_2 , SLOT( setValue(int) ) );
        disconnect( m_slider2_2 , SIGNAL( valueChanged(int) ) , m_slider2_1 , SLOT( setValue(int) ) );
    }
}

void Q2DViewerExtension::chooseNewSerie()
{
    emit newSerie();
}

void Q2DViewerExtension::readSettings()
{
    QSettings settings("GGG", "StarViewer-App-2DViewer");
    settings.beginGroup("StarViewer-App-2DViewer");

    m_splitter->restoreState( settings.value("splitter").toByteArray() );

    settings.endGroup();
}

void Q2DViewerExtension::writeSettings()
{
    QSettings settings("GGG", "StarViewer-App-2DViewer");
    settings.beginGroup("StarViewer-App-2DViewer");

    settings.setValue("splitter", m_splitter->saveState() );

    settings.endGroup();
}

}
