// CodeInjection.cpp : Implementation of CCodeInjection
#include "stdafx.h"
#include "CodeInjection.h"

#include "Method.h"

// CCodeInjection
HRESULT STDMETHODCALLTYPE CCodeInjection::Initialize( 
    /* [in] */ IUnknown *pICorProfilerInfoUnk) 
{
    OLECHAR szGuid[40]={0};
    int nCount = ::StringFromGUID2(CLSID_CodeInjection, szGuid, 40);
    ATLTRACE(_T("::Initialize - %s"), W2CT(szGuid));

    m_profilerInfo3 = pICorProfilerInfoUnk;
    if (m_profilerInfo3 == NULL) return E_FAIL;

    DWORD dwMask = 0;
    dwMask |= COR_PRF_MONITOR_MODULE_LOADS;         // Controls the ModuleLoad, ModuleUnload, and ModuleAttachedToAssembly callbacks.
    dwMask |= COR_PRF_MONITOR_JIT_COMPILATION;      // Controls the JITCompilation, JITFunctionPitched, and JITInlining callbacks.
    dwMask |= COR_PRF_DISABLE_INLINING;             // Disables all inlining.
    dwMask |= COR_PRF_DISABLE_OPTIMIZATIONS;        // Disables all code optimizations.

    m_profilerInfo3->SetEventMask(dwMask);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CCodeInjection::Shutdown( void) 
{
    ATLTRACE(_T("::Shutdown"));
    return S_OK;
}

HRESULT CCodeInjection::GetInjectedRef(ModuleID moduleId, mdModuleRef &mscorlibRef)
{
    // get interfaces
    CComPtr<IMetaDataEmit2> metaDataEmit;
    COM_FAIL_RETURN(m_profilerInfo3->GetModuleMetaData(moduleId, 
        ofRead | ofWrite, IID_IMetaDataEmit2, (IUnknown**)&metaDataEmit), S_OK);      
    
    CComPtr<IMetaDataAssemblyEmit> metaDataAssemblyEmit;
    COM_FAIL_RETURN(metaDataEmit->QueryInterface(
        IID_IMetaDataAssemblyEmit, (void**)&metaDataAssemblyEmit), S_OK);

    // find injected
    ASSEMBLYMETADATA assembly;
    ZeroMemory(&assembly, sizeof(assembly));
    assembly.usMajorVersion = 1;
    assembly.usMinorVersion = 0;
    assembly.usBuildNumber = 0; 
    assembly.usRevisionNumber = 0;
    COM_FAIL_RETURN(metaDataAssemblyEmit->DefineAssemblyRef(NULL, 
        0, L"Injected", &assembly, NULL, 0, 0, 
        &mscorlibRef), S_OK);
}

HRESULT CCodeInjection::GetMsCorlibRef(ModuleID moduleId, mdModuleRef &mscorlibRef)
{
    // get interfaces
    CComPtr<IMetaDataEmit> metaDataEmit;
    COM_FAIL_RETURN(m_profilerInfo3->GetModuleMetaData(moduleId, 
        ofRead | ofWrite, IID_IMetaDataEmit, (IUnknown**)&metaDataEmit), S_OK);

    CComPtr<IMetaDataAssemblyEmit> metaDataAssemblyEmit;
    COM_FAIL_RETURN(metaDataEmit->QueryInterface(
        IID_IMetaDataAssemblyEmit, (void**)&metaDataAssemblyEmit), S_OK);

    // find mscorlib
    ASSEMBLYMETADATA assembly;
    ZeroMemory(&assembly, sizeof(assembly));
    assembly.usMajorVersion = 4;
    assembly.usMinorVersion = 0;
    assembly.usBuildNumber = 0; 
    assembly.usRevisionNumber = 0;
    BYTE publicKey[] = { 0xB7, 0x7A, 0x5C, 0x56, 0x19, 0x34, 0xE0, 0x89 };
    COM_FAIL_RETURN(metaDataAssemblyEmit->DefineAssemblyRef(publicKey, 
        sizeof(publicKey), L"mscorlib", &assembly, NULL, 0, 0, &mscorlibRef), S_OK);
}

/// <summary>Handle <c>ICorProfilerCallback::ModuleAttachedToAssembly</c></summary>
/// <remarks>Inform the host that we have a new module attached and that it may be 
/// of interest</remarks>
HRESULT STDMETHODCALLTYPE CCodeInjection::ModuleAttachedToAssembly( 
    /* [in] */ ModuleID moduleId,
    /* [in] */ AssemblyID assemblyId)
{
    ULONG dwNameSize = 512;
    WCHAR szAssemblyName[512] = { 0 };
    COM_FAIL_RETURN(m_profilerInfo3->GetAssemblyInfo(assemblyId, dwNameSize, &dwNameSize, szAssemblyName, NULL, NULL), S_OK);

    ULONG dwModuleNameSize = 512;
    WCHAR szModuleName[512] = { 0 };
    COM_FAIL_RETURN(m_profilerInfo3->GetModuleInfo(moduleId, NULL, dwModuleNameSize, &dwModuleNameSize, szModuleName, NULL), S_OK);
    ATLTRACE(_T("::ModuleAttachedToAssembly(%X => %s, %X => %s)"), assemblyId, W2CT(szAssemblyName), moduleId, W2CT(szModuleName));

    if (lstrcmp(L"ProfilerTarget", szAssemblyName) == 0) 
    {
        m_targetMethodRefLog = 0;
        m_targetMethodRefLogAfter = 0;
        m_targetMethodRefMocked = 0;
        m_targetMethodRefShouldMock = 0;

        // get reference to injected
        mdModuleRef injectedRef;
        COM_FAIL_RETURN(GetInjectedRef(moduleId, injectedRef), S_OK);

        // get interfaces
        CComPtr<IMetaDataEmit> metaDataEmit;
        COM_FAIL_RETURN(m_profilerInfo3->GetModuleMetaData(moduleId, 
            ofRead | ofWrite, IID_IMetaDataEmit, (IUnknown**)&metaDataEmit), S_OK);

        static COR_SIGNATURE signatureLog[] = 
        {
            IMAGE_CEE_CS_CALLCONV_DEFAULT,
            0x01,
            ELEMENT_TYPE_VOID,
            ELEMENT_TYPE_OBJECT
        };

        static COR_SIGNATURE signatureLogAfter[] =
        {
            IMAGE_CEE_CS_CALLCONV_DEFAULT,
            0x00,               // arg count
            ELEMENT_TYPE_VOID,  // return value
        };

        // get method to call
        mdTypeRef classTypeRef;
        COM_FAIL_RETURN(metaDataEmit->DefineTypeRefByName(injectedRef, L"Injected.Logger", &classTypeRef), S_OK);
        COM_FAIL_RETURN(metaDataEmit->DefineMemberRef(classTypeRef, L"Log", signatureLog, sizeof(signatureLog), &m_targetMethodRefLog), S_OK);
        COM_FAIL_RETURN(metaDataEmit->DefineMemberRef(classTypeRef, L"LogAfter", signatureLogAfter, sizeof(signatureLogAfter), &m_targetMethodRefLogAfter), S_OK);

        static COR_SIGNATURE signatureMocked[] =
        {
            IMAGE_CEE_CS_CALLCONV_DEFAULT,
            0x00,            // arg count
            ELEMENT_TYPE_I4, // return value (is this right, Int32 = 4 bytes??)
        };

        static COR_SIGNATURE signatureShouldMock[] =
        {
            IMAGE_CEE_CS_CALLCONV_DEFAULT,
            0x01,                 // arg count
            ELEMENT_TYPE_BOOLEAN, // return value 
            ELEMENT_TYPE_STRING,  // paramater
        };

        COM_FAIL_RETURN(metaDataEmit->DefineTypeRefByName(injectedRef, L"Injected.Mocked", &classTypeRef), S_OK);
        COM_FAIL_RETURN(metaDataEmit->DefineMemberRef(classTypeRef, L"MockedMethod", signatureMocked, sizeof(signatureMocked), &m_targetMethodRefMocked), S_OK);
        COM_FAIL_RETURN(metaDataEmit->DefineMemberRef(classTypeRef, L"ShouldMock", signatureShouldMock, sizeof(signatureShouldMock), &m_targetMethodRefShouldMock), S_OK);

        // get object ref
        mdModuleRef mscorlibRef;
        COM_FAIL_RETURN(GetMsCorlibRef(moduleId, mscorlibRef), S_OK);
        COM_FAIL_RETURN(metaDataEmit->DefineTypeRefByName(mscorlibRef, L"System.Object", &m_objectTypeRef), S_OK);
    }

    return S_OK;
}

std::wstring CCodeInjection::GetMethodName(FunctionID functionId, 
    ModuleID& funcModule, mdToken& funcToken)
{
    ClassID funcClass;
    COM_FAIL_RETURN(m_profilerInfo3->GetFunctionInfo2(functionId, 
        NULL, &funcClass, &funcModule, &funcToken, 0, NULL, 
        NULL), std::wstring());

    CComPtr<IMetaDataImport2> metaDataImport2;
    COM_FAIL_RETURN(m_profilerInfo3->GetModuleMetaData(funcModule, 
        ofRead, IID_IMetaDataImport2, (IUnknown**) &metaDataImport2), std::wstring());

    ULONG dwNameSize = 512;
    WCHAR szMethodName[512] = {};
    COM_FAIL_RETURN(metaDataImport2->GetMethodProps(funcToken, NULL, 
        szMethodName, dwNameSize, &dwNameSize, NULL, 
        NULL, NULL, NULL, NULL), S_OK);

    mdTypeDef typeDef;
    COM_FAIL_RETURN(m_profilerInfo3->GetClassIDInfo(funcClass, 
        NULL, &typeDef), std::wstring());

    dwNameSize = 512;
    WCHAR szClassName[512] = {};
    DWORD typeDefFlags = 0;

    COM_FAIL_RETURN(metaDataImport2->GetTypeDefProps(typeDef, szClassName, 
        dwNameSize, &dwNameSize, &typeDefFlags, NULL), std::wstring());

    std::wstring name = szClassName;
    name += L".";
    name += szMethodName;
    return name;
}

/// <summary>Handle <c>ICorProfilerCallback::JITCompilationStarted</c></summary>
/// <remarks>The 'workhorse' </remarks>
HRESULT STDMETHODCALLTYPE CCodeInjection::JITCompilationStarted( 
        /* [in] */ FunctionID functionId, /* [in] */ BOOL fIsSafeToBlock) 
{
    ModuleID moduleId; mdToken funcToken;
    std::wstring methodName = GetMethodName(functionId, moduleId, funcToken);
    ATLTRACE(_T("::JITCompilationStarted(%X -> %s)"), functionId, W2CT(methodName.c_str()));

    if (L"ProfilerTarget.Program.OnMethodToInstrument" == methodName && m_targetMethodRefLog != 0 && m_targetMethodRefLogAfter != 0)
    {
        HRESULT status = AddLoggingToMethod(moduleId, functionId, funcToken);
        if (status != S_OK)
            return status;
    }
    else if (L"ProfilerTarget.ClassToMock.StaticMethodToMock" == methodName && m_targetMethodRefMocked != 0)
    {
        HRESULT status = AddMockingToMethod(moduleId, functionId, funcToken, methodName);
        if (status != S_OK)
            return status;
    }//自定义测试sayHello方法注入日志
	else if (L"ProfilerTarget.InjectTest.SayHello" == methodName && m_targetMethodRefLog != 0 && m_targetMethodRefLogAfter != 0)
	{
		HRESULT status = AddLogToMethod(moduleId, functionId, funcToken);
		if (status != S_OK)
			return status;
	}

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CCodeInjection::AddMockingToMethod(ModuleID moduleId, FunctionID functionId, mdToken funcToken, std::wstring methodName)
{
    // get method body
    LPCBYTE pMethodHeader = NULL;
    ULONG iMethodSize = 0;
    COM_FAIL_RETURN(m_profilerInfo3->GetILFunctionBody(moduleId, funcToken, &pMethodHeader, &iMethodSize), S_OK);

    CComPtr<IMetaDataEmit> metaDataEmit;
    COM_FAIL_RETURN(m_profilerInfo3->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataEmit, (IUnknown**)&metaDataEmit), S_OK);

    // parse IL
    Method instMethod((IMAGE_COR_ILMETHOD*)pMethodHeader); // <--
    instMethod.SetMinimumStackSize(3); // should be correct for this sample

    /* Taken from Reflector (DEBUG Version) - NOTE all the nop's and the locals (used to store values returned from method calls)
    .maxstack 2
    .locals init(
        [0] int32 CS$1$0000,
        [1] bool CS$4$0001)
    L_0000: nop 
    L_0001: ldstr "Profilier.ClassToMock.StaticMethodToMock"
    L_0006: call bool [Injected]Injected.Mocked::ShouldMock(string)
    L_000b: ldc.i4.0 
    L_000c: ceq 
    L_000e: stloc.1 
    L_000f: ldloc.1 
    L_0010: brtrue.s L_001a
    L_0012: call int32 [Injected]Injected.Mocked::MockedMethod()
    L_0017: stloc.0 
    L_0018: br.s L_002a
    // From this point onwards it's the original code, which we can leave alone
    L_001a: ldstr "StaticMethodToMockWhatWeWantToDo called, returning 42"
    L_001f: call void [mscorlib]System.Console::WriteLine(string)
    L_0024: nop 
    L_0025: ldc.i4.s 0x2a
    L_0027: stloc.0 
    L_0028: br.s L_002a
    L_002a: ldloc.0 
    L_002b: ret */

    /* Taken from Reflector (RELEASE Version)
    .maxstack 8
    L_0000: ldstr "Profilier.ClassToMock.StaticMethodToMock"
    L_0005: call bool [Injected]Injected.Mocked::ShouldMock(string)
    L_000a: brfalse.s L_0012
    L_000c: call int32 [Injected]Injected.Mocked::MockedMethod()
    L_0011: ret 
    // From this point onwards it's the original code, which we can leave alone
    L_0012: ldstr "StaticMethodToMockWhatWeWantToDo called, returning 42"
    L_0017: call void [mscorlib]System.Console::WriteLine(string)
    L_001c: ldc.i4.s 0x2a
    L_001e: ret  */

    mdString mdsMessage = mdStringNil;
    COM_FAIL_RETURN(metaDataEmit->DefineUserString(methodName.c_str(), methodName.size(), &mdsMessage), S_OK);

    // insert new IL block
    InstructionList instructions; // NOTE: this IL will be different for an instance method or if the local vars signature is different
    instructions.push_back(new Instruction(CEE_LDSTR, mdsMessage));
    instructions.push_back(new Instruction(CEE_CALL, m_targetMethodRefShouldMock));
    Instruction *returnInstr = new Instruction(CEE_RET);
    instructions.push_back(new Instruction(CEE_BRFALSE_S, returnInstr));
    instructions.push_back(new Instruction(CEE_CALL, m_targetMethodRefMocked));
    instructions.push_back(returnInstr);

    instMethod.InsertSequenceInstructionsAtOriginalOffset(0, instructions);

    ATLTRACE(_T("Re-written IL (AddMockingToMethod)"));
    instMethod.DumpIL();

    // allocate memory
    CComPtr<IMethodMalloc> methodMalloc;
    COM_FAIL_RETURN(m_profilerInfo3->GetILFunctionBodyAllocator(moduleId, &methodMalloc), S_OK);
    void* pNewMethod = methodMalloc->Alloc(instMethod.GetMethodSize());

    // write new method
    instMethod.WriteMethod((IMAGE_COR_ILMETHOD*)pNewMethod);
    COM_FAIL_RETURN(m_profilerInfo3->SetILFunctionBody(moduleId, funcToken, (LPCBYTE)pNewMethod), S_OK);

    // update IL maps
    ULONG mapSize = instMethod.GetILMapSize();
    void* pMap = CoTaskMemAlloc(mapSize * sizeof(COR_IL_MAP));
    instMethod.PopulateILMap(mapSize, (COR_IL_MAP*)pMap);

    COM_FAIL_RETURN(m_profilerInfo3->SetILInstrumentedCodeMap(functionId, TRUE, mapSize, (COR_IL_MAP*)pMap), S_OK);
    CoTaskMemFree(pMap);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CCodeInjection::AddLoggingToMethod(ModuleID moduleId, FunctionID functionId, mdToken funcToken)
{
    // get method body
    LPCBYTE pMethodHeader = NULL;
    ULONG iMethodSize = 0;
    COM_FAIL_RETURN(m_profilerInfo3->GetILFunctionBody(moduleId, funcToken, &pMethodHeader, &iMethodSize), S_OK);

    CComPtr<IMetaDataEmit> metaDataEmit;
    COM_FAIL_RETURN(m_profilerInfo3->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataEmit, (IUnknown**)&metaDataEmit), S_OK);

    // parse IL
    Method instMethod((IMAGE_COR_ILMETHOD*)pMethodHeader); // <--
    instMethod.SetMinimumStackSize(3); // should be correct for this sample

    // NOTE: build signature (in the knowledge that the method we are instrumenting currently has no local vars)
    static COR_SIGNATURE localSignature[] =
    {
        IMAGE_CEE_CS_CALLCONV_LOCAL_SIG,
        0x02,
        ELEMENT_TYPE_ARRAY, ELEMENT_TYPE_OBJECT, 01, 00, 00,
        ELEMENT_TYPE_ARRAY, ELEMENT_TYPE_OBJECT, 01, 00, 00
    };

    mdSignature signature;
    COM_FAIL_RETURN(metaDataEmit->GetTokenFromSig(localSignature, sizeof(localSignature), &signature), S_OK);
    instMethod.m_header.LocalVarSigTok = signature;

    // insert new IL block
    InstructionList instructions; // NOTE: this IL will be different for an instance method or if the local vars signature is different
    // Signature of the method we are instrumenting "static void OnMethodToInstrument(object sender, EventArgs e)"
    instructions.push_back(new Instruction(CEE_NOP));
    instructions.push_back(new Instruction(CEE_LDC_I4_2));
    instructions.push_back(new Instruction(CEE_NEWARR, m_objectTypeRef)); // var paramaters = new Object[2] { sender, e };
    instructions.push_back(new Instruction(CEE_STLOC_1));
    instructions.push_back(new Instruction(CEE_LDLOC_1));
    instructions.push_back(new Instruction(CEE_LDC_I4_0));
    instructions.push_back(new Instruction(CEE_LDARG_0));                 // push local arg0 onto the stack ("sender")
    instructions.push_back(new Instruction(CEE_STELEM_REF));              // store "sender" in parameters array[0]
    instructions.push_back(new Instruction(CEE_LDLOC_1));
    instructions.push_back(new Instruction(CEE_LDC_I4_1));
    instructions.push_back(new Instruction(CEE_LDARG_1));                 // push local arg1 onto the stack ("e")
    instructions.push_back(new Instruction(CEE_STELEM_REF));              // store "e" in parameters array[1]
    instructions.push_back(new Instruction(CEE_LDLOC_1));
    instructions.push_back(new Instruction(CEE_STLOC_0));
    instructions.push_back(new Instruction(CEE_LDLOC_0));
    instructions.push_back(new Instruction(CEE_CALL, m_targetMethodRefLog)); // call Injected.Logger.Log(parameters)

    instMethod.InsertSequenceInstructionsAtOriginalOffset(0, instructions);

    ATLTRACE(_T("Re-written IL (AddLoggingToMethod, before adding call to Logger.LogAfter())"));
    instMethod.DumpIL();

    InstructionList afterInstructions;
    afterInstructions.push_back(new Instruction(CEE_CALL, m_targetMethodRefLogAfter)); // call Injected.Logger.LogAfter()
    //afterInstructions.push_back(new Instruction(CEE_NOP)); Do we need this??

    Instruction* lastButOneInst = instMethod.m_instructions.at(instMethod.m_instructions.size() - 2);
    ATLTRACE(_T("lastButOneInst->m_offset %i "), lastButOneInst->m_offset, lastButOneInst);

    instMethod.InsertSequenceInstructionsAtOffset(lastButOneInst->m_offset, afterInstructions);

    ATLTRACE(_T("Re-written IL (AddLoggingToMethod)"));
    instMethod.DumpIL();

    // allocate memory
    CComPtr<IMethodMalloc> methodMalloc;
    COM_FAIL_RETURN(m_profilerInfo3->GetILFunctionBodyAllocator(moduleId, &methodMalloc), S_OK);
    void* pNewMethod = methodMalloc->Alloc(instMethod.GetMethodSize());

    // write new method
    instMethod.WriteMethod((IMAGE_COR_ILMETHOD*)pNewMethod);
    COM_FAIL_RETURN(m_profilerInfo3->SetILFunctionBody(moduleId, funcToken, (LPCBYTE)pNewMethod), S_OK);

    // update IL maps
    ULONG mapSize = instMethod.GetILMapSize();
    void* pMap = CoTaskMemAlloc(mapSize * sizeof(COR_IL_MAP));
    instMethod.PopulateILMap(mapSize, (COR_IL_MAP*)pMap);

    COM_FAIL_RETURN(m_profilerInfo3->SetILInstrumentedCodeMap(functionId, TRUE, mapSize, (COR_IL_MAP*)pMap), S_OK);
    CoTaskMemFree(pMap);

    return S_OK;
}

//自定义添加日志方法到方法中

HRESULT STDMETHODCALLTYPE CCodeInjection::AddLogToMethod(ModuleID moduleId, FunctionID functionId, mdToken funcToken)
{
	// get method body
	LPCBYTE pMethodHeader = NULL;
	ULONG iMethodSize = 0;
	COM_FAIL_RETURN(m_profilerInfo3->GetILFunctionBody(moduleId, funcToken, &pMethodHeader, &iMethodSize), S_OK);

	CComPtr<IMetaDataEmit> metaDataEmit;
	COM_FAIL_RETURN(m_profilerInfo3->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataEmit, (IUnknown**)&metaDataEmit), S_OK);

	// parse IL
	Method instMethod((IMAGE_COR_ILMETHOD*)pMethodHeader); // <--
	instMethod.SetMinimumStackSize(3); // should be correct for this sample

	// NOTE: build signature (in the knowledge that the method we are instrumenting currently has no local vars)
	static COR_SIGNATURE localSignature[] =
	{
		IMAGE_CEE_CS_CALLCONV_LOCAL_SIG,
		0x02,
		ELEMENT_TYPE_ARRAY, ELEMENT_TYPE_OBJECT, 01, 00, 00,
		ELEMENT_TYPE_ARRAY, ELEMENT_TYPE_OBJECT, 01, 00, 00
	};

	mdSignature signature;
	COM_FAIL_RETURN(metaDataEmit->GetTokenFromSig(localSignature, sizeof(localSignature), &signature), S_OK);
	instMethod.m_header.LocalVarSigTok = signature;

	// insert new IL block
	InstructionList instructions; // NOTE: this IL will be different for an instance method or if the local vars signature is different
	// Signature of the method we are instrumenting "static void OnMethodToInstrument(object sender, EventArgs e)"
	instructions.push_back(new Instruction(CEE_NOP));
	instructions.push_back(new Instruction(CEE_LDC_I4_2));
	instructions.push_back(new Instruction(CEE_NEWARR, m_objectTypeRef)); // var paramaters = new Object[2] { sender, e };
	instructions.push_back(new Instruction(CEE_STLOC_1));
	instructions.push_back(new Instruction(CEE_LDLOC_1));
	instructions.push_back(new Instruction(CEE_LDC_I4_0));
	instructions.push_back(new Instruction(CEE_LDARG_0));                 // push local arg0 onto the stack ("sender")
	instructions.push_back(new Instruction(CEE_STELEM_REF));              // store "sender" in parameters array[0]
	instructions.push_back(new Instruction(CEE_LDLOC_1));
	instructions.push_back(new Instruction(CEE_LDC_I4_1));
	instructions.push_back(new Instruction(CEE_LDARG_1));                 // push local arg1 onto the stack ("e")
	instructions.push_back(new Instruction(CEE_STELEM_REF));              // store "e" in parameters array[1]
	instructions.push_back(new Instruction(CEE_LDLOC_1));
	instructions.push_back(new Instruction(CEE_STLOC_0));
	instructions.push_back(new Instruction(CEE_LDLOC_0));
	instructions.push_back(new Instruction(CEE_CALL, m_targetMethodRefLog)); // call Injected.Logger.Log(parameters)

	instMethod.InsertSequenceInstructionsAtOriginalOffset(0, instructions);

	ATLTRACE(_T("Re-written IL (AddLoggingToMethod, before adding call to Logger.LogAfter())"));
	instMethod.DumpIL();

	InstructionList afterInstructions;
	afterInstructions.push_back(new Instruction(CEE_CALL, m_targetMethodRefLogAfter)); // call Injected.Logger.LogAfter()
	//afterInstructions.push_back(new Instruction(CEE_NOP)); Do we need this??

	Instruction* lastButOneInst = instMethod.m_instructions.at(instMethod.m_instructions.size() - 2);
	ATLTRACE(_T("lastButOneInst->m_offset %i "), lastButOneInst->m_offset, lastButOneInst);

	instMethod.InsertSequenceInstructionsAtOffset(lastButOneInst->m_offset, afterInstructions);

	ATLTRACE(_T("Re-written IL (AddLoggingToMethod)"));
	instMethod.DumpIL();

	// allocate memory
	CComPtr<IMethodMalloc> methodMalloc;
	COM_FAIL_RETURN(m_profilerInfo3->GetILFunctionBodyAllocator(moduleId, &methodMalloc), S_OK);
	void* pNewMethod = methodMalloc->Alloc(instMethod.GetMethodSize());

	// write new method
	instMethod.WriteMethod((IMAGE_COR_ILMETHOD*)pNewMethod);
	COM_FAIL_RETURN(m_profilerInfo3->SetILFunctionBody(moduleId, funcToken, (LPCBYTE)pNewMethod), S_OK);

	// update IL maps
	ULONG mapSize = instMethod.GetILMapSize();
	void* pMap = CoTaskMemAlloc(mapSize * sizeof(COR_IL_MAP));
	instMethod.PopulateILMap(mapSize, (COR_IL_MAP*)pMap);

	COM_FAIL_RETURN(m_profilerInfo3->SetILInstrumentedCodeMap(functionId, TRUE, mapSize, (COR_IL_MAP*)pMap), S_OK);
	CoTaskMemFree(pMap);

	return S_OK;
}