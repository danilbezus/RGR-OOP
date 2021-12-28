#include "TableView.h"
#include "RGR.h"
#include <fstream>


static TableView* pThis = nullptr;
static bool GetRealNumber(std::wstring, double*);
static int CALLBACK SortAscending(LPARAM, LPARAM, LPARAM);
static int CALLBACK SortDescending(LPARAM, LPARAM, LPARAM);

TableView::TableView()
{
	this->hList = NULL;
	this->previousCol = 0;
	this->filePath = NULL;
	this->sotring = 0;
	this->rect = { };
	pThis = this;
}

TableView::TableView(HWND hWnd)
{
	InitCommonControls();
	HINSTANCE hInst = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
	GetClientRect(hWnd, &rect);
	hList = CreateWindow(WC_LISTVIEW, L"",
		WS_VISIBLE | WS_BORDER | WS_CHILD | LVS_REPORT | LVS_EDITLABELS,
		0, 0, rect.right - rect.left, rect.bottom - rect.top,
		hWnd, (HMENU)IDC_LISTVIEW, hInst, 0);
	previousCol = 0;
	filePath = NULL;
	sotring = 0;
	pThis = this;
}

TableView::~TableView()
{
}

void TableView::InsertCol(int index, std::wstring str, int width)
{
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = width;
	lvc.iSubItem = index;
	lvc.cchTextMax = 128;
	lvc.pszText = const_cast<LPWSTR>(str.c_str());

	ListView_InsertColumn(hList, index, &lvc);
}

void TableView::InsertRow(int cols, int rowIndex, std::vector<std::wstring> row)
{
	const int bufSize = 128;

	for (int i = 0; i < cols; ++i)
	{
		std::wstring value = row[i];
		LVITEM lvi;

		lvi.mask = LVIF_TEXT;
		lvi.iItem = rowIndex;
		lvi.iSubItem = i;
		lvi.pszText = const_cast<LPWSTR>(value.c_str());
		lvi.cchTextMax = bufSize;

		if (i > 0) ListView_SetItem(hList, &lvi);
		else ListView_InsertItem(hList, &lvi);
	}
}

std::vector<std::wstring> TableView::MakeStrings()
{
	HWND hWndHeader = (HWND)::SendMessage(this->hList, LVM_GETHEADER, 0, 0);
	int cols = (int)::SendMessage(hWndHeader, HDM_GETITEMCOUNT, 0, 0L);
	int rows = ListView_GetItemCount(this->hList);

	std::vector<std::wstring> vect;
	std::wstring colsStringify;
	for (int i = 0; i < cols; ++i)
	{
		if (i == columns.size() - 1)
		{
			colsStringify = colsStringify + columns[i] + L"\n";
			break;
		}
		colsStringify = colsStringify + columns[i] + L"\t";
	}
	vect.push_back(colsStringify);

	for (int i = 0; i < rows; ++i)
	{
		std::wstring string;
		for (int j = 0; j < cols; ++j)
		{
			if (j == columns.size() - 1)
			{
				string = string + GetCell(i, j) + L"\n";
				break;
			}
			string = string + GetCell(i, j) + L"\t";
		}
		vect.push_back(string);
	}

	return vect;
}

void TableView::ReadFile(WCHAR* file)
{
	if (file != this->filePath) delete this->filePath;
	this->filePath = file;
	columns.clear();
	rows.clear();

	std::wstring symbol = L"\t";
	std::vector<std::vector<std::wstring>> vectToWrite;
	std::wstring str;
	std::wifstream fileSteam(file);
	while (getline(fileSteam, str))
	{
		std::vector<std::wstring> strings;
		size_t pos = 0;
		std::wstring token;
		while (pos != std::wstring::npos)
		{
			pos = str.find(symbol);
			token = str.substr(0, pos);
			strings.push_back(token);
			str.erase(0, pos + symbol.length());
		}
		vectToWrite.push_back(strings);
	}

	columns = vectToWrite[0];
	vectToWrite.erase(vectToWrite.begin());
	rows = vectToWrite;
}

void TableView::WriteFileRows(WCHAR* fileSave)
{
	if (!fileSave) fileSave = this->filePath;
	else this->filePath = fileSave;

	std::wofstream outfile;
	outfile.open(fileSave, std::ios_base::out);

	auto entitiesStrs = MakeStrings();
	for (int i = 0; i < entitiesStrs.size(); ++i)
		outfile << entitiesStrs[i];
}

void TableView::DeleteAll()
{
	ListView_DeleteAllItems(this->hList);
	HWND hWndHeader = (HWND)::SendMessage(this->hList, LVM_GETHEADER, 0, 0);
	int columnCount = (int)::SendMessage(hWndHeader, HDM_GETITEMCOUNT, 0, 0L);
	for (int iCol = columnCount - 1; iCol >= 0; --iCol)
		ListView_DeleteColumn(this->hList, iCol);
}

void TableView::MoveInfoToTable(WCHAR* fileInput)
{
	ReadFile(fileInput);
	DeleteAll();

	for (int index = 0; index < columns.size(); ++index)
	{
		std::wstring textStr = columns[index];
		int width = (rect.right - rect.left) * 0.1;
		InsertCol(index, textStr, width);
	}

	for (int rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
	{
		auto row = rows[rowIndex];
		InsertRow(this->columns.size(), rowIndex, row);
	}
}

void TableView::OnColumnClick(LPARAM lParam)
{
	auto pLVInfo = (LPNMLISTVIEW)lParam;
	static int nSortColumn = 0;
	static BOOL bSortAscending = TRUE;
	LPARAM lParamSort;

	if (pLVInfo->iSubItem == nSortColumn)
		bSortAscending = !bSortAscending;
	else
	{
		nSortColumn = pLVInfo->iSubItem;
		bSortAscending = TRUE;
	}

	lParamSort = 1 + nSortColumn;
	if (!bSortAscending)
		lParamSort = -lParamSort;

	int nColumn = abs(lParamSort) - 1;
	if (this->previousCol != nColumn)
	{
		this->sotring = 1;
	}
	this->previousCol = nColumn;

	if (this->sotring == 0)
	{
		MoveInfoToTable(this->filePath);
	}
	else if (this->sotring == 1)
	{
		ListView_SortItemsEx(pLVInfo->hdr.hwndFrom, SortAscending, lParamSort);
	}
	else if (this->sotring == 2)
	{
		ListView_SortItemsEx(pLVInfo->hdr.hwndFrom, SortDescending, lParamSort);
	}

	sotring = (sotring + 1) % 3;
}

static int CALLBACK SortAscending(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	BOOL bSortAscending = (lParamSort > 0);
	int nColumn = abs(lParamSort) - 1;

	std::wstring strItem1 = pThis->GetCell((int)lParam1, nColumn);
	std::wstring strItem2 = pThis->GetCell((int)lParam2, nColumn);

	double numItem1, numItem2;
	bool isNumberSorting = GetRealNumber(strItem1, &numItem1)
		&& GetRealNumber(strItem2, &numItem2);

	if (isNumberSorting)
	{
		if (numItem1 > numItem2) return 1;
		else if (numItem1 < numItem2) return -1;
		else return 0;
	}
	else
	{
		if (strItem1 > strItem2) return 1;
		else if (strItem1 < strItem2) return -1;
		else return 0;
	}
}

static int CALLBACK SortDescending(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	BOOL bSortAscending = (lParamSort > 0);
	int nColumn = abs(lParamSort) - 1;

	std::wstring strItem1 = pThis->GetCell((int)lParam1, nColumn);
	std::wstring strItem2 = pThis->GetCell((int)lParam2, nColumn);

	double numItem1, numItem2;
	bool isNumberSorting = GetRealNumber(strItem1, &numItem1)
		&& GetRealNumber(strItem2, &numItem2);

	if (isNumberSorting)
	{
		if (numItem1 < numItem2) return 1;
		else if (numItem1 > numItem2) return -1;
		else return 0;
	}
	else
	{
		if (strItem1 < strItem2) return 1;
		else if (strItem1 > strItem2) return -1;
		else return 0;
	}
}

bool GetRealNumber(std::wstring string, double* result)
{
	try
	{
		*result = stod(string);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

std::wstring TableView::GetCell(int row, int col)
{
	WCHAR buffer[256] = L"";

	LVITEM lvi;
	lvi.mask = LVIF_TEXT;
	lvi.iItem = row;
	lvi.pszText = buffer;
	lvi.iSubItem = col;
	lvi.cchTextMax = 256;
	ListView_GetItem(this->hList, &lvi);

	return std::wstring(buffer);
}

void TableView::OnSize(HWND hWnd)
{
	RECT rc;

	GetClientRect(hWnd, &rc);
	MoveWindow(hList, 0, 0, rc.right, rc.bottom, FALSE);

}