#include "mvApp.h"
#include "mvModule_DearPyGui.h"
#include "mvViewport.h"
#include "mvCallbackRegistry.h"
#include "mvInput.h"
#include <thread>
#include <future>
#include <chrono>
#include "mvProfiler.h"
#include <implot.h>
#include "mvFontManager.h"
#include "mvCallbackRegistry.h"
#include "mvPythonTranslator.h"
#include "mvPythonExceptions.h"
#include "mvGlobalIntepreterLock.h"
#include <frameobject.h>
#include "mvModule_DearPyGui.h"
#include "mvLog.h"
#include "mvEventMacros.h"
#include "mvToolManager.h"
#include <imnodes.h>
#include <thread>
#include <stb_image.h>
#include "mvBuffer.h"
#include "mvAppItemCommons.h"
#include "mvItemRegistry.h"

namespace Marvel {

	mvApp* mvApp::s_instance = nullptr;
	std::atomic_bool mvApp::s_started = false;
	std::atomic_bool mvApp::s_manualMutexControl = false;
	std::atomic_bool mvApp::s_waitOneFrame = false;
	float mvApp::s_deltaTime = 0.0f;
	int mvApp::s_frame = 0;
	int mvApp::s_framerate = 0;
	double mvApp::s_time = 0.0;
	std::mutex mvApp::s_mutex = {};
	mvUUID mvApp::s_id = MV_START_UUID;

	mvApp* mvApp::GetApp()
	{
		
		if (s_instance)
			return s_instance;

		s_instance = new mvApp();
		return s_instance;
	}

	mvUUID mvApp::GenerateUUID()
	{
		return ++s_id;
	}

	void mvApp::SetDefaultTheme()
	{
		ImGuiStyle* style = &ImGui::GetStyle();
		ImVec4* colors = style->Colors;

		colors[ImGuiCol_Text] = MV_BASE_COL_textColor;
		colors[ImGuiCol_TextDisabled] = MV_BASE_COL_textDisabledColor;
		colors[ImGuiCol_WindowBg] = mvImGuiCol_WindowBg;
		colors[ImGuiCol_ChildBg] = mvImGuiCol_ChildBg;
		colors[ImGuiCol_PopupBg] = mvImGuiCol_PopupBg;
		colors[ImGuiCol_Border] = mvImGuiCol_Border;
		colors[ImGuiCol_BorderShadow] = mvImGuiCol_BorderShadow;
		colors[ImGuiCol_FrameBg] = mvImGuiCol_FrameBg;
		colors[ImGuiCol_FrameBgHovered] = mvImGuiCol_FrameBgHovered;
		colors[ImGuiCol_FrameBgActive] = mvImGuiCol_FrameBgActive;
		colors[ImGuiCol_TitleBg] = mvImGuiCol_TitleBg;
		colors[ImGuiCol_TitleBgActive] = mvImGuiCol_TitleBgActive;
		colors[ImGuiCol_TitleBgCollapsed] = mvImGuiCol_TitleBgCollapsed;
		colors[ImGuiCol_MenuBarBg] = mvImGuiCol_MenuBarBg;
		colors[ImGuiCol_ScrollbarBg] = mvImGuiCol_ScrollbarBg;
		colors[ImGuiCol_ScrollbarGrab] = mvImGuiCol_ScrollbarGrab;
		colors[ImGuiCol_ScrollbarGrabHovered] = mvImGuiCol_ScrollbarGrabHovered;
		colors[ImGuiCol_ScrollbarGrabActive] = mvImGuiCol_ScrollbarGrabActive;
		colors[ImGuiCol_CheckMark] = mvImGuiCol_CheckMark;
		colors[ImGuiCol_SliderGrab] = mvImGuiCol_SliderGrab;
		colors[ImGuiCol_SliderGrabActive] = mvImGuiCol_SliderGrabActive;
		colors[ImGuiCol_Button] = mvImGuiCol_Button;
		colors[ImGuiCol_ButtonHovered] = mvImGuiCol_ButtonHovered;
		colors[ImGuiCol_ButtonActive] = mvImGuiCol_ButtonActive;
		colors[ImGuiCol_Header] = mvImGuiCol_Header;
		colors[ImGuiCol_HeaderHovered] = mvImGuiCol_HeaderHovered;
		colors[ImGuiCol_HeaderActive] = mvImGuiCol_HeaderActive;
		colors[ImGuiCol_Separator] = mvImGuiCol_Separator;
		colors[ImGuiCol_SeparatorHovered] = mvImGuiCol_SeparatorHovered;
		colors[ImGuiCol_SeparatorActive] = mvImGuiCol_SeparatorActive;
		colors[ImGuiCol_ResizeGrip] = mvImGuiCol_ResizeGrip;
		colors[ImGuiCol_ResizeGripHovered] = mvImGuiCol_ResizeGripHovered;
		colors[ImGuiCol_ResizeGripActive] = mvImGuiCol_ResizeGripHovered;
		colors[ImGuiCol_Tab] = mvImGuiCol_Tab;
		colors[ImGuiCol_TabHovered] = mvImGuiCol_TabHovered;
		colors[ImGuiCol_TabActive] = mvImGuiCol_TabActive;
		colors[ImGuiCol_TabUnfocused] = mvImGuiCol_TabUnfocused;
		colors[ImGuiCol_TabUnfocusedActive] = mvImGuiCol_TabUnfocusedActive;
		colors[ImGuiCol_DockingPreview] = mvImGuiCol_DockingPreview;
		colors[ImGuiCol_DockingEmptyBg] = mvImGuiCol_DockingEmptyBg;
		colors[ImGuiCol_PlotLines] = mvImGuiCol_PlotLines;
		colors[ImGuiCol_PlotLinesHovered] = mvImGuiCol_PlotLinesHovered;
		colors[ImGuiCol_PlotHistogram] = mvImGuiCol_PlotHistogram;
		colors[ImGuiCol_PlotHistogramHovered] = mvImGuiCol_PlotHistogramHovered;
		colors[ImGuiCol_TableHeaderBg] = mvImGuiCol_TableHeaderBg;
		colors[ImGuiCol_TableBorderStrong] = mvImGuiCol_TableBorderStrong;   // Prefer using Alpha=1.0 here
		colors[ImGuiCol_TableBorderLight] = mvImGuiCol_TableBorderLight;   // Prefer using Alpha=1.0 here
		colors[ImGuiCol_TableRowBg] = mvImGuiCol_TableRowBg;
		colors[ImGuiCol_TableRowBgAlt] = mvImGuiCol_TableRowBgAlt;
		colors[ImGuiCol_TextSelectedBg] = mvImGuiCol_TextSelectedBg;
		colors[ImGuiCol_DragDropTarget] = mvImGuiCol_DragDropTarget;
		colors[ImGuiCol_NavHighlight] = mvImGuiCol_NavHighlight;
		colors[ImGuiCol_NavWindowingHighlight] = mvImGuiCol_NavWindowingHighlight;
		colors[ImGuiCol_NavWindowingDimBg] = mvImGuiCol_NavWindowingDimBg;
		colors[ImGuiCol_ModalWindowDimBg] = mvImGuiCol_ModalWindowDimBg;

		imnodes::GetStyle().colors[imnodes::ColorStyle_NodeBackground] = mvColor::ConvertToUnsignedInt(mvColor(62, 62, 62, 255));
		imnodes::GetStyle().colors[imnodes::ColorStyle_NodeBackgroundHovered] = mvColor::ConvertToUnsignedInt(mvColor(75, 75, 75, 255));
		imnodes::GetStyle().colors[imnodes::ColorStyle_NodeBackgroundSelected] = mvColor::ConvertToUnsignedInt(mvColor(75, 75, 75, 255));
		imnodes::GetStyle().colors[imnodes::ColorStyle_NodeOutline] = mvColor::ConvertToUnsignedInt(mvColor(100, 100, 100, 255));
		imnodes::GetStyle().colors[imnodes::ColorStyle_TitleBar] = mvColor::ConvertToUnsignedInt(mvImGuiCol_TitleBg);
		imnodes::GetStyle().colors[imnodes::ColorStyle_TitleBarHovered] = mvColor::ConvertToUnsignedInt(mvImGuiCol_TitleBgActive);
		imnodes::GetStyle().colors[imnodes::ColorStyle_TitleBarSelected] = mvColor::ConvertToUnsignedInt(mvImGuiCol_FrameBgActive);
		imnodes::GetStyle().colors[imnodes::ColorStyle_Link] = mvColor::ConvertToUnsignedInt(mvColor(255, 255, 255, 200));
		imnodes::GetStyle().colors[imnodes::ColorStyle_LinkHovered] = mvColor::ConvertToUnsignedInt(mvColor(66, 150, 250, 255));
		imnodes::GetStyle().colors[imnodes::ColorStyle_LinkSelected] = mvColor::ConvertToUnsignedInt(mvColor(66, 150, 250, 255));
		imnodes::GetStyle().colors[imnodes::ColorStyle_Pin] = mvColor::ConvertToUnsignedInt(mvColor(199, 199, 41, 255));
		imnodes::GetStyle().colors[imnodes::ColorStyle_PinHovered] = mvColor::ConvertToUnsignedInt(mvColor(255, 255, 50, 255));
		imnodes::GetStyle().colors[imnodes::ColorStyle_BoxSelector] = mvColor::ConvertToUnsignedInt(mvColor(61, 133, 224, 30));
		imnodes::GetStyle().colors[imnodes::ColorStyle_BoxSelectorOutline] = mvColor::ConvertToUnsignedInt(mvColor(61, 133, 224, 150));
		imnodes::GetStyle().colors[imnodes::ColorStyle_GridBackground] = mvColor::ConvertToUnsignedInt(mvColor(35, 35, 35, 255));
		imnodes::GetStyle().colors[imnodes::ColorStyle_GridLine] = mvColor::ConvertToUnsignedInt(mvColor(0, 0, 0, 255));

	}

	void mvApp::DeleteApp()
	{
		if (s_instance)
		{
			delete s_instance;
			s_instance = nullptr;
		}

		s_started = false;
		s_id = MV_START_UUID;
	}

	void mvApp::cleanup()
	{
		getCallbackRegistry().submitCallback([=]() {
			mvApp::GetApp()->getCallbackRegistry().stop();
			});
		_future.get();
		if(_viewport)
			delete _viewport;
		s_started = false;
	}

	mvApp::mvApp()
	{
		// create managers
		
		itemRegistry = CreateOwnedPtr<mvItemRegistry>();
        _callbackRegistry = CreateOwnedPtr<mvCallbackRegistry>();
	}

    mvCallbackRegistry& mvApp::getCallbackRegistry()
    { 
        return *_callbackRegistry; 
    }

	mvApp::~mvApp()
	{

		ClearItemRegistry(*itemRegistry);

		constexpr_for<1, (int)mvAppItemType::ItemTypeCount, 1>(
			[&](auto i) {
				using item_type = typename mvItemTypeMap<i>::type;
				item_type::s_class_theme_component = nullptr;
			});

	}

	void mvApp::turnOnDocking(bool dockSpace)
	{ 
		_docking = true; 
		_dockingViewport = dockSpace;
	}

	void mvApp::render()
	{

		// update timing
		s_deltaTime = ImGui::GetIO().DeltaTime;
		s_time = ImGui::GetTime();
		s_frame = ImGui::GetFrameCount();
		s_framerate = ImGui::GetIO().Framerate;

		ImGui::GetIO().FontGlobalScale = mvToolManager::GetFontManager().getGlobalFontScale();

		if (_dockingViewport)
			ImGui::DockSpaceOverViewport();


		mvEventBus::Publish(mvEVT_CATEGORY_APP, mvEVT_FRAME, {CreateEventArgument("FRAME", ImGui::GetFrameCount() )});


		// route input callbacks
		mvInput::CheckInputs();

		mvToolManager::Draw();

		{
			std::lock_guard<std::mutex> lk(s_mutex);
			if (_resetTheme)
			{
				mvApp::SetDefaultTheme();
				_resetTheme = false;
			}

			mvEventBus::Publish(mvEVT_CATEGORY_APP, mvEVT_PRE_RENDER);
			mvEventBus::Publish(mvEVT_CATEGORY_APP, mvEVT_RENDER);
			RenderItemRegistry(*itemRegistry);
			mvEventBus::Publish(mvEVT_CATEGORY_APP, mvEVT_END_FRAME);
		}

		if (s_waitOneFrame == true)
			s_waitOneFrame = false;
	}

	std::map<std::string, mvPythonParser>& mvApp::getParsers()
	{ 
		return const_cast<std::map<std::string, mvPythonParser>&>(mvModule_DearPyGui::GetModuleParsers());
	}

	void mvApp::InsertParser(std::map<std::string, mvPythonParser>* parsers)
	{

		{
			std::vector<mvPythonDataElement> args;

			mvPythonParserSetup setup;
			setup.about = "Returns app configuration.";
			setup.category = { "General" };
			setup.returnType = mvPyDataType::Dict;

			mvPythonParser parser = FinalizeParser(setup, args);
			parsers->insert({ "get_app_configuration", parser });
		}

		{
			std::vector<mvPythonDataElement> args;
			args.push_back({ mvPyDataType::Bool, "docking", mvArgType::KEYWORD_ARG, "False", "Enables docking support." });
			args.push_back({ mvPyDataType::Bool, "docking_space", mvArgType::KEYWORD_ARG, "False", "add explicit dockspace over viewport" });
			args.push_back({ mvPyDataType::String, "load_init_file", mvArgType::KEYWORD_ARG, "''", "Load .ini file." });
			args.push_back({ mvPyDataType::String, "init_file", mvArgType::KEYWORD_ARG, "''" });
			args.push_back({ mvPyDataType::Integer, "device", mvArgType::KEYWORD_ARG, "-1", "Which display adapter to use. (-1 will use default)" });
			args.push_back({ mvPyDataType::Bool, "auto_device", mvArgType::KEYWORD_ARG, "False", "Let us pick the display adapter." });
			args.push_back({ mvPyDataType::Bool, "allow_alias_overwrites", mvArgType::KEYWORD_ARG, "False" });
			args.push_back({ mvPyDataType::Bool, "manual_alias_management", mvArgType::KEYWORD_ARG, "False" });
			args.push_back({ mvPyDataType::Bool, "skip_required_args", mvArgType::KEYWORD_ARG, "False" });
			args.push_back({ mvPyDataType::Bool, "skip_positional_args", mvArgType::KEYWORD_ARG, "False" });
			args.push_back({ mvPyDataType::Bool, "skip_keyword_args", mvArgType::KEYWORD_ARG, "False" });

			mvPythonParserSetup setup;
			setup.about = "Configures app.";
			setup.category = { "General" };

			mvPythonParser parser = FinalizeParser(setup, args);
			parsers->insert({ "configure_app", parser });
		}

		{
			std::vector<mvPythonDataElement> args;
			args.push_back({ mvPyDataType::String, "file" });

			mvPythonParserSetup setup;
			setup.about = "Save dpg.ini file.";
			setup.category = { "General" };

			mvPythonParser parser = FinalizeParser(setup, args);
			parsers->insert({ "save_init_file", parser });
		}

		{
			std::vector<mvPythonDataElement> args;
			args.push_back({ mvPyDataType::Integer, "delay", mvArgType::KEYWORD_ARG, "32", "Minimal delay in in milliseconds" });

			mvPythonParserSetup setup;
			setup.about = "Waits one frame.";
			setup.category = { "General" };

			mvPythonParser parser = FinalizeParser(setup, args);
			parsers->insert({ "split_frame", parser });
		}

		{
			std::vector<mvPythonDataElement> args;

			mvPythonParserSetup setup;
			setup.about = "Returns frame count.";
			setup.category = { "General" };
			setup.returnType = mvPyDataType::Integer;

			mvPythonParser parser = FinalizeParser(setup, args);
			parsers->insert({ "get_frame_count", parser });
		}

		{
			std::vector<mvPythonDataElement> args;
			args.push_back({ mvPyDataType::String, "file" });
			args.push_back({ mvPyDataType::Float, "gamma", mvArgType::KEYWORD_ARG, "1.0", "Gamma correction factor. (default is 1.0 to avoid automatic gamma correction on loading." });
			args.push_back({ mvPyDataType::Float, "gamma_scale_factor", mvArgType::KEYWORD_ARG, "1.0", "Gamma scale factor." });
			
			mvPythonParserSetup setup;
			setup.about = "Loads an image. Returns width, height, channels, mvBuffer";
			setup.category = { "Textures", "Utilities"};
			setup.returnType = mvPyDataType::Object;
			
			mvPythonParser parser = FinalizeParser(setup, args);
			parsers->insert({ "load_image", parser });
		}

		{
			std::vector<mvPythonDataElement> args;

			mvPythonParserSetup setup;
			setup.about = "Generate a new UUID.";
			setup.category = { "General" };
			setup.returnType = mvPyDataType::UUID;

			mvPythonParser parser = FinalizeParser(setup, args);
			parsers->insert({ "generate_uuid", parser });
		}

		{
			std::vector<mvPythonDataElement> args;

			mvPythonParserSetup setup;
			setup.about = "Locks render thread mutex.";
			setup.category = { "General" };

			mvPythonParser parser = FinalizeParser(setup, args);
			parsers->insert({ "lock_mutex", parser });
		}

		{
			std::vector<mvPythonDataElement> args;

			mvPythonParserSetup setup;
			setup.about = "Unlocks render thread mutex";
			setup.category = { "General" };

			mvPythonParser parser = FinalizeParser(setup, args);
			parsers->insert({ "unlock_mutex", parser });
		}

		{
			std::vector<mvPythonDataElement> args;

			mvPythonParserSetup setup;
			setup.about = "Checks if Dear PyGui is running";
			setup.category = { "General" };
			setup.returnType = mvPyDataType::Bool;

			mvPythonParser parser = FinalizeParser(setup, args);
			parsers->insert({ "is_dearpygui_running", parser });
		}

		{
			std::vector<mvPythonDataElement> args;

			mvPythonParserSetup setup;
			setup.about = "Sets up Dear PyGui";
			setup.category = { "General" };

			mvPythonParser parser = FinalizeParser(setup, args);
			parsers->insert({ "setup_dearpygui", parser });
		}

		{
			std::vector<mvPythonDataElement> args;

			mvPythonParserSetup setup;
			setup.about = "Render a single Dear PyGui frame.";
			setup.category = { "General" };

			mvPythonParser parser = FinalizeParser(setup, args);
			parsers->insert({ "render_dearpygui_frame", parser });
		}

		{
			std::vector<mvPythonDataElement> args;
	
			mvPythonParserSetup setup;
			setup.about = "Cleans up Dear PyGui";
			setup.category = { "General" };

			mvPythonParser parser = FinalizeParser(setup, args);
			parsers->insert({ "cleanup_dearpygui", parser });
		}

		{
			std::vector<mvPythonDataElement> args;

			mvPythonParserSetup setup;
			setup.about = "Stops Dear PyGui";
			setup.category = { "General" };

			mvPythonParser parser = FinalizeParser(setup, args);
			parsers->insert({ "stop_dearpygui", parser });
		}

		{
			std::vector<mvPythonDataElement> args;

			mvPythonParserSetup setup;
			setup.about = "Returns total time since Dear PyGui has started.";
			setup.category = { "General" };
			setup.returnType = mvPyDataType::Float;

			mvPythonParser parser = FinalizeParser(setup, args);
			parsers->insert({ "get_total_time", parser });
		}

		{
			std::vector<mvPythonDataElement> args;

			mvPythonParserSetup setup;
			setup.about = "Returns time since last frame.";
			setup.category = { "General" };
			setup.returnType = mvPyDataType::Float;

			mvPythonParser parser = FinalizeParser(setup, args);
			parsers->insert({ "get_delta_time", parser });
		}

		{
			std::vector<mvPythonDataElement> args;

			mvPythonParserSetup setup;
			setup.about = "Returns the average frame rate across 120 frames.";
			setup.category = { "General" };
			setup.returnType = mvPyDataType::Float;

			mvPythonParser parser = FinalizeParser(setup, args);
			parsers->insert({ "get_frame_rate", parser });
		}

	}

	PyObject* mvApp::save_init_file(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		const char* file;

		if (!Parse((mvApp::GetApp()->getParsers())["save_init_file"], args, kwargs, __FUNCTION__, &file))
			return GetPyNone();

		if (mvApp::IsAppStarted())
			ImGui::SaveIniSettingsToDisk(file);
		else
			mvThrowPythonError(mvErrorCode::mvNone, "Dear PyGui must be started to use \"save_init_file\".");

		return GetPyNone();
	}

	PyObject* mvApp::split_frame(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		int delay = 32;

		if (!Parse((mvApp::GetApp()->getParsers())["split_frame"], args, kwargs, __FUNCTION__,
			&delay))
			return GetPyNone();

		if (!mvApp::s_manualMutexControl) std::lock_guard<std::mutex> lk(mvApp::s_mutex);

		Py_BEGIN_ALLOW_THREADS;
		mvApp::s_waitOneFrame = true;
		while (s_waitOneFrame)
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		Py_END_ALLOW_THREADS;

		return GetPyNone();
	}

	PyObject* mvApp::lock_mutex(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		mvApp::s_mutex.lock();
		mvApp::s_manualMutexControl = true;

		return GetPyNone();
	}

	PyObject* mvApp::unlock_mutex(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		mvApp::s_mutex.unlock();
		mvApp::s_manualMutexControl = false;

		return GetPyNone();
	}

	PyObject* mvApp::get_frame_count(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		int frame = 0;

		if (!Parse((mvApp::GetApp()->getParsers())["get_frame_count"], args, kwargs, __FUNCTION__,
			&frame))
			return GetPyNone();

		if (!mvApp::s_manualMutexControl) std::lock_guard<std::mutex> lk(mvApp::s_mutex);
		return ToPyInt(mvApp::s_frame);
	}

	PyObject* mvApp::load_image(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		const char* file;
		float gamma = 1.0f;
		float gamma_scale = 1.0f;

		if (!Parse((mvApp::GetApp()->getParsers())["load_image"], args, kwargs, __FUNCTION__,
			&file, &gamma, &gamma_scale))
			return GetPyNone();

		// Vout = (Vin / 255)^v; Where v = gamma

		if (stbi_is_hdr(file))
		{
			stbi_hdr_to_ldr_gamma(gamma);
			stbi_hdr_to_ldr_scale(gamma_scale);
		}
		else
		{
			stbi_ldr_to_hdr_gamma(gamma);
			stbi_ldr_to_hdr_scale(gamma_scale);
		}
		

		// Load from disk into a raw RGBA buffer
		int image_width = 0;
		int image_height = 0;

		// automatic gamma correction
		float* image_data = stbi_loadf(file, &image_width, &image_height, NULL, 4);
		if (image_data == NULL)
			return GetPyNone();

		PyObject* newbuffer = nullptr;
		PymvBuffer* newbufferview = nullptr;
		newbufferview = PyObject_New(PymvBuffer, &PymvBufferType);
		newbufferview->arr.length = image_width * image_height * 4;
		newbufferview->arr.data = (float*)image_data;
		newbuffer = PyObject_Init((PyObject*)newbufferview, &PymvBufferType);

		PyObject* result = PyTuple_New(4);
		PyTuple_SetItem(result, 0, Py_BuildValue("i", image_width));
		PyTuple_SetItem(result, 1, Py_BuildValue("i", image_height));
		PyTuple_SetItem(result, 2, PyLong_FromLong(4));
		PyTuple_SetItem(result, 3, newbuffer);

		return result;
	}

	PyObject* mvApp::is_dearpygui_running(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		return ToPyBool(mvApp::IsAppStarted());
	}

	PyObject* mvApp::setup_dearpygui(PyObject* self, PyObject* args, PyObject* kwargs)
	{

		if (!mvApp::s_manualMutexControl) std::lock_guard<std::mutex> lk(mvApp::s_mutex);

		Py_BEGIN_ALLOW_THREADS;

		if (mvApp::IsAppStarted())
		{
			mvThrowPythonError(mvErrorCode::mvNone, "Cannot call \"setup_dearpygui\" while a Dear PyGUI app is already running.");
			return GetPyNone();
		}

		while (!GetApp()->itemRegistry->containers.empty())
			GetApp()->itemRegistry->containers.pop();
		MV_ITEM_REGISTRY_INFO("Container stack emptied.");

		s_started = true;
		GetApp()->_future = std::async(std::launch::async, []() {return GetApp()->_callbackRegistry->runCallbacks(); });

		MV_CORE_INFO("application starting");

		Py_END_ALLOW_THREADS;

		return GetPyNone();
	}

	PyObject* mvApp::render_dearpygui_frame(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		MV_PROFILE_SCOPE("Frame")

		Py_BEGIN_ALLOW_THREADS;
		auto window = mvApp::GetApp()->getViewport();
		window->renderFrame();
		Py_END_ALLOW_THREADS;

		return GetPyNone();
	}

	PyObject* mvApp::cleanup_dearpygui(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		if (!mvApp::s_manualMutexControl) std::lock_guard<std::mutex> lk(mvApp::s_mutex);

		Py_BEGIN_ALLOW_THREADS;
		mvApp::GetApp()->cleanup();	
		mvApp::DeleteApp();
		mvEventBus::Reset();
		Py_END_ALLOW_THREADS;

		return GetPyNone();
	}

	PyObject* mvApp::stop_dearpygui(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		if (!mvApp::s_manualMutexControl) std::lock_guard<std::mutex> lk(mvApp::s_mutex);
		mvApp::StopApp();
		auto viewport = mvApp::GetApp()->getViewport();
		if (viewport)
			viewport->stop();
		return GetPyNone();
	}

	PyObject* mvApp::get_total_time(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		if(!mvApp::s_manualMutexControl) std::lock_guard<std::mutex> lk(mvApp::s_mutex);
		return ToPyFloat((float)mvApp::s_time);
	}

	PyObject* mvApp::get_delta_time(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		if (!mvApp::s_manualMutexControl) std::lock_guard<std::mutex> lk(mvApp::s_mutex);
		return ToPyFloat(mvApp::s_deltaTime);

	}

	PyObject* mvApp::get_frame_rate(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		if (!mvApp::s_manualMutexControl) std::lock_guard<std::mutex> lk(mvApp::s_mutex);
		return ToPyFloat(mvApp::s_framerate);

	}

	PyObject* mvApp::generate_uuid(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		return ToPyUUID(mvApp::GenerateUUID());
	}

	PyObject* mvApp::configure_app(PyObject* self, PyObject* args, PyObject* kwargs)
	{

		int docking = false;
		int docking_space = false;
		const char* load_init_file = "";
		const char* init_file = "";
		int device = -1;
		int auto_device = false;
		int allow_alias_overwrites = false;
		int manual_alias_management = false;
		int skip_required_args = false;
		int skip_positional_args = false;
		int skip_keyword_args = false;

		if (!Parse((mvApp::GetApp()->getParsers())["configure_app"], args, kwargs, __FUNCTION__,
			&docking, &docking_space, &load_init_file, &init_file, &device, &auto_device,
			&allow_alias_overwrites, &manual_alias_management, &skip_required_args, &skip_positional_args, &skip_keyword_args))
			return GetPyNone();

		if (!mvApp::s_manualMutexControl) std::lock_guard<std::mutex> lk(mvApp::s_mutex);

		if (docking)
		{
			mvApp::GetApp()->_docking = docking;
			mvApp::GetApp()->_dockingViewport = docking_space;
		}

		mvApp::GetApp()->_iniFile = init_file;
		mvApp::GetApp()->_info_device = device;
		mvApp::GetApp()->_info_auto_device = auto_device;
		mvApp::GetApp()->itemRegistry->allowAliasOverwrites = allow_alias_overwrites;
		mvApp::GetApp()->itemRegistry->manualAliasManagement = manual_alias_management;
		mvApp::GetApp()->itemRegistry->skipPositionalArgs = skip_positional_args;
		mvApp::GetApp()->itemRegistry->skipKeywordArgs = skip_keyword_args;
		mvApp::GetApp()->itemRegistry->skipRequiredArgs = skip_required_args;

		if (!std::string(load_init_file).empty())
		{
			mvApp::GetApp()->_iniFile = load_init_file;
			mvApp::GetApp()->_loadIniFile = true;
		}

		return GetPyNone();
	}

	PyObject* mvApp::get_app_configuration(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		if (!mvApp::s_manualMutexControl) std::lock_guard<std::mutex> lk(mvApp::s_mutex);
		PyObject* pdict = PyDict_New();
		PyDict_SetItemString(pdict, "auto_device", mvPyObject(ToPyBool(mvApp::GetApp()->_info_auto_device)));
		PyDict_SetItemString(pdict, "docking", mvPyObject(ToPyBool(mvApp::GetApp()->_docking)));
		PyDict_SetItemString(pdict, "docking_space", mvPyObject(ToPyBool(mvApp::GetApp()->_docking)));
		PyDict_SetItemString(pdict, "load_init_file", mvPyObject(ToPyBool(mvApp::GetApp()->_loadIniFile)));
		PyDict_SetItemString(pdict, "version", mvPyObject(ToPyString(mvApp::GetApp()->GetVersion())));
		PyDict_SetItemString(pdict, "init_file", mvPyObject(ToPyString(mvApp::GetApp()->_iniFile)));
		PyDict_SetItemString(pdict, "platform", mvPyObject(ToPyString(mvApp::GetPlatform())));
		PyDict_SetItemString(pdict, "device", mvPyObject(ToPyInt(mvApp::GetApp()->_info_device)));
		PyDict_SetItemString(pdict, "device_name", mvPyObject(ToPyString(mvApp::GetApp()->_info_device_name)));
		PyDict_SetItemString(pdict, "allow_alias_overwrites", mvPyObject(ToPyBool(mvApp::GetApp()->itemRegistry->allowAliasOverwrites)));
		PyDict_SetItemString(pdict, "manual_alias_management", mvPyObject(ToPyBool(mvApp::GetApp()->itemRegistry->manualAliasManagement)));
		PyDict_SetItemString(pdict, "skip_keyword_args", mvPyObject(ToPyBool(mvApp::GetApp()->itemRegistry->skipKeywordArgs)));
		PyDict_SetItemString(pdict, "skip_positional_args", mvPyObject(ToPyBool(mvApp::GetApp()->itemRegistry->skipPositionalArgs)));
		PyDict_SetItemString(pdict, "skip_required_args", mvPyObject(ToPyBool(mvApp::GetApp()->itemRegistry->skipRequiredArgs)));
		return pdict;
	}
}
