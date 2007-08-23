/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGSERIES_H
#define UDGSERIES_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QPixmap>
#include "identifier.h"

namespace udg {

class Volume;
class Image;
class Study;

/**
Classe que encapsula la sèrie d'un pacient.

La classe conté tot tipu d'informació relacionada amb la sèrie d'un pacient. Una sèrie pot equivaler a un o més volums, per tant tindrem la llista de Volums corresponents a la sèrie.

	@author Grup de Gràfics de Girona  ( GGG ) <vismed@ima.udg.es>
*/
class Series : public QObject
{
Q_OBJECT
public:
    Series(QObject *parent = 0);

    ~Series();

    /// Assignar/Obtenir l'identificador universal de la sèrie
    void setInstanceUID( QString uid );
    QString getInstanceUID() const;

    /// Assignar/Obtenir la modalitat de la sèrie
    void setModality( QString modality );
    QString getModality() const;

    /// Assignar/Obtenir el número de la sèrie
    void setSeriesNumber( QString number );
    QString getSeriesNumber() const;

    /// Assignar/Obtenir el FrameOfReferenceUID
    void setFrameOfReferenceUID( QString uid );
    QString getFrameOfReferenceUID() const;

    /// Assignar/Obtenir el PositionReferenceIndicator
    void setPositionReferenceIndicator( QString position );
    QString getPositionReferenceIndicator() const;

    /// Assignar/Obtenir la descripció de la sèrie
    void setDescription( QString description );
    QString getDescription() const;

    /// Assignar/Obtenir la posició del pacient relativa a la màquina
    void setPatientPosition( QString position );
    QString getPatientPosition() const;

    /// Assignar/Obtenir el protocol de la sèrie
    void setProtocolName( QString protocolName );
    QString getProtocolName() const;
    
    /// Assignar/Obtenir la data i hora d'adquisició de la sèrie. El format de la data serà YYYYMMDD i el del
    /// time hhmmss.frac on frac és una fracció de segon de rang 000000-999999
    ///  Retorna fals si hi ha algun error en el format
    bool setDateTime( int day , int month , int year , int hour , int minute, int second = 0 );
    bool setDateTime( QString date , QString time );
    bool setDate( int day , int month , int year );
    bool setDate( QString date );
    bool setDate( QDate date );
    bool setTime( int hour , int minute, int second = 0 );
    bool setTime( QString time );
    bool setTime( QTime time );
    QDate getDate();
    QString getDateAsString();
    QTime getTime();
    QString getTimeAsString();

    /// Assignar/Obtenir la institució on s'ha realitzat l'estudi
    void setInstitutionName( QString institutionName );
    QString getInstitutionName() const;

    /**
     * Assigna/Retorna la part del cos examinada
     */
    void setBodyPartExamined( QString bodyPart );
    QString getBodyPartExamined() const;

    /**
     * Assigna/Retorna la vista radiogràfica associada amb la posició del pacient
     */
    void setViewPosition( QString viewPosition );
    QString getViewPosition() const;
    
    /// Assignar/Obtenir el número de fases
    void setNumberOfPhases( int phases );
    int getNumberOfPhases() const;

    /// Assignar/Obtenir el número de llesques per fases
    void setNumberOfSlicesPerPhase( int slices );
    int getNumberOfSlicesPerPhase() const;

    /// assigna l'estudi pare de la sèrie
    void setParentStudy( Study *study );
    Study *getParentStudy() const;

    /// afegeix un objecte imatge a la sèrie i li assigna com a parent aquest objecte series.
    /// Si la imatge ja existeix al conjunt retorna fals, cert altrament
    bool addImage( Image *image );

    /// obté l'objecte imatge pel sopInstanceUID donat. Si no existeix cap imatge amb aquest UID es retorna NUL
    Image *getImage( QString SOPInstanceUID );

    /**
     * Ens diu si existeix una imatge amb aquest sopInstanceUID a la llista
     * @param sopInstanceUID l'uid que busquem
     * @return Cert si existeix, fals altrament
     */
    bool imageExists( QString sopInstanceUID );

    /// Retorna una llista de totes les imatges de la sèrie
    QList<Image *> getImages() const;

    /**
     * Ens diu quantes imatges té aquesta sèrie
     * @return El nombre d'imatges. 0 en cas que no sigui una sèrie d'imatges o no en contingui
     */
    int getNumberOfImages();
    
    /// Indica si una sèrie té imatges
    bool hasImages() const;

    /// Assignar/Obtenir el path de les imatges de la sèrie
    void setImagesPath( QString imagesPath );
    QString getImagesPath() const;

    /// Assignar/Obtenir identificador del volum al repositori corresponent a la sèrie
    //\TODO estem assumint que un volum = una sèrie i això no és del tot cert. L'id, en tot cas, hauria d'anar relacionat amb el subvolum
    void setVolumeIdentifier( Identifier id );
    Identifier getVolumeIdentifier() const;

    /// Retorna el nombre de volums dels que es composa la sèrie.
    int getNumberOfVolumes();

    /// Retorna el subvolum amb índex 'index', per defecte, el 0
    Volume *getVolume( int index = 0 );

    /// Afegeix un fitxer a la sèrie
    void addFilePath(QString filePath);

    /// Esborra un fitxer de la sèrie
    void removeFilePath(QString filePath);

    /// Retorna la llista de fitxers DICOM que formen la sèrie sense tenir cap ordre en concret.
    QStringList getFilesPathList();

    /// Llista dels noms de fitxer de les imatges
    QStringList getImagesPathList();

    /// Ens diu si la sèrie està marcada com a seleccionada o no
    bool isSelected() const;

    /// Mètode per afegir un sol volum a la llista de volums de la serie \TODO mètode de proves no definitiu
    void setVolume(Volume * volume);

    QString toString(bool verbose = false);

    /// Assigna/Obté la imatge de previsualització de la sèrie
    void setThumbnail( QPixmap thumb );
    QPixmap getThumbnail() const;

public slots:
    /// Selecciona/deselecciona aquesta sèrie
    void select();
    void unSelect();
    void setSelectStatus(bool select);

private:
    /**
     * Inserta una imatge a la llista d'imatges ordenat pel criteri d'ordenació d'imatges
     * TODO falta definir quina és l'estrategia d'ordenació per defecte
     * Pre: se presuposa que s'ha comprovat anteriorment que la imatge no existeix a la llista
     * @param image
     */
    void insertImage( Image *image );

    /**
     * Troba l'índex de la imatge amb el sopInstanceUID donat a la llista d'imatges
     * @param sopInstanceUID l'uid de la imatge que volem trobar
     * @return L'índex d'aquella imatge dins de la llista, -1 si no existeix la imatge amb aquell uid.
     */
    int findImageIndex( QString sopInstanceUID );

private:
    /// Informació comuna de la sèrie. C.7.3.1 General Series Module - PS 3.3.

    /// Tipus d'equipament que originalment va adquirir les dades per crear les imatges creades en aquesta sèrie.
    /// Veure C.7.3.1.1.1 pels termes definits. (0008,0060) Tipus 1
    QString m_modality;

    /// Identidicador universal de la sèrie. (0020,000E) Tipus 1
    QString m_seriesInstanceUID;

    /// Nombre que identifica la sèrie. (0020,0011) Tipus 2
    QString m_seriesNumber;

    /// Data i hora en la que s'ha començat la sèrie. (0008,0021),(0008,0031) Tipus 3.
    QDate m_date;
    QTime m_time;

    //\TODO aquest tag està dins de l'Equipment IE module. Se suposa que és mes correcte posar-ho a nivell de sèrie.
    //Llegir A.1.2.3 SERIES IE, apartat c., A.1.2.4 EQUIPMENT IE i C.7.5 Common Equipment IE Modules - PS 3.3
    /// Nom de l'institució en la que s'ha fet
    QString m_institutionName;

    /// Descriptor de la posició del pacient relativa a la màquina. Requerit per imatges CT i MR. No hi ha d'estar present
    /// si Patient Orientation Code Sequence (0054,0410) està present. Veure C.7.3.1.1.2 per termes definits i una explicació més profunda.
    /// (0018,5100) Tipus 2C
    QString m_patientPosition;

    /// Protocol que s'ha aplicat per obtenir les imatges definit per l'usuari. (0018,1030) Tipus 3
    QString m_protocolName;

    /// Descripció de la sèrie definida per l'usuari. (0008,103E) Tipus 3
    QString m_description;

    // FRAME OF REFERENCE MODULE. C.7.4
    //\TODO En principi no cal implementar-ho com a una entitat per separat i incloent-ho dins de sèries n'hauria d'haver
    //prou per poder tractar el que necessitem
    /// Identifica el frame of reference universalment. Veure C.7.4.1.1.1 per una explicació més profunda. (0020,0052) Tipus 1
    QString m_frameOfReferenceUID;

    /// Part de l'anatomia del pacient usat com a referència. Veure C.7.4.1.1.2, només per propòsits d'annotació. (0020,1040) Tipus 2
    QString m_positionReferenceIndicator;

    /// Indica si la sèrie està marcada com a seleccionada o no
    bool m_selected;

    /// Directori sota el qual ens trobem les imatges
    QString m_imagesPath;

    /// Imatge de previsualització associada a la sèrie
    QPixmap m_previewImage;

    /// Identificador del volum al repositori
    Identifier m_volumeID;

    /// Llista de volums que composen la sèrie. La sèrie es pot separar en diversos volums per diverses raons,
    /// com pot ser mides d'imatge diferent, sèries amb dinàmics o fases, stacks, etc.
    QList<Volume *> m_volumesList;

    /// Llista de les Image de la serie ordenades per criteris d'ordenació com SliceLocation,InstanceNumber, etc
    /// TODO falta definir quina és l'estrategia d'ordenació per defecte
    QList<Image *> m_imageSet;

    QList<QString> m_filesPathList;

    /// Estudi pare
    Study *m_parentStudy;

    /// Número de fases i número de llesques per fase per poder tractar sèries dinàmiques
    int m_numberOfPhases, m_numberOfSlicesPerPhase;

    /// Part del cos examinada. (0018,015). Tipus 2/3, segons modalitat
    QString m_bodyPartExamined;

    /// Vista radiogràfica associada amb la posició del pacient[(0018,5101)]. (0018,5101). Tipus 2/3, segons modalitat
    QString m_viewPosition;
};

}

#endif
