add_rules("mode.debug", "mode.release", "mode.coverage")
add_linkdirs("/usr/local/lib")

lcpkg_dir = "./lcpkg/installed/$(arch)-$(os)"
lcpkg_incdir = lcpkg_dir.."/include"
lcpkg_pkgdir = lcpkg_dir
if is_mode("debug") then
    lcpkg_pkgdir = lcpkg_dir.."/debug"
end
lcpkg_libdir = lcpkg_pkgdir.."/lib"

add_includedirs("include", lcpkg_incdir)
add_linkdirs(lcpkg_libdir, "/usr/local/lib", "/usr/lib")

target("LCUIRouter")
    set_kind("static")
    add_files("src/*.c")

target("test")
    set_kind("binary")
    add_files("test/test.c")
    add_links("LCUI")
