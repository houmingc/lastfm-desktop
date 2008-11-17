/***************************************************************************
 *   Copyright (C) 2007 by                                                 *
 *      Last.fm Ltd <client@last.fm>                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/

/*! \class LocalCollection
    \brief Local Content Resolution db interface.
    Db stores metadata for on-disk media
*/

#ifndef LOCAL_COLLECTION_H
#define LOCAL_COLLECTION_H

#include <QObject>
#include <QMutex>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "ChainableQuery.h"
#include "lib/lastfm/core/WeightedStringList.h"


class LocalCollection : public QObject
{
    Q_OBJECT

public:
    class File
    {
    public:
        File(int id, QString name, unsigned lastModified)
            :m_id(id)
            ,m_name(name)
            ,m_lastModified(lastModified)
        {}

        int id() const { return m_id; }
        QString name() const { return m_name; }
        unsigned lastModified() const { return m_lastModified; }

    private:
        int m_id;
        QString m_name;
        unsigned m_lastModified;
    };

    class Source
    {
    public:
        Source(int id, const QString& volume, bool available)
            :m_id(id)
            ,m_volume(volume)
            ,m_available(available)
        {}

        int m_id;
        QString m_volume;
        bool m_available;
    };

    class Exclusion
    {
        QString m_path;
        bool m_bSubdirs;    // subdirectories excluded too

    public:
        Exclusion(const QString path, bool bSubdirs = true);
        bool operator==(const Exclusion& that) const;
        bool subdirsToo() const;
    };

    class FileMeta
    {
    public:
        FileMeta(const QString& artist, const QString& album, const QString& title, 
            unsigned kbps, unsigned duration)
            : m_artist( artist )
            , m_album( album )
            , m_title( title )
            , m_kbps( kbps )
            , m_duration ( duration )
        {}

        QString m_artist;
        QString m_album;
        QString m_title;
        unsigned m_kbps;
        unsigned m_duration;
    };

    class ResolveResult : public FileMeta
    {
    public:
        ResolveResult(
            const QString& artist, 
            const QString& album, 
            const QString& title, 
            float artistMatch, 
            float titleMatch,
            const QString& filename, 
            unsigned kbps, 
            unsigned duration, 
            const QString& path, 
            const QString& sourcename)
            : FileMeta( artist, album, title, kbps, duration )
            , m_matchQuality ( artistMatch * titleMatch )
            , m_filename( filename )
            , m_path( path )
            , m_sourcename ( sourcename )
        {}

        float m_matchQuality;
        QString m_filename;
        QString m_path;
        QString m_sourcename;
    };

    static LocalCollection* create(QString connectionName);
    ~LocalCollection();

    void versionCheck();

    /** \brief Temp method: Gets a fingerprint id. Returns -1 if none found. */
    QString getFingerprint( const QString& filePath );
    /** \brief Temp method: Sets a fingerprint id. */
    void setFingerprint( const QString& filePath, QString fpId );

    LocalCollection::Source addSource(const QString& volume);
    QList<Source> getAllSources();
    void setSourceAvailability(int sourceId, bool bAvailable);
    bool removeSource();
    bool getSourceId(QString name, int &result);

    bool getDirectoryId(int sourceId, QString path, int &result);

    int getArtistId(QString artistName, bool bCreate);
    void updateArtist(int artistId);

    bool addDirectory(int sourceId, QString path, int &resultId);
    void removeDirectory(int directoryId);
    QList<Exclusion> getExcludedDirectories(int sourceId);
    QList<QString> getStartDirectories(int sourceId);

    QList<File> getFiles(int directoryId);
    void updateFile(int fileId, unsigned lastModified, const FileMeta& info);
    void addFile(int directoryId, QString name, unsigned lastModified, const FileMeta& info);
    void removeFiles(QList<int> ids);

    QList<LocalCollection::ResolveResult> resolve(const QString artist, const QString album, const QString title);

    // tag handling
    int getTagId(QString tag, bool bCreate);
    QStringList artistsWithExpiredTags();
    QStringList artistsNeedingTagUpdate();

    void setGlobalTagsForArtist(QString artist, WeightedStringList globalTags);
    void setUserTagsForArtist(QString artist, QStringList userTags, unsigned userId);
    void updateArtistDownload(QString artist, QDateTime nextDlTime, QDateTime dlTime = QDateTime());

    void deleteUserTrackTagsForArtist(int artistId, unsigned userId);
    void deleteGlobalTrackTagsForArtist(int artistId);
    void deleteTrackTagsForArtist(int artistId, unsigned userId);

    void insertUserArtistTag(int artistId, int tagId, unsigned userId);
    void insertGlobalArtistTag(int artistId, int tagId, int weight);
    void insertTrackTag(int artistId, int tagId, unsigned userId, int weight);


private:
    LocalCollection(QString connectionName);

    /** the database version
        * version 1: from 2.?.? */
    int version() const;
    void initDatabase();
    QSqlQuery query( const QString& sql ) const;
    ChainableQuery prepare( const QString& sql ) const;

    static QMutex m_mutex;

    QSqlDatabase m_db;
    QString m_dbPath;
    QString m_connectionName;
};

#endif // COLLECTION_H
