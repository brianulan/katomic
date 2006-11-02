/****************************************************************
**
** Implementation Molek class, derieved from Qt tutorial 8
**
****************************************************************/

// bemerkungen : wenn paintEvent aufgerufen wird, wird das komplette
//               widget gelöscht und nur die sachen gezeichnet, die in
//               paintEvent stehen ! sollen dinge z.b nur bei maustasten-
//               druck gezeichnet werden, so muß dies in mousePressEvent
//               stehen !
//               paintEvent wird aufgerufen, falls fenster überdeckt wird,
//               oder auch einfach bewegt wird

#include <config.h>
#include <kdefakes.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <klocale.h>
#include <kdebug.h>

#include <QPainter>
#include <QPaintEvent>

#include "molek.h"
#include "katomicrenderer.h"

extern int level;

Molek::Molek( QWidget *parent ) 
   : QWidget( parent )
{
    QPalette palette;
    palette.setColor( backgroundRole(), Qt::black );
    setPalette(palette);
    setAutoFillBackground( true );
    setMinimumSize(240, 200);

    m_elemSize = 20;

    m_renderer = new KAtomicRenderer( KStandardDirs::locate("appdata", "pics/default_theme.svgz"), this );
    m_renderer->setElementSize( m_elemSize );
}

Molek::~Molek ()
{
}

const atom& Molek::getAtom(int index) const
{
    static atom none = { 0, "" };

    if (index > atoms.count() || index == 0)
        return none;

    return atoms.at(index - 1);
}

void Molek::load (const KSimpleConfig& config)
{
    atoms.clear();
    QString key;

    atom current;

    int atom_index = 1;
    QString value;
    while (true) {
        key.sprintf("atom_%c", int2atom(atom_index));
        value = config.readEntry(key,QString());
        if (value.isEmpty())
            break;

        current.obj = value.at(0).toLatin1();
        value = value.mid(2);
        if (value.isNull())
            value = "";

        strlcpy(current.conn, value.toAscii(), sizeof(current.conn));
        kWarning( atoms.indexOf(current) != -1 )
            << "OOOPS, duplicate atom definition in " << key << endl;
        atoms.append(current);
        atom_index++;
    }

    QString line;

    for (int j = 0; j < MOLEK_SIZE; j++) {

        key.sprintf("mole_%d", j);
        line = config.readEntry(key,QString());

        for (int i = 0; i < MOLEK_SIZE; i++)
        {
            if (i >= line.size()) molek[i][j] = 0;
            else molek[i][j] = atom2int(line.at(i).toLatin1());
        }
    }

    mname = i18n(config.readEntry("Name", I18N_NOOP("Noname")).toLatin1());

    int& height = _size.rheight();
    int& width = _size.rwidth();

    height = 0;
    width = 0;

    for (int i = 0; i < MOLEK_SIZE; i++)
        for (int j = 0; j < MOLEK_SIZE; j++) {
            if (molek [i][j] == 0)
                continue;
            if (i > width) width = i;
            if (j > height) height = j;
        }
    height++;
    width++;

    update();
}

void Molek::paintEvent( QPaintEvent * )
{
    QString st = i18n("Level: %1", level);

    QPainter paint (this);
    paint.setPen (QColor (190, 190, 190));
    paint.drawText (7, height() - 36, mname);
    paint.drawText (7, height() - 18, st);
    // spielfeld gleich zeichnen
    for (int i = 0; i < MOLEK_SIZE; i++)
        for (int j = 0; j < MOLEK_SIZE; j++) {
            int x = 10 + i * m_elemSize;
            int y = 10 + j * m_elemSize;

            if (molek[i][j] == 0)
                continue;

            QPixmap aPix = m_renderer->renderAtom(getAtom(molek[i][j]));
            paint.drawPixmap(x, y, aPix);
        }

    paint.end ();
}

#include "molek.moc"
