#include "hlp_imgui.hh"

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx11.h"

#include <iostream>

namespace hlp
{
	U0 InitStyle()
	{
		// 1. Start from Dark style (you could also begin from Classic or Light)
		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();

		// 2. Override all relevant colors
		ImVec4& colWindowBg = style.Colors[ImGuiCol_WindowBg];
		ImVec4& colFrameBg = style.Colors[ImGuiCol_FrameBg];
		ImVec4& colFrameBgHovered = style.Colors[ImGuiCol_FrameBgHovered];
		ImVec4& colFrameBgActive = style.Colors[ImGuiCol_FrameBgActive];
		ImVec4& colHeader = style.Colors[ImGuiCol_Header];
		ImVec4& colHeaderHovered = style.Colors[ImGuiCol_HeaderHovered];
		ImVec4& colHeaderActive = style.Colors[ImGuiCol_HeaderActive];
		ImVec4& colButton = style.Colors[ImGuiCol_Button];
		ImVec4& colButtonHovered = style.Colors[ImGuiCol_ButtonHovered];
		ImVec4& colButtonActive = style.Colors[ImGuiCol_ButtonActive];
		ImVec4& colText = style.Colors[ImGuiCol_Text];
		ImVec4& colTextDisabled = style.Colors[ImGuiCol_TextDisabled];
		ImVec4& colBorder = style.Colors[ImGuiCol_Border];
		ImVec4& colSeparator = style.Colors[ImGuiCol_Separator];
		ImVec4& colScrollbarBg = style.Colors[ImGuiCol_ScrollbarBg];
		ImVec4& colScrollbarGrab = style.Colors[ImGuiCol_ScrollbarGrab];
		ImVec4& colScrollbarGrabH = style.Colors[ImGuiCol_ScrollbarGrabHovered];
		ImVec4& colScrollbarGrabA = style.Colors[ImGuiCol_ScrollbarGrabActive];

		// Window background
		colWindowBg = ImVec4(0.13f, 0.13f, 0.13f, 1.00f); // #212121

		// Frames (inputs, buttons, etc.)
		colFrameBg = ImVec4(0.20f, 0.20f, 0.20f, 1.00f); // #333333
		colFrameBgHovered = ImVec4(0.30f, 0.30f, 0.30f, 1.00f); // #4D4D4D
		colFrameBgActive = ImVec4(0.24f, 0.24f, 0.24f, 1.00f); // #3D3D3D

		// Headers (tree nodes, collapsing headers, etc.)
		colHeader = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colHeaderHovered = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
		colHeaderActive = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);

		// Buttons
		colButton = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);   // #3D3D3D
		colButtonHovered = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);  // #4D4D4D
		colButtonActive = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);  // #5C5C5C

		// Text
		colText = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);   // #E6E6E6
		colTextDisabled = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);   // #7F7F7F

		// Borders & separators
		colBorder = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);   // #666666
		colSeparator = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);

		// Scrollbars
		colScrollbarBg = ImVec4(0.10f, 0.10f, 0.10f, 1.00f); // #1A1A1A
		colScrollbarGrab = ImVec4(0.30f, 0.30f, 0.30f, 1.00f); // #4D4D4D
		colScrollbarGrabH = ImVec4(0.36f, 0.36f, 0.36f, 1.00f); // #5C5C5C
		colScrollbarGrabA = ImVec4(0.42f, 0.42f, 0.42f, 1.00f); // #6B6B6B

		// 3. Tweak style variables for a more polished look
		//style.FrameRounding = 4.0f;
		//style.WindowRounding = 6.0f;
		//style.GrabRounding = 3.0f;
		//style.TabRounding = 3.0f;

		style.WindowPadding = ImVec2(10, 10);
		style.FramePadding = ImVec2(6, 4);
		style.ItemSpacing = ImVec2(6, 6);
		style.ItemInnerSpacing = ImVec2(4, 4);

		style.WindowBorderSize = 1.0f;
		style.FrameBorderSize = 1.0f;
		style.PopupBorderSize = 1.0f;

		style.ScrollbarSize = 12.0f;
	}

	U0 InitImgui(U0* phWnd, U0* pDevice, U0* pDevCon)
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(*((HWND*)phWnd));
		ImGui_ImplDX11_Init((ID3D11Device*)pDevice, (ID3D11DeviceContext*)pDevCon);

		InitStyle();
	}

	U0 NewImguiFrame(FAppState* pAppState)
	{
		// New Frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
#if 1
		ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Once);

		// Static values keep their state across frames
		static char inputText[128] = "Type here";

		ImGui::Begin("HLP GUI");
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

		// Renderer Settings
		ImGui::Separator();
		ImGui::Text("Rendering Settings");
		ImGui::NewLine();
		if (ImGui::Button("Solid"))
		{
			pAppState->pRenderState->bChangeRasterizer = !pAppState->pRenderState->bChangeRasterizer;
			pAppState->pRenderState->bRenderSolid = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Wireframe"))
		{
			pAppState->pRenderState->bChangeRasterizer = !pAppState->pRenderState->bChangeRasterizer;
			pAppState->pRenderState->bRenderWireframe = true;
		}

		ImGui::Text("Rendering Stats");
		ImGui::Text("IBuf size: %d", pAppState->pRenderState->TotNumIdx);
		ImGui::Text("VBuf size: %d", pAppState->pRenderState->TotNumVerts);
		ImGui::Text("Num Meshes: %d", pAppState->pSceneState->NumGeometries);

		// Cam Settings
		ImGui::Separator();
		ImGui::Text("Cam Settings");
		ImGui::SliderFloat("Camera Speed", &pAppState->CamSpeed, 0.001f, 1.0f);
		ImGui::SliderFloat("Mouse Sens", &pAppState->MouseSens, 0.001f, 1.0f);
		ImGui::SliderFloat("FOV", &pAppState->pSceneState->Cam.HorizontalFOVDeg, 1.0f, 180.0f);

		// Text input
		ImGui::InputText("Input String", inputText, IM_ARRAYSIZE(inputText));

		// Buttons
		if (ImGui::Button("Print String"))
		{
			std::cout << "User Input: " << inputText << std::endl;
		}

		ImGui::SameLine();

		if (ImGui::Button("Clear"))
		{
			inputText[0] = '\0';
		}

		ImGui::Separator();

		const char* items[] = { "Option A", "Option B", "Option C" };
		static int current = 0;

		if (ImGui::BeginCombo("Custom Combo", items[current]))  // Label + Preview
		{
			for (int i = 0; i < IM_ARRAYSIZE(items); i++)
			{
				bool is_selected = (current == i);
				if (ImGui::Selectable(items[i], is_selected))
					current = i;
				if (is_selected)
					ImGui::SetItemDefaultFocus(); // Optional: set focus on the selected item
			}
			ImGui::EndCombo();
		}

		ImGui::End();
#else
		ImGui::ShowDemoWindow();
#endif
	}

	U0 DrawImguiFrame()
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	U0 CleanUpImgui()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}