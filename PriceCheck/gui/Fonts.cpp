#include "pch.h"
#include "Fonts.h"

#include <utility>
#include "bakkesmod/wrappers/GuiManagerWrapper.h"

Fonts::Descriptor::Descriptor(const char* inName, const char* inPath, int inSize)
	: name(inName), path(inPath), size(inSize)
{
}

void Fonts::Initialize(std::shared_ptr<GameWrapper> gameWrapper)
{
	Fonts& f = instance();
	f.gw = std::move(gameWrapper);

	// Start loading everything we got so far now
	for (auto [_, val] : f.instanceMap)
		val.load(f.gw.get());
}

void Fonts::LoadFont(const Descriptor& desc)
{
	Fonts& f = instance();
	const size_t key = hashString(desc.name);

	// Start loading now if initialized
	if (f.gw != nullptr)
	{
		if (auto [pair, success] = f.instanceMap.emplace(key, FontInstance{ desc, 0, nullptr }); success)
			pair->second.load(f.gw.get());
	}
}

ImFont* Fonts::GetDefaultFont()
{
	static ImFont* defaultFont = instance().gw->GetGUIManager().GetFont("default");
	return defaultFont;
}

ImFont* Fonts::GetFont(const char* font)
{
	Fonts& f = instance();
	const auto it = f.instanceMap.find(hashString(font));
	if (it == f.instanceMap.end())
	{
		LOG("No font named `{}`", font);
		return nullptr;
	}

	FontInstance& inst = (*it).second;
	return inst.load(f.gw.get());
}

Fonts& Fonts::instance()
{
    static Fonts s_fonts;
    return s_fonts;
}

size_t Fonts::hashString(const char* str)
{
	return std::hash<std::string_view>{}(str);
}

ImFont* Fonts::FontInstance::load(GameWrapper* localGameWrapper)
{
	// Perform the actual load
	auto [result, ptr] =
		localGameWrapper->GetGUIManager().LoadFont(desc.name, desc.path, desc.size);

	switch (result)
	{
	default:
	case 0:
	{
		LOG("Failed to load font `{}-{}` from `{}`", desc.name, desc.size, desc.path);
		return nullptr;
	}
	case 1:
		status = 1;
		break;
	case 2:
		status = 2;
		loaded = ptr;
		break;
	}

	return loaded;
}

