/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2016 Ralf Habacker <ralf.habacker@freenet.de>

*/
#include <QCoreApplication>
#include <QtSvg>
#include <QPainter>
#include <QImage>
#include <QtDebug>

#include <iostream>

bool verbose = false;

void usage()
{
    std::cerr << "svg2png <svg-file> <png-file> <width> <height>" << std::endl;
    std::cerr << "svg2png <svg-file> <png-file> <width> <height> <cursor-template-file>" << std::endl;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (a.arguments().count() < 5) {
        std::cerr << "invalid number of arguments" << std::endl;
        usage();
        return -1;
    }

    QString svgFile = a.arguments().at(1);
    QString pngFile = a.arguments().at(2);
    int width = a.arguments().at(3).toInt();
    int height = a.arguments().at(4).toInt();
    QString templateFile;
    bool useTemplate = false;
    if (a.arguments().count() == 6) {
        templateFile = a.arguments().at(5);
        useTemplate = true;
    }

    if (useTemplate) {
        QSvgRenderer cursorSvg(svgFile);
        if (!cursorSvg.isValid()) {
            std::cerr << "could not open svg file '" << qPrintable(svgFile) << std::endl;
            return -2;
        }
        if (verbose)
            std::cerr << cursorSvg.defaultSize().width() << "x" << cursorSvg.defaultSize().height()  << std::endl;

        QSvgRenderer templateSvg(templateFile);
        if (!cursorSvg.isValid()) {
            std::cerr << "could not open template file '" << qPrintable(templateFile) << std::endl;
            return -3;
        }

        QPainter painter2;
        QImage image2(21, 21, QImage::Format_ARGB32);
        image2.fill(qRgba(0, 0, 0, 0));
        painter2.begin(&image2);
        cursorSvg.render(&painter2);
        painter2.end();

        QImage image(width, height, QImage::Format_ARGB32);
        image.fill(qRgba(0, 0, 0, 0));
        QPainter painter;
        painter.begin(&image);
        templateSvg.render(&painter);
        painter.drawImage(11, 11, image2);
        painter.end();

        if (!image.save(pngFile)) {
            std::cerr << "could not open png file '" << qPrintable(pngFile) << std::endl;
            return -4;
        }
    }
    else {
        QSvgRenderer r(svgFile);
        if (!r.isValid()) {
            std::cerr << "could not open svg file '" << qPrintable(svgFile) << std::endl;
            return -2;
        }

        QImage image(width, height, QImage::Format_ARGB32);
        image.fill(qRgba(0, 0, 0, 0));
        QPainter painter;
        painter.begin(&image);
        r.render(&painter);
        painter.end();

        if (!image.save(pngFile)) {
            std::cerr << "could not open png file '" << qPrintable(pngFile) << std::endl;
            return -4;
        }
    }
    return 0;
}
