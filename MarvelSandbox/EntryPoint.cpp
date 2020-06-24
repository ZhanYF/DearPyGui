﻿#include "Core/PythonUtilities/mvPythonModule.h"
#include "Core/PythonInterfaces/mvStdOutput.h"
#include "Core/Concurrency/mvThreadPool.h"
#include "Core/mvWindow.h"
#include "Platform/Windows/mvWindowsWindow.h"
#include <iostream>
#include <fstream>
#include "Core/PythonInterfaces/mvInterfaceRegistry.h"
#include "Core/PythonInterfaces/mvInterfaces.h"

using namespace Marvel;

mvAppLog* mvAppLog::s_instance = nullptr;

// declare our modules
MV_DECLARE_PYMODULE(pyMod1, "sbApp", {});
MV_DECLARE_PYMODULE(pyMod2, "sbInput", {});
MV_DECLARE_PYMODULE(pyMod3, "sbLog", {});
MV_DECLARE_PYMODULE(pyMod4, "sbPlot", {});
MV_DECLARE_PYMODULE(pyMod5, "sbDraw", {});
MV_DECLARE_PYMODULE(pyMod6, "sbWidgets", {});
MV_DECLARE_PYMODULE(pyMod7, "sbConstants", mvInterfaceRegistry::GetRegistry()->getConstants());

bool doesFileExists(const char* filepath, const char** modname = nullptr);

int main(int argc, char* argv[])
{
	mvThreadPool::InitThreadPool();

	wchar_t* program = Py_DecodeLocale(argv[0], NULL);
	if (program == NULL) {
		fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
		exit(1);
	}
	Py_SetProgramName(program);  /* optional but recommended */

	// initialize our modules
	MV_INIT_PYMODULE(pyMod1, CreatePythonInterface);
	MV_INIT_PYMODULE(pyMod2, CreateInputInterface);
	MV_INIT_PYMODULE(pyMod3, CreateLoggerInterface);
	MV_INIT_PYMODULE(pyMod4, CreatePlotInterface);
	MV_INIT_PYMODULE(pyMod5, CreateDrawingInterface);
	MV_INIT_PYMODULE(pyMod6, CreateWidgetAddingInterface);
	MV_INIT_PYMODULE(pyMod7, CreateConstantsInterface);

	const wchar_t* path;
	std::string addedpath;
	const char* module_name = nullptr;
	doesFileExists("SandboxConfig.txt", &module_name);
	
#ifdef MV_RELEASE
	HWND hWnd = GetConsoleWindow();
	ShowWindow(hWnd, SW_HIDE);
#else
	HWND hWnd = GetConsoleWindow();
	ShowWindow(hWnd, SW_SHOW);
#endif // MV_RELEASE

	// get path
	if (module_name) // ran with config file
	{
		addedpath = "";
		path = L"python38.zip;";
	}
	else if (argc < 2) // ran from visual studio
	{
		addedpath = std::string(MV_MAIN_DIR) + std::string("MarvelSandbox/");
		path = L"python38.zip;../../MarvelSandbox";
	}
	else if (argc == 2) // ran without path
	{
		addedpath = "";
		path = L"python38.zip;";
	}

	else // ran from command line with path
	{
		wchar_t* deco = Py_DecodeLocale(argv[1], nullptr);
		std::wstring* wpath = new std::wstring(std::wstring(deco) + std::wstring(L";python38.zip"));
		path = wpath->c_str();
		addedpath = argv[1];
	}

	// add our custom module
	PyImport_AppendInittab("sandboxout", &PyInit_embOut);

	// set path and start the interpreter
	Py_SetPath(path);
	Py_NoSiteFlag = 1; // this must be set to 1

	Py_Initialize();
	if (!Py_IsInitialized())
	{
		printf("Error initializing Python interpreter\n");
		return 1;
	}
	PyEval_InitThreads();
	
	// import our custom module to capture stdout/stderr
	PyObject* m = PyImport_ImportModule("sandboxout");
	PySys_SetObject("stdout", m);
	PySys_SetObject("stderr", m);

	// get module name
	if (module_name) // ran with config file
		module_name = module_name;
	else if (argc < 2)
		module_name = "App";
	else if (argc == 2)
		module_name = argv[1];
	else
		module_name = argv[2];
	
	// info
	mvAppLog::getLogger()->AddLog("[Sandbox Version] %0s\n", mvApp::getVersion());
	mvAppLog::getLogger()->AddLog("[Python Version] %0s\n", PY_VERSION);
	mvAppLog::getLogger()->AddLog("[ImGui Version] %0s\n", IMGUI_VERSION);
	mvAppLog::getLogger()->AddLog("[Compiler] MSVC version %0d\n", _MSC_VER);

	// get module
	PyObject* pModule = PyImport_ImportModule(module_name); // new reference

	// check if error occurred
	if (!PyErr_Occurred() && pModule != nullptr)
	{
		// returns the dictionary object representing the module namespace
		PyObject* pDict = PyModule_GetDict(pModule); // borrowed reference
		mvApp::GetApp()->setModuleDict(pDict);
		std::string filename = addedpath + std::string(module_name) + ".py";
		mvApp::GetApp()->setFile(filename);
		PyEval_SaveThread(); // releases global lock
		mvApp::GetApp()->preRender();
		mvApp::GetApp()->setStarted();

		// create window
		mvWindow* window = new mvWindowsWindow(mvApp::GetApp()->getWindowWidth(), mvApp::GetApp()->getWindowHeight());
		window->show();

		window->run();
		PyGILState_STATE gstate = PyGILState_Ensure();
		Py_XDECREF(pModule);

	}
	else
	{
		PyErr_Print();
		mvApp::GetApp()->setOk(false);
		mvApp::GetApp()->showLogger();

		// create window
		mvWindow* window = new mvWindowsWindow(mvApp::GetApp()->getWindowWidth(), mvApp::GetApp()->getWindowHeight());
		window->show();

		window->run();
	}

	Py_XDECREF(m);

	PyMem_RawFree(program);

	// shutdown the interpreter
	if (Py_FinalizeEx() < 0)
		exit(120);

}

bool doesFileExists(const char* filepath, const char** modname)
{
	/* try to open file to read */
	std::ifstream ifile;
	ifile.open(filepath, std::ios::in);
	if (ifile)
	{
		std::string line;
		std::getline(ifile, line);
		auto pline = new std::string(line);
		*modname = pline->data();
		return true;
	}
	return false;
}