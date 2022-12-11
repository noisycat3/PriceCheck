#pragma once

/*
 * Fonts are tricky, because they are not loaded immediately.
 * If queried soon enough, they will probably be ready in time.
 * It's nice to handle all cases tho.
 * This manager provides utils for loading and fallback for the loading period.
 */
class Fonts
{
public:
	// A font that we want to load/use
	struct Descriptor
	{
		Descriptor(const char* inName, const char* inPath, int inSize);

		const char* name;
		const char* path;
		const int size;
	};

	// Initializes font manager
	static void Initialize(std::shared_ptr<GameWrapper> gameWrapper); // call before getting any thing!

	// Schedules font to be loaded
	static void LoadFont(const Descriptor& desc);

	// Get default font
	static ImFont* GetDefaultFont();

	// Get font by name
	static ImFont* GetFont(const char* font);

private:
	Fonts() = default; // Singleton, private constructor. Use static methods.
	static Fonts& instance();

	// Utility to hash strings with std::hash
	static size_t hashString(const char* str);

	// Instance data
	std::shared_ptr<GameWrapper> gw;

	struct FontInstance {
		const Descriptor desc;
		int64_t status;
		ImFont* loaded;

		ImFont* load(GameWrapper* localGameWrapper);
	};
	
	std::unordered_map<size_t, FontInstance> instanceMap;
};

/*
 * Tries to grab a given font, fallback to default
 * Font must be already loaded by Fonts::LoadFont(...)
 * Usage:
 *		static FontLoader fontNameLoader("loadedFontIdentifier");
 *		ImFont* fontName = fontNameLoader.get();
 */
struct FontLoader
{
	FontLoader(const char* name) : targetName(name), cachedFont(nullptr) { }

	// Access the font
	ImFont* get()
	{
		if (cachedFont != nullptr)
			return cachedFont;
		cachedFont = Fonts::GetFont(targetName);
		return cachedFont ? cachedFont : Fonts::GetDefaultFont();
	}

private:
	const char* targetName;
	ImFont* cachedFont;
};

// Macro to cache the font in any place
#define USE_FONT( font ) \
	[]() { static FontLoader fl( font ); return fl.get(); }()
