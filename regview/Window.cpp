#include <Windows.h>
#include <vcclr.h>

#include "Window.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")

typedef unsigned long long int QWORD;

using namespace regview;

[STAThread]
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    Window ^window = gcnew Window();
    Application::Run(window);

    return 0;
}

HKEY Window::OpenKeyByNode(TreeNode ^node) {
    LONG result;
    HKEY root, hKey;
    String ^path;
    GetNodePath(node, root, path);
    pin_ptr<const TCHAR> wcsPath = PtrToStringChars(path);
    result = RegOpenKeyEx(
        root,
        wcsPath,
        0,
        KEY_ALL_ACCESS,
        &hKey
    );
    if(result != ERROR_SUCCESS) {
        return nullptr;
    }
    return hKey;
}

void Window::CloseKey(HKEY key) {
    RegCloseKey(key);
}

void Window::PopulateRoots() {
    for each(String ^root in HKEY_ROOTS->Keys) {
        TreeNode ^node = gcnew TreeNode(root);
        PopulateChildren(node);
        regTree->Nodes->Add(node);
    }
}

void Window::PopulateChildren(TreeNode ^rootNode) {
    LONG result;
    HKEY hKey = OpenKeyByNode(rootNode);
    if(hKey == nullptr) {
        return;
    }

    DWORD nbSubKeys = 0;
    DWORD maxSubKeyNameLen = 0;
    result = RegQueryInfoKey(
        hKey,
        NULL,
        NULL,
        NULL,
        &nbSubKeys,
        &maxSubKeyNameLen,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    if(result != ERROR_SUCCESS) {
        goto close_key;
    }

    TCHAR *subKeyName = new TCHAR[maxSubKeyNameLen];
    for(DWORD dwIndex = 0; dwIndex < nbSubKeys; dwIndex++) {
        DWORD subKeyNameLen = maxSubKeyNameLen;
        result = RegEnumKeyEx(
            hKey,
            dwIndex,
            subKeyName,
            &subKeyNameLen,
            NULL,
            NULL,
            NULL,
            NULL
        );
        switch(result) {
        case ERROR_SUCCESS:
            rootNode->Nodes->Add(gcnew String(subKeyName, 0, subKeyNameLen));
            break;
        case ERROR_NO_MORE_ITEMS:
            dwIndex = nbSubKeys;
            break;
        }
    }
    delete[] subKeyName;

close_key:
    CloseKey(hKey);
}

void Window::PopulateValues(TreeNode ^node) {
    LONG result;
    HKEY hKey = OpenKeyByNode(node);
    if(hKey == nullptr) {
        return;
    }

    regValues->Items->Clear();

    const int maxNameSize = 1024;
    const int maxDataSize = 1024;

    TCHAR *name = (TCHAR*)new char[maxNameSize];
    BYTE *dataBuffer = new BYTE[maxDataSize];

    for(DWORD dwIndex = 0; ; dwIndex++) {
        DWORD type;
        DWORD nameLength = maxNameSize;
        DWORD dataBufferSize = maxDataSize;
        result = RegEnumValue(
            hKey,
            dwIndex,
            name,
            &nameLength,
            NULL,
            &type,
            dataBuffer,
            &dataBufferSize
        );
        switch(result) {
        case ERROR_SUCCESS:
            PopulateValue(
                gcnew String(name, 0, nameLength),
                RegTypeToString(type),
                RegDataToString(type, dataBuffer, dataBufferSize)
            );
            break;
        case ERROR_NO_MORE_ITEMS:
            goto break_for;
        }
    }
break_for:
    delete[] name;

    CloseKey(hKey);
}

void Window::PopulateValue(String ^name, String ^type, String ^data) {
    ListViewItem ^row = gcnew ListViewItem(name);
    row->SubItems->Add(type);
    row->SubItems->Add(data);
    regValues->Items->Add(row);
}

String^ Window::RegTypeToString(DWORD type) {
    return REG_TYPE_NAMES[type];
}

String^ Window::RegDataToString(DWORD type, BYTE *data, SIZE_T length) {
    switch(type) {
    default:
    case REG_NONE:
        return "{ NONE }";
    case REG_SZ:
    case REG_EXPAND_SZ:
    case REG_LINK:
        return gcnew String((LPTSTR)data, 0, length);
    case REG_BINARY:
        return "{ BINARY }";
    case REG_DWORD:
        return Convert::ToString((int)*(DWORD*)data);
    case REG_DWORD_BIG_ENDIAN:
        return Convert::ToString(
            (data[0] << 24) |
            (data[1] << 16) |
            (data[2] << 8) |
            (data[3] << 0)
        );
    case REG_MULTI_SZ:
        return "{ LIST }";
        break;
    case REG_RESOURCE_LIST:
    case REG_FULL_RESOURCE_DESCRIPTOR:
    case REG_RESOURCE_REQUIREMENTS_LIST:
        return "{ UNSUPPORTED RESOURCE TYPE }";
    case REG_QWORD:
        return Convert::ToString((long long)*(QWORD*)data);
        break;
    }
}

void Window::GetNodePath(TreeNode ^node, [Out] HKEY %key, [Out] String ^%path) {
    path = "";
    while(node->Parent != nullptr) {
        path = node->Text + "\\" + path;
        node = node->Parent;
    }
    if(path->Length > 0)
        path += "\\";
    key = (HKEY)HKEY_ROOTS[node->Text].ToPointer();
}

System::Void Window::regTree_BeforeCollapse(System::Object ^sender, System::Windows::Forms::TreeViewCancelEventArgs ^e) {

}

System::Void Window::regTree_BeforeExpand(System::Object ^sender, System::Windows::Forms::TreeViewCancelEventArgs ^e) {
    for each(TreeNode ^child in e->Node->Nodes) {
        child->Nodes->Clear();
        PopulateChildren(child);
    }
}

System::Void Window::regTree_AfterSelect(System::Object ^sender, System::Windows::Forms::TreeViewEventArgs ^e) {
    PopulateValues(e->Node);
}