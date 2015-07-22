-- Define the i99 tools solution.
solution "i99-tools"
	configurations { "Debug", "Release" }
	location ("build")
	
	--i99-map-transform
	project "i99-map-transform"
		kind "ConsoleApp"
		language "C++"
		location ("build/" .. project().name )
		files { "src/**.hpp", "src/**.cpp" }
		
		--Set the target properties
		targetname "i99-map-transform"
		
		
		--Search the lib folder for includes.
		includedirs { "lib" }
		
		configuration "Debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetdir ("bin/debug")
			
		configuration "Release"
			defines { "NDEBUG" }
			flags { "Optimize", "NoRTTI" }
			targetdir ("bin/release")
			
		configuration {}
		
		--Link against the static runtime
		flags { "StaticRuntime" }
		
		--Set the compiler to compile C++0x/11 code.
		configuration { "linux", "gmake" }
			buildoptions { "--std=c++0x" }
			
		configuration {}
			
		--Link to the boost libraries
		libdirs { "lib", "lib/boost" }
		links { "boost_system", "boost_filesystem", "boost_program_options" }
