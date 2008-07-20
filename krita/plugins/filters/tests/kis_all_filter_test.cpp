/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_all_filter_test.h"
#include <qtest_kde.h>
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_selection.h"
#include "kis_processing_information.h"
#include "filter/kis_filter.h"
#include "kis_threaded_applicator.h"
#include "kis_selection.h"
#include "kis_pixel_selection.h"
#include <KoColorSpaceRegistry.h>

bool compareQImages( QPoint & pt, const QImage & img1, const QImage & img2 )
{
//     QTime t;
//     t.start();

    int w1 = img1.width();
    int h1 = img1.height();
    int w2 = img2.width();
    int h2 = img2.height();

    if ( w1 != w2 || h1 != h2 ) {
        pt.setX( -1 );
        pt.setY( -1 );
        return false;
    }

    for ( int x = 0; x < w1; ++x ) {
        for ( int y = 0; y < h1; ++y ) {
            if ( img1.pixel(x, y) != img2.pixel( x, y ) ) {
                pt.setX( x );
                pt.setY( y );
                return false;
            }
        }
    }
//     qDebug() << "compareQImages time elapsed:" << t.elapsed();
    return true;
}

bool testFilter(KisFilterSP f)
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QImage qimg( QString(FILES_DATA_DIR) + QDir::separator() + "lena.png");
    QImage result( QString(FILES_DATA_DIR) + QDir::separator() + "lena_" + f->id() + ".png" );
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(qimg, "", 0, 0);

    // Get the predefined configuration from a file
    KisFilterConfiguration * kfc = f->defaultConfiguration(dev);

    QFile file(QString(FILES_DATA_DIR) + QDir::separator() + f->id() + ".cfg");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //qDebug() << "creating new file for " << f->id();
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out << kfc->toXML();
    }
    else {
        QString s;
        QTextStream in(&file);
        s = in.readAll();
        //qDebug() << "Read for " << f->id() << "\n" << s;
        kfc->fromXML( s );
    }
    qDebug() << f->id();// << "\n" << kfc->toXML() << "\n";

    KisConstProcessingInformation src( dev,  QPoint(0, 0), 0 );
    KisProcessingInformation dst( dev, QPoint(0, 0), 0 );

    f->process(src, dst, qimg.size(), kfc);

    QPoint errpoint;

    if ( !compareQImages( errpoint, result, dev->convertToQImage(0, 0, 0, qimg.width(), qimg.height() ) ) ) {
        dev->convertToQImage(0, 0, 0, qimg.width(), qimg.height()).save(QString("lena_%1.png").arg(f->id()));
        return false;
    }
    return true;
}


bool testFilterWithSelections(KisFilterSP f)
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QImage qimg( QString(FILES_DATA_DIR) + QDir::separator() + "lena.png");
    QImage result( QString(FILES_DATA_DIR) + QDir::separator() + "lena_" + f->id() + ".png" );
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(qimg, "", 0, 0);

    // Get the predefined configuration from a file
    KisFilterConfiguration * kfc = f->defaultConfiguration(dev);

    QFile file(QString(FILES_DATA_DIR) + QDir::separator() + f->id() + ".cfg");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //qDebug() << "creating new file for " << f->id();
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out << kfc->toXML();
    }
    else {
        QString s;
        QTextStream in(&file);
        s = in.readAll();
        //qDebug() << "Read for " << f->id() << "\n" << s;
        kfc->fromXML( s );
    }
    qDebug() << f->id();// << "\n"; << kfc->toXML() << "\n";

    KisSelectionSP sel1 = new KisSelection(dev);
    sel1->getOrCreatePixelSelection()->select( qimg.rect() );

    KisSelectionSP sel2 = new KisSelection(dev);
    sel2->getOrCreatePixelSelection()->select( qimg.rect() );

    KisConstProcessingInformation src( dev,  QPoint(0, 0), sel1 );
    KisProcessingInformation dst( dev, QPoint(0, 0), sel2 );

    f->process(src, dst, qimg.size(), kfc);

    QPoint errpoint;

    if ( !compareQImages( errpoint, result, dev->convertToQImage(0, 0, 0, qimg.width(), qimg.height() ) ) ) {
        dev->convertToQImage(0, 0, 0, qimg.width(), qimg.height()).save(QString("sel_lena_%1.png").arg(f->id()));
        return false;
    }


    return true;
}

void KisAllFilterTest::testAllFilters()
{
    QStringList failures;
    QStringList successes;

    QList<QString> filterList = KisFilterRegistry::instance()->keys();
    for ( QList<QString>::Iterator it = filterList.begin(); it != filterList.end(); ++it ) {
        if (testFilter(KisFilterRegistry::instance()->value(*it)))
            successes << *it;
        else
            failures << *it;
    }
    qDebug() << "Success: " << successes;
    if (failures.size() > 0) {
        QFAIL(QString("Failed filters:\n\t %1").arg(failures.join("\n\t")).toAscii());
    }
}

void KisAllFilterTest::testAllFiltersWithSelections()
{
    QStringList failures;
    QStringList successes;

    QList<QString> filterList = KisFilterRegistry::instance()->keys();
    for ( QList<QString>::Iterator it = filterList.begin(); it != filterList.end(); ++it ) {
        if (testFilterWithSelections(KisFilterRegistry::instance()->value(*it)))
            successes << *it;
        else
            failures << *it;
    }
    qDebug() << "Success: " << successes;
    if (failures.size() > 0) {
        QFAIL(QString("Failed filters with selections:\n\t %1").arg(failures.join("\n\t")).toAscii());
    }
}



QTEST_KDEMAIN(KisAllFilterTest, GUI)
#include "kis_all_filter_test.moc"
