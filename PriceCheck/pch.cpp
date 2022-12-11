#include "pch.h"

#include "IMGUI/imgui_internal.h"
#include "shellapi.h"

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_wcsicmp(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

bool operator==(const ProductInstanceID& A, const ProductInstanceID& B)
{
	return A.lower_bits == B.lower_bits && A.upper_bits == B.upper_bits;
}

// From https://discord.com/channels/862068148328857700/862081441080410143/1003917914254094387
//  Thanks ItsBranK#1337 !
namespace ImGui
{
    bool Hyperlink(const std::string& label, const std::string& url, const ImVec4& textColor, const ImVec4& hoverColor)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) { return false; }

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        const char* text_begin = label.c_str();
        const char* text_end = (label.c_str() + strlen(label.c_str()));

        const ImVec2 text_pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);
        const float wrap_pos_x = window->DC.TextWrapPos;
        const bool wrap_enabled = (wrap_pos_x >= 0.0f);

        const float wrap_width = (wrap_enabled ? ImGui::CalcWrapWidthForPos(window->DC.CursorPos, wrap_pos_x) : 0.0f);
        const ImVec2 text_size = ImGui::CalcTextSize(text_begin, text_end, false, wrap_width);

        ImRect bb(text_pos, text_pos + text_size);
        ImGui::ItemSize(text_size, 0.0f);
        if (!ImGui::ItemAdd(bb, 0)) { return false; }

        bool hovered = ImGui::IsItemHovered();
        window->DrawList->AddLine(ImVec2(bb.Min.x, bb.Max.y), ImVec2(bb.Max.x, bb.Max.y), hovered ? ImGui::GetColorU32(hoverColor) : ImGui::GetColorU32(textColor));

        if (hovered) { ImGui::PushStyleColor(ImGuiCol_Text, hoverColor); }
        else { ImGui::PushStyleColor(ImGuiCol_Text, textColor); }
        ImGui::RenderTextWrapped(bb.Min, text_begin, text_end, wrap_width);
        ImGui::PopStyleColor();

        if (ImGui::IsItemClicked())
        {
            ShellExecuteA(NULL, NULL, url.c_str(), NULL, NULL, SW_SHOW);
            return true;
        }

        return false;
    }

    bool Hyperlink(const std::string& label, const std::string& url, const ImVec4& hoverColor)
    {
        return Hyperlink(label, url, GImGui->Style.Colors[ImGuiCol_Text], hoverColor);
    }

    bool Hyperlink(const std::string& url, const ImVec4& textColor, const ImVec4& hoverColor)
    {
        return Hyperlink(url, url, textColor, hoverColor);
    }

    bool Hyperlink(const std::string& url, const ImVec4& hoverColor)
    {
        return Hyperlink(url, url, hoverColor);
    }
}
