#ifndef UDGIMAGE_H
#define UDGIMAGE_H

#include <QObject>
#include <QDateTime>
#include <QList>
#include <QPair>
#include <QStringList>
#include <QPixmap>

#include "dicomsource.h"
#include "imageorientation.h"
#include "patientorientation.h"

namespace udg {

class Series;

/**
    Classe que encapsula les propietats d'una imatge d'una sèrie de la classe Series
  */
class Image : public QObject {
Q_OBJECT
public:
    Image(QObject *parent = 0);
    ~Image();

    /// Assigna/obté el SOPInstanceUID de la imatge
    void setSOPInstanceUID(const QString &uid);
    QString getSOPInstanceUID() const;

    /// Assigna/obté l'instance number
    void setInstanceNumber(const QString &number);
    QString getInstanceNumber() const;

    /**
     * Assignar/Obtenir la orientació del pla de la imatge, també anomenat direction cosines.
     * Els valors són els vectors que formen el pla d'imatge.
     * A partir d'aquests dos vectors, es calcula la normal del pla d'imatge
     * @param orientation[] Els valors dels vectors que defineixen el pla d'imatge.
     */
    void setImageOrientationPatient(const ImageOrientation &imageOrientation);
    ImageOrientation getImageOrientationPatient() const;

    /// Assignar/Obtenir l'orientació del pacient
    void setPatientOrientation(const PatientOrientation &orientation);
    PatientOrientation getPatientOrientation() const;

    /// Assignar/Obtenir l'espaiat dels pixels
    void setPixelSpacing(double x, double y);
    const double* getPixelSpacing() const;

    /// Assignar/Obtenir l'slice thickness, aka espaiat de les Z
    void setSliceThickness(double z);
    double getSliceThickness() const;

    /// Assignar/Obtenir la posició de la imatge.
    void setImagePositionPatient(double position[3]);
    const double* getImagePositionPatient() const;

    /// Assignar/Obtenir els samples per pixel
    void setSamplesPerPixel(int samples);
    int getSamplesPerPixel() const;

    /// Assignar/Obtenir la interpretació fotomètrica
    void setPhotometricInterpretation(const QString &value);
    QString getPhotometricInterpretation() const;

    /// Assignar/Obtenir files/columnes
    void setRows(int rows);
    int getRows() const;
    void setColumns(int columns);
    int getColumns() const;

    /// Assignar/Obtenir els bits allotjats
    void setBitsAllocated(int bits);
    int getBitsAllocated() const;

    /// Assignar/Obtenir els bits emmagatzemats
    void setBitsStored(int bits);
    int getBitsStored() const;

    /// Assignar/Obtenir el bit més alt
    void setHighBit(int highBit);
    int getHighBit() const;

    /// Assignar/Obtenir la representació dels pixels
    void setPixelRepresentation(int representation);
    int getPixelRepresentation() const;

    /// Assignar/Obtenir els valors del rescalat de la MODALITY LUT que s'apliquen sobre la imatge
    /// la fòrmula és f(x) = a*x + b, on 'x' és el valor del pixel de la imatge, 'a' l'Slope i 'b' l'Intercept
    void setRescaleSlope(double slope);
    double getRescaleSlope() const;
    void setRescaleIntercept(double intercept);
    double getRescaleIntercept() const;

    /// Assignar/Obtenir els valors del rescalat de la VOI LUT que s'apliquen sobre la imatge
    void addWindowLevel(double window, double level);
    double getWindowCenter(int index = 0) const;
    double getWindowWidth(int index = 0) const;
    QPair<double, double> getWindowLevel(int index = 0) const;

    /// Ens retorna el nombre de window levels que tenim
    int getNumberOfWindowLevels();

    /// Assignar/Obtenir textes descriptius dels window level
    void addWindowLevelExplanation(const QString &explanation);
    void setWindowLevelExplanations(const QStringList &explanations);
    QString getWindowLevelExplanation(int index = 0) const;

    /// Li indiquem quina és la sèrie pare a la qual pertany
    void setParentSeries(Series *series);
    Series* getParentSeries() const;

    /// Assigna/retorna el path absolut de la imatge
    void setPath(const QString &path);
    QString getPath() const;

    /// Assigna / retorna el slice location de la imatge
    void setSliceLocation(const QString &sliceLocation);
    QString getSliceLocation() const;

    /// Assignar/Obtenir la data i hora en que la sèrie s'ha descarregat a la base de dades Local
    void setRetrievedDate(QDate date);
    void setRetrievedTime(QTime time);
    QDate getRetrievedDate();
    QTime getRetrievedTime();

    /// Assignar/Obtenir la descripció del tipus d'imatge
    void setImageType(const QString &imageType);
    QString getImageType() const;

    /// Assignar/Obtenir la viewPosition
    void setViewPosition(const QString &viewPosition);
    QString getViewPosition() const;

    /// Assignar/Obtenir la lateritat de la imatge
    void setImageLaterality(const QChar &imageLaterality);
    QChar getImageLaterality() const;

    /// Assignar/Obtenir la descripció del View Code. De moment només s'aplicarà per imatges de mammografia.
    void setViewCodeMeaning(const QString &viewCodeMeaning);
    QString getViewCodeMeaning() const;

    /// Assignar/Obtenir el número de frame
    void setFrameNumber(int frameNumber);
    int getFrameNumber() const;

    /// Assignar/Obtenir el número de fase
    void setPhaseNumber (int phaseNumber);
    int getPhaseNumber() const;

    /// Assigna/Obtenir el número de volum al qual pertany la imatge dins la sèrie
    void setVolumeNumberInSeries (int volumeNumberInSeries);
    int getVolumeNumberInSeries() const;

    /// Assignar/Obtenir el número que ocupa la imatge dins volum
    void setOrderNumberInVolume(int orderNumberInVolume);
    int getOrderNumberInVolume() const;

    /// Assignar/Obtenir el Content Time (moment de creació de les dades)
    void setImageTime(const QString &imageTime);
    QString getImageTime() const;

    /// Ens retorna la hora en format hh:mm:ss en que va començar la creació de la imatge
    QString getFormattedImageTime() const;

    /// Assingar/Obtenir el DICOMSource de la imatge. Indica quin és l'origen dels fitxers DICOM que conté la imatge
    void setDICOMSource(const DICOMSource &imageDICOMSource);
    DICOMSource getDICOMSource() const;

    /// Ens retorna la clau que identifica la imatge
    QString getKeyIdentifier() const;

    /**
     * El mètode ens retorna el thumbnail de la imatge. Es crearà el primer cop que es demani
     * @param getFromCache Si és cert intentarà carregar el thumbnail si es troba creat a la cache.
     *                     Altrament, simplement comprobarà que no estigui creat a memòria i prou
     * @param resolution La resolució amb la que volem el thumbnail
     * @return Un QPixmap amb el thumbnail
     */
    QPixmap getThumbnail(bool getFromCache = false, int resolution = 100);

    /// Ens retorna una llista amb les modalitats que suportem com a Image
    static QStringList getSupportedModalities();

private:
    /// Atributs DICOM

    /// Identificador de la imatge/arxiu. (0008,0018)
    QString m_SOPInstanceUID;

    /// Informació general de la imatge. C.7.6 General Image Module - PS 3.3.

    /// Nombre que identifica la imatge. (0020,0013) Tipus 2
    QString m_instanceNumber;

    /// Orientació anatòmica de les files i columnes de la imatge (LR/AP/HF). Requerit si la imatge no requereix Image Orientation(Patient)(0020,0037) i
    /// Image Position(Patient)(0020,0032). Veure C.6.7.1.1.1. (0020,0020) Tipus 2C.
    PatientOrientation m_patientOrientation;

    // TODO Referenced Image Sequence (0008,1140) Tipus 3. Seqüència que referència altres imatges significativament relacionades amb aquestes,
    // com un post-localizer per CT.

    // TODO Icon Image Sequence (0088,0200) Tipus 3. La següent imatge d'icona és representativa d'aquesta imatge. veure C.7.6.1.1.6

    // Image Plane Module C.6.7.2
    /// Distància física entre el centre de cada píxel (row,column) en mm. Veure 10.7.1.3. (0028,0030) Tipus 1
    double m_pixelSpacing[2];

    /// Vectors d'orientació de la imatge respecte al pacient.
    /// Veure C.6.7.2.1.1. (020,0037) Tipus 1.
    ImageOrientation m_imageOrientationPatient;

    /// Posició de la imatge. Les coordenades x,y,z la cantonada superior esquerre (primer pixel transmés) de la imatge, en mm.
    /// Veure C.6.7.2.1.1. (0020,0032) Tipus 1. \TODO aka origen?.
    double m_imagePositionPatient[3];

    /// Gruix de llesca en mm. (0018,0050) Tipus 2.
    double m_sliceThickness;

    // Image Pixel Module C.6.7.3
    /// Nombre de mostres per pixel en la imatge. Veure C.6.7.3.1.1. (0028,0002) Tipus 1.
    int m_samplesPerPixel;

    /// Interpretació fotomètrica (monocrom,color...). Veure C.6.7.3.1.2. (0028,0004) Tipus 1.
    QString m_photometricInterpretation;

    /// Files i columnes de la imatge. (0028,0010),(0028,0011) Tipus 1
    int m_rows;
    int m_columns;

    /// Bits allotjats per cada pixel. Cada mostra ha de tenir el mateix nombre de pixels allotjats. Veure PS 3.5 (0028,0100)
    int m_bitsAllocated;

    /// Bits emmagatzemats per cada pixel. Cada mostra ha de tenir el mateix nombre de pixels emmagatzemats. Veure PS 3.5 (0028,0101)
    int m_bitsStored;

    /// Bit més significant. Veure PS 3.5. (0028,0102) Tipus 1
    int m_highBit;

    /// Representació de cada mostra. Valors enumerats 0000H=unsigned integer, 0001H=complement a 2. (0028,0103) Tipus 1
    int m_pixelRepresentation;

    /// Valors de rescalat de la MODALITY LUT. (0028,1053),(0028,1054). Tipus 1
    double m_rescaleSlope, m_rescaleIntercept;

    /// Valors de rescalat de la VOI LUT. (0028,1050),(0028,1051) Tipus 1C, present si no hi ha VOI LUT Sequence
    /// Com que podem tenir més d'un tindrem una llista
    QList<QPair<double, double> > m_windowLevelList;

    /// "Explicació" dels window levels, texte descriptiu.(0028,1055) Tipus 3.
    QStringList m_windowLevelExplanationList;

    /// Nombre de frames de la imatge. (0028,0008) Tipus 1
    int m_numberOfFrames;

    // TODO millorar definició
    /** Situació especial de la llesca en mm. (0020,1041)
        SC->tipus 3
        NM->tipus 3
        CT-> A la documentació dicom aquest camp no hi figura però philips l'utiliza com a Table Position
      */
    QString m_sliceLocation;

    /// Tipus d'imatge. Ens pot definir si es tracta d'un localizer, per exemple. Conté els valors separats per '\\'
    /// Es troba al mòdul General Image C.7.6.1 i als mòduls Enhanced MR/CT/XA/XRF Image (C.8.13.1/C.8.15.2/C.8.19.2)
    /// En el cas d'imatges Enhanced CT/MR l'omplirem amb el valor FrameType contingut al functional group CT/MR Image Frame Type
    QString m_imageType;

    /**
        Vista radiogràfica associada a Patient Position. El trobem als mòduls CR Series (C.8.1.1) i DX Positioning (C.8.11.5)
        Valors definits:
        AP = Anterior/Posterior
        PA = Posterior/Anterior
        LL = Left Lateral
        RL = Right Lateral
        RLD = Right Lateral Decubitus
        LLD = Left Lateral Decubitus
        RLO = Right Lateral Oblique
        LLO = Left Lateral Oblique
    */
    QString m_viewPosition;

    /**
        Lateralitat de la possiblement aparellada part del cos examinada.
        El trobem als mòduls DX Anatomy (C.8.11.2), Mammography Image (C.8.11.7), Intra-oral Image (C.8.11.9) i Ocular Region Imaged (C.8.17.5)
        També el trobem al mòdul Frame Anatomy (C.7.6.16.2.8) comú a tots els enhanced, però el tag s'anomena Frame Laterality en comptes d'Image Laterality.
        Valors definits:
        R = right
        L = left
        U = unpaired
        B = both left and right
    */
    QChar m_imageLaterality;

    /**
        Descripció del tipus de vista de la imatge. El seu ús l'aplicarem bàsicament pels casos de mammografia definits a
        PS 3.16 - Context ID 4014 (cranio-caudal, medio-lateral oblique, etc...) però podríem extendre el seu ús a d'altres tipus d'imatge
        que també fan ús d'aquest tag per guardar aquest tipus d'informació amb altres possibles valors específics.
    */
    QString m_viewCodeMeaning;

    /// Número de frame
    int m_frameNumber;

    /// Número de fase de la imatge
    int m_phaseNumber;

    /// Número de volum al qual pertany la imatge dins la sèrie
    int m_volumeNumberInSeries;

    /// Número d'ordre de la imatge dins el vo
    int m_orderNumberInVolume;

    /// Moment en el que es va crear el pixel data
    QString m_imageTime;

    // TODO C.7.6.5 CINE MODULE: Multi-frame Cine Image
    /// Atributs NO-DICOM

    /// El path absolut de la imatge
    QString m_path;

    /// Data en que la imatge s'ha descarregat a la base de dades local
    QDate m_retrievedDate;
    QTime m_retrieveTime;

    /// La sèrie pare
    Series *m_parentSeries;

    /// Cache de la imatge de previsualització
    QPixmap m_thumbnail;

    //Indica quin és l'origen de les imatges DICOM
    DICOMSource m_imageDICOMSource;
};

}

#endif
