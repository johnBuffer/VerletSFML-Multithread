-- dependencies
add_requires("sfml 2.6.x")

-- project
target("verlet-multi")

    -- settings
    set_kind("binary")
    set_languages("c++17")

    -- sources
    add_files("src/**.cpp")
    add_includedirs("src")

    -- linking
    add_packages("sfml")

    -- copy res function
    function copy_res(target)
        os.cp("res", path.join(target:targetdir(), "res"))
    end

    -- after build
    after_build(copy_res)