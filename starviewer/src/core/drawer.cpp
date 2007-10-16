/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "drawer.h"
#include "line.h"
#include "text.h"
#include "point.h"
#include "polygon.h"
#include "ellipse.h"
#include "drawingprimitive.h"
#include "logging.h"
#include "colorpalette.h"
#include "q2dviewer.h"
#include "representation.h"
#include "distancerepresentation.h"
#include "ellipserepresentation.h"
#include "distance.h"

//includes vtk
#include <vtkProp.h>
#include <vtkActor2D.h>
#include <vtkLineSource.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkRegularPolygonSource.h>
#include <vtkCaptionActor2D.h>
#include <vtkTextProperty.h>
#include <vtkProperty.h>
#include <vtkTextActor.h>
#include <vtkPoints.h>
#include <vtkMath.h>
#include <vtkCellArray.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkLine.h>

//includes Qt
#include <QColor>
#include <QList>
#include <QString>
///\TODO tenir en compte el tipus de pantalla per a determinar la paleta

///\TODO treure mètodes que no es fan servir
namespace udg {

Drawer::Drawer( Q2DViewer *m_viewer , QObject *parent ) : QObject( parent )
{
    //Per defecte paleta de colors rgb. caldria validar si rgb o monocrom
    m_colorPalette = new ColorPalette();
    m_2DViewer = m_viewer;

    //Creem les connexions entre el Q2DViewer i el drawer
    connect( m_2DViewer , SIGNAL( sliceChanged(int) ) , this , SLOT( setCurrentSlice( int ) ) );
    connect( m_2DViewer , SIGNAL( viewChanged(int) ) , this , SLOT( setCurrentView( int ) ) );
}

Drawer::~Drawer()
{
    delete m_colorPalette;
}

void Drawer::drawPoint( Point *point, int slice, int view )
{
    validateCoordinates( point->getPosition(), view );

    vtkRegularPolygonSource *circle = vtkRegularPolygonSource::New();
    circle->SetRadius( point->getWidth() );

    if ( point->isRounded() )
        circle->SetNumberOfSides( 60 );
    else
        circle->SetNumberOfSides( 4 );

    //assignem posició
    circle->SetCenter( point->getPosition()[0], point->getPosition()[1], point->getPosition()[2] );

    //mirem si s'a de pintar ple o buit
    circle->SetGeneratePolygon( point->isFilled() );

    vtkPolyDataMapper2D *polyMapper = vtkPolyDataMapper2D::New();
    polyMapper->SetInput( circle->GetOutput() ) ;

    polyMapper->SetTransformCoordinate( getCoordinateSystem( point->getCoordinatesSystemAsString() ) );
    vtkActor2D *actor = vtkActor2D::New();
    actor->SetMapper( polyMapper );

     //Assignem les propietats al punt
    vtkProperty2D *properties = actor->GetProperty();

    //Assignem opacitat del punt
    properties->SetOpacity( point->getOpacity() );

    //mirem la visibilitat de l'actor
    if ( !point->isVisible() )
        actor->VisibilityOff();

     //Assignem color
    QColor pointColor = point->getColor();
    actor->GetProperty()->SetColor( pointColor.redF(), pointColor.greenF(), pointColor.blueF() );

    addActorAndRefresh( actor, point, slice, view );

    //esborrem els objectes auxiliars
    actor->Delete();
    circle->Delete();
    polyMapper->Delete();
}

void Drawer::drawLine( Line *line, int slice, int view )
{
    vtkActor2D *lineActor = vtkActor2D::New();
    vtkLineSource *lineSource = vtkLineSource::New();
    vtkPolyDataMapper2D *lineMapper = vtkPolyDataMapper2D::New();

    //Assignem el tipus de coordenades seleccionades
    lineMapper->SetTransformCoordinate( getCoordinateSystem( line->getCoordinatesSystemAsString() ) );

    lineMapper->SetInputConnection( lineSource->GetOutputPort() );
    lineActor->SetMapper( lineMapper );

    //Assignem les propietats a la línia
    vtkProperty2D *properties = lineActor->GetProperty();

    lineSource->SetResolution ( 2 );

    //Assignem color
    QColor lineColor = line->getColor();
    properties->SetColor( lineColor.redF(), lineColor.greenF(), lineColor.blueF() );

    //Assignem discontinuïtat
    if ( line->isDiscontinuous() )
        properties->SetLineStipplePattern( 2000 );
    else
        properties->SetLineStipplePattern( 65535 );

    //Assignem gruix de la línia
    properties->SetLineWidth( line->getWidth() );

    //Assignem opacitat de la línia
    properties->SetOpacity( line->getOpacity() );

    //assignem els punts a la línia
    lineSource->SetPoint1( line->getFirstPoint() );
    lineSource->SetPoint2( line->getSecondPoint() );

    //mirem la visibilitat de l'actor
    if ( !line->isVisible() )
        lineActor->VisibilityOff();

    addActorAndRefresh( lineActor, line, slice, view );

    //connectem la línia amb el drawer per quan s'actualitzi algun dels seus atributs
    connect( line, SIGNAL( lineChanged( Line* ) ), this, SLOT( updateChangedLine( Line* ) ) );

    //esborrem els objectes auxiliars
    lineActor->Delete();
    lineSource->Delete();
    lineMapper->Delete();
}

void Drawer::drawText( Text *text, int slice, int view )
{
    vtkCaptionActor2D *textActor = vtkCaptionActor2D::New();

    //Assignem el tipus de coordenades seleccionades
    setCoordinateSystem( text->getCoordinatesSystemAsString(), textActor->GetAttachmentPointCoordinate() );

    //mirem si s'ha d'escalar el text
    if ( text->isTextScaled() )
        textActor->GetTextActor()->ScaledTextOn();
    else
        textActor->GetTextActor()->ScaledTextOff();

    //mirem l'opacitat
    textActor->GetCaptionTextProperty()->SetOpacity( text->getOpacity() );

    //Assignem color
    QColor textColor = text->getColor();
    textActor->GetCaptionTextProperty()->SetColor( textColor.redF(), textColor.greenF(), textColor.blueF() );

    textActor->SetPadding( text->getPadding() );

    textActor->SetPosition( -1.0 , -1.0 );
    textActor->SetHeight( text->getHeight() );
    textActor->SetWidth( text->getWidth() );

    //deshabilitem la línia que va des del punt de situació al text
    textActor->LeaderOff();
    textActor->ThreeDimensionalLeaderOff();

    if ( text->hasShadow() )
        textActor->GetCaptionTextProperty()->ShadowOn();
    else
        textActor->GetCaptionTextProperty()->ShadowOff();

    if ( text->isItalic() )
        textActor->GetCaptionTextProperty()->ItalicOn();
    else
        textActor->GetCaptionTextProperty()->ItalicOff();

    //Assignem el tipus de font al text
    if ( text->getFontFamily() == "Arial" )
        textActor->GetCaptionTextProperty()->SetFontFamilyToArial();
    else if ( text->getFontFamily() == "Courier" )
        textActor->GetCaptionTextProperty()->SetFontFamilyToCourier();
    else if ( text->getFontFamily() == "Times" )
        textActor->GetCaptionTextProperty()->SetFontFamilyToTimes();
    else
        DEBUG_LOG( "Tipus de font no reconegut a l'intentar crear text!!" );

    //Assignem el tamany de la font
    textActor->GetCaptionTextProperty()->SetFontSize( text->getFontSize() );

    //Assignem el tipus de justificació horitzontal
    if ( text->getHorizontalJustification() == "Left" )
        textActor->GetCaptionTextProperty()->SetJustificationToLeft();
    else if ( text->getHorizontalJustification() == "Centered" )
        textActor->GetCaptionTextProperty()->SetJustificationToCentered();
    else if ( text->getHorizontalJustification() == "Right" )
        textActor->GetCaptionTextProperty()->SetJustificationToRight();
    else
    {
        DEBUG_LOG( "Tipus de justificació horitzontal no reconegut a l'intentar crear text!!" );
    }

    //Assignem el tipus de justificació vertical
    if ( text->getVerticalJustification() == "Top" )
        textActor->GetCaptionTextProperty()->SetVerticalJustificationToTop();
    else if ( text->getVerticalJustification() == "Centered" )
        textActor->GetCaptionTextProperty()->SetVerticalJustificationToCentered();
    else if ( text->getVerticalJustification() == "Bottom" )
        textActor->GetCaptionTextProperty()->SetVerticalJustificationToBottom();
    else
    {
        DEBUG_LOG( "Tipus de justificació vertical no reconegut a l'intentar crear text!!" );
    }

    //Assignem el text
    textActor->SetCaption( qPrintable ( text->getText() ) );

    //Assignem la posició en pantalla
    textActor->SetAttachmentPoint( text->getAttatchmentPoint() );

    //mirem si el text té fons o no
    if ( text->isBorderEnabled() )
         textActor->BorderOn();
     else
         textActor->BorderOff();

    //mirem la visibilitat de l'actor
    if ( !text->isVisible() )
        textActor->VisibilityOff();

    addActorAndRefresh( textActor, text, slice, view );

    //connectem el text amb el drawer per quan s'actualitzi algun dels seus atributs
    connect( text, SIGNAL( textChanged( Text* ) ), this, SLOT( updateChangedText( Text* ) ) );

    //esborrem els objectes auxiliars
    textActor->Delete();
}

void Drawer::drawPolygon( Polygon *polygon, int slice, int view )
{
    //Ens assegurem que el polígon és tancat, és a dir, que el primer punt coincideix amb el primer
    double *firstPoint = polygon->getPoints().first();
    double *lastPoint = polygon->getPoints().last();

    if ( ( firstPoint[0] != lastPoint[0] ) || ( firstPoint[1] != lastPoint[1] ) || ( firstPoint[2] != lastPoint[2] ) )
    {
        polygon->addPoint( firstPoint );
    }

    vtkPolyData *polydata = vtkPolyData::New();
    vtkPoints *points = vtkPoints::New();
    vtkCellArray *vertexs = vtkCellArray::New();

    //especifiquem el nombre de vèrtexs que té el polígon
    vertexs->InsertNextCell( polygon->getNumberOfPoints() );
    points->SetNumberOfPoints( polygon->getNumberOfPoints() );

    //Calculem els punts
    for ( int i = 0; i < polygon->getNumberOfPoints(); i++ )
    {
        points->InsertPoint( i, polygon->getPoints().at( i ) );
        vertexs->InsertCellPoint( i );
    }
    //assignem els punts al polydata
    polydata->SetPoints( points );

    if ( polygon->isBackgroundEnabled() )
        polydata->SetPolys( vertexs );
    else
        polydata->SetLines( vertexs );

    vtkActor2D *actor = vtkActor2D::New();
    vtkPolyDataMapper2D *mapper = vtkPolyDataMapper2D::New();

    actor->SetMapper( mapper );
    mapper->SetTransformCoordinate( getCoordinateSystem( polygon->getCoordinatesSystemAsString() ) );
    mapper->SetInput( polydata );

    //Assignem discontinuïtat
    if ( polygon->isDiscontinuous() )
        actor->GetProperty()->SetLineStipplePattern( 2000 );
    else
        actor->GetProperty()->SetLineStipplePattern( 65535 );

    //Assignem gruix de la línia
    actor->GetProperty()->SetLineWidth( polygon->getWidth() );

    //Assignem opacitat de la línia
    actor->GetProperty()->SetOpacity( polygon->getOpacity() );

    //mirem la visibilitat de l'actor
    if ( !polygon->isVisible() )
        actor->VisibilityOff();

    //Assignem color
    QColor color = polygon->getColor();
    actor->GetProperty()->SetColor( color.redF(), color.greenF(), color.blueF() );

    addActorAndRefresh( actor, polygon, slice, view );

    actor->Delete();
    mapper->Delete();
    points->Delete();
    vertexs->Delete();
    polydata->Delete();
}

void Drawer::addActorAndRefresh( vtkProp *prop, DrawingPrimitive *drawingPrimitive, int slice, int view )
{
    //afegim l'actor al renderer del visor
    m_2DViewer->getRenderer()->AddActor( prop );

    //creem l'objecte PrimitiveActorPair per introduir al mapa
    PrimitiveActorPair *pair = new PrimitiveActorPair;
    pair->first = drawingPrimitive;
    pair->second = prop;

    //introduïm l'associació al mapa corresponent
    addPrimitive( pair, slice, view );

    m_2DViewer->refresh();
}

void Drawer::validateCoordinates( double coordinates[3], int view )
{
    double aux;
    switch( view )
    {
        case Q2DViewer::Axial:
            //no cal fer res perquè les coordenades ja són les desitjades
            break;

        case Q2DViewer::Sagittal:
            aux = coordinates[2];
            coordinates[2] = coordinates[0];
            coordinates[0] = aux;
            break;

        case Q2DViewer::Coronal:
            aux = coordinates[1];
            coordinates[1] = coordinates[2];
            coordinates[2] = aux;
            break;

        default:
            DEBUG_LOG( "Vista no reconeguda!!" );
            return;
            break;
    }
}

void Drawer::adaptCoordinatesToCurrentView( double *coordinates, int view )
{
    double aux;
    switch( view )
    {
        case Q2DViewer::Axial:
            //no cal fer res perquè les coordenades ja són les desitjades
            break;

        case Q2DViewer::Sagittal:
            aux = coordinates[2];
            coordinates[2] = coordinates[0];
            coordinates[0] = aux;
            break;

        case Q2DViewer::Coronal:
            aux = coordinates[1];
            coordinates[1] = coordinates[2];
            coordinates[2] = aux;
            break;

        default:
            DEBUG_LOG( "Vista no reconeguda!!" );
            return;
            break;
    }
}

void Drawer::drawEllipse( double rectangleCoordinate1[3], double rectangleCoordinate2[3], QColor color, QString behavior, int slice, int view )
{
    Ellipse *ellipse = new Ellipse( rectangleCoordinate1, rectangleCoordinate2, behavior );
    ellipse->setColor( color );
    drawEllipse( ellipse, slice, view );
}

void Drawer::drawEllipse( Ellipse *ellipse, int slice, int view )
{
    double *topLeft, *bottomRight;
    //validem les coordenades segons la vista en la que estem
    topLeft = ellipse->getTopLeftPoint();
    bottomRight = ellipse->getBottomRightPoint();

    //validem les coordenades
    validateCoordinates( topLeft, view );
    validateCoordinates( bottomRight, view );

    //assignem les noves coordenades corretgides
    ellipse->setTopLeftPoint( topLeft );
    ellipse->setBottomRightPoint( bottomRight );

    vtkPolyData *polydata = vtkPolyData::New();
    
    //calculem els punts de l'el·lipse i els retornem amb una parella
    EllipsePoints ellipsePoints = computeEllipsePoints( ellipse );

    polydata->SetPoints( ellipsePoints.first );

    if ( ellipse->isBackgroundEnabled() )
        polydata->SetPolys( ellipsePoints.second );
    else
        polydata->SetLines( ellipsePoints.second );

    vtkActor2D *actor = vtkActor2D::New();
    vtkPolyDataMapper2D *mapper = vtkPolyDataMapper2D::New();

    actor->SetMapper( mapper );
    mapper->SetTransformCoordinate( getCoordinateSystem( ellipse->getCoordinatesSystemAsString() ) );
    mapper->SetInput( polydata );

    //Assignem discontinuïtat
    if ( ellipse->isDiscontinuous() )
        actor->GetProperty()->SetLineStipplePattern( 2000 );
    else
        actor->GetProperty()->SetLineStipplePattern( 65535 );

    //Assignem gruix de la línia
    actor->GetProperty()->SetLineWidth( ellipse->getWidth() );

    //Assignem opacitat de la línia
    actor->GetProperty()->SetOpacity( ellipse->getOpacity() );

    //mirem la visibilitat de l'actor
    if ( !ellipse->isVisible() )
        actor->VisibilityOff();

    //Assignem color
    QColor color = ellipse->getColor();
    actor->GetProperty()->SetColor( color.redF(), color.greenF(), color.blueF() );

    addActorAndRefresh( actor, ellipse, slice, view );

    //connectem la línia amb el drawer per quan s'actualitzi algun dels seus atributs
    connect( ellipse, SIGNAL( ellipseChanged( Ellipse* ) ), this, SLOT( updateChangedEllipse( Ellipse* ) ) );

    actor->Delete();
    mapper->Delete();
    ellipsePoints.first->Delete();
    ellipsePoints.second->Delete();
    polydata->Delete();
}

Drawer::EllipsePoints Drawer::computeEllipsePoints( Ellipse *ellipse )
{
    double intersection[2], degrees, xAxis1[2], xAxis2[2], yAxis1[2], yAxis2[2], xRadius, yRadius, *center, *minor, *major,*topLeft, *bottomRight;
    int i;
    
    vtkPoints *points = vtkPoints::New();
    vtkCellArray *vertexs = vtkCellArray::New();

    topLeft = ellipse->getTopLeftPoint();
    bottomRight = ellipse->getBottomRightPoint();
    center = ellipse->getCenter();
    minor = ellipse->getMinorRadius();
    major = ellipse->getMajorRadius();

    //especifiquem el nombre de vèrtexs que té l'el·lipse
    vertexs->InsertNextCell( 61 );

    xAxis1[0] = center[0];
    xAxis1[1] = center[1];
    xAxis2[0] = bottomRight[0];
    xAxis2[1] = center[1];
    yAxis1[0] = center[0];
    yAxis1[1] = center[1];
    yAxis2[0] = center[0];
    yAxis2[1] = bottomRight[1];

    //calculem els radis i la intersecció d'aquests, segons tractem una el·lipse o un cercle
    xRadius = fabs( xAxis1[0] - xAxis2[0] );

    if ( ellipse->getBehavior() == "Ellipse" )
        yRadius = fabs( yAxis1[1] - yAxis2[1] );
    else
        yRadius = xRadius;

    intersection[0] = ((yAxis2[0] - yAxis1[0]) / 2.0) + yAxis1[0];
    intersection[1] = ((xAxis2[1] - xAxis1[1]) / 2.0) + xAxis1[1];

    //Calculem els punts de l'el·lipse
    for ( i = 0; i < 60; i++ )
    {
        degrees = i*6*vtkMath::DoubleDegreesToRadians();
        if ( m_2DViewer->getView() == Q2DViewer::Axial )
        {
            points->InsertPoint( i, cos( degrees )*xRadius + intersection[0], sin( degrees )*yRadius + intersection[1], .0 );
        }
        else if ( m_2DViewer->getView() == Q2DViewer::Sagittal )
        {
            points->InsertPoint( i, 0., sin( degrees )*yRadius + intersection[1],cos( degrees )*xRadius + intersection[0] );
        }
        else
        {
            points->InsertPoint( i, sin( degrees )*yRadius + intersection[1], 0., cos( degrees )*xRadius + intersection[0] );
        }
        vertexs->InsertCellPoint( i );
    }

    //afegim l'últim punt per tancar la ROI
    vertexs->InsertCellPoint( 0 );
    
    EllipsePoints ellipsePoints( points, vertexs );
    
    return ( ellipsePoints );
}

vtkCoordinate* Drawer::getCoordinateSystem( QString coordinateSystem )
{
    vtkCoordinate *coordinates = vtkCoordinate::New();

    if ( coordinateSystem == "DISPLAY" )
        coordinates->SetCoordinateSystemToDisplay();
    else if ( coordinateSystem == "NORMALIZED_DISPLAY" )
        coordinates->SetCoordinateSystemToNormalizedDisplay();
    else if ( coordinateSystem == "VIEWPORT" )
        coordinates->SetCoordinateSystemToViewport();
    else if ( coordinateSystem == "NORMALIZED_VIEWPORT" )
        coordinates->SetCoordinateSystemToNormalizedViewport();
    else if ( coordinateSystem == "VIEW" )
        coordinates->SetCoordinateSystemToView();
    else if ( coordinateSystem == "WORLD" )
        coordinates->SetCoordinateSystemToWorld();
    else
        DEBUG_LOG( "Sistema de coordenades no esperat per a mapejar la primitiva!!" );

    return( coordinates );
}

void Drawer::setCoordinateSystem( QString coordinateSystem, vtkCoordinate *coordinates )
{
    if ( coordinateSystem == "DISPLAY" )
        coordinates->SetCoordinateSystemToDisplay();
    else if ( coordinateSystem == "NORMALIZED_DISPLAY" )
        coordinates->SetCoordinateSystemToNormalizedDisplay();
    else if ( coordinateSystem == "VIEWPORT" )
        coordinates->SetCoordinateSystemToViewport();
    else if ( coordinateSystem == "NORMALIZED_VIEWPORT" )
        coordinates->SetCoordinateSystemToNormalizedViewport();
    else if ( coordinateSystem == "VIEW" )
        coordinates->SetCoordinateSystemToView();
    else if ( coordinateSystem == "WORLD" )
        coordinates->SetCoordinateSystemToWorld();
    else
        DEBUG_LOG( "Sistema de coordenades no esperat!!" );
}

Drawer::PrimitivesPairsList Drawer::getPrimitivesPairsList( int slice, int view )
{
    PrimitivesPairsList list;
    list.clear();

    switch( view )
    {
        case Q2DViewer::Axial:
            list = m_axialPairs.values( slice );
            break;

        case Q2DViewer::Sagittal:
            list = m_sagittalPairs.values( slice );
            break;

        case Q2DViewer::Coronal:
            list = m_coronalPairs.values( slice );
            break;

        default:
            DEBUG_LOG( "Valor no esperat" );
            break;
    }
    return list;
}

void Drawer::addPrimitive( PrimitiveActorPair *pair, int slice, int view )
{
    bool ok = true;
    switch( view )
    {
        case Q2DViewer::Axial:
            m_axialPairs.insert( slice, pair );
            break;

        case Q2DViewer::Sagittal:
            m_sagittalPairs.insert( slice, pair );
            break;

        case Q2DViewer::Coronal:
            m_coronalPairs.insert( slice, pair );
            break;

        default:
            DEBUG_LOG( "Valor no esperat" );
            ok = false;
            break;
    }
    if( ok )
    {
        // si la primitiva l'estem afegint en la llesca i vista actuals serà per defecte visible, altrament no
        if( slice == m_currentSlice && view == m_currentView )
            setVisibility( pair, true );
        else
            setVisibility( pair, false );
    }
}

void Drawer::setVisibility( PrimitiveActorPair *pair, bool visibility )
{
    if( visibility && pair )
    {
        pair->first->visibilityOn();
        pair->second->VisibilityOn();
    }
    else
    {
        pair->first->visibilityOff();
        pair->second->VisibilityOff();
    }
}

void Drawer::hidePrimitivesFrom( int slice, int view )
{
    PrimitivesPairsList listToHide = this->getPrimitivesPairsList( slice, view );

    foreach( PrimitiveActorPair *pair, listToHide )
    {
        setVisibility( pair, false );
    }
    //actualitzem el Q2DViewer
    m_2DViewer->refresh();
}

void Drawer::hidePrimitivesOfView( int view )
{
    QList< PrimitiveActorPair* > allPairsOfSelectedView;

    bool ok = true;
    switch( view )
    {
        case Q2DViewer::Axial:
            allPairsOfSelectedView = m_axialPairs.values();
            break;

        case Q2DViewer::Sagittal:
            allPairsOfSelectedView = m_sagittalPairs.values();
            break;

        case Q2DViewer::Coronal:
            allPairsOfSelectedView = m_coronalPairs.values();
            break;

        default:
            DEBUG_LOG( "Valor no esperat" );

            ok = false;
            break;
    }
    if( ok )
    {

       // fem invisibles totes les primitives
        foreach ( PrimitiveActorPair *pair, allPairsOfSelectedView )
        {
            setVisibility( pair, false );
            pair->first->highlightOff();
        }
    }
}

void Drawer::showPrimitivesFrom( int slice, int view )
{
    PrimitivesPairsList listToShow = this->getPrimitivesPairsList( slice, view );

    foreach( PrimitiveActorPair *pair, listToShow )
    {
        setVisibility( pair, true );
        setNormalColor( pair );
        pair->first->highlightOff();
    }
    //actualitzem el Q2DViewer
    m_2DViewer->refresh();
}

void Drawer::setCurrentSlice( int slice )
{
    //diem que no hi ha cap conjunt proper ni seleccionat
    m_nearestSet = NULL;
    m_selectedSet = NULL;

     // Primer fem insvisibles els de la llesca en la que ens trobàvem fins ara, en la corresponent vista
    hidePrimitivesFrom( m_currentSlice, m_currentView );

    // actualitzem la llesca
    m_currentSlice = slice;

    // I ara fem visibles els de la nova llesca, en la corresponent vista
    showPrimitivesFrom( slice, m_currentView );
}

void Drawer::setCurrentView( int view )
{
    if( m_currentView != view )
    {
        //diem que no hi ha cap conjunt proper ni seleccionat
        m_nearestSet = NULL;
        m_selectedSet = NULL;

        //cal fer invisible les primitives de la vista actual abans d'actualitzar a la nova vista
        hidePrimitivesOfView( m_currentView );

        //actualitzem la nova vista
        m_currentView = view;

        // s'ha canviat de vista, per tant cal netejar la última vista. Quan es faci el set slice de la nova vista, ja es faran visibles els que toquin
        showPrimitivesFrom( m_currentSlice, m_currentView );
    }
}

void Drawer::removeAllPrimitives()
{
    // recorrem cadscun dels maps, primer retirem l'actor de l'escena i després l'esborrem de la llista
    foreach( PrimitiveActorPair *pair, m_axialPairs )
    {
        m_2DViewer->getRenderer()->RemoveActor( pair->second );
    }

    foreach( PrimitiveActorPair *pair, m_sagittalPairs )
    {
        m_2DViewer->getRenderer()->RemoveActor( pair->second );
    }

    foreach( PrimitiveActorPair *pair, m_coronalPairs )
    {
        m_2DViewer->getRenderer()->RemoveActor( pair->second );
    }

    //Esborrem la llista que conté les associacions entre primitives
    m_primitivesSetList.clear();

    //Esborrem el contingut dels maps
    m_axialPairs.clear();
    m_sagittalPairs.clear();
    m_coronalPairs.clear();

    // refresquem l'escena
    m_2DViewer->refresh();
}

Drawer::PrimitiveActorPair* Drawer::findPrimitiveActorPair( DrawingPrimitive *drawingPrimitive, int slice, int view )
{
    PrimitiveActorPair *desiredPrimitiveActorPair;
    PrimitivesPairsList list =  getPrimitivesPairsList( slice, view );

    bool found = false;

    for ( int i = 0; i < (list.size()) && !found ; i++ )
    {
        desiredPrimitiveActorPair = list.at( i );

        if ( desiredPrimitiveActorPair->first == drawingPrimitive )
            found = true;
    }

    if ( !found )
    {
        ERROR_LOG( "No s'ha trobat la primitiva gràfica desitjada!!!" );
        desiredPrimitiveActorPair->first = NULL;
        desiredPrimitiveActorPair->second = NULL;
    }

    return desiredPrimitiveActorPair;
}

Drawer::PrimitiveActorPair* Drawer::findPrimitiveActorPair( DrawingPrimitive *drawingPrimitive )
{
    int i;
    PrimitiveActorPair *desiredPrimitiveActorPair;
    PrimitivesPairsList axialList = m_axialPairs.values();
    PrimitivesPairsList sagitalList = m_sagittalPairs.values();
    PrimitivesPairsList coronalList = m_coronalPairs.values();

    bool found = false;

    //mirem per a cadascuna de les llistes si hem trobat la primitiva
    for ( i = 0; ( i < axialList.size() ) && !found ; i++ )
    {
        desiredPrimitiveActorPair = axialList.at( i );

        if ( desiredPrimitiveActorPair->first == drawingPrimitive )
            found = true;
    }

    for ( i = 0; ( i < sagitalList.size() ) && !found ; i++ )
    {
        desiredPrimitiveActorPair = sagitalList.at( i );

        if ( desiredPrimitiveActorPair->first == drawingPrimitive )
            found = true;
    }

    for ( i = 0; ( i < coronalList.size() ) && !found ; i++ )
    {
        desiredPrimitiveActorPair = coronalList.at( i );

        if ( desiredPrimitiveActorPair->first == drawingPrimitive )
            found = true;
    }

    if ( !found )
    {
        ERROR_LOG( "No s'ha trobat la primitiva gràfica desitjada!!!" );
        desiredPrimitiveActorPair->first = NULL;
        desiredPrimitiveActorPair->second = NULL;
    }

    return desiredPrimitiveActorPair;
}

bool Drawer::isValid( PrimitiveActorPair *pair )
{
    return( pair->first != NULL && pair->second != NULL );
}

void Drawer::updateChangedLine( Line *line )
{
    PrimitiveActorPair *pair = this->findPrimitiveActorPair( line );

    if ( isValid( pair ) )
    {
        vtkActor2D *lineActor = vtkActor2D::SafeDownCast( pair->second );
        Line *line = static_cast<Line*> ( pair->first );

        vtkPolyDataMapper2D *lineMapper = vtkPolyDataMapper2D::SafeDownCast( lineActor->GetMapper() );

        vtkLineSource *lineSource = vtkLineSource::New();
        
        lineMapper->SetInputConnection( lineSource->GetOutputPort() );
    
        lineSource->SetResolution ( 2 );
    
        //assignem els punts a la línia
        lineSource->SetPoint1( line->getFirstPoint() );
        lineSource->SetPoint2( line->getSecondPoint() );

        //Assignem el tipus de coordenades seleccionades
        lineMapper->SetTransformCoordinate( getCoordinateSystem( line->getCoordinatesSystemAsString() ) );

        //Assignem les propietats a la línia
        vtkProperty2D *properties = lineActor->GetProperty();

        //Assignem gruix de la línia
        properties->SetLineWidth( line->getWidth() );

        //Assignem opacitat de la línia
        properties->SetOpacity( line->getOpacity() );

        //Assignem discontinuïtat
        if ( line->isDiscontinuous() )
            properties->SetLineStipplePattern( 2000 );
        else
            properties->SetLineStipplePattern( 65535 );

        //mirem si la línia està en estat de resaltat o no. Segons això assignem el color
        QColor lineColor = line->getColor();
        QColor highlightColor = m_colorPalette->getHighlightColor();
        QColor selectionColor = m_colorPalette->getSelectionColor();

        if ( line->isHighlighted() )
            properties->SetColor( highlightColor.redF(), highlightColor.greenF(), highlightColor.blueF() );
        else if ( line->isSelected() )
            properties->SetColor( selectionColor.redF(), selectionColor.greenF(), selectionColor.blueF() );
        else
            properties->SetColor( lineColor.redF(), lineColor.greenF(), lineColor.blueF() );

        //mirem la visibilitat de l'actor
        if ( !line->isVisible() )
            lineActor->VisibilityOff();

        lineSource->Delete();
        m_2DViewer->refresh();
    }
}

void Drawer::updateChangedEllipse( Ellipse *ellipse )
{
    PrimitiveActorPair *pair = this->findPrimitiveActorPair( ellipse );

    if ( isValid( pair ) )
    {
        vtkActor2D *ellipseActor = vtkActor2D::SafeDownCast( pair->second );
        Ellipse *ellipse = static_cast<Ellipse*> ( pair->first );

        vtkPolyDataMapper2D *ellipseMapper = vtkPolyDataMapper2D::SafeDownCast( ellipseActor->GetMapper() );

        EllipsePoints ellipsePoints = computeEllipsePoints( ellipse );

        vtkPolyData *ellipsePolyData = vtkPolyData::New();
        
        ellipsePolyData->SetPoints( ellipsePoints.first );

        if ( ellipse->isBackgroundEnabled() )
            ellipsePolyData->SetPolys( ellipsePoints.second );
        else
            ellipsePolyData->SetLines( ellipsePoints.second );
            
        ellipseMapper->SetInput( ellipsePolyData );

        //Assignem el tipus de coordenades seleccionades
        ellipseMapper->SetTransformCoordinate( getCoordinateSystem( ellipse->getCoordinatesSystemAsString() ) );

        //Assignem les propietats a la línia
        vtkProperty2D *properties = ellipseActor->GetProperty();

        //Assignem gruix de la línia
        properties->SetLineWidth( ellipse->getWidth() );

        //Assignem opacitat de la línia
        properties->SetOpacity( ellipse->getOpacity() );

        //Assignem discontinuïtat
        if ( ellipse->isDiscontinuous() )
            properties->SetLineStipplePattern( 2000 );
        else
            properties->SetLineStipplePattern( 65535 );

        //mirem si la línia està en estat de resaltat o no. Segons això assignem el color
        QColor ellipseColor = ellipse->getColor();
        QColor highlightColor = m_colorPalette->getHighlightColor();
        QColor selectionColor = m_colorPalette->getSelectionColor();

        if ( ellipse->isHighlighted() )
            properties->SetColor( highlightColor.redF(), highlightColor.greenF(), highlightColor.blueF() );
        else if ( ellipse->isSelected() )
            properties->SetColor( selectionColor.redF(), selectionColor.greenF(), selectionColor.blueF() );
        else
            properties->SetColor( ellipseColor.redF(), ellipseColor.greenF(), ellipseColor.blueF() );

        //mirem la visibilitat de l'actor
        if ( !ellipse->isVisible() )
            ellipseActor->VisibilityOff();

        m_2DViewer->refresh();
        ellipsePolyData->Delete();
    }
}

void Drawer::updateChangedText( Text *text )
{
    PrimitiveActorPair *pair = this->findPrimitiveActorPair( text );
    
    if ( isValid( pair ) )
    {
        //obtenim l'actor de text
        vtkCaptionActor2D *textActor = vtkCaptionActor2D::SafeDownCast( pair->second );
        
        //fem la conversió estàtica per a poder executar els mètodes de la primitiva de text
        Text *text = static_cast<Text*> ( pair->first );
    
        //mirem si s'ha d'escalar el text
        if ( text->isTextScaled() )
            textActor->GetTextActor()->ScaledTextOn();
        else
            textActor->GetTextActor()->ScaledTextOff();
        
        //mirem l'opacitat
        textActor->GetCaptionTextProperty()->SetOpacity( text->getOpacity() );
        
        //Assignem color
        QColor textColor = text->getColor();
        QColor highlightColor = m_colorPalette->getHighlightColor();
        QColor selectionColor = m_colorPalette->getSelectionColor();
        
        if ( text->isHighlighted() )
            textActor->GetCaptionTextProperty()->SetColor( highlightColor.redF(), highlightColor.greenF(), highlightColor.blueF() );
        else if ( text->isSelected() )
            textActor->GetCaptionTextProperty()->SetColor( selectionColor.redF(), selectionColor.greenF(), selectionColor.blueF() );
        else
            textActor->GetCaptionTextProperty()->SetColor( textColor.redF(), textColor.greenF(), textColor.blueF() );
        
        textActor->SetPadding( text->getPadding() );
        
        textActor->SetPosition( -1.0 , -1.0 );
        textActor->SetHeight( text->getHeight() );
        textActor->SetWidth( text->getWidth() );
        
        //deshabilitem la línia que va des del punt de situació al text
        textActor->LeaderOff();
        textActor->ThreeDimensionalLeaderOff();
        
        if ( text->hasShadow() )
            textActor->GetCaptionTextProperty()->ShadowOn();
        else
            textActor->GetCaptionTextProperty()->ShadowOff();
        
        if ( text->isItalic() )
            textActor->GetCaptionTextProperty()->ItalicOn();
        else
            textActor->GetCaptionTextProperty()->ItalicOff();
    
        //Assignem el tipus de font al text
        if ( text->getFontFamily() == "Arial" )
            textActor->GetCaptionTextProperty()->SetFontFamilyToArial();
        else if ( text->getFontFamily() == "Courier" )
            textActor->GetCaptionTextProperty()->SetFontFamilyToCourier();
        else if ( text->getFontFamily() == "Times" )
            textActor->GetCaptionTextProperty()->SetFontFamilyToTimes();
        else
            DEBUG_LOG( "Tipus de font no reconegut a l'intentar crear text!!" );
        
        //Assignem el tamany de la font
        textActor->GetCaptionTextProperty()->SetFontSize( text->getFontSize() );
        
        //Assignem el tipus de justificació horitzontal
        if ( text->getHorizontalJustification() == "Left" )
            textActor->GetCaptionTextProperty()->SetJustificationToLeft();
        else if ( text->getHorizontalJustification() == "Centered" )
            textActor->GetCaptionTextProperty()->SetJustificationToCentered();
        else if ( text->getHorizontalJustification() == "Right" )
            textActor->GetCaptionTextProperty()->SetJustificationToRight();
        else
        {
            DEBUG_LOG( "Tipus de justificació horitzontal no reconegut a l'intentar crear text!!" );
        }
        
        //Assignem el tipus de justificació vertical
        if ( text->getVerticalJustification() == "Top" )
            textActor->GetCaptionTextProperty()->SetVerticalJustificationToTop();
        else if ( text->getVerticalJustification() == "Centered" )
            textActor->GetCaptionTextProperty()->SetVerticalJustificationToCentered();
        else if ( text->getVerticalJustification() == "Bottom" )
            textActor->GetCaptionTextProperty()->SetVerticalJustificationToBottom();
        else
        {
            DEBUG_LOG( "Tipus de justificació vertical no reconegut a l'intentar crear text!!" );
        }
        
        //Assignem el text
        textActor->SetCaption( qPrintable ( text->getText() ) );
        
        //Assignem la posició en pantalla
        textActor->SetAttachmentPoint( text->getAttatchmentPoint() );
        
        //mirem si el text té fons o no
        if ( text->isBorderEnabled() )
             textActor->BorderOn();
         else
             textActor->BorderOff();
         
        //mirem la visibilitat de l'actor
        if ( !text->isVisible() )
            textActor->VisibilityOff();
        else
            textActor->VisibilityOn();
            
        m_2DViewer->refresh();
    }
}

int Drawer::getNumberOfDrawedPrimitives()
{
    return( m_axialPairs.count() + m_sagittalPairs.count() + m_coronalPairs.count() );
}

Drawer::PrimitivesSet* Drawer::getSetOf( DrawingPrimitive *primitive )
{
    PrimitivesSet *set;
    bool notFound = true;

    for ( int i = 0; ( i < m_primitivesSetList.count() ) && notFound; i++ )
    {
        if ( m_primitivesSetList[i]->contains( primitive ) )
        {
            set = m_primitivesSetList[i];
            notFound = false;
        }
    }

    if ( notFound )
    {
        ERROR_LOG( "No s'ha trobat el conjunt de primitives associades!!!!!" );
        set = NULL;
    }
    return( set );
}

void Drawer::highlightNearestPrimitives()
{
    double point[3] = { .0, .0, .0 };
    int x, y;
    x = m_2DViewer->getInteractor()->GetEventPosition()[0];
    y = m_2DViewer->getInteractor()->GetEventPosition()[1];
    double toWorld[4];
    m_2DViewer->computeDisplayToWorld( m_2DViewer->getRenderer() , x, y , 0 , toWorld );

    PrimitivesMap currentViewMap;

    switch( m_2DViewer->getView() )
    {
        case Q2DViewer::Axial:
            point[0] = toWorld[0];
            point[1] = toWorld[1];
            currentViewMap = m_axialPairs;
            break;
        case Q2DViewer::Sagittal:
            point[1] = toWorld[1];
            point[2] = toWorld[2];
            currentViewMap = m_sagittalPairs;
            break;
        case Q2DViewer::Coronal:
            point[0] = toWorld[0];
            point[2] = toWorld[2];
            currentViewMap = m_coronalPairs;
            break;
        default:
            DEBUG_LOG( "El Q2DViewer no té assignada cap de les 3 vistes possibles!?" );
            break;
    }
    //la llista de totes les primitives de la vista actual
    PrimitivesPairsList list = currentViewMap.values();

    //agafem la primitiva més propera
    PrimitiveActorPair *nearestPair = getNearestPrimitivePair( point );
    
    if ( !isValid( nearestPair ) )
    {
        DEBUG_LOG( "No hi ha primitiva propera vàlida!!" );
        return;
    }    
    m_nearestSet = getSetOf( nearestPair->first );

    foreach( PrimitiveActorPair *pair, list )
    {
        if ( m_nearestSet && m_nearestSet->contains( pair->first ) )
        {
            if ( !pair->first->isSelected() )//cal que el més proper no sigui el seleccionat perquè li treuria el color de seleccionat i no ho volem
            {
                setHighlightColor( pair );
                pair->first->highlightOn();
            }
            else /*if ( pair->first->isSelected() )*/
            {
                setSelectedColor( pair );
                pair->first->selectedOn();
            }
        }
        else
        {
            if ( m_selectedSet ) //el pintem de com a normal si no és el seleccionat, ja que si ho és cal que es mantingui la selecció
            {
                if ( !m_selectedSet->contains( pair->first ) )
                {
                    setNormalColor( pair );
                    pair->first->selectedOff();
                }
            }
            else
            {
                setNormalColor( pair );
                pair->first->selectedOff();
            }
        }
    }
    m_2DViewer->refresh();
}

void Drawer::selectNearestSet()
{
    m_selectedSet = m_nearestSet;

    PrimitivesMap currentViewMap;

    switch( m_2DViewer->getView() )
    {
        case Q2DViewer::Axial:
            currentViewMap = m_axialPairs;
            break;
        case Q2DViewer::Sagittal:
            currentViewMap = m_sagittalPairs;
            break;
        case Q2DViewer::Coronal:
            currentViewMap = m_coronalPairs;
            break;
        default:
            DEBUG_LOG( "El Q2DViewer no té assignada cap de les 3 vistes possibles!?" );
            break;
    }

    //la llista de totes les primitives de la vista actual
    PrimitivesPairsList list = currentViewMap.values();

    foreach( PrimitiveActorPair *pair, list )
    {
        if ( m_selectedSet->contains( pair->first ) )
        {
            setSelectedColor( pair );
            pair->first->selectedOn();
        }
        else
        {
            setNormalColor( pair );
            pair->first->selectedOff();
        }
    }
    
    m_2DViewer->refresh();
}

void Drawer::unselectSet()
{
    m_selectedSet = NULL;
    m_nearestSet  = NULL;

    PrimitivesMap currentViewMap;

    switch( m_2DViewer->getView() )
    {
        case Q2DViewer::Axial:
            currentViewMap = m_axialPairs;
            break;
        case Q2DViewer::Sagittal:
            currentViewMap = m_sagittalPairs;
            break;
        case Q2DViewer::Coronal:
            currentViewMap = m_coronalPairs;
            break;
        default:
            DEBUG_LOG( "El Q2DViewer no té assignada cap de les 3 vistes possibles!?" );
            break;
    }

    //la llista de totes les primitives de la vista actual
    PrimitivesPairsList list = currentViewMap.values();

    foreach( PrimitiveActorPair *pair, list )
    {
            setNormalColor( pair );
            pair->first->selectedOff();
    }
    m_2DViewer->refresh();
}

void Drawer::setHighlightColor( PrimitiveActorPair *pair )
{
    QColor highlightColor = m_colorPalette->getHighlightColor();

    if ( pair->first->getPrimitiveType() == "Line" )
    {
        vtkActor2D *actor = vtkActor2D::SafeDownCast( pair->second );
        vtkProperty2D *properties = actor->GetProperty();
        properties->SetColor( highlightColor.redF(), highlightColor.greenF(), highlightColor.blueF() );
        pair->first->setColor( highlightColor );
    }
    else if ( pair->first->getPrimitiveType() == "Text" )
    {
        vtkCaptionActor2D *textActor = vtkCaptionActor2D::SafeDownCast( pair->second );
        vtkTextProperty *property = textActor->GetCaptionTextProperty();
        property->SetColor( highlightColor.redF(), highlightColor.greenF(), highlightColor.blueF() );
        pair->first->setColor( highlightColor );
    }
}

void Drawer::setSelectedColor( PrimitiveActorPair *pair )
{
    QColor selectedColor = m_colorPalette->getSelectionColor();

    if ( pair->first->getPrimitiveType() == "Line" )
    {
        vtkActor2D *actor = vtkActor2D::SafeDownCast( pair->second );
        vtkProperty2D *properties = actor->GetProperty();
        properties->SetColor( selectedColor.redF(), selectedColor.greenF(), selectedColor.blueF() );
        pair->first->setColor( selectedColor );
    }
    else if ( pair->first->getPrimitiveType() == "Text" )
    {
        vtkCaptionActor2D *textActor = vtkCaptionActor2D::SafeDownCast( pair->second );
        vtkTextProperty *property = textActor->GetCaptionTextProperty();
        property->SetColor( selectedColor.redF(), selectedColor.greenF(), selectedColor.blueF() );
        pair->first->setColor( selectedColor );
    }
}

void Drawer::setNormalColor( PrimitiveActorPair *pair )
{
    QColor normalColor = m_colorPalette->getNormalColor();

    if ( pair->first->getPrimitiveType() == "Line" )
    {
        vtkActor2D *actor = vtkActor2D::SafeDownCast( pair->second );
        vtkProperty2D *properties = actor->GetProperty();
        properties->SetColor( normalColor.redF(), normalColor.greenF(), normalColor.blueF() );
        pair->first->setColor( normalColor );
    }
    else if ( pair->first->getPrimitiveType() == "Text" )
    {
        vtkCaptionActor2D *textActor = vtkCaptionActor2D::SafeDownCast( pair->second );
        vtkTextProperty *property = textActor->GetCaptionTextProperty();
        property->SetColor( normalColor.redF(), normalColor.greenF(), normalColor.blueF() );
        pair->first->setColor( normalColor );
    }
}

Drawer::PrimitivesPairsList Drawer::getAllPrimitivesOfType( QString primitiveType )
{
    PrimitivesPairsList result;
    PrimitivesMap map;

    switch( m_2DViewer->getView() )
    {
        case Q2DViewer::Axial:
            map = m_axialPairs;
            break;
        case Q2DViewer::Sagittal:
            map = m_sagittalPairs;
            break;
        case Q2DViewer::Coronal:
            map = m_coronalPairs;
            break;
        default:
            ERROR_LOG( "El Q2DViewer no té assignada cap de les 3 vistes possibles!?" );
            break;
    }

    foreach( PrimitiveActorPair *pair, map )
    {
        if ( pair->first->getPrimitiveType() == primitiveType )
            result << pair;
    }
    return( result );
}

int Drawer::getIndexOfPairWhenTypeIs( PrimitivesPairsList nearestPairslist, QString primitiveType )
{
    int index, i = 0;

    foreach( PrimitiveActorPair *pair, nearestPairslist )
    {
        if ( pair->first->getPrimitiveType() == primitiveType )
            index = i;

        i++;
    }
    return( index );
}

bool Drawer::hasPrimitiveOfType( PrimitivesPairsList nearestPairslist, QString primitiveType )
{
    bool response = false;
    foreach( PrimitiveActorPair *pair, nearestPairslist )
    {
        if ( pair->first->getPrimitiveType() == primitiveType )
            response = true;
    }
    return( response );
}

Drawer::PrimitiveActorPair* Drawer::getNearestPrimitivePair( double point[3] )
{
    PrimitivesPairsList list;
    PrimitiveActorPair *nearestPair;
    int coordinateToZero;

    switch( m_2DViewer->getView() )
    {
        case Q2DViewer::Axial:
            list = m_axialPairs.values( m_2DViewer->getCurrentSlice() );
            //la coordenada que s'ha de deixar a 0 és la z.
            coordinateToZero = 2;
            break;

        case Q2DViewer::Sagittal:
            list = m_sagittalPairs.values( m_2DViewer->getCurrentSlice() );
            //la coordenada que s'ha de deixar a 0 és la x.
            coordinateToZero = 0;
            break;

        case Q2DViewer::Coronal:
            list = m_coronalPairs.values( m_2DViewer->getCurrentSlice() );
            //la coordenada que s'ha de deixar a 0 és la y.
            coordinateToZero = 1;
            break;

        default:
            DEBUG_LOG( "vista del visor 2D no esperada!" );
            break;
    }

    double minDistanceLine = VTK_DOUBLE_MAX;
    double *p1, *p2, distance;
    ///\TODO tenir en compte tots els tipus de primitives

    foreach( PrimitiveActorPair *pair, list )
    {
        DrawingPrimitive *primitive = pair->first;
        if ( primitive->getPrimitiveType() == "Line" )
        {
            Line *line = static_cast<Line*> ( primitive );

            p1 = line->getFirstPoint();
            p1[coordinateToZero] = 0.0;
            p2 = line->getSecondPoint();
            p2[coordinateToZero] = 0.0;
            distance = vtkLine::DistanceToLine( point , p1 , p2 );

            if ( ( ( ( minDistanceLine != VTK_DOUBLE_MAX ) && ( distance < minDistanceLine ) ) || ( distance < 20.0 ) ) && ( isPointIncludedInLineBounds( point, p1, p2 ) ) )
            {
                    minDistanceLine = distance;
                    nearestPair = pair;
            }
        }
    }
    return nearestPair;
}

bool Drawer::isPointIncludedInLineBounds( double point[3], double *lineP1, double *lineP2 )
{
    double x1, x2, y1, y2;
    int coordinate1, coordinate2;
    bool correctPoint = false;
    double range = 5.;
    
    switch( m_2DViewer->getView() )
    {
        case Q2DViewer::Axial:
            coordinate1 = 0;
            coordinate2 = 1;
            break;
        case Q2DViewer::Sagittal:
            coordinate1 = 2;
            coordinate2 = 1;
            break;
        case Q2DViewer::Coronal:
            coordinate1 = 0;
            coordinate2 = 2;
            break;
        default:
            ERROR_LOG( "El Q2DViewer no té assignada cap de les 3 vistes possibles!?" );
            break;
    }
    
    //posem a _1 el valor més petit i a _2 el més gran 
    if ( lineP1[coordinate1] <= lineP2[coordinate1] )
    {
        x1 = lineP1[coordinate1];
        x2 = lineP2[coordinate1];
    }
    else
    {
        x2 = lineP1[coordinate1];
        x1 = lineP2[coordinate1];
    }
    
    if ( lineP1[coordinate2] <= lineP2[coordinate2] )
    {
        y1 = lineP1[coordinate2];
        y2 = lineP2[coordinate2];
    }
    else
    {
        y2 = lineP1[coordinate2];
        y1 = lineP2[coordinate2];
    }
    
    if ( ( point[coordinate1] >= ( x1 - range ) ) && ( point[coordinate1] <= ( x2 + range ) ) && ( point[coordinate2] >= ( y1 - range ) ) && ( point[coordinate2] <= ( y2 + range ) ) )
    {
        correctPoint = true;
    }
    
    return( correctPoint );
}

void Drawer::addSetOfPrimitives( Representation *representation )
{
    PrimitivesSet *set = new PrimitivesSet;

    if ( representation->getRepresentationType() == "DistanceRepresentation" )
    {
        DistanceRepresentation *distanceRepresentation = static_cast<DistanceRepresentation*> ( representation );
        set->insert( distanceRepresentation->getLine() );
        set->insert( distanceRepresentation->getText() );
//         set->insert( distanceRepresentation->getPolygon() );
// 
//         //li diem que dibuixi el polígon que representa el voltant del text.
//         distanceRepresentation->getPolygon()->enableBackground();
//         distanceRepresentation->getPolygon()->setColor( QColor( 0, 0, 0 ) );
//         drawPolygon( distanceRepresentation->getPolygon(), m_2DViewer->getCurrentSlice(), m_2DViewer->getView() );

        //li diem al drawer que dibuixi el text de la distància
        drawText( distanceRepresentation->getText(), m_2DViewer->getCurrentSlice(), m_2DViewer->getView() );
    }
    else if( representation->getRepresentationType() == "EllipseRepresentation" )
    {
        EllipseRepresentation *ellipseRepresentation = static_cast<EllipseRepresentation*> ( representation );
        set->insert( ellipseRepresentation->getEllipse() );
        set->insert( ellipseRepresentation->getText() );

        //li diem al drawer que dibuixi el text de la distància
        drawText( ellipseRepresentation->getText(), m_2DViewer->getCurrentSlice(), m_2DViewer->getView() );
    }

    //afegim el conjunt a la llista si realment s'ha emplenat
    if ( set->count() > 0 )
        m_primitivesSetList.append( set );
}

void Drawer::removeSelectedSet()
{
    if ( hasSelectedSet() )
    {
        //agafem la posició on es troba aquest conjunt dins de la llista
        int position = m_primitivesSetList.indexOf( m_selectedSet );

        //obtenim la llista dels elements que estan dins del conjunt.
        QList< DrawingPrimitive *> listOfPrimitives = m_selectedSet->values();
        
        PrimitiveActorPair *pair;
        
        QMutableMapIterator< int , PrimitiveActorPair* > *iterator = NULL;
        switch( m_currentView )
        {
        case Q2DViewer::Axial:
            iterator = new QMutableMapIterator< int , PrimitiveActorPair* >( m_axialPairs );
        break;

        case Q2DViewer::Sagittal:
            iterator = new QMutableMapIterator< int , PrimitiveActorPair* >( m_sagittalPairs );
        break;

        case Q2DViewer::Coronal:
            iterator = new QMutableMapIterator< int , PrimitiveActorPair* >( m_coronalPairs );
        break;

        default:
            DEBUG_LOG( "Valor inesperat" );
        break;
        }
        
        if ( iterator )
        {
            foreach( DrawingPrimitive *primitive, listOfPrimitives )
            {
                pair = findPrimitiveActorPair( primitive );
            
                iterator->toFront();
                if( iterator->findNext( pair ) )
                {
                    iterator->remove();
                    m_2DViewer->getRenderer()->RemoveActor( pair->second );
                    delete pair->first;
                    
                }
            }
            m_primitivesSetList.removeAt( position );
            m_selectedSet = NULL;
            m_nearestSet = NULL;
            m_2DViewer->refresh();
        }   
    }
}
};  // end namespace udg
