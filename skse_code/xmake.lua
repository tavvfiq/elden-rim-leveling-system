set_xmakever("2.8.2")

set_project("eldenrimlevelingsystem")
set_version("0.1.0")

set_languages("c++23")
set_warnings("allextra")

add_rules("mode.debug", "mode.releasedbg")
set_defaultmode("releasedbg")

-- TODO: make this path configurable/relative
includes("D:/Modding/CommonLibSSE-NG")

add_requires("nlohmann_json")

target("eldenrimlevelingsystem")
    add_deps("commonlibsse-ng")
    add_rules("commonlibsse-ng.plugin", {
        name = "Elden Rim Leveling System",
        author = "taufiq.nugroho",
        description = "ER-style attribute system and derived stats for Skyrim.",
        email = "unknown@example.com"
    })

    set_pcxxheader("src/pch.h")
    add_includedirs("src", { public = true })

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_packages("nlohmann_json")

