#include <afxwin.h>
#include <afxdlgs.h>
#include <fstream>
#include "grammar.h"
#include "resource.h"

#define NUM_STRINGS 12
#define CLEAR 0
#define ACCEPT 1
#define REJECT 2

using namespace std;

namespace cs451_project
{

	/*
	================================
	CLASS DEFINITION: ContextFreeApp
	================================
	*/
	class ContextFreeApp : public CWinApp
	{
	public:
			BOOL InitInstance();
	};

	// Global string variable with which to fill the rules control
	static CString grammar_rules = "    Entering a Context-Free Grammar\n\n 1)" 
			" Enter each rule separated by a semicolon (;)\n\n 2) The left-hand "
			"side of each rule must be a  single uppercase letter.\n\n 3) The right"
			"-hand side of each rule can only contain the following:\n   a) Lower-"
			"case letters (terminals)\n   b) Upper-case letters (variables) \n"
			"   c) Dollar sign (\'$\') to indicate empty string\n\n 4) The left-hand side"
			" and the right-hand side must be separated by the symbols \'->\' (no single quotes)\n\n"
			" 5) Rules can be compressed with the usual \'|\' symbol.";

	// Global (ah geez) variable to hold the current context-gree grammar
	grammar cfg;

	/*
	===========================
	CLASS DEFINITION: CFGDialog
	===========================
	*/
	class CFGDialog : public CDialog 
	{
	public:
		
		// Controls
		CEdit m_Grammar;
		CStatic m_Rules;
		CButton m_Continue;
		CButton m_Quit;
		CBrush* m_EditBrush;

		// Which resource we are representing
		enum {IDD = IDD_DIALOG1};

		// Public Member Functions
		CFGDialog();
		~CFGDialog();
		void OnClose();
		BOOL OnInitDialog();
		void OnContClick();
		void OnQuitClick();
		virtual void DoDataExchange(CDataExchange* pDX);
		afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		afx_msg void OnDestroy();

		// Declare the class's message map macro
		DECLARE_MESSAGE_MAP()
	private:
		bool brush_alloced;
	};

	/*
	==============================
	CLASS DEFINITION: StringDialog
	==============================
	*/
	class StringDialog : public CDialog 
	{
	public:
		CEdit m_Controls[NUM_STRINGS];
		CButton m_Test;
		CButton m_Exit;
		CButton m_Clear;
		CButton m_NewCFG;
		CBrush* m_pEditBkBrush;
		CWnd m_Window;

		enum {IDD = IDD_DIALOG2};
		StringDialog();
		~StringDialog();
		void onClose();
		BOOL OnInitDialog();
		void onTestClick();
		void onExitClick();
		void onNewClick();
		afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		afx_msg void OnDestroy();
		void onClearClick();
		virtual void DoDataExchange(CDataExchange* pDX);
		DECLARE_MESSAGE_MAP()

	private:

		// Flags for the background color
		unsigned int flags[NUM_STRINGS];

		// To more elegantly access the edit controls
		unsigned int controls[NUM_STRINGS];

	};
	
	/*
	======================================
	CFGDialog::DoDataExchange

	"Attach" variables to their GUI objects
	======================================
	*/
	void CFGDialog::DoDataExchange(CDataExchange* pDX)
	{
		CDialog::DoDataExchange(pDX);
		DDX_Control(pDX, IDC_EDITCFG, m_Grammar);
		DDX_Control(pDX, IDC_CFGRULES, m_Rules);
		DDX_Control(pDX, IDC_CONTBUT, m_Continue);
		DDX_Control(pDX, IDC_QUITBUT, m_Quit);
		DDX_Text(pDX, IDC_CFGRULES, grammar_rules);
	}

	/*
	=======================================
	StringDialog::DoDataExchange

	"Attach" variables to their GUI objects
	=======================================
	*/
	void StringDialog::DoDataExchange(CDataExchange* pDX)
	{
		CDialog::DoDataExchange(pDX);
		DDX_Control(pDX, IDC_STRING1, m_Controls[0]);
		DDX_Control(pDX, IDC_STRING2, m_Controls[1]);
		DDX_Control(pDX, IDC_STRING3, m_Controls[2]);
		DDX_Control(pDX, IDC_STRING4, m_Controls[3]);
		DDX_Control(pDX, IDC_STRING5, m_Controls[4]);
		DDX_Control(pDX, IDC_STRING6, m_Controls[5]);
		DDX_Control(pDX, IDC_STRING7, m_Controls[6]);
		DDX_Control(pDX, IDC_STRING8, m_Controls[7]);
		DDX_Control(pDX, IDC_STRING9, m_Controls[8]);
		DDX_Control(pDX, IDC_STRING10, m_Controls[9]);
		DDX_Control(pDX, IDC_STRING11, m_Controls[10]);
		DDX_Control(pDX, IDC_STRING12, m_Controls[11]);
		DDX_Control(pDX, IDC_CLEARBUT, m_Clear);
		DDX_Control(pDX, IDC_TESTBUT, m_Test);
		DDX_Control(pDX, IDC_NEWCFGBUT, m_NewCFG);
		DDX_Control(pDX, IDC_EXITBUT, m_Exit);
	}

	/*
	=========================================
	Message Map macro declarations

	(i.e. Microsoft's stupid messaging system
	=========================================
	*/
	BEGIN_MESSAGE_MAP(StringDialog, CDialog)
		ON_WM_CLOSE()
		ON_BN_CLICKED(IDC_CLEARBUT, onClearClick)
		ON_BN_CLICKED(IDC_EXITBUT, onExitClick)
		ON_BN_CLICKED(IDC_TESTBUT, onTestClick)
		ON_BN_CLICKED(IDC_NEWCFGBUT, onNewClick)
		ON_WM_CTLCOLOR()
		ON_WM_DESTROY()
	END_MESSAGE_MAP()

	BEGIN_MESSAGE_MAP(CFGDialog, CDialog)
		ON_WM_CLOSE()
		ON_BN_CLICKED(IDC_CONTBUT, OnContClick)
		ON_BN_CLICKED(IDC_QUITBUT, OnQuitClick)
		ON_WM_CTLCOLOR()
		ON_WM_DESTROY()
	END_MESSAGE_MAP()

	// IMPLEMENTATION OF CFGDialog

	/*
	======================================
	CFGDialog::CFGDialog

	Constructs the first Dialog Box object
	======================================
	*/
	CFGDialog::CFGDialog() : CDialog(CFGDialog::IDD)
	{
		brush_alloced = false;
	}

	// Useless function (for now)
	CFGDialog::~CFGDialog()
	{

	}

	/*
	=========================================
	CFGDialog::OnDestroy

	Destroy any dynamically allocated objects
	=========================================
	*/
	void CFGDialog::OnDestroy()
	{
		CDialog::OnDestroy();
		if(brush_alloced)
			delete m_EditBrush;
		
	}

	/*
	===========================================
	CFGDialog::OnQuitClick

	Handle the first dialog's quit button click
	===========================================
	*/
	void CFGDialog::OnQuitClick()
	{
		EndDialog(0);
		return;
	}

	/*
	===============================================================
	CFGDialog::OnContClick

	Handle the Continue button click and propagate CNF conversion;
	Handle any exceptions that occur during CNF conversion and open
	the second dialog window.
	===============================================================
	*/
	void CFGDialog::OnContClick()
	{
		CString result;
		ofstream output;
		output.open("output.txt");
		GetDlgItemText(IDC_EDITCFG, result);
		size_t len = result.GetLength();
		if(len <= 0)
		{
			MessageBox(_T("You didn't enter a grammar."), _T("Error"), MB_ICONASTERISK|MB_OK);
			
		}
		else
		{
			char *buffer = new char[len+1];
			for(size_t i = 0; i < len; ++i)
			{
				buffer[i] = result[i];
			}
			buffer[len] = '\0';
			string test(buffer);
			try
			{
				grammar temp(test);
				cfg = temp;
			}
			catch(logic_error e)
			{
				CString error = "Initializing Grammar: ";
				error += e.what();
				MessageBox(error, _T("Error"), MB_ICONWARNING|MB_OK);
			}
			
			try
			{
				cfg.check_format();
				cfg.cnf_convert();
				StringDialog dlg2;
				ShowWindow(FALSE);
				dlg2.DoModal();
				vector<rule> cnf = cfg.get_rules();
				for(vector<rule>::iterator it = cnf.begin();
					it != cnf.end(); ++it)
				{
						output << *it << "\n";
				}
			}
			catch(domain_error e)
			{
				CString error = "Checking Format: ";
				error += e.what();
				MessageBox(error, _T("Error"), MB_ICONWARNING|MB_OK);
			}
			catch(logic_error e)
			{
				CString error = "Converting to Chomsky Normal Form: ";
				error += e.what();
				MessageBox(error, _T("Error"), MB_ICONWARNING|MB_OK);
			}
			
			output.close();

		}
	}

	// Useless Function (for now)
	void CFGDialog::OnClose()
	{
		
		CDialog::OnClose();
	}

	/*
	========================================
	CFGDialog::OnInitDialog

	Handle first dialog initialization tasks
	========================================
	*/
	BOOL CFGDialog::OnInitDialog()
	{
		CDialog::OnInitDialog();
		// Handle the set icon message
		//SetIcon((HICON)(LoadImage(NULL, _T("prism.jpg"), IMAGE_ICON, 48, 48, LR_LOADFROMFILE)));
		UpdateData();
		return TRUE;
	}

	/*
	=========================================================
	CFGDialog::OnCtlColor

	Handle messages for changing the color of the GUI objects
	=========================================================
	*/
	HBRUSH CFGDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
	{
		int id = pWnd->GetDlgCtrlID();
		switch(nCtlColor)
		{
		case CTLCOLOR_STATIC:
		case CTLCOLOR_EDIT:
			
			pDC->SetTextColor(RGB(0, 191, 255));
			pDC->SetBkColor(RGB(25, 25, 25));
			m_EditBrush = new CBrush(RGB(25, 25, 25));
			brush_alloced = true;
			return (HBRUSH)(m_EditBrush->GetSafeHandle());
			break;
		case CTLCOLOR_BTN:
		case CTLCOLOR_DLG:
			pDC->SetTextColor(RGB(0, 191, 255));
			m_EditBrush = new CBrush(RGB(75, 70, 70));
			brush_alloced = true;
			return (HBRUSH)(m_EditBrush->GetSafeHandle());
			break;
		default:
			return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
			break;
		}
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
		
	}

	
	/*
	==========================================
	ContextFreeApp::InitInstance

	Actually start the app and show the window
	==========================================
	*/
	BOOL ContextFreeApp::InitInstance()
	{
		CFGDialog Dlg;
		m_pMainWnd = &Dlg;
		Dlg.DoModal();
		return TRUE;
	}

	// IMPLEMENTATION OF StringDialog
	
	/*
	=========================================
	StringDialog::StringDialog

	Construct an object for the second dialog
	=========================================
	*/
	StringDialog::StringDialog() : CDialog(StringDialog::IDD)
	{
		for(size_t i = 0; i < NUM_STRINGS; ++i)
		{
			flags[i] = CLEAR;
		}

		controls[0] = IDC_STRING1;
		controls[1] = IDC_STRING2;
		controls[2] = IDC_STRING3;
		controls[3] = IDC_STRING4;
		controls[4] = IDC_STRING5;
		controls[5] = IDC_STRING6;
		controls[6] = IDC_STRING7;
		controls[7] = IDC_STRING8;
		controls[8] = IDC_STRING9;
		controls[9] = IDC_STRING10;
		controls[10] = IDC_STRING11;
		controls[11] = IDC_STRING12;

		m_pEditBkBrush = new CBrush(RGB(25, 25, 25));

	}

	// Useless Function (for now)
	StringDialog::~StringDialog()
	{

	}

	/*
	==========================================
	StringDialog::OnDestroy

	Clean up any dynamically allocated objects
	==========================================
	*/
	void StringDialog::OnDestroy()
	{
		CDialog::OnDestroy();
		delete m_pEditBkBrush;
	}

	/*
	=======================================================
	StringDialog::OnInitDialog

	Complete any initialization tasks for the second dialog
	=======================================================
	*/
	BOOL StringDialog::OnInitDialog()
	{
		CDialog::OnInitDialog();
		return TRUE;
	}

	/*
	===========================================
	StringDialog::onClearClick

	Clear the input fields of the second dialog
	===========================================
	*/
	void StringDialog::onClearClick()
	{
		for(size_t i = 0; i < NUM_STRINGS; ++i)
		{
			(m_Controls[i]).SetWindowTextW(_T(""));
			flags[i] = CLEAR;
			GetDlgItem(controls[i])->Invalidate();
		}
	}

	/*
	==================================================
	StringDialog::OnCtlColor

	Handle color change messages for the second dialog
	WARNING: Thar be lots of redundant code below!
	==================================================
	*/
	HBRUSH StringDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
	{
			
		switch(nCtlColor)
		{
		case CTLCOLOR_DLG:
			delete m_pEditBkBrush;
			m_pEditBkBrush = new CBrush(RGB(75, 70, 70));
			return (HBRUSH)(m_pEditBkBrush->GetSafeHandle());
			break;
		case CTLCOLOR_STATIC:
			pDC->SetTextColor(RGB(25, 245, 0));
			pDC->SetBkColor(RGB(75, 70, 70));
			delete m_pEditBkBrush;
			m_pEditBkBrush = new CBrush(RGB(75, 70, 70));
			return (HBRUSH)(m_pEditBkBrush->GetSafeHandle());
			break;
		default:
			break;
		}
		int id = pWnd->GetDlgCtrlID();
		switch(id)
		{
		case IDC_STRING1:
			if(flags[0] == ACCEPT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(124, 252, 0));
			}
			else if(flags[0] == REJECT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(255, 0, 0));
			}
			else
			{
				pDC->SetTextColor(RGB(0, 191, 255));
				pDC->SetBkColor(RGB(25, 25, 25));
			}
			delete m_pEditBkBrush;
			m_pEditBkBrush = new CBrush(RGB(25, 25, 25));
			return (HBRUSH)(m_pEditBkBrush->GetSafeHandle());
			break;
		case IDC_STRING2:
			if(flags[1] == ACCEPT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(124, 252, 0));
			}
			else if(flags[1] == REJECT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(255, 0, 0));
			}
			else
			{
				pDC->SetTextColor(RGB(0, 191, 255));
				pDC->SetBkColor(RGB(25, 25, 25));
			}
			break;
		case IDC_STRING3:
			if(flags[2] == ACCEPT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(124, 252, 0));
			}
			else if(flags[2] == REJECT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(255, 0, 0));
			}
			else
			{
				pDC->SetTextColor(RGB(0, 191, 255));
				pDC->SetBkColor(RGB(25, 25, 25));
			}
			break;
		case IDC_STRING4:
			if(flags[3] == ACCEPT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(124, 252, 0));
			}
			else if(flags[3] == REJECT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(255, 0, 0));
			}
			else
			{
				pDC->SetTextColor(RGB(0, 191, 255));
				pDC->SetBkColor(RGB(25, 25, 25));
			}
			break;
		case IDC_STRING5:
			if(flags[4] == ACCEPT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(124, 252, 0));
			}
			else if(flags[4] == REJECT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(255, 0, 0));
			}
			else
			{
				pDC->SetTextColor(RGB(0, 191, 255));
				pDC->SetBkColor(RGB(25, 25, 25));
			}
			break;
		case IDC_STRING6:
			if(flags[5] == ACCEPT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(124, 252, 0));
			}
			else if(flags[5] == REJECT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(255, 0, 0));
			}
			else
			{
				pDC->SetTextColor(RGB(0, 191, 255));
				pDC->SetBkColor(RGB(25, 25, 25));
			}
			break;
		case IDC_STRING7:
			if(flags[6] == ACCEPT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(124, 252, 0));
			}
			else if(flags[6] == REJECT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(255, 0, 0));
			}
			else
			{
				pDC->SetTextColor(RGB(0, 191, 255));
				pDC->SetBkColor(RGB(25, 25, 25));
			}
			break;
		case IDC_STRING8:
			if(flags[7] == ACCEPT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(124, 252, 0));
			}
			else if(flags[7] == REJECT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(255, 0, 0));
			}
			else
			{
				pDC->SetTextColor(RGB(0, 191, 255));
				pDC->SetBkColor(RGB(25, 25, 25));
			}
			break;
		case IDC_STRING9:
			if(flags[8] == ACCEPT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(124, 252, 0));
			}
			else if(flags[8] == REJECT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(255, 0, 0));
			}
			else
			{
				pDC->SetTextColor(RGB(0, 191, 255));
				pDC->SetBkColor(RGB(25, 25, 25));
			}
			break;
		case IDC_STRING10:
			if(flags[9] == ACCEPT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(124, 252, 0));
			}
			else if(flags[9] == REJECT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(255, 0, 0));
			}
			else
			{
				pDC->SetTextColor(RGB(0, 191, 255));
				pDC->SetBkColor(RGB(25, 25, 25));
			}
			break;
		case IDC_STRING11:
			if(flags[10] == ACCEPT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(124, 252, 0));
			}
			else if(flags[10] == REJECT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(255, 0, 0));
			}
			else
			{
				pDC->SetTextColor(RGB(0, 191, 255));
				pDC->SetBkColor(RGB(25, 25, 25));
			}
			break;
		case IDC_STRING12:
			if(flags[11] == ACCEPT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(124, 252, 0));
			}
			else if(flags[11] == REJECT)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(255, 0, 0));
			}
			else
			{
				pDC->SetTextColor(RGB(0, 191, 255));
				pDC->SetBkColor(RGB(25, 25, 25));
			}
			break;
		default:
			return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
		}
		return (HBRUSH)(m_pEditBkBrush->GetSafeHandle());
			
	}

	/*
	===================================================================
	StringDialog::onTestClick

	Grab the strings from the input fields and attempt to generate them
	===================================================================
	*/
	void StringDialog::onTestClick()
	{
		CString result;
		for(size_t i = 0; i < NUM_STRINGS; ++i)
		{
			GetDlgItemText(controls[i], result);
			CT2CA converted(result);
			string test(converted);
			size_t str_len = test.length();
			bool in_grammar = false;
			if(str_len > 0)
			{
				in_grammar = cfg.generate_string(test);
				cfg.output_table();
				cfg.clear_table();
				if(in_grammar)
				{
					flags[i] = ACCEPT;
					GetDlgItem(controls[i])->Invalidate();
				}
				else
				{
					flags[i] = REJECT;
					GetDlgItem(controls[i])->Invalidate();
				}
			}
		}
		
		
	}

	/*
	==================================================
	StringDialog::onExitClick

	Handle the quit button click for the second dialog
	==================================================
	*/
	void StringDialog::onExitClick()
	{

		CFGDialog *par = (CFGDialog*)GetParent();
		EndDialog(0);
		par->EndDialog(0);
		exit(0);
		return;
		
	}

	/*
	=======================================
	StringDialog::onNewClick

	Propagate user back to the first dialog
	=======================================
	*/
	void StringDialog::onNewClick()
	{
		EndDialog(0);
		GetParent()->ShowWindow(TRUE);
	}

	// Useless Function (another one?)
	void StringDialog::onClose()
	{

		CDialog::OnClose();
	}

	// Global variable for the application itself
	ContextFreeApp theApp;
}