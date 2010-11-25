#include <ApplicationWindow.h>

#include <ole2.h>
#include <adodb.h>

using namespace ADODB;

QString makeConnStr(const QString& filename, bool header = true)
{
	QString s;
	QString hdr = header ? "YES" : "NO";

	if(!filename.isEmpty()){
		if(filename.endsWith('x'))
			s = "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=" + filename + ";Extended Properties=\"Excel 12.0 Xml;HDR=" + hdr + "\"";
		else
			s = "Provider='Microsoft.JET.OLEDB.4.0';Data Source=" + filename + ";Extended Properties=\"Excel 8.0;HDR=" + hdr + "\"";
	}

	return s;
}

QString getSheetName(const QString& connStr, int sheetIndex)
{
	Connection *pCon = new Connection();
	if (!pCon)
		return QString();

	pCon->Open(connStr);
	_Recordset *pSchema = pCon->OpenSchema(adSchemaTables);
	if (!pSchema)
		return QString();

	for(int i = 0; i < sheetIndex; ++i)
		pSchema->MoveNext();

	return pSchema->Fields()->Item("TABLE_NAME")->Value().toString();
}


Table* ApplicationWindow::importExcelADO(const QString& fileName, int sheetIndex, bool header)
{
	if(FAILED(::CoInitialize(NULL)))
		return 0;

	QString connStr = makeConnStr(fileName, header);
	QString sheetName = getSheetName(connStr, sheetIndex);
	if (sheetName.isEmpty())
		return 0;

	Recordset *pRec = new Recordset();
	if (!pRec)
		return 0;

	pRec->Open("SELECT * FROM [" + sheetName + "]", connStr, adOpenStatic, adLockOptimistic, adCmdText);

	Fields *fields = pRec->Fields();
	if (!fields)
		return 0;

	int cols = fields->Count();
	int rows = pRec->RecordCount();
	if (!cols || ! rows)
		return 0;

	Table *t = newTable(rows, cols);
	t->setWindowLabel(fileName + ", " + tr("sheet") + ": " + sheetName.remove("$"));
	t->setCaptionPolicy(MdiSubWindow::Both);

	pRec->MoveFirst();
	QString format = "yyyy-MM-dd hh:mm:ss";
	for(int j = 0; j < rows; ++j){
		for(int i = 0; i < cols; ++i){
			Field *f = fields->Item(i);
			if (!f)
				continue;

			QVariant v = f->Value();
			switch (v.type()){
				case QVariant::Double:
					t->setCell(j, i, v.toDouble());
				break;
				case QVariant::DateTime:
					t->setText(j, i, v.toDateTime().toString(format));
					t->setDateFormat(format, i, false);
				break;
				case QVariant::Time:
					t->setText(j, i, v.toTime().toString("hh:mm:ss"));
					t->setTimeFormat("hh:mm:ss", i, false);
				break;
				default:
					t->setText(j, i, v.toString());
				break;
			}
		}
		pRec->MoveNext();
	}

	::CoUninitialize();

	for(int i = 0; i < rows; ++i){
		if (t->isEmptyRow(i))
			t->deleteRows(i, 1);
		else
			break;
	}
	t->showNormal();

	updateRecentProjectsList(fileName);

	return t;
}
