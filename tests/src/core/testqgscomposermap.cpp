/***************************************************************************
                         testqgscomposermap.cpp
                         ----------------------
    begin                : Juli 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgscomposition.h"
#include "qgsmultirenderchecker.h"
#include "qgscomposermap.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsproject.h"
#include "qgsmapthemecollection.h"
#include <QObject>
#include "qgstest.h"

class TestQgsComposerMap : public QObject
{
    Q_OBJECT

  public:
    TestQgsComposerMap()
        : mComposition( 0 )
        , mComposerMap( 0 )
        , mMapSettings( 0 )
        , mRasterLayer( 0 )
        , mPointsLayer( 0 )
        , mPolysLayer( 0 )
        , mLinesLayer( 0 )
    {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void render(); //test if rendering of the composition with composr map is correct
    void uniqueId(); //test if map id is adapted when doing copy paste
    void worldFileGeneration(); // test world file generation
    void mapPolygonVertices(); // test mapPolygon function with no map rotation
    void dataDefinedLayers(); //test data defined layer string
    void dataDefinedStyles(); //test data defined styles

  private:
    QgsComposition *mComposition;
    QgsComposerMap *mComposerMap;
    QgsMapSettings *mMapSettings;
    QgsRasterLayer* mRasterLayer;
    QgsVectorLayer* mPointsLayer;
    QgsVectorLayer* mPolysLayer;
    QgsVectorLayer* mLinesLayer;
    QString mReport;
};

void TestQgsComposerMap::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  QFileInfo rasterFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" );
  mRasterLayer = new QgsRasterLayer( rasterFileInfo.filePath(),
                                     rasterFileInfo.completeBaseName() );
  QgsMultiBandColorRenderer* rasterRenderer = new QgsMultiBandColorRenderer( mRasterLayer->dataProvider(), 2, 3, 4 );
  mRasterLayer->setRenderer( rasterRenderer );

  QFileInfo pointFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/points.shp" );
  mPointsLayer = new QgsVectorLayer( pointFileInfo.filePath(),
                                     pointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  QFileInfo polyFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/polys.shp" );
  mPolysLayer = new QgsVectorLayer( polyFileInfo.filePath(),
                                    polyFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  QFileInfo lineFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/lines.shp" );
  mLinesLayer = new QgsVectorLayer( lineFileInfo.filePath(),
                                    lineFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  // some layers need to be in project for data-defined layers functionality
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer*>() << mRasterLayer << mPointsLayer << mPolysLayer << mLinesLayer );
}

void TestQgsComposerMap::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  QgsApplication::exitQgis();
}

void TestQgsComposerMap::init()
{
  mMapSettings = new QgsMapSettings();

  //create composition with composer map
  mMapSettings->setLayers( QList<QgsMapLayer*>() << mRasterLayer );
  mMapSettings->setCrsTransformEnabled( false );
  mComposition = new QgsComposition( *mMapSettings, QgsProject::instance() );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposerMap = new QgsComposerMap( mComposition, 20, 20, 200, 100 );
  mComposerMap->setFrameEnabled( true );
  mComposition->addComposerMap( mComposerMap );

  mReport = QStringLiteral( "<h1>Composer Map Tests</h1>\n" );
}

void TestQgsComposerMap::cleanup()
{
  delete mComposition;
  delete mMapSettings;
}

void TestQgsComposerMap::render()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  QgsCompositionChecker checker( QStringLiteral( "composermap_render" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_map" ) );

  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
}

void TestQgsComposerMap::uniqueId()
{
  QDomDocument doc;
  QDomElement documentElement = doc.createElement( QStringLiteral( "ComposerItemClipboard" ) );
  mComposerMap->writeXml( documentElement, doc );
  mComposition->addItemsFromXml( documentElement, doc, 0, false );

  //test if both composer maps have different ids
  const QgsComposerMap* newMap = 0;
  QList<const QgsComposerMap*> mapList = mComposition->composerMapItems();
  QList<const QgsComposerMap*>::const_iterator mapIt = mapList.constBegin();
  for ( ; mapIt != mapList.constEnd(); ++mapIt )
  {
    if ( *mapIt != mComposerMap )
    {
      newMap = *mapIt;
      break;
    }
  }

  QVERIFY( newMap );

  int oldId = mComposerMap->id();
  int newId = newMap->id();

  mComposition->removeComposerItem( const_cast<QgsComposerMap*>( newMap ) );

  QVERIFY( oldId != newId );
}

void TestQgsComposerMap::worldFileGeneration()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  mComposerMap->setMapRotation( 30.0 );

  mComposition->setGenerateWorldFile( true );
  mComposition->setWorldFileMap( mComposerMap );

  double a, b, c, d, e, f;
  mComposition->computeWorldFileParameters( a, b, c, d, e, f );

  QVERIFY( qgsDoubleNear( a, 4.18048, 0.001 ) );
  QVERIFY( qgsDoubleNear( b, 2.41331, 0.001 ) );
  QVERIFY( qgsDoubleNear( c, 779444, 1 ) );
  QVERIFY( qgsDoubleNear( d, 2.4136, 0.001 ) );
  QVERIFY( qgsDoubleNear( e, -4.17997, 0.001 ) );
  QVERIFY( qgsDoubleNear( f, 3.34241e+06, 1e+03 ) );

  //test with map on second page. Parameters should be the same
  mComposerMap->setItemPosition( 20, 20, QgsComposerItem::UpperLeft, 2 );
  mComposition->computeWorldFileParameters( a, b, c, d, e, f );

  QVERIFY( qgsDoubleNear( a, 4.18048, 0.001 ) );
  QVERIFY( qgsDoubleNear( b, 2.41331, 0.001 ) );
  QVERIFY( qgsDoubleNear( c, 779444, 1 ) );
  QVERIFY( qgsDoubleNear( d, 2.4136, 0.001 ) );
  QVERIFY( qgsDoubleNear( e, -4.17997, 0.001 ) );
  QVERIFY( qgsDoubleNear( f, 3.34241e+06, 1e+03 ) );

  //test computing parameters for specific region
  mComposerMap->setItemPosition( 20, 20, QgsComposerItem::UpperLeft, 2 );
  mComposition->computeWorldFileParameters( QRectF( 10, 5, 260, 200 ), a, b, c, d, e, f );

  QVERIFY( qgsDoubleNear( a, 4.18061, 0.001 ) );
  QVERIFY( qgsDoubleNear( b, 2.41321, 0.001 ) );
  QVERIFY( qgsDoubleNear( c, 773810, 1 ) );
  QVERIFY( qgsDoubleNear( d, 2.4137, 0.001 ) );
  QVERIFY( qgsDoubleNear( e, -4.1798, 0.001 ) );
  QVERIFY( qgsDoubleNear( f, 3.35331e+06, 1e+03 ) );

  mComposition->setGenerateWorldFile( false );
  mComposerMap->setMapRotation( 0.0 );

}

void TestQgsComposerMap::mapPolygonVertices()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  QPolygonF visibleExtent = mComposerMap->visibleExtentPolygon();

  //vertices should be returned in clockwise order starting at the top-left point
  QVERIFY( fabs( visibleExtent[0].x() - 781662.375 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[0].y() - 3345223.125 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[1].x() - 793062.375 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[1].y() - 3345223.125 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[2].x() - 793062.375 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[2].y() - 3339523.125 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[3].x() - 781662.375 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[3].y() - 3339523.125 ) < 0.001 );

  //polygon should be closed
  QVERIFY( visibleExtent.isClosed() );

  //now test with rotated map
  mComposerMap->setMapRotation( 10 );
  visibleExtent = mComposerMap->visibleExtentPolygon();

  //vertices should be returned in clockwise order starting at the top-left point
  QVERIFY( fabs( visibleExtent[0].x() - 781254.0735015 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[0].y() - 3344190.0324834 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[1].x() - 792480.881886 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[1].y() - 3346169.62171 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[2].x() - 793470.676499 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[2].y() - 3340556.21752 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[3].x() - 782243.868114 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[3].y() - 3338576.62829 ) < 0.001 );

  //polygon should be closed
  QVERIFY( visibleExtent.isClosed() );

  mComposerMap->setMapRotation( 0 );

}

void TestQgsComposerMap::dataDefinedLayers()
{
  delete mComposition;
  QgsMapSettings ms;
  ms.setLayers( QList<QgsMapLayer*>() << mRasterLayer << mPolysLayer << mPointsLayer << mLinesLayer );
  ms.setCrsTransformEnabled( true );

  mComposition = new QgsComposition( ms, QgsProject::instance() );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposerMap = new QgsComposerMap( mComposition, 20, 20, 200, 100 );
  mComposerMap->setFrameEnabled( true );
  mComposition->addComposerMap( mComposerMap );

  //test malformed layer set string
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapLayers, true, true, QStringLiteral( "'x'" ), QString() );
  QList<QgsMapLayer*> result = mComposerMap->layersToRender();
  QVERIFY( result.isEmpty() );

  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapLayers, true, true, QStringLiteral( "'x|'" ), QString() );
  result = mComposerMap->layersToRender();
  QVERIFY( result.isEmpty() );

  //test subset of valid layers
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapLayers, true, true,
                                        QStringLiteral( "'%1|%2'" ).arg( mPolysLayer->name(), mRasterLayer->name() ), QString() );
  result = mComposerMap->layersToRender();
  QCOMPARE( result.count(), 2 );
  QVERIFY( result.contains( mPolysLayer ) );
  QVERIFY( result.contains( mRasterLayer ) );

  //test non-existant layer
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapLayers, true, true,
                                        QStringLiteral( "'x|%1|%2'" ).arg( mLinesLayer->name(), mPointsLayer->name() ), QString() );
  result = mComposerMap->layersToRender();
  QCOMPARE( result.count(), 2 );
  QVERIFY( result.contains( mLinesLayer ) );
  QVERIFY( result.contains( mPointsLayer ) );

  //test no layers
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapLayers, true, true,
                                        QStringLiteral( "''" ), QString() );
  result = mComposerMap->layersToRender();
  QVERIFY( result.isEmpty() );


  //test with atlas feature evaluation
  QgsVectorLayer* atlasLayer = new QgsVectorLayer( QStringLiteral( "Point?field=col1:string" ), QStringLiteral( "atlas" ), QStringLiteral( "memory" ) );
  QVERIFY( atlasLayer->isValid() );
  QgsFeature f1( atlasLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), mLinesLayer->name() );
  QgsFeature f2( atlasLayer->dataProvider()->fields(), 1 );
  f2.setAttribute( QStringLiteral( "col1" ), mPointsLayer->name() );
  atlasLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 );
  mComposition->atlasComposition().setCoverageLayer( atlasLayer );
  mComposition->atlasComposition().setEnabled( true );
  mComposition->setAtlasMode( QgsComposition::ExportAtlas );
  mComposition->atlasComposition().beginRender();
  mComposition->atlasComposition().prepareForFeature( 0 );

  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapLayers, true, true, QStringLiteral( "\"col1\"" ), QString() );
  result = mComposerMap->layersToRender();
  QCOMPARE( result.count(), 1 );
  QCOMPARE( result.at( 0 ), mLinesLayer );
  mComposition->atlasComposition().prepareForFeature( 1 );
  result = mComposerMap->layersToRender();
  QCOMPARE( result.count(), 1 );
  QCOMPARE( result.at( 0 ), mPointsLayer );
  mComposition->atlasComposition().setEnabled( false );
  delete atlasLayer;

  //render test
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapLayers, true, true,
                                        QStringLiteral( "'%1|%2'" ).arg( mPolysLayer->name(), mPointsLayer->name() ), QString() );
  mComposerMap->setNewExtent( QgsRectangle( -110.0, 25.0, -90, 40.0 ) );

  QgsCompositionChecker checker( QStringLiteral( "composermap_ddlayers" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_map" ) );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
}

void TestQgsComposerMap::dataDefinedStyles()
{
  delete mComposition;
  QgsMapSettings ms;
  ms.setLayers( QList<QgsMapLayer*>() << mRasterLayer << mPolysLayer << mPointsLayer << mLinesLayer );
  ms.setCrsTransformEnabled( true );

  mComposition = new QgsComposition( ms, QgsProject::instance() );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposerMap = new QgsComposerMap( mComposition, 20, 20, 200, 100 );
  mComposerMap->setFrameEnabled( true );
  mComposition->addComposerMap( mComposerMap );

  QgsMapThemeCollection::MapThemeRecord rec;
  rec.setLayerRecords( QList<QgsMapThemeCollection::MapThemeLayerRecord>()
                       << QgsMapThemeCollection::MapThemeLayerRecord( mPointsLayer )
                       << QgsMapThemeCollection::MapThemeLayerRecord( mLinesLayer )
                     );

  QgsProject::instance()->mapThemeCollection()->insert( QStringLiteral( "test preset" ), rec );

  // test following of preset
  mComposerMap->setFollowVisibilityPreset( true );
  mComposerMap->setFollowVisibilityPresetName( QStringLiteral( "test preset" ) );
  QSet<QgsMapLayer*> result = mComposerMap->layersToRender().toSet();
  QCOMPARE( result.count(), 2 );
  mComposerMap->setFollowVisibilityPresetName( QString() );

  //test malformed style string
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapStylePreset, true, true, QStringLiteral( "5" ), QString() );
  result = mComposerMap->layersToRender().toSet();
  QCOMPARE( result, ms.layers().toSet() );

  //test valid preset
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapStylePreset, true, true, QStringLiteral( "'test preset'" ), QString() );
  result = mComposerMap->layersToRender().toSet();
  QCOMPARE( result.count(), 2 );
  QVERIFY( result.contains( mLinesLayer ) );
  QVERIFY( result.contains( mPointsLayer ) );

  //test non-existant preset
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapStylePreset, true, true,
                                        QStringLiteral( "'bad preset'" ), QString() );
  result = mComposerMap->layersToRender().toSet();
  QCOMPARE( result, ms.layers().toSet() );

  //test that dd layer set overrides style layers
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapStylePreset, true, true, QStringLiteral( "'test preset'" ), QString() );
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapLayers, true, true,
                                        QStringLiteral( "'%1'" ).arg( mPolysLayer->name() ), QString() );
  result = mComposerMap->layersToRender().toSet();
  QCOMPARE( result.count(), 1 );
  QVERIFY( result.contains( mPolysLayer ) );
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapLayers, false, true, QString(), QString() );

  //render test
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapStylePreset, true, true,
                                        QStringLiteral( "'test preset'" ), QString() );
  mComposerMap->setNewExtent( QgsRectangle( -110.0, 25.0, -90, 40.0 ) );

  QgsCompositionChecker checker( QStringLiteral( "composermap_ddstyles" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_map" ) );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
}

QGSTEST_MAIN( TestQgsComposerMap )
#include "testqgscomposermap.moc"
