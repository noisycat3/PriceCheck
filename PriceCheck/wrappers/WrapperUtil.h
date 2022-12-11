#pragma once

#include <vector>

// Single pointer path
struct PointerPath
{
	PointerPath() = default;
	PointerPath(ptrdiff_t inModuleOffset, std::initializer_list<uint16_t> inOffsets)
		: moduleOffset(inModuleOffset), offsets(inOffsets)
	{ }

	[[nodiscard]] void* get() const;
	void getAll(std::vector<void*>& out) const;

	template <typename T>
	[[nodiscard]] T* get() const { return static_cast<T*>(get()); }

	[[nodiscard]] size_t length() const { return offsets.size(); }
	[[nodiscard]] bool isValid() const { return moduleOffset != 0; }

private:
	ptrdiff_t moduleOffset = 0;
	std::vector<uint16_t> offsets;

	friend struct fmt::formatter<PointerPath>;
};

template <> struct fmt::formatter<PointerPath> : formatter<double> {
	auto format(PointerPath c, format_context& ctx) const
	{
		format_to(ctx.out(), "[ .exe+{:#016x}", c.moduleOffset);
		for (uint16_t offset : c.offsets)
			format_to(ctx.out(), " -> {:#04x}", offset);
		return format_to(ctx.out(), " ]");
	}
};

// Version for each of supported environments
struct HostDependentPointerPath
{
	enum EHost
	{
		STEAM = 0,
		EPIC = 1,

		HOST_Count // Keep last
	};

	// Empty paths
	HostDependentPointerPath() = default;

	// Build with paths
	template <typename... ARGS>
	HostDependentPointerPath(ARGS... paths) : paths{ paths... } { }

	// Access
	[[nodiscard]] const PointerPath& getFor(EHost host) const { return paths[host]; }

private:
	// Data storage
	std::array<PointerPath, HOST_Count> paths = { PointerPath(), PointerPath() };
};

struct VersionedPointerPath
{
	// Initialize the system using this call. Versions will be resolved with this.
	static void setVersion(const std::string& version, HostDependentPointerPath::EHost host)
	{
		s_version = version;
		s_versionCrc = crc32(version);
		s_hostEnv = host;
	}

private:
	static std::string s_version;
	static uint32_t s_versionCrc;
	static HostDependentPointerPath::EHost s_hostEnv;

public:
	VersionedPointerPath() = default;

	template <typename... ARGS>
	VersionedPointerPath(ARGS... args) : map( { std::forward<ARGS>(args)... } ) { }

	[[nodiscard]] const PointerPath& resolve() const { return map.at(s_versionCrc).getFor(s_hostEnv); }
	[[nodiscard]] void* get() const { return resolve().get(); }
	template <typename T>
	[[nodiscard]] T* get() const { return resolve().get<T>(); }

private:
	std::unordered_map<uint32_t, HostDependentPointerPath> map;
};

#define MAKE_VPP( name, ... ) \
	static const VersionedPointerPath name = VersionedPointerPath( __VA_ARGS__ )

#define VPP_CASE( ver, ... ) \
	std::pair<uint32_t, HostDependentPointerPath>{ crc32(#ver), HostDependentPointerPath( __VA_ARGS__ ) }