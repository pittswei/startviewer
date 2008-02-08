/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "erasertool.h"
#include "q2dviewer.h"
#include "logging.h"
#include "drawer.h"
#include "drawerpolyline.h"
// vtk
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCommand.h>

namespace udg {

EraserTool::EraserTool( QViewer *viewer, QObject *parent )
 : Tool(viewer,parent)
{
    m_toolName = "EraserTool";
    m_hasSharedData = false;

    m_2DViewer = qobject_cast<Q2DViewer *>( viewer );
    if( !m_2DViewer )
        DEBUG_LOG(QString("El casting no ha funcionat!!! És possible que viewer no sigui un Q2DViewer!!!-> ")+ viewer->metaObject()->className() );

    m_polyline=NULL;
    
    m_state = NONE;
    DEBUG_LOG("ERASER TOOL CREADA ");
}

EraserTool::~EraserTool()
{
}

void EraserTool::handleEvent( unsigned long eventID )
{
    //         m_viewer->setCursor( QCursor( QPixmap(":/images/zoom.png") ) );
        // \TODO es podria afegir una variable membre 'm_factor' per poder controlar desde fora com de ràpid és l'augment o disminució del zoom
//         m_viewer->zoom( pow((double)1.1, 2.0) );
//         m_viewer->setCursor( Qt::ArrowCursor );
    switch( eventID )
    {
        case vtkCommand::LeftButtonPressEvent:
            startEraserAction();
        break;
    
        case vtkCommand::MouseMoveEvent:
            if ( m_polyline && m_state == START_CLICK )
                drawAreaOfErasure();
        break;
    
        case vtkCommand::LeftButtonReleaseEvent:
            erasePrimitive();
        break;
    
        default:
        break;
    }
}

void EraserTool::startEraserAction()
{
    if (!m_polyline )
    {
        m_polyline = new DrawerPolyline;
        m_2DViewer->getDrawer()->draw( m_polyline , Q2DViewer::Top2DPlane, m_2DViewer->getCurrentSlice() );
    }

    //guardem el primer punt de la zona d'esborrat i posem l'estat adequat a la tool
    double position[4];
        
    //capturem l'event de clic esquerre
    int x = m_2DViewer->getEventPositionX();
    int y = m_2DViewer->getEventPositionY();
        
    m_2DViewer->computeDisplayToWorld( m_2DViewer->getRenderer() , x, y, m_2DViewer->getCurrentSlice(), position );
    m_startPoint[0] = position[0];
    m_startPoint[1] = position[1];
    m_startPoint[2] = position[2];

    //afegim el punt
    m_polyline->addPoint( m_startPoint );

    //actualitzem els atributs de la polilinia
    m_polyline->update( DrawerPrimitive::VTKRepresentation );
  
    m_state = START_CLICK;
}

void EraserTool::drawAreaOfErasure()
{
    double position[4], p2[3], p3[3];   
    
    //capturem l'event de clic esquerre
    int x = m_2DViewer->getEventPositionX();
    int y = m_2DViewer->getEventPositionY();
            
    m_2DViewer->computeDisplayToWorld( m_2DViewer->getRenderer() , x, y, m_2DViewer->getCurrentSlice(), position );
    m_endPoint[0] = position[0];
    m_endPoint[1] = position[1];
    m_endPoint[2] = position[2];
    
    if ( m_polyline->getNumberOfPoints() > 1 )
    {
        m_polyline->removePoint( 4 );
        m_polyline->removePoint( 3 );
        m_polyline->removePoint( 2 );
        m_polyline->removePoint( 1 );
    }
    
    //calculem el segon punt i el tercer
    switch( m_2DViewer->getView() )
    {
        case Q2DViewer::AxialPlane:
            p2[0] = m_endPoint[0];
            p2[1] = m_startPoint[1];
            p2[2] = m_2DViewer->getCurrentSlice();
            
            p3[0] = m_startPoint[0];
            p3[1] = m_endPoint[1];
            p3[2] = m_2DViewer->getCurrentSlice();
            break;
        case Q2DViewer::SagitalPlane:
            p2[0] = m_2DViewer->getCurrentSlice();
            p2[1] = m_startPoint[1];
            p2[2] = m_endPoint[2];
            
            p3[0] = m_2DViewer->getCurrentSlice();
            p3[1] = m_endPoint[1];
            p3[2] = m_startPoint[2];
            break;
        case Q2DViewer::CoronalPlane:
            p2[0] = m_startPoint[0];
            p2[1] = m_2DViewer->getCurrentSlice();
            p2[2] = m_endPoint[2];
            
            p3[0] = m_endPoint[0];
            p3[1] = m_2DViewer->getCurrentSlice();
            p3[2] = m_startPoint[2];
            break;   
    }
    m_polyline->addPoint( p2 );
    m_polyline->addPoint( m_endPoint );
    m_polyline->addPoint( p3 );
    m_polyline->addPoint( m_startPoint );
    
    //actualitzem els atributs de la polilinia
    m_polyline->update( DrawerPrimitive::VTKRepresentation );
    
    m_2DViewer->refresh();
}

void EraserTool::erasePrimitive()
{
    m_2DViewer->getDrawer()->erasePrimitivesInsideBounds( m_startPoint, m_endPoint, m_2DViewer->getView(), m_2DViewer->getCurrentSlice() );
    m_polyline->deleteAllPoints();
    m_polyline->update( DrawerPrimitive::VTKRepresentation );
    m_2DViewer->getDrawer()->refresh();
    m_2DViewer->refresh();
    
    m_state = NONE;
}
}
