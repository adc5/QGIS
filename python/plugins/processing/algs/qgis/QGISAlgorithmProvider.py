# -*- coding: utf-8 -*-

"""
***************************************************************************
    QGISAlgorithmProvider.py
    ---------------------
    Date                 : December 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'December 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

try:
    import matplotlib.pyplot
    assert matplotlib  # NOQA silence pyflakes
    hasMatplotlib = True
except:
    hasMatplotlib = False

try:
    import shapely
    assert shapely  # silence pyflakes
    hasShapely = True
except:
    hasShapely = False

from qgis.PyQt.QtGui import QIcon

from qgis.core import Qgis, QgsWkbTypes

from processing.core.AlgorithmProvider import AlgorithmProvider
from processing.script.ScriptUtils import ScriptUtils

from .RegularPoints import RegularPoints
from .SymmetricalDifference import SymmetricalDifference
from .VectorSplit import VectorSplit
from .VectorGridLines import VectorGridLines
from .VectorGridPolygons import VectorGridPolygons
from .RandomExtract import RandomExtract
from .RandomExtractWithinSubsets import RandomExtractWithinSubsets
from .ExtractByLocation import ExtractByLocation
from .ExtractByExpression import ExtractByExpression
from .PointsInPolygon import PointsInPolygon
from .PointsInPolygonUnique import PointsInPolygonUnique
from .PointsInPolygonWeighted import PointsInPolygonWeighted
from .SumLines import SumLines
from .BasicStatisticsNumbers import BasicStatisticsNumbers
from .BasicStatisticsStrings import BasicStatisticsStrings
from .NearestNeighbourAnalysis import NearestNeighbourAnalysis
from .LinesIntersection import LinesIntersection
from .MeanCoords import MeanCoords
from .PointDistance import PointDistance
from .UniqueValues import UniqueValues
from .ReprojectLayer import ReprojectLayer
from .ExportGeometryInfo import ExportGeometryInfo
from .Centroids import Centroids
from .Delaunay import Delaunay
from .VoronoiPolygons import VoronoiPolygons
from .DensifyGeometries import DensifyGeometries
from .MultipartToSingleparts import MultipartToSingleparts
from .SimplifyGeometries import SimplifyGeometries
from .LinesToPolygons import LinesToPolygons
from .PolygonsToLines import PolygonsToLines
from .SinglePartsToMultiparts import SinglePartsToMultiparts
from .ExtractNodes import ExtractNodes
from .ConvexHull import ConvexHull
from .FixedDistanceBuffer import FixedDistanceBuffer
from .VariableDistanceBuffer import VariableDistanceBuffer
from .Clip import Clip
from .Difference import Difference
from .Dissolve import Dissolve
from .Intersection import Intersection
from .ExtentFromLayer import ExtentFromLayer
from .RandomSelection import RandomSelection
from .RandomSelectionWithinSubsets import RandomSelectionWithinSubsets
from .SelectByLocation import SelectByLocation
from .Union import Union
from .DensifyGeometriesInterval import DensifyGeometriesInterval
from .Eliminate import Eliminate
from .SpatialJoin import SpatialJoin
from .DeleteColumn import DeleteColumn
from .DeleteHoles import DeleteHoles
from .DeleteDuplicateGeometries import DeleteDuplicateGeometries
from .TextToFloat import TextToFloat
from .ExtractByAttribute import ExtractByAttribute
from .SelectByAttribute import SelectByAttribute
from .GridPolygon import GridPolygon
from .GridLine import GridLine
from .Gridify import Gridify
from .HubDistancePoints import HubDistancePoints
from .HubDistanceLines import HubDistanceLines
from .HubLines import HubLines
from .Merge import Merge
from .GeometryConvert import GeometryConvert
from .ConcaveHull import ConcaveHull
from .RasterLayerStatistics import RasterLayerStatistics
from .StatisticsByCategories import StatisticsByCategories
from .EquivalentNumField import EquivalentNumField
from .AddTableField import AddTableField
from .FieldsCalculator import FieldsCalculator
from .SaveSelectedFeatures import SaveSelectedFeatures
from .Explode import Explode
from .AutoincrementalField import AutoincrementalField
from .FieldPyculator import FieldsPyculator
from .JoinAttributes import JoinAttributes
from .CreateConstantRaster import CreateConstantRaster
from .PointsLayerFromTable import PointsLayerFromTable
from .PointsDisplacement import PointsDisplacement
from .ZonalStatistics import ZonalStatistics
from .PointsFromPolygons import PointsFromPolygons
from .PointsFromLines import PointsFromLines
from .RandomPointsExtent import RandomPointsExtent
from .RandomPointsLayer import RandomPointsLayer
from .RandomPointsPolygonsFixed import RandomPointsPolygonsFixed
from .RandomPointsPolygonsVariable import RandomPointsPolygonsVariable
from .RandomPointsAlongLines import RandomPointsAlongLines
from .PointsToPaths import PointsToPaths
from .SpatialiteExecuteSQL import SpatialiteExecuteSQL
from .PostGISExecuteSQL import PostGISExecuteSQL
from .ImportIntoSpatialite import ImportIntoSpatialite
from .ImportIntoPostGIS import ImportIntoPostGIS
from .SetVectorStyle import SetVectorStyle
from .SetRasterStyle import SetRasterStyle
from .SelectByExpression import SelectByExpression
from .SelectByAttributeSum import SelectByAttributeSum
from .HypsometricCurves import HypsometricCurves
from .SplitWithLines import SplitWithLines
from .SplitLinesWithLines import SplitLinesWithLines
from .FieldsMapper import FieldsMapper
from .Datasources2Vrt import Datasources2Vrt
from .CheckValidity import CheckValidity
from .OrientedMinimumBoundingBox import OrientedMinimumBoundingBox
from .Smooth import Smooth
from .ReverseLineDirection import ReverseLineDirection
from .SpatialIndex import SpatialIndex
from .DefineProjection import DefineProjection
from .RectanglesOvalsDiamondsVariable import RectanglesOvalsDiamondsVariable
from .RectanglesOvalsDiamondsFixed import RectanglesOvalsDiamondsFixed
from .MergeLines import MergeLines
from .BoundingBox import BoundingBox
from .Boundary import Boundary
from .PointOnSurface import PointOnSurface
from .OffsetLine import OffsetLine
from .PolygonCentroids import PolygonCentroids
from .Translate import Translate
from .SingleSidedBuffer import SingleSidedBuffer
from .PointsAlongGeometry import PointsAlongGeometry
from .Aspect import Aspect
from .Slope import Slope
from .Ruggedness import Ruggedness
from .Hillshade import Hillshade
from .Relief import Relief
from .IdwInterpolation import IdwInterpolation
from .TinInterpolation import TinInterpolation
from .ZonalStatisticsQgis import ZonalStatisticsQgis
from .RemoveNullGeometry import RemoveNullGeometry
from .ExtendLines import ExtendLines
from .ExtractSpecificNodes import ExtractSpecificNodes
from .GeometryByExpression import GeometryByExpression
from .SnapGeometries import SnapGeometriesToLayer
from .PoleOfInaccessibility import PoleOfInaccessibility
from .RasterCalculator import RasterCalculator
from .CreateAttributeIndex import CreateAttributeIndex
from .DropGeometry import DropGeometry
from .BasicStatistics import BasicStatisticsForField
from .Heatmap import Heatmap
from .Orthogonalize import Orthogonalize
from .ShortestPathPointToPoint import ShortestPathPointToPoint
from .ShortestPathPointToLayer import ShortestPathPointToLayer
from .ShortestPathLayerToPoint import ShortestPathLayerToPoint
from .ServiceAreaFromPoint import ServiceAreaFromPoint
from .ServiceAreaFromLayer import ServiceAreaFromLayer
from .TruncateTable import TruncateTable

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))


class QGISAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self._icon = QIcon(os.path.join(pluginPath, 'images', 'qgis.svg'))

        self.alglist = [SumLines(), PointsInPolygon(),
                        PointsInPolygonWeighted(), PointsInPolygonUnique(),
                        BasicStatisticsStrings(), BasicStatisticsNumbers(),
                        NearestNeighbourAnalysis(), MeanCoords(),
                        LinesIntersection(), UniqueValues(), PointDistance(),
                        ReprojectLayer(), ExportGeometryInfo(), Centroids(),
                        Delaunay(), VoronoiPolygons(), SimplifyGeometries(),
                        DensifyGeometries(), DensifyGeometriesInterval(),
                        MultipartToSingleparts(), SinglePartsToMultiparts(),
                        PolygonsToLines(), LinesToPolygons(), ExtractNodes(),
                        Eliminate(), ConvexHull(), FixedDistanceBuffer(),
                        VariableDistanceBuffer(), Dissolve(), Difference(),
                        Intersection(), Union(), Clip(), ExtentFromLayer(),
                        RandomSelection(), RandomSelectionWithinSubsets(),
                        SelectByLocation(), RandomExtract(), DeleteHoles(),
                        RandomExtractWithinSubsets(), ExtractByLocation(),
                        SpatialJoin(), RegularPoints(), SymmetricalDifference(),
                        VectorSplit(), VectorGridLines(), VectorGridPolygons(),
                        DeleteColumn(), DeleteDuplicateGeometries(), TextToFloat(),
                        ExtractByAttribute(), SelectByAttribute(), GridPolygon(),
                        GridLine(), Gridify(), HubDistancePoints(),
                        HubDistanceLines(), HubLines(), Merge(),
                        GeometryConvert(), AddTableField(), FieldsCalculator(),
                        SaveSelectedFeatures(), JoinAttributes(),
                        AutoincrementalField(), Explode(), FieldsPyculator(),
                        EquivalentNumField(), PointsLayerFromTable(),
                        StatisticsByCategories(), ConcaveHull(),
                        RasterLayerStatistics(), PointsDisplacement(),
                        ZonalStatistics(), PointsFromPolygons(),
                        PointsFromLines(), RandomPointsExtent(),
                        RandomPointsLayer(), RandomPointsPolygonsFixed(),
                        RandomPointsPolygonsVariable(),
                        RandomPointsAlongLines(), PointsToPaths(),
                        SpatialiteExecuteSQL(), ImportIntoSpatialite(),
                        PostGISExecuteSQL(), ImportIntoPostGIS(),
                        SetVectorStyle(), SetRasterStyle(),
                        SelectByExpression(), HypsometricCurves(),
                        SplitWithLines(), SplitLinesWithLines(), CreateConstantRaster(),
                        FieldsMapper(), SelectByAttributeSum(), Datasources2Vrt(),
                        CheckValidity(), OrientedMinimumBoundingBox(), Smooth(),
                        ReverseLineDirection(), SpatialIndex(), DefineProjection(),
                        RectanglesOvalsDiamondsVariable(),
                        RectanglesOvalsDiamondsFixed(), MergeLines(),
                        BoundingBox(), Boundary(), PointOnSurface(),
                        OffsetLine(), PolygonCentroids(), Translate(),
                        SingleSidedBuffer(), PointsAlongGeometry(),
                        Aspect(), Slope(), Ruggedness(), Hillshade(),
                        Relief(), ZonalStatisticsQgis(),
                        IdwInterpolation(), TinInterpolation(),
                        RemoveNullGeometry(), ExtractByExpression(),
                        ExtendLines(), ExtractSpecificNodes(),
                        GeometryByExpression(), SnapGeometriesToLayer(),
                        PoleOfInaccessibility(), CreateAttributeIndex(),
                        DropGeometry(), BasicStatisticsForField(),
                        RasterCalculator(), Heatmap(), Orthogonalize(),
                        ShortestPathPointToPoint(), ShortestPathPointToLayer(),
                        ShortestPathLayerToPoint(), ServiceAreaFromPoint(),
                        ServiceAreaFromLayer(), TruncateTable()
                        ]

        if hasMatplotlib:
            from .VectorLayerHistogram import VectorLayerHistogram
            from .RasterLayerHistogram import RasterLayerHistogram
            from .VectorLayerScatterplot import VectorLayerScatterplot
            from .MeanAndStdDevPlot import MeanAndStdDevPlot
            from .BarPlot import BarPlot
            from .PolarPlot import PolarPlot

            self.alglist.extend([
                VectorLayerHistogram(), RasterLayerHistogram(),
                VectorLayerScatterplot(), MeanAndStdDevPlot(), BarPlot(),
                PolarPlot(),
            ])

        if hasShapely:
            from .Polygonize import Polygonize
            self.alglist.extend([Polygonize()])

        if Qgis.QGIS_VERSION_INT >= 21300:
            from .ExecuteSQL import ExecuteSQL
            self.alglist.extend([ExecuteSQL()])

        self.externalAlgs = []  # to store algs added by 3rd party plugins as scripts

        folder = os.path.join(os.path.dirname(__file__), 'scripts')
        scripts = ScriptUtils.loadFromFolder(folder)
        for script in scripts:
            script.allowEdit = False
        self.alglist.extend(scripts)
        for alg in self.alglist:
            alg._icon = self._icon

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)

    def unload(self):
        AlgorithmProvider.unload(self)

    def getName(self):
        return 'qgis'

    def getDescription(self):
        return 'QGIS'

    def getIcon(self):
        return self._icon

    def _loadAlgorithms(self):
        self.algs = list(self.alglist) + self.externalAlgs

    def supportsNonFileBasedOutput(self):
        return True
