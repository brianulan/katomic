/*******************************************************************
 *
 * Copyright 2006 Dmitry Suzdalev <dimsuz@gmail.com>
 * Copyright 2007 Carsten Niehaus <cniehaus@kde.org>
 *
 * This file is part of the KDE project "KAtomic"
 *
 * KAtomic is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * KAtomic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with KAtomic; see the file COPYING.  If not, write to
 * the Free Software Foundation, 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 ********************************************************************/

#include "fielditem.h"

#include <QPainter>
#include <QTimeLine>

// these are used by MoleculePreviewItem to draw a button
#include <QStyle>
#include <QStyleOptionButton>
#include <QApplication>
#include <QGraphicsSceneHoverEvent>

#include <kdebug.h>
#include <klocale.h>
#include "molecule.h"
#include "katomicrenderer.h"
#include "playfield.h"

ArrowFieldItem::ArrowFieldItem( QGraphicsScene* scene )
    : FieldItem(scene), m_opacity(0.0)
{
    m_timeLine = new QTimeLine(200);
    m_timeLine->setFrameRange( 0, 30 );
    connect(m_timeLine, SIGNAL(valueChanged(qreal)), SLOT(setOpacity(qreal)) );
}

ArrowFieldItem::~ArrowFieldItem()
{
    delete m_timeLine;
}

void ArrowFieldItem::setOpacity( qreal opacity )
{
    m_opacity = opacity;
    update();
}

void ArrowFieldItem::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
    painter->setOpacity( m_opacity );
    QGraphicsPixmapItem::paint( painter, option, widget );
}

QVariant ArrowFieldItem::itemChange( GraphicsItemChange change, const QVariant& value )
{
    if(change == ItemVisibleChange)
    {
        if(value.toBool())
        {
            m_timeLine->stop();
            m_timeLine->setCurrentTime(0);
            m_timeLine->start();
        }
    }
    return value;
}

// ----------------- MoleculePreviewItem ----------------------------

MoleculePreviewItem::MoleculePreviewItem( PlayField* scene )
    : QGraphicsItem( 0, scene ), m_width(0),
      m_atomSize(20), m_maxAtomSize(30), m_mol( 0 )
{
}

MoleculePreviewItem::~MoleculePreviewItem()
{
}

void MoleculePreviewItem::setMolecule( const Molecule* mol )
{
    m_mol = mol;
    setWidth( m_width ); // trigger atom size update
}

void MoleculePreviewItem::setMaxAtomSize(int maxSize)
{
    m_maxAtomSize = maxSize;
    setWidth( m_width ); // trigger atom size update
}

void MoleculePreviewItem::setWidth(int width)
{
    m_width = width;

    int w = m_mol->width();
    int h = m_mol->height();
    int atomSize = width / qMax(w,h);
    m_atomSize = qMin(atomSize, m_maxAtomSize);
    update();
}

void MoleculePreviewItem::paint( QPainter * painter, const QStyleOptionGraphicsItem*, QWidget *)
{
    painter->setBrush(Qt::gray);
    painter->setOpacity(0.5);
    painter->drawRect(boundingRect());
    painter->setOpacity(1.0);

    int originX = m_width/2 - m_atomSize*m_mol->width()/2;
    int originY = m_width/2 - m_atomSize*m_mol->height()/2;

    // Paint the playing field
    for (int i = 0; i < MOLECULE_SIZE; i++)
        for (int j = 0; j < MOLECULE_SIZE; j++)
        {
            int x = originX + i * m_atomSize;
            int y = originY + j * m_atomSize;

            if (m_mol->getAtom(i,j) == 0)
                continue;

            int atomIdx = m_mol->getAtom(i,j);
            QPixmap aPix = KAtomicRenderer::self()->renderAtom(m_mol->getAtom(atomIdx), m_atomSize);
            painter->drawPixmap(x, y, aPix);
        }
}

// ----------------- MoleculeInfoItem ----------------------------
MoleculeInfoItem::MoleculeInfoItem( QGraphicsScene* scene )
    : QGraphicsTextItem( 0, scene ), m_width(10), m_height(10),
    m_showInfo(true)
{
}

void MoleculeInfoItem::setSize( int width, int height )
{
    setTextWidth( width );
    m_width = width;
    m_height = height;
}

void MoleculeInfoItem::setMolecule( const Molecule& mol )
{
    double weight = mol.molecularWeight();
    QString weightString = weight == 0 ?  i18n("Unknown weight") : QString::number( weight );

    QString str = "<h3 align=\"center\"><u>";
    str.append(mol.moleculeName());
    str.append("</u></h3><br>");
    str.append("<table border=\"1\" width=\"100%\"><tr><td><b>");
    str.append(i18n("Molecular weight"));
    str.append("</b></td><td>");
    str.append(weightString);
    str.append("</td></tr>");
    str.append("<tr><td><b>");
    str.append(i18n("Some other property"));
    str.append("</b></td><td>");
    str.append(i18n("Some description of <i>%1</i>", mol.moleculeName()));
    str.append("</i></td></tr></table>");
    setHtml(str);
}

void MoleculeInfoItem::paint( QPainter * painter, const QStyleOptionGraphicsItem* opt, QWidget *w)
{
    painter->setBrush(Qt::gray);
    painter->drawRect( boundingRect() );
    //painter->setOpacity(0.9);
    if(m_showInfo)
        QGraphicsTextItem::paint(painter, opt, w);
}

#include "fielditem.moc"
