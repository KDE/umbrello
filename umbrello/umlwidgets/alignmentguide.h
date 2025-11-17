/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2025 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ALIGNMENTGUIDE_H
#define ALIGNMENTGUIDE_H

#include <QPointF>
#include <QList>
#include <QLineF>

class UMLWidget;
class UMLScene;

/**
 * @brief Manages alignment guides and snapping for widget movement.
 *
 * This class provides visual alignment guides and automatic snapping
 * when dragging widgets near alignment points of other widgets.
 *
 * Alignment points include:
 * - Horizontal guides: top edge, bottom edge, vertical center
 * - Vertical guides: left edge, right edge, horizontal center
 */
class AlignmentGuide
{
public:
    /**
     * Types of alignment guides.
     */
    enum class GuideType {
        None,
        LeftEdge,
        RightEdge,
        HorizontalCenter,
        TopEdge,
        BottomEdge,
        VerticalCenter
    };

    /**
     * Represents an active alignment guide line.
     */
    struct GuideLine {
        GuideType type;
        qreal position;  // X coordinate for vertical lines, Y for horizontal

        GuideLine() : type(GuideType::None), position(0.0) {}
        GuideLine(GuideType t, qreal pos) : type(t), position(pos) {}

        bool isHorizontal() const {
            return type == GuideType::TopEdge ||
                   type == GuideType::BottomEdge ||
                   type == GuideType::VerticalCenter;
        }

        bool isVertical() const {
            return type == GuideType::LeftEdge ||
                   type == GuideType::RightEdge ||
                   type == GuideType::HorizontalCenter;
        }
    };

    explicit AlignmentGuide(UMLScene *scene);
    ~AlignmentGuide();

    /**
     * Check for alignment opportunities and snap the position if near an alignment.
     *
     * @param widget The widget being moved
     * @param proposedPos The proposed new position
     * @return The snapped position (may be same as proposedPos if no snap)
     */
    QPointF snapPosition(UMLWidget *widget, const QPointF &proposedPos);

    /**
     * Check for alignment opportunities and snap the size during resize.
     *
     * @param widget The widget being resized
     * @param proposedWidth The proposed new width
     * @param proposedHeight The proposed new height
     * @param newWidth Output parameter for snapped width
     * @param newHeight Output parameter for snapped height
     */
    void snapResize(UMLWidget *widget, qreal proposedWidth, qreal proposedHeight,
                    qreal &newWidth, qreal &newHeight);

    /**
     * Get the list of active guide lines to render.
     *
     * @return List of guide lines
     */
    QList<GuideLine> activeGuides() const;

    /**
     * Clear all active guides.
     */
    void clear();

    /**
     * Set the snap threshold distance in pixels (screen coordinates).
     *
     * @param threshold Distance within which snapping occurs
     */
    void setSnapThreshold(qreal threshold);

    /**
     * Get the current snap threshold in scene coordinates.
     * This is adjusted based on the current zoom level to maintain
     * a consistent screen-space distance.
     *
     * @return Snap threshold in scene coordinates
     */
    qreal snapThreshold() const;

    /**
     * Enable or disable alignment guides.
     *
     * @param enabled True to enable, false to disable
     */
    void setEnabled(bool enabled);

    /**
     * Check if alignment guides are enabled.
     *
     * @return True if enabled
     */
    bool isEnabled() const;

private:
    /**
     * Calculate alignment points for a widget.
     */
    struct AlignmentPoints {
        qreal left;
        qreal right;
        qreal hCenter;  // horizontal center
        qreal top;
        qreal bottom;
        qreal vCenter;  // vertical center
    };

    AlignmentPoints getAlignmentPoints(UMLWidget *widget, const QPointF &pos) const;

    /**
     * Find the best snap among candidates for a given position.
     *
     * @param position The position to check
     * @param candidates List of candidate alignment values
     * @param minDistance In/out parameter for minimum distance found
     * @param bestSnap Out parameter for the best snap position
     * @param offset Out parameter for the offset to apply
     * @param bestGuideType In/out parameter for tracking the best guide type
     * @param guideType The guide type to set if this is the best match
     */
    void findBestSnap(qreal position, const QList<qreal> &candidates,
                      qreal &minDistance, qreal &bestSnap, qreal &offset,
                      GuideType &bestGuideType, GuideType guideType) const;

    UMLScene *m_scene;
    QList<GuideLine> m_activeGuides;
    qreal m_snapThreshold;
    bool m_enabled;
};

#endif // ALIGNMENTGUIDE_H
