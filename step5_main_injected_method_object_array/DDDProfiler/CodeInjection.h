// CodeInjection.h : Declaration of the CCodeInjection
#pragma once
#include "resource.h"       // main symbols

#include "DDDProfiler_i.h"
#include "ProfileBase.h"

using namespace ATL;

#define COM_FAIL_RETURN(hr, ret) if (!SUCCEEDED(hr)) return (ret)

// CCodeInjection
class ATL_NO_VTABLE CCodeInjection :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CCodeInjection, &CLSID_CodeInjection>,
	public ICodeInjection, CProfilerBase
{
public:
	CCodeInjection()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_CODEINJECTION)

BEGIN_COM_MAP(CCodeInjection)
	COM_INTERFACE_ENTRY(ICodeInjection)
    COM_INTERFACE_ENTRY(ICorProfilerCallback)
    COM_INTERFACE_ENTRY(ICorProfilerCallback2)
    COM_INTERFACE_ENTRY(ICorProfilerCallback3)
END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

private:
    CComQIPtr<ICorProfilerInfo3> m_profilerInfo3;
    mdMemberRef m_targetMethodRefLog;
    mdMemberRef m_targetMethodRefLogAfter;
    mdMemberRef m_targetMethodRefMocked;
    mdMemberRef m_targetMethodRefShouldMock;
    mdTypeRef m_objectTypeRef;

    std::wstring GetMethodName(FunctionID functionId, ModuleID& moduleId, mdToken& funcToken);
    HRESULT GetInjectedRef(ModuleID moduleId, mdModuleRef &mscorlibRef);
    HRESULT GetMsCorlibRef(ModuleID moduleId, mdModuleRef &mscorlibRef);

	HRESULT STDMETHODCALLTYPE AddLogToMethod(ModuleID moduleId, FunctionID functionId, mdToken funcToken);
    HRESULT STDMETHODCALLTYPE AddLoggingToMethod(ModuleID moduleId, FunctionID functionId, mdToken funcToken);
	HRESULT STDMETHODCALLTYPE AddMockingToMethod(ModuleID moduleId, FunctionID functionId, mdToken funcToken, std::wstring methodName);
public:
    virtual HRESULT STDMETHODCALLTYPE Initialize( 
        /* [in] */ IUnknown *pICorProfilerInfoUnk);
        
    virtual HRESULT STDMETHODCALLTYPE Shutdown(void);

    virtual HRESULT STDMETHODCALLTYPE ModuleAttachedToAssembly( 
        /* [in] */ ModuleID moduleId,
        /* [in] */ AssemblyID assemblyId);
    
    virtual HRESULT STDMETHODCALLTYPE JITCompilationStarted( 
        /* [in] */ FunctionID functionId,
        /* [in] */ BOOL fIsSafeToBlock);
};

OBJECT_ENTRY_AUTO(__uuidof(CodeInjection), CCodeInjection)
