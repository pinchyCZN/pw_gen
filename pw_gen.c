#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"

typedef struct{
	int symbols;
	int numbers;
	int ucase;
	int lcase;
	int exc_similar;
	int exc_ambiguous;
	int len;
}PARAMS;
void gen_pw(char *buf,int buf_len,PARAMS *params)
{
	const char *similar="il1Lo0O|";
	const char *symbols="!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
	const char *ambig="{}[]()/\\'\"`~,;:.<>";
	const char *numbers="0123456789";
	char upper[26];
	char lower[26];
	int i,index,count;
	struct STR_ENTRY{
		const char *str;
		int len;
	};
	struct STR_ENTRY list[4]={0};
	for(i=0;i<26;i++){
		upper[i]='A'+i;
		lower[i]='a'+i;
	}
	index=0;
	if(params->numbers){
		list[index].str=numbers;
		list[index].len=strlen(numbers);
		index++;
	}
	if(params->ucase){
		list[index].str=upper;
		list[index].len=sizeof(upper);
		index++;
	}
	if(params->lcase){
		list[index].str=lower;
		list[index].len=sizeof(lower);
		index++;
	}
	if(params->symbols){
		list[index].str=symbols;
		list[index].len=strlen(symbols);
		index++;
	}
	srand(GetTickCount());
	memset(buf,0,buf_len);
	count=index;
	index=0;
	if(0==count)
		return;
	for(i=0;i<params->len;i++){
		int j,len;
		const char *tmp;
		unsigned char a=0;
		j=rand()%count;
		len=sizeof(list)/sizeof(struct STR_ENTRY);
		if(j>=len)
			j=len-1;
		tmp=list[j].str;
		len=list[j].len;
		if(tmp){
			j=rand()%len;
			a=tmp[j];
		}
		if(params->exc_similar){
			if(strchr(similar,a)){
				i--;
				continue;
			}
		}
		if(params->exc_ambiguous){
			if(strchr(ambig,a)){
				i--;
				continue;
			}
		}
		if(index>=buf_len)
			break;
		buf[index++]=a;
	}
	if(buf_len>0)
		buf[buf_len-1]=0;
}

int is_checked(HWND hwnd,int id)
{
	return IsDlgButtonChecked(hwnd,id)==BST_CHECKED;
}
int copy_str_clipboard(char *str)
{
	int len,result=FALSE;
	HGLOBAL hmem;
	char *lock;
	len=strlen(str);
	if(len==0)
		return result;
	len++;
	hmem=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,len);
	if(hmem!=0){
		lock=GlobalLock(hmem);
		if(lock!=0){
			memcpy(lock,str,len);
			GlobalUnlock(hmem);
			if(OpenClipboard(NULL)!=0){
				EmptyClipboard();
				SetClipboardData(CF_TEXT,hmem);
				CloseClipboard();
				result=TRUE;
			}
		}
		if(!result)
			GlobalFree(hmem);
	}
	return result;
}
void init_combo(HWND hwnd)
{
	int i;
	SendMessage(hwnd,CB_LIMITTEXT,4,0);
	SendMessage(hwnd,CB_RESETCONTENT,0,0);
	for(i=4;i<100;i++){
		char str[40];
		_snprintf(str,sizeof(str),"%i",i);
		SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)str);
	}
	SendMessage(hwnd,CB_SETCURSEL,18-4,0);
}
int get_ctrl_int(HWND hwnd)
{
	char str[40];
	GetWindowText(hwnd,str,sizeof(str));
	return atoi(str);
}
#define GRIPPIE_SQUARE_SIZE 15
HWND create_grippy(HWND hwnd)
{
	RECT client_rect;
	GetClientRect(hwnd,&client_rect);
	
	return CreateWindow("Scrollbar",NULL,WS_CHILD|WS_VISIBLE|SBS_SIZEGRIP,
		client_rect.right-GRIPPIE_SQUARE_SIZE,
		client_rect.bottom-GRIPPIE_SQUARE_SIZE,
		GRIPPIE_SQUARE_SIZE,GRIPPIE_SQUARE_SIZE,
		hwnd,NULL,NULL,NULL);
}
int grippy_move(HWND hwnd,HWND grippy)
{
	RECT client_rect;
	GetClientRect(hwnd,&client_rect);
	if(grippy!=0)
	{
		SetWindowPos(grippy,NULL,
			client_rect.right-GRIPPIE_SQUARE_SIZE,
			client_rect.bottom-GRIPPIE_SQUARE_SIZE,
			GRIPPIE_SQUARE_SIZE,GRIPPIE_SQUARE_SIZE,
			SWP_NOZORDER|SWP_SHOWWINDOW);
	}
	return 0;
}
void center_window(HWND hwnd)
{
	RECT rect;
	if(GetWindowRect(GetDesktopWindow(),&rect)!=0){
		int cx,cy;
		cx=(rect.left+rect.right)/2;
		cy=(rect.top+rect.bottom)/2;
		if(GetWindowRect(hwnd,&rect)!=0){
			int w,h;
			w=rect.right-rect.left;
			h=rect.bottom-rect.top;
			SetWindowPos(hwnd,NULL,cx-w/2,cy-h/2,0,0,SWP_NOSIZE);
		}
	}
}

WNDPROC old_edit_proc=0;
LRESULT CALLBACK edit_proc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{	
	switch(msg){
	case WM_KEYDOWN:
		{
			int c=wparam;
			if(c=='A'){
				if(GetKeyState(VK_CONTROL)&0x8000)
					SendMessage(hwnd,EM_SETSEL,0,-1);
			}
		}
		break;
	}
	return CallWindowProc(old_edit_proc,hwnd,msg,wparam,lparam); 
}

BOOL CALLBACK dlg_proc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	static HWND hgrippy;
	switch(msg){
	case WM_INITDIALOG:
		init_combo(GetDlgItem(hwnd,IDC_LENGTH));
		CheckDlgButton(hwnd,IDC_NUMBERS,BST_CHECKED);
		CheckDlgButton(hwnd,IDC_UPPERCASE,BST_CHECKED);
		CheckDlgButton(hwnd,IDC_LOWERCASE,BST_CHECKED);
		SendDlgItemMessage(hwnd,IDC_OUTPUT,WM_SETFONT,(WPARAM)GetStockObject(SYSTEM_FIXED_FONT),0);
		old_edit_proc=SetWindowLong(GetDlgItem(hwnd,IDC_OUTPUT),GWL_WNDPROC,edit_proc);
		hgrippy=create_grippy(hwnd);
		center_window(hwnd);
		SetFocus(GetDlgItem(hwnd,IDOK));
		break;
	case WM_SIZING:
	case WM_SIZE:
		{
			RECT rect;
			int h,w;
			HWND hedit=GetDlgItem(hwnd,IDC_OUTPUT);
			GetWindowRect(hedit,&rect);
			h=rect.bottom-rect.top;
			GetClientRect(hwnd,&rect);
			w=rect.right-rect.left;
			SetWindowPos(hedit,NULL,0,0,w,h,SWP_NOMOVE|SWP_NOZORDER);
			grippy_move(hwnd,hgrippy);
		}
		break;
	case WM_COMMAND:
		{
			int id=LOWORD(wparam);
			switch(id){
			case IDC_COPY:
				{
					char tmp[256]={0};
					GetDlgItemText(hwnd,IDC_OUTPUT,tmp,sizeof(tmp));
					copy_str_clipboard(tmp);
				}
				break;
			case IDC_ONTOP:
				SetWindowPos(hwnd,IsDlgButtonChecked(hwnd,LOWORD(wparam))?HWND_TOPMOST:HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
				break;
			case IDOK:
				{
					char tmp[256]={0};
					PARAMS params={0};
					params.symbols=is_checked(hwnd,IDC_SYMBOLS);
					params.numbers=is_checked(hwnd,IDC_NUMBERS);
					params.ucase=is_checked(hwnd,IDC_UPPERCASE);
					params.lcase=is_checked(hwnd,IDC_LOWERCASE);
					params.exc_similar=is_checked(hwnd,IDC_EXCLUDE_SIM);
					params.exc_ambiguous=is_checked(hwnd,IDC_EXCLUDE_AMB);
					params.len=get_ctrl_int(GetDlgItem(hwnd,IDC_LENGTH));
					gen_pw(tmp,sizeof(tmp),&params);
					SetDlgItemText(hwnd,IDC_OUTPUT,tmp);

				}
				break;
			case IDCANCEL:
				EndDialog(hwnd,0);
				break;
			}

		}
		break;
	}
	return FALSE;
}

int APIENTRY WinMain(HINSTANCE hinstance,HINSTANCE hprevinstance,LPSTR cmdline,int show)
{
	DialogBox(hinstance,MAKEINTRESOURCE(IDD_MAINDLG),NULL,dlg_proc);
}