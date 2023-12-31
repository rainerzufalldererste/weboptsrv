ProjectName = "weboptsrv"
project(ProjectName)

  --Settings
  kind "ConsoleApp"
  language "C++"
  staticruntime "On"

  dependson { "zydec", "execution-flow" }

  filter { "system:windows" }
    ignoredefaultlibraries { "msvcrt" }
  filter { "system:linux" }
    cppdialect "C++20"
    buildoptions { "-DASIO_STANDALONE" } -- is this needed?
  filter { }

  filter { "system:windows", "configurations:not *Clang" }
    buildoptions { '/std:c++20' }
    buildoptions { '/Gm-' }
    buildoptions { '/MP' }

  filter { "system:windows", "configurations:*Clang" }
    toolset("clang")
    cppdialect "C++17"
    defines { "__llvm__" }
  
  filter { }
  
  defines { "_CRT_SECURE_NO_WARNINGS", "SSE2" }
  
  objdir "intermediate/obj"

  files { "src/**.cpp", "src/**.c", "src/**.cc", "src/**.h", "src/**.hh", "src/**.hpp", "src/**.inl", "src/**rc" }
  files { "project.lua" }

  includedirs { "src**" }
  includedirs { "3rdParty/crow/include" }
  includedirs { "3rdParty/asio/include" }
  includedirs { "3rdParty/zydec/zydec/include" }
  includedirs { "3rdParty/zydec/3rdParty/Zydis/include" }
  includedirs { "3rdParty/execution-flow/execution-flow/include" }

  filter { "configurations:Debug", "system:Windows" }
    ignoredefaultlibraries { "libcmt" }
  filter { }

  filter { "system:windows" }
    links { "3rdParty/execution-flow/builds/lib/execution-flow.lib" }
    links { "3rdParty/zydec/builds/lib/zydec.lib" }
    links { "3rdParty/zydec/3rdParty/zydis/lib/Zydis.lib" }
    links { "3rdParty/asio/.lib" }
  filter { "system:linux" }
    links { "execution-flow" }
    links { "zydec" }
    
    libdirs { "3rdParty/zydec/3rdParty/zydis/lib" }
    links { "Zydis" }
    
    libdirs { "/usr/lib/llvm-17/lib" }
    links { "LLVMAnalysis", "LLVMBitstreamReader", "LLVMCodeGen", "LLVMTransformUtils", "LLVMTarget", "LLVMSelectionDAG", "LLVMGlobalISel", "LLVMCFGuard", "LLVMScalarOpts", "LLVMObjCARCOpts", "LLVMDebugInfoCodeView", "LLVMDebugInfoPDB", "LLVMMC", "LLVMMCParser", "LLVMAsmParser", "LLVMRemarks", "LLVMTargetParser", "LLVMX86Disassembler", "LLVMBinaryFormat", "LLVMCodeGenTypes", "LLVMDebugInfoDWARF", "LLVMDemangle", "LLVMMCA", "LLVMObject", "LLVMSupport", "LLVMTextAPI", "LLVMX86Info", "LLVMBitReader", "LLVMCore", "LLVMDebugInfoMSF", "LLVMIRReader", "LLVMMCDisassembler", "LLVMProfileData", "LLVMSymbolize", "LLVMX86CodeGen", "LLVMX86Desc", "LLVMX86TargetMCA" }
    
    libdirs { "/usr/lib/x86_64-linux-gnu" }
    links { "tinfo", "z", "zstd" }
  filter { }
    
  targetname(ProjectName)
  targetdir "builds/bin"
  debugdir "builds/bin"
  
filter {}

warnings "Extra"
flags { "FatalWarnings" }

filter {"configurations:Release"}
  targetname "%{prj.name}"
filter {"configurations:Debug"}
  targetname "%{prj.name}D"

filter {}
flags { "NoMinimalRebuild", "NoPCH" }
rtti "On"
floatingpoint "Fast"
exceptionhandling "On"

filter { "configurations:Debug*" }
	defines { "_DEBUG" }
	optimize "Off"
	symbols "On"

filter { "configurations:Release" }
	defines { "NDEBUG" }
	optimize "Speed"
	flags { "NoBufferSecurityCheck", "NoIncrementalLink" }
	omitframepointer "On"
  symbols "On"

filter { "system:linux", "configurations:ReleaseClang" }
  buildoptions { "-O3" }

filter { "system:windows", "configurations:Release" }
	flags { "NoIncrementalLink" }

editandcontinue "Off"
