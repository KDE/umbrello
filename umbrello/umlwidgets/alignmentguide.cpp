/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2025 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "alignmentguide.h"
#include "umlwidget.h"
#include "umlscene.h"

#include <QGraphicsView>
#include <QtMath>

/**
 * Default snap threshold in screen pixels.
 * This value represents the distance in screen space (independent of zoom)
 * within which snapping occurs.
 */
static const qreal DEFAULT_SNAP_THRESHOLD = 10.0;

AlignmentGuide::AlignmentGuide(UMLScene *scene)
    : m_scene(scene)
    , m_snapThreshold(DEFAULT_SNAP_THRESHOLD)
    , m_enabled(true)
{
}

AlignmentGuide::~AlignmentGuide()
{
}

/**
 * Calculate alignment points for a widget at a given position.
 */
AlignmentGuide::AlignmentPoints AlignmentGuide::getAlignmentPoints(UMLWidget *widget, const QPointF &pos) const
{
    AlignmentPoints points;
    qreal w = widget->width();
    qreal h = widget->height();

    points.left = pos.x();
    points.right = pos.x() + w;
    points.hCenter = pos.x() + w / 2.0;
    points.top = pos.y();
    points.bottom = pos.y() + h;
    points.vCenter = pos.y() + h / 2.0;

    return points;
}

/**
 * Find the best snap among candidates for a given position.
 */
void AlignmentGuide::findBestSnap(qreal position, const QList<qreal> &candidates,
                                   qreal &minDistance, qreal &bestSnap, qreal &offset,
                                   GuideType &bestGuideType, GuideType guideType) const
{
    for (qreal candidate : candidates) {
        qreal distance = qAbs(position - candidate);
        if (distance < minDistance) {
            minDistance = distance;
            bestSnap = candidate;
            offset = candidate - position;
            bestGuideType = guideType;
        }
    }
}

/**
 * Snap the proposed position to nearby alignment guides.
 */
QPointF AlignmentGuide::snapPosition(UMLWidget *widget, const QPointF &proposedPos)
{
    // Clear previous guides
    m_activeGuides.clear();

    if (!m_enabled || !widget) {
        return proposedPos;
    }

    // Get alignment points for the widget being moved
    AlignmentPoints movingPoints = getAlignmentPoints(widget, proposedPos);

    // Collect alignment points from all other widgets
    QList<qreal> leftEdges;
    QList<qreal> rightEdges;
    QList<qreal> hCenters;
    QList<qreal> topEdges;
    QList<qreal> bottomEdges;
    QList<qreal> vCenters;

    // Get all widgets from the scene
    UMLWidgetList allWidgets = m_scene->widgetList();

    for (UMLWidget *otherWidget : allWidgets) {
        // Skip the widget being moved and selected widgets (they move together)
        if (otherWidget == widget || otherWidget->isSelected()) {
            continue;
        }

        AlignmentPoints otherPoints = getAlignmentPoints(otherWidget, otherWidget->pos());

        // Group 1: Vertical guides (left edge, right edge, horizontal center)
        leftEdges.append(otherPoints.left);
        rightEdges.append(otherPoints.right);
        hCenters.append(otherPoints.hCenter);

        // Group 2: Horizontal guides (top edge, bottom edge, vertical center)
        topEdges.append(otherPoints.top);
        bottomEdges.append(otherPoints.bottom);
        vCenters.append(otherPoints.vCenter);
    }

    // Start with the proposed position
    QPointF snappedPos = proposedPos;
    qreal offsetX = 0.0;
    qreal offsetY = 0.0;

    // Combine all vertical alignment candidates (Group 1: affects X coordinate)
    QList<qreal> allVerticalCandidates;
    allVerticalCandidates.append(leftEdges);
    allVerticalCandidates.append(rightEdges);
    allVerticalCandidates.append(hCenters);

    // Combine all horizontal alignment candidates (Group 2: affects Y coordinate)
    QList<qreal> allHorizontalCandidates;
    allHorizontalCandidates.append(topEdges);
    allHorizontalCandidates.append(bottomEdges);
    allHorizontalCandidates.append(vCenters);

    // Check vertical guide alignment (snaps X coordinate)
    // Find the best snap among all widget points vs all candidates
    qreal threshold = snapThreshold();  // Get zoom-adjusted threshold
    qreal minXDistance = threshold + 1.0;
    qreal bestXSnap = 0.0;
    GuideType bestXGuideType = GuideType::None;

    // Check left edge
    findBestSnap(movingPoints.left, allVerticalCandidates, minXDistance, bestXSnap, offsetX, bestXGuideType, GuideType::LeftEdge);

    // Check right edge
    findBestSnap(movingPoints.right, allVerticalCandidates, minXDistance, bestXSnap, offsetX, bestXGuideType, GuideType::RightEdge);

    // Check horizontal center
    findBestSnap(movingPoints.hCenter, allVerticalCandidates, minXDistance, bestXSnap, offsetX, bestXGuideType, GuideType::HorizontalCenter);

    if (minXDistance <= threshold) {
        // Apply the offset
        QPointF tempPos(proposedPos.x() + offsetX, proposedPos.y());
        AlignmentPoints snappedPoints = getAlignmentPoints(widget, tempPos);

        // Now check ALL alignments that match at the snapped position
        for (qreal candidate : allVerticalCandidates) {
            if (qAbs(snappedPoints.left - candidate) < 0.5) {
                m_activeGuides.append(GuideLine(GuideType::LeftEdge, candidate));
            }
            if (qAbs(snappedPoints.right - candidate) < 0.5) {
                m_activeGuides.append(GuideLine(GuideType::RightEdge, candidate));
            }
            if (qAbs(snappedPoints.hCenter - candidate) < 0.5) {
                m_activeGuides.append(GuideLine(GuideType::HorizontalCenter, candidate));
            }
        }
    } else {
        offsetX = 0.0;
    }

    // Check horizontal guide alignment (snaps Y coordinate)
    // Find the best snap among all widget points vs all candidates
    qreal minYDistance = threshold + 1.0;
    qreal bestYSnap = 0.0;
    GuideType bestYGuideType = GuideType::None;

    // Check top edge
    findBestSnap(movingPoints.top, allHorizontalCandidates, minYDistance, bestYSnap, offsetY, bestYGuideType, GuideType::TopEdge);

    // Check bottom edge
    findBestSnap(movingPoints.bottom, allHorizontalCandidates, minYDistance, bestYSnap, offsetY, bestYGuideType, GuideType::BottomEdge);

    // Check vertical center
    findBestSnap(movingPoints.vCenter, allHorizontalCandidates, minYDistance, bestYSnap, offsetY, bestYGuideType, GuideType::VerticalCenter);

    if (minYDistance <= threshold) {
        // Apply the offset
        QPointF tempPos(proposedPos.x() + offsetX, proposedPos.y() + offsetY);
        AlignmentPoints snappedPoints = getAlignmentPoints(widget, tempPos);

        // Now check ALL alignments that match at the snapped position
        for (qreal candidate : allHorizontalCandidates) {
            if (qAbs(snappedPoints.top - candidate) < 0.5) {
                m_activeGuides.append(GuideLine(GuideType::TopEdge, candidate));
            }
            if (qAbs(snappedPoints.bottom - candidate) < 0.5) {
                m_activeGuides.append(GuideLine(GuideType::BottomEdge, candidate));
            }
            if (qAbs(snappedPoints.vCenter - candidate) < 0.5) {
                m_activeGuides.append(GuideLine(GuideType::VerticalCenter, candidate));
            }
        }
    } else {
        offsetY = 0.0;
    }

    // Apply offsets
    snappedPos.setX(proposedPos.x() + offsetX);
    snappedPos.setY(proposedPos.y() + offsetY);

    return snappedPos;
}

/**
 * Snap the size during resize operations.
 */
void AlignmentGuide::snapResize(UMLWidget *widget, qreal proposedWidth, qreal proposedHeight,
                                 qreal &newWidth, qreal &newHeight)
{
    // Clear previous guides
    m_activeGuides.clear();

    newWidth = proposedWidth;
    newHeight = proposedHeight;

    if (!m_enabled || !widget) {
        return;
    }

    // Calculate the right and bottom edges with the proposed size
    qreal proposedRight = widget->x() + proposedWidth;
    qreal proposedBottom = widget->y() + proposedHeight;
    qreal proposedHCenter = widget->x() + proposedWidth / 2.0;
    qreal proposedVCenter = widget->y() + proposedHeight / 2.0;

    // Collect alignment points from all other widgets
    QList<qreal> leftEdges;
    QList<qreal> rightEdges;
    QList<qreal> hCenters;
    QList<qreal> topEdges;
    QList<qreal> bottomEdges;
    QList<qreal> vCenters;

    // Get all widgets from the scene
    UMLWidgetList allWidgets = m_scene->widgetList();

    for (UMLWidget *otherWidget : allWidgets) {
        // Skip the widget being resized and selected widgets
        if (otherWidget == widget || otherWidget->isSelected()) {
            continue;
        }

        AlignmentPoints otherPoints = getAlignmentPoints(otherWidget, otherWidget->pos());

        leftEdges.append(otherPoints.left);
        rightEdges.append(otherPoints.right);
        hCenters.append(otherPoints.hCenter);
        topEdges.append(otherPoints.top);
        bottomEdges.append(otherPoints.bottom);
        vCenters.append(otherPoints.vCenter);
    }

    // Combine all vertical alignment candidates
    QList<qreal> allVerticalCandidates;
    allVerticalCandidates.append(leftEdges);
    allVerticalCandidates.append(rightEdges);
    allVerticalCandidates.append(hCenters);

    // Combine all horizontal alignment candidates
    QList<qreal> allHorizontalCandidates;
    allHorizontalCandidates.append(topEdges);
    allHorizontalCandidates.append(bottomEdges);
    allHorizontalCandidates.append(vCenters);

    // Check if the right edge or horizontal center should snap
    qreal threshold = snapThreshold();  // Get zoom-adjusted threshold
    qreal minXDistance = threshold + 1.0;
    qreal bestXSnap = 0.0;
    GuideType bestXGuideType = GuideType::None;
    qreal offsetX = 0.0;

    // Check right edge
    findBestSnap(proposedRight, allVerticalCandidates, minXDistance, bestXSnap, offsetX, bestXGuideType, GuideType::RightEdge);

    // Also check horizontal center during resize
    findBestSnap(proposedHCenter, allVerticalCandidates, minXDistance, bestXSnap, offsetX, bestXGuideType, GuideType::HorizontalCenter);

    if (minXDistance <= threshold) {
        if (bestXGuideType == GuideType::RightEdge) {
            newWidth = bestXSnap - widget->x();
        } else if (bestXGuideType == GuideType::HorizontalCenter) {
            newWidth = (bestXSnap - widget->x()) * 2.0;
        }

        // Check ALL vertical alignments that match after snapping
        qreal snappedRight = widget->x() + newWidth;
        qreal snappedHCenter = widget->x() + newWidth / 2.0;

        for (qreal candidate : allVerticalCandidates) {
            if (qAbs(widget->x() - candidate) < 0.5) {
                m_activeGuides.append(GuideLine(GuideType::LeftEdge, candidate));
            }
            if (qAbs(snappedRight - candidate) < 0.5) {
                m_activeGuides.append(GuideLine(GuideType::RightEdge, candidate));
            }
            if (qAbs(snappedHCenter - candidate) < 0.5) {
                m_activeGuides.append(GuideLine(GuideType::HorizontalCenter, candidate));
            }
        }
    }

    // Check if the bottom edge or vertical center should snap
    qreal minYDistance = threshold + 1.0;
    qreal bestYSnap = 0.0;
    GuideType bestYGuideType = GuideType::None;
    qreal offsetY = 0.0;

    // Check bottom edge
    findBestSnap(proposedBottom, allHorizontalCandidates, minYDistance, bestYSnap, offsetY, bestYGuideType, GuideType::BottomEdge);

    // Also check vertical center during resize
    findBestSnap(proposedVCenter, allHorizontalCandidates, minYDistance, bestYSnap, offsetY, bestYGuideType, GuideType::VerticalCenter);

    if (minYDistance <= threshold) {
        if (bestYGuideType == GuideType::BottomEdge) {
            newHeight = bestYSnap - widget->y();
        } else if (bestYGuideType == GuideType::VerticalCenter) {
            newHeight = (bestYSnap - widget->y()) * 2.0;
        }

        // Check ALL horizontal alignments that match after snapping
        qreal snappedBottom = widget->y() + newHeight;
        qreal snappedVCenter = widget->y() + newHeight / 2.0;

        for (qreal candidate : allHorizontalCandidates) {
            if (qAbs(widget->y() - candidate) < 0.5) {
                m_activeGuides.append(GuideLine(GuideType::TopEdge, candidate));
            }
            if (qAbs(snappedBottom - candidate) < 0.5) {
                m_activeGuides.append(GuideLine(GuideType::BottomEdge, candidate));
            }
            if (qAbs(snappedVCenter - candidate) < 0.5) {
                m_activeGuides.append(GuideLine(GuideType::VerticalCenter, candidate));
            }
        }
    }
}

/**
 * Get the list of active guide lines.
 */
QList<AlignmentGuide::GuideLine> AlignmentGuide::activeGuides() const
{
    return m_activeGuides;
}

/**
 * Clear all active guides.
 */
void AlignmentGuide::clear()
{
    m_activeGuides.clear();
}

/**
 * Set the snap threshold.
 */
void AlignmentGuide::setSnapThreshold(qreal threshold)
{
    m_snapThreshold = threshold;
}

/**
 * Get the snap threshold in scene coordinates.
 * Adjusts for zoom level to maintain consistent screen-space distance.
 */
qreal AlignmentGuide::snapThreshold() const
{
    // Get the first view (if any) to determine zoom level
    QList<QGraphicsView*> viewsList = m_scene->views();
    if (viewsList.isEmpty()) {
        return m_snapThreshold;
    }

    QGraphicsView *view = viewsList.first();

    // Get the view's transformation matrix
    QTransform transform = view->transform();

    // Calculate the scale factor (we use m11 which represents horizontal scaling)
    // For uniform scaling, m11 and m22 should be the same
    qreal scaleFactor = transform.m11();

    // Avoid division by zero
    if (scaleFactor < 0.0001) {
        scaleFactor = 1.0;
    }

    // Convert screen-space threshold to scene coordinates
    // Higher zoom = smaller scene coordinates needed for same screen distance
    return m_snapThreshold / scaleFactor;
}

/**
 * Enable or disable alignment guides.
 */
void AlignmentGuide::setEnabled(bool enabled)
{
    m_enabled = enabled;
    if (!enabled) {
        clear();
    }
}

/**
 * Check if alignment guides are enabled.
 */
bool AlignmentGuide::isEnabled() const
{
    return m_enabled;
}
