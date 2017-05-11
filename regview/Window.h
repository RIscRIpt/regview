#pragma once

namespace regview {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Collections::Generic;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
    using namespace System::Runtime::InteropServices;

	/// <summary>
	/// Summary for Window
	/// </summary>
	public ref class Window : public System::Windows::Forms::Form
	{
	public:
		Window(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//

            HKEY_ROOTS = gcnew Dictionary<String^, IntPtr>();
            HKEY_ROOTS->Add(L"HKEY_CLASSES_ROOT", IntPtr(HKEY_CLASSES_ROOT));
            HKEY_ROOTS->Add(L"HKEY_CURRENT_CONFIG", IntPtr(HKEY_CURRENT_CONFIG));
            HKEY_ROOTS->Add(L"HKEY_CURRENT_USER", IntPtr(HKEY_CURRENT_USER));
            HKEY_ROOTS->Add(L"HKEY_LOCAL_MACHINE", IntPtr(HKEY_LOCAL_MACHINE));
            HKEY_ROOTS->Add(L"HKEY_USERS", IntPtr(HKEY_USERS));

            REG_TYPE_NAMES = gcnew Dictionary<DWORD, String^>();
            REG_TYPE_NAMES->Add(REG_NONE, L"REG_NONE");
            REG_TYPE_NAMES->Add(REG_SZ, L"REG_SZ");
            REG_TYPE_NAMES->Add(REG_EXPAND_SZ, L"REG_EXPAND_SZ");
            REG_TYPE_NAMES->Add(REG_BINARY, L"REG_BINARY");
            REG_TYPE_NAMES->Add(REG_DWORD, L"REG_DWORD");
            REG_TYPE_NAMES->Add(REG_DWORD_BIG_ENDIAN, L"REG_DWORD_BIG_ENDIAN");
            REG_TYPE_NAMES->Add(REG_LINK, L"REG_LINK");
            REG_TYPE_NAMES->Add(REG_MULTI_SZ, L"REG_MULTI_SZ");
            REG_TYPE_NAMES->Add(REG_RESOURCE_LIST, L"REG_RESOURCE_LIST");
            REG_TYPE_NAMES->Add(REG_FULL_RESOURCE_DESCRIPTOR, L"REG_FULL_RESOURCE_DESCRIPTOR");
            REG_TYPE_NAMES->Add(REG_RESOURCE_REQUIREMENTS_LIST, L"REG_RESOURCE_REQUIREMENTS_LIST");
            REG_TYPE_NAMES->Add(REG_QWORD, L"REG_QWORD");

            PopulateRoots();
        }

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Window()
		{
			if (components)
			{
				delete components;
			}
		}
    private: System::Windows::Forms::SplitContainer^  splitContainer1;
    private: System::Windows::Forms::ListView^  regValues;
    private: System::Windows::Forms::ColumnHeader^  regValueHname;
    private: System::Windows::Forms::ColumnHeader^  regValueHtype;
    private: System::Windows::Forms::ColumnHeader^  regValueHdata;
    protected:

    private: System::Windows::Forms::TreeView^  regTree;
    protected:

    protected:

	private:
        HKEY OpenKeyByNode(TreeNode ^node);
        void CloseKey(HKEY key);

        void PopulateRoots();
        void PopulateChildren(TreeNode ^rootNode);

        void PopulateValues(TreeNode ^node);
        void PopulateValue(String ^name, String ^type, String ^data);
        String^ RegTypeToString(DWORD type);
        String^ RegDataToString(DWORD type, BYTE *data, SIZE_T length);

        void GetNodePath(TreeNode ^node, [Out] HKEY %key, [Out] String^% path);

        Dictionary<String^, IntPtr> ^HKEY_ROOTS;
        Dictionary<DWORD, String^> ^REG_TYPE_NAMES;

		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
            this->regTree = (gcnew System::Windows::Forms::TreeView());
            this->splitContainer1 = (gcnew System::Windows::Forms::SplitContainer());
            this->regValues = (gcnew System::Windows::Forms::ListView());
            this->regValueHname = (gcnew System::Windows::Forms::ColumnHeader());
            this->regValueHtype = (gcnew System::Windows::Forms::ColumnHeader());
            this->regValueHdata = (gcnew System::Windows::Forms::ColumnHeader());
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->splitContainer1))->BeginInit();
            this->splitContainer1->Panel1->SuspendLayout();
            this->splitContainer1->Panel2->SuspendLayout();
            this->splitContainer1->SuspendLayout();
            this->SuspendLayout();
            // 
            // regTree
            // 
            this->regTree->Dock = System::Windows::Forms::DockStyle::Fill;
            this->regTree->HideSelection = false;
            this->regTree->Location = System::Drawing::Point(0, 0);
            this->regTree->Name = L"regTree";
            this->regTree->Size = System::Drawing::Size(160, 353);
            this->regTree->TabIndex = 0;
            this->regTree->BeforeCollapse += gcnew System::Windows::Forms::TreeViewCancelEventHandler(this, &Window::regTree_BeforeCollapse);
            this->regTree->BeforeExpand += gcnew System::Windows::Forms::TreeViewCancelEventHandler(this, &Window::regTree_BeforeExpand);
            this->regTree->AfterSelect += gcnew System::Windows::Forms::TreeViewEventHandler(this, &Window::regTree_AfterSelect);
            // 
            // splitContainer1
            // 
            this->splitContainer1->Dock = System::Windows::Forms::DockStyle::Fill;
            this->splitContainer1->FixedPanel = System::Windows::Forms::FixedPanel::Panel1;
            this->splitContainer1->Location = System::Drawing::Point(0, 0);
            this->splitContainer1->Name = L"splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this->splitContainer1->Panel1->Controls->Add(this->regTree);
            this->splitContainer1->Panel1MinSize = 0;
            // 
            // splitContainer1.Panel2
            // 
            this->splitContainer1->Panel2->Controls->Add(this->regValues);
            this->splitContainer1->Panel2MinSize = 0;
            this->splitContainer1->Size = System::Drawing::Size(561, 353);
            this->splitContainer1->SplitterDistance = 160;
            this->splitContainer1->TabIndex = 1;
            // 
            // regValues
            // 
            this->regValues->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(3) {
                this->regValueHname, this->regValueHtype,
                    this->regValueHdata
            });
            this->regValues->Dock = System::Windows::Forms::DockStyle::Fill;
            this->regValues->HideSelection = false;
            this->regValues->LabelEdit = true;
            this->regValues->LabelWrap = false;
            this->regValues->Location = System::Drawing::Point(0, 0);
            this->regValues->MultiSelect = false;
            this->regValues->Name = L"regValues";
            this->regValues->ShowGroups = false;
            this->regValues->Size = System::Drawing::Size(397, 353);
            this->regValues->Sorting = System::Windows::Forms::SortOrder::Ascending;
            this->regValues->TabIndex = 0;
            this->regValues->UseCompatibleStateImageBehavior = false;
            this->regValues->View = System::Windows::Forms::View::Details;
            // 
            // regValueHname
            // 
            this->regValueHname->Text = L"Name";
            // 
            // regValueHtype
            // 
            this->regValueHtype->Text = L"Type";
            // 
            // regValueHdata
            // 
            this->regValueHdata->Text = L"Data";
            // 
            // Window
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(561, 353);
            this->Controls->Add(this->splitContainer1);
            this->Name = L"Window";
            this->Text = L"regview";
            this->splitContainer1->Panel1->ResumeLayout(false);
            this->splitContainer1->Panel2->ResumeLayout(false);
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->splitContainer1))->EndInit();
            this->splitContainer1->ResumeLayout(false);
            this->ResumeLayout(false);

        }
#pragma endregion
    private:
        System::Void regTree_BeforeCollapse(System::Object ^sender, System::Windows::Forms::TreeViewCancelEventArgs ^e);
        System::Void regTree_BeforeExpand(System::Object ^sender, System::Windows::Forms::TreeViewCancelEventArgs ^e);
        System::Void regTree_AfterSelect(System::Object ^sender, System::Windows::Forms::TreeViewEventArgs ^e);
};
}
