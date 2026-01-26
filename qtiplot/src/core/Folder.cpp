/***************************************************************************
    File                 : Folder.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
    Description          : Folder for the project explorer

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "Folder.h"
#include "ApplicationWindow.h"

#include <QApplication>
#include <QDateTime>
#include <QContextMenuEvent>
#include <QDrag>
#include <QMimeData>
#include <QDropEvent>
#include <QKeyEvent>
#include <QMouseEvent>

Folder::Folder( Folder *parent, const QString &name )
    : QObject(parent), d_log_info(QString()), d_active_window(0)
{
	birthdate = QDateTime::currentDateTime ().toString(Qt::LocalDate);
	setObjectName(name);
}

QList<Folder*> Folder::folders()
{
	QList<Folder*> lst;
	foreach(QObject *f, children())
		lst.append((Folder*) f);
	return lst;
}

QStringList Folder::subfolders()
{
	QStringList list = QStringList();
	QObjectList folderList = children();
	if (!folderList.isEmpty()){
		QObject * f;
		foreach(f,folderList)
			list << static_cast<Folder *>(f)->objectName();
	}
	return list;
}

QString Folder::path()
{
    QString s = "/" + QString(objectName()) + "/";
    Folder *parentFolder = (Folder *)parent();
    while (parentFolder){
        s.prepend("/" + QString(parentFolder->objectName()));
        parentFolder = (Folder *)parentFolder->parent();
	}
    return s;
}

int Folder::depth()
{
	int d = 0;
    Folder *parentFolder = (Folder *)parent();
    while (parentFolder){
        ++d;
        parentFolder = (Folder *)parentFolder->parent();
	}
    return d;
}

Folder* Folder::folderBelow()
{
	QList<Folder*> lst = folders();
	if (!lst.isEmpty())
		return lst.first();

	Folder *parentFolder = (Folder *)parent();
	Folder *childFolder = this;
	while (parentFolder && childFolder){
		lst = parentFolder->folders();
		int pos = lst.indexOf(childFolder) + 1;
		if (pos < lst.size())
			return lst.at(pos);

		childFolder = parentFolder;
		parentFolder = (Folder *)parentFolder->parent();
	}
	return nullptr;
}

Folder* Folder::findSubfolder(const QString& s, bool caseSensitive, bool partialMatch)
{
	QObjectList folderList = children();
	if (!folderList.isEmpty()){
		QObject * f;
		foreach(f,folderList){
			QString name = static_cast<Folder *>(f)->objectName();
			if (partialMatch){
				if (caseSensitive && name.startsWith(s,Qt::CaseSensitive))
					return static_cast<Folder *>(f);
				else if (!caseSensitive && name.startsWith(s,Qt::CaseInsensitive))
					return static_cast<Folder *>(f);
			} else {// partialMatch == false
				if (caseSensitive && name == s)
					return static_cast<Folder *>(f);
				else if ( !caseSensitive && (name.toLower() == s.toLower()) )
					return static_cast<Folder *>(f);
			}

			Folder* folder = ((Folder*)f)->findSubfolder(s, caseSensitive, partialMatch);
            if(folder)
                return folder;
		}
	}
	return 0;
}

MdiSubWindow* Folder::findWindow(const QString& s, bool windowNames, bool labels,
							 bool caseSensitive, bool partialMatch)
{
	MdiSubWindow* w;
	foreach(w,lstWindows){
		if (windowNames){
			QString name = w->objectName();
			if (partialMatch && name.contains(s, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive))
				return w;
			else if (caseSensitive && name == s)
				return w;
			else {
				QString text = s;
				if (name == text.toLower())
					return w;
			}
		}

		if (labels){
			QString label = w->windowLabel();
			if (partialMatch && label.contains(s, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive))
				return w;
			else if (caseSensitive && label == s)
				return w;
			else {
				QString text = s;
				if (label == text.toLower())
					return w;
			}
		}
	}
	return 0;
}

MdiSubWindow *Folder::window(const QString &name, const char *cls, bool recursive)
{
	foreach (MdiSubWindow *w, lstWindows){
		if (w->inherits(cls) && name == w->objectName())
			return w;
	}

	if (!recursive) return nullptr;
	foreach (QObject *f, children()){
		MdiSubWindow *w = ((Folder*)f)->window(name, cls, true);
		if (w) return w;
	}
	return nullptr;
}

void Folder::addWindow( MdiSubWindow *w )
{
	if (w){
		lstWindows.append(w);
		w->setFolder(this);
		if (!d_active_window)
			d_active_window = w;
	}
}

void Folder::removeWindow( MdiSubWindow *w )
{
	if (!w)
		return;

	int index = lstWindows.indexOf(w);
	if (index >= 0 && index < lstWindows.size())
		lstWindows.removeAt(index);

	if (d_active_window && d_active_window == w){
		if (!lstWindows.isEmpty())
			d_active_window = lstWindows.first();
		else
			d_active_window = nullptr;
	}
}

QString Folder::sizeToString()
{
	int size = 0;

	QObjectList folderList = children();
	if (!folderList.isEmpty()){
		QObject *f;
		foreach(f,folderList)
			size +=  sizeof(static_cast<Folder *>(f)); // FIXME: Doesn't this function add the size of pointers together? For what?
	}

	MdiSubWindow * w;
	foreach(w, lstWindows)
		size += sizeof(w);

	return QString::number(8*size/1024.0,'f',1)+" "+tr("kB")+" ("+QString::number(8*size)+" "+tr("bytes")+")";
}

Folder* Folder::rootFolder()
{
	Folder *i = this;
	while(i->parent())
		i = (Folder*)i->parent();
	return i;
}

/*****************************************************************************
 *
 * Class FolderListItem
 *
 *****************************************************************************/

FolderListItem::FolderListItem( QTreeWidget *parent, Folder *f )
    : QTreeWidgetItem( parent, FolderListItem::RTTI )
{
    myFolder = f;

    setText(0, f->objectName());
	setExpanded(true);
	setActive(true);
	// setDragEnabled (true); // QTreeWidgetItem doesn't have setDragEnabled
	// setDropEnabled (true); // QTreeWidgetItem doesn't have setDropEnabled
    // Flags are handled via flags() method usually, default is selectable/enabled
}

FolderListItem::FolderListItem( FolderListItem *parent, Folder *f )
    : QTreeWidgetItem( parent, FolderListItem::RTTI )
{
    myFolder = f;

    setText(0, f->objectName());
	setExpanded(true);
	setActive(true);
}

void FolderListItem::setActive( bool o )
{
	if (o)
		setIcon(0, QIcon(":/folder_open.png"));
	else
		setIcon(0, QIcon(":/folder_closed.png"));

	setSelected(o);
}

bool FolderListItem::isChildOf(FolderListItem *src)
{
	FolderListItem *parent = (FolderListItem *)this->parent();
	while (parent){
		if (parent == src)
			return true;

		parent = (FolderListItem *)parent->parent();
	}
	return false;
}

/*****************************************************************************
 *
 * Class FolderListView
 *
 *****************************************************************************/

FolderListView::FolderListView( QWidget *parent, const char *name )
    : QTreeWidget( parent ), mousePressed( false )
{
    if (name)
        setObjectName(name);
    setAcceptDrops( true );
    viewport()->setAcceptDrops( true );
    setDragEnabled(true);

	if (parent){
		connect(this, &FolderListView::itemCollapsed, (ApplicationWindow *)parent, [parent](){ ((ApplicationWindow *)parent)->modifiedProject(); });
		connect(this, &FolderListView::itemExpanded, (ApplicationWindow *)parent, [parent](){ ((ApplicationWindow *)parent)->modifiedProject(); });
		connect(this, &FolderListView::itemExpanded, this, &FolderListView::expandedItem);
        connect(this, &FolderListView::itemChanged, this, &FolderListView::onItemChanged);
	}
}

void FolderListView::onItemChanged(QTreeWidgetItem *item, int col)
{
    emit itemRenamed(item, col, item->text(col));
    // emit modified(); // If needed?
}

void FolderListView::contextMenuEvent( QContextMenuEvent *e )
{
    QTreeWidgetItem *item = itemAt(e->pos());
    emit contextMenuRequested(item, e->globalPos(), 0);
    e->accept();
}

void FolderListView::expandedItem(QTreeWidgetItem *item)
{
    // itemBelow equivalent in QTreeWidget?
    // We might need to iterate.
    // For now, let's skip the selection logic update or implement a simple next item check.
    // QTreeWidget doesn't have direct itemBelow().
    // We can use iterator or item logic.
    // Simplifying:
	// Q3ListViewItem *next = item->itemBelow();
	// if (next)
	// 	setSelected (next, false);
}

void FolderListView::startDrag(Qt::DropActions supportedActions)
{
	QTreeWidgetItem *item = currentItem();
	if (!item)
		return;

    // Root decoration check replacement
	if (item == topLevelItem(0)) // Assuming root is first top level
		return;//it's the project folder so we don't want the user to move it

	QPixmap pix;
    // RTTI check
    // Assuming FolderListItem type check. QTreeWidgetItems don't have rtti() by default.
    // We can cast using dynamic_cast or check type() if we set it.
    // FolderListItem::RTTI was 1001. We should have passed it to constructor.
    // But QTreeWidgetItem constructor accepts type.
    // Assuming we can rely on FolderListItem cast.
    FolderListItem *fItem = dynamic_cast<FolderListItem*>(item);
	if (fItem)
		pix = QPixmap(":/folder_closed.png");
	else
		pix = item->icon(0).pixmap(16,16); // Fallback

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    // Add dummy data or specific format if needed
    mimeData->setText(item->text(0)); // Minimal data
    drag->setMimeData(mimeData);
    drag->setPixmap(pix);
    drag->setHotSpot(QPoint(pix.width()/2, pix.height()/2));

	QList<QTreeWidgetItem *> lst = selectedItems();
	emit dragItems(lst); // Signal ApplicationWindow to handle internal logic?

	drag->exec(supportedActions);
}

void FolderListView::dropEvent( QDropEvent *e )
{
	QTreeWidgetItem *dest = itemAt( e->pos() );
    // Check type of dest
    FolderListItem *fItem = dynamic_cast<FolderListItem*>(dest);
	if (dest && fItem){
		emit dropItems(dest);
		e->accept();
	} else
		e->ignore();
}

void FolderListView::keyPressEvent ( QKeyEvent * e )
{
	if (state() == QAbstractItemView::EditingState){ // isRenaming equivalent
		e->ignore();
		return;
	}

	QTreeWidgetItem *item = currentItem();
	if (!item) {
		QTreeWidget::keyPressEvent ( e );
		 return;
	}

    FolderListItem *fItem = dynamic_cast<FolderListItem*>(item);

	if (fItem &&
		(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)){
		emit itemActivated(item, 0); // Emit standard activate or double click signal equivalent?
		e->accept();
	} else if (e->key() == Qt::Key_F2) {
		if (item)
			emit renameItem(item);
		e->accept();
	} else if(e->key() == Qt::Key_A && e->modifiers() == Qt::ControlModifier){
		selectAll();
		e->accept();
	} else if(e->key() == Qt::Key_F7) {
		emit addFolderItem();
		e->accept();
	} else if(e->key() == Qt::Key_F8){
		emit deleteSelection();
		e->accept();
	} else
		QTreeWidget::keyPressEvent ( e );
}

void FolderListView::mouseDoubleClickEvent( QMouseEvent* e )
{
	if (state() == QAbstractItemView::EditingState)
		{
		e->ignore();
		return;
		}

	QTreeWidget::mouseDoubleClickEvent( e );
}

void FolderListView::mousePressEvent( QMouseEvent* e )
{
	QTreeWidget::mousePressEvent(e);
	QPoint p( e->pos() );
	QTreeWidgetItem *i = itemAt( p );
    // simplified drag check logic
	if ( i )
    {
        presspos = e->pos();
        mousePressed = true;
    }
}

void FolderListView::mouseMoveEvent( QMouseEvent* e )
{
    if ( mousePressed && ( presspos - e->pos() ).manhattanLength() > QApplication::startDragDistance() )
	{
        mousePressed = false;
        QTreeWidgetItem *item = itemAt( presspos );
        if ( item )
            startDrag(Qt::CopyAction|Qt::MoveAction);
    }
}

void FolderListView::adjustColumns()
{
    for (int i=0; i < columnCount(); i++)
        resizeColumnToContents(i);
}

/*****************************************************************************
 *
 * Class WindowListItem
 *
 *****************************************************************************/

WindowListItem::WindowListItem( QTreeWidget *parent, MdiSubWindow *w )
    : QTreeWidgetItem( parent, WindowListItem::RTTI )
{
    myWindow = w;

	// setDragEnabled ( true );
}
