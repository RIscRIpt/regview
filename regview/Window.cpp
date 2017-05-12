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

HKEY Window::OpenKeyByNode(TreeNode ^node, REGSAM access) {
    LONG result;
    HKEY root, hKey;
    String ^path;
    GetNodePath(node, root, path);
    pin_ptr<const TCHAR> wcsPath = PtrToStringChars(path);
    result = RegOpenKeyEx(
        root,
        wcsPath,
        0,
        access,
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
    HKEY hKey32 = OpenKeyByNode(rootNode, KEY_ALL_ACCESS | KEY_WOW64_32KEY);
    if(hKey32 == nullptr) {
        return;
    }
    HKEY hKey64 = OpenKeyByNode(rootNode, KEY_ALL_ACCESS | KEY_WOW64_64KEY);
    if(hKey64 == nullptr) {
        return;
    }

    auto set = gcnew HashSet<String^>();
    PopulateChildren(set, hKey32);
    PopulateChildren(set, hKey64);

    for each(String ^key in set) {
        rootNode->Nodes->Add(key);
    }
}

void Window::PopulateChildren(HashSet<String^> ^dest, HKEY hKey) {
    LONG result;
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
            dest->Add(gcnew String(subKeyName, 0, subKeyNameLen));
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
    regValues->Items->Clear();

    HKEY hKey32 = OpenKeyByNode(node, KEY_ALL_ACCESS | KEY_WOW64_32KEY);
    if(hKey32 == nullptr) {
        return;
    }
    HKEY hKey64 = OpenKeyByNode(node, KEY_ALL_ACCESS | KEY_WOW64_64KEY);
    if(hKey64 == nullptr) {
        return;
    }
    
    auto set = gcnew Dictionary<String^, ListViewItem^>();
    PopulateValues(set, hKey32);
    PopulateValues(set, hKey64);

    for each(auto item in set) {
        regValues->Items->Add(item.Value);
    }
}

void Window::PopulateValues(Dictionary<String^, ListViewItem^> ^dest, HKEY hKey) {
    LONG result;

    const int maxNameSize = 1024;
    const int maxDataSize = 1024;

    TCHAR name[maxNameSize] = { 0 };
    BYTE dataBuffer[maxDataSize] = { 0 };

    DWORD type;
    DWORD nameLength = maxNameSize;
    DWORD dataBufferSize = maxDataSize;

    //result = RegQueryValueEx(
    //    hKey,
    //    NULL,
    //    NULL,
    //    &type,
    //    dataBuffer,
    //    &dataBufferSize
    //);
    //if(result == ERROR_SUCCESS) {
    //    PopulateValue(
    //        L"(Default)",
    //        RegTypeToString(type),
    //        RegDataToString(type, dataBuffer, dataBufferSize)
    //    );
    //}

    for(DWORD dwIndex = 0; ; dwIndex++) {
        nameLength = maxNameSize;
        dataBufferSize = maxDataSize;
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
                dest,
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

    CloseKey(hKey);
}

void Window::PopulateValue(Dictionary<String^, ListViewItem^> ^dest, String ^name, String ^type, String ^data) {
    ListViewItem ^row = gcnew ListViewItem(name);
    row->SubItems->Add(type);
    row->SubItems->Add(data);
    dest[name] = row;
}

String^ Window::RegTypeToString(DWORD type) {
    return REG_TYPE_NAMES[type];
}

template<typename T>
array<T>^ u2m(T *data, size_t size) {
    array<T> ^a = gcnew array<T>(size);
    Marshal::Copy((IntPtr)data, a, 0, size);
    return a;
}

List<String^>^ toArrayOfStrings(BYTE *data) {
    auto strings = gcnew List<String^>();
    do {
        strings->Add(gcnew String((LPTSTR)data));
        data += strings[strings->Count - 1]->Length;
    } while(*data);
    return strings;
}

String^ Window::RegDataToString(DWORD type, BYTE *data, SIZE_T length) {
    switch(type) {
    default:
    case REG_NONE:
        return "";
    case REG_SZ:
    case REG_EXPAND_SZ:
    case REG_LINK:
        return gcnew String((LPTSTR)data, 0, length);
    case REG_RESOURCE_LIST:
    case REG_FULL_RESOURCE_DESCRIPTOR:
    case REG_RESOURCE_REQUIREMENTS_LIST:
    case REG_BINARY:
        return BitConverter::ToString(u2m(data, length))->Replace("-", " ");
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
        return String::Join(", ", toArrayOfStrings(data));
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
    regValues->Sort();
}
