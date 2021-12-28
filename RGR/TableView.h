#pragma once
#include "framework.h"
#include <commctrl.h>
#pragma comment (lib, "comctl32.lib")
#include <string>
#include <vector>

class TableView
{
private:
	HWND hList;
	RECT rect;
	WCHAR* filePath;
	std::vector<std::wstring> columns;
	std::vector<std::vector<std::wstring>> rows;
	int sotring;
	int previousCol;
private:
	std::vector<std::wstring> MakeStrings();
	void InsertCol(int, std::wstring, int);
	void InsertRow(int, int, std::vector<std::wstring>);
	void DeleteAll();
	void ReadFile(WCHAR*);
public:
	TableView();
	TableView(HWND);
	~TableView();
	std::wstring GetCell(int, int);
	void MoveInfoToTable(WCHAR*);
	void WriteFileRows(WCHAR*);
	void OnColumnClick(LPARAM);
	void OnSize(HWND);
};
