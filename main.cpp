#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int Panic(LPCSTR lpszAction, HRESULT hr) {
    fprintf(stderr, "%s error: 0x%08X\n", lpszAction, hr);
    return 1;
}

HRESULT ToOleStr(LPCSTR szInput, LPOLESTR *ppszOutput) {
    HRESULT hr = S_OK;

    hr = (szInput != NULL) && (ppszOutput != NULL) ? S_OK : E_INVALIDARG;
    if (SUCCEEDED(hr)) {
        UINT cchInput = strlen(szInput);
        UINT nSize = cchInput + 1;
        LPOLESTR pOleStr = (LPOLESTR)CoTaskMemAlloc(sizeof(OLECHAR) * nSize);
        hr = pOleStr != NULL ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr)) {
            ZeroMemory(pOleStr, sizeof(OLECHAR) * nSize);
            mbstowcs(pOleStr, szInput, nSize);
            *ppszOutput = pOleStr;
        }
    }

    return hr;
}

int main(int nArgs, LPSTR *rgszArgs) {
    if (nArgs == 1) {
        printf("Usage: %s <absolute dir>\n", rgszArgs[0]);
        return 1;
    }

    HRESULT hr = S_OK;
    int nResult = 0;

    hr = CoInitialize(NULL);
    if (SUCCEEDED(hr)) {
        IShellFolder *psfDesktop;
        hr = SHGetDesktopFolder(&psfDesktop);
        if (SUCCEEDED(hr)) {
            LPOLESTR szDir;
            hr = ToOleStr(rgszArgs[1], &szDir);
            if (SUCCEEDED(hr)) {
                LPITEMIDLIST pidlDir;
                hr = psfDesktop->ParseDisplayName(NULL, NULL, szDir, NULL, &pidlDir, NULL);
                if (SUCCEEDED(hr)) {
                    IShellFolder *psfDir;
                    hr = psfDesktop->BindToObject(pidlDir, NULL, IID_IShellFolder, (LPVOID *)&psfDir);
                    if (SUCCEEDED(hr)) {
                        IEnumIDList *peidlDir;
                        hr = psfDir->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &peidlDir);
                        if (SUCCEEDED(hr)) {
                            printf("=== Contents of %s: ===\n", szDir);

                            LPITEMIDLIST pidlObject;
                            BOOL fList = FALSE;
                            while (SUCCEEDED(hr = peidlDir->Next(1, &pidlObject, NULL)) && (hr != S_FALSE)) {
                                fList = TRUE;

                                STRRET strName;
                                hr = psfDir->GetDisplayNameOf(pidlObject, SHGDN_NORMAL, &strName);
                                if (SUCCEEDED(hr)) {
                                    LPSTR szName;
                                    hr = StrRetToStr(&strName, pidlObject, &szName);
                                    if (SUCCEEDED(hr)) {
                                        printf("%s\n", szName);
                                        CoTaskMemFree(szName);
                                    } else {
                                        Panic("StrRetToStr", hr);
                                    }
                                } else {
                                    Panic("IShellFolder::GetDisplayNameOf", hr);
                                }
                                CoTaskMemFree(&pidlObject);
                            }
                            if (FAILED(hr) && !fList) {
                                nResult = Panic("IEnumIDList::Next", hr);
                            }
                            peidlDir->Release();
                        } else {
                            nResult = Panic("IShellFolder::EnumObjects", hr);
                        }
                        psfDir->Release();
                    } else {
                        nResult = Panic("IShellFolder::BindToObject", hr);
                    }
                    CoTaskMemFree(pidlDir);
                } else {
                    nResult = Panic("IShellFolder::ParseDisplayName", hr);
                }
            } else {
                nResult = Panic("ToOleStr", hr);
            }
            psfDesktop->Release();
        } else {
            nResult = Panic("SHGetDesktopFolder", hr);
        }
        CoUninitialize();
    } else {
        nResult = Panic("CoInitialize", hr);
    }

    return nResult;
}
