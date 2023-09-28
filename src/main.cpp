#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <exception>

#ifdef _MSC_VER
#pragma optimize("", off)
#endif

#define ASIO_STANDALONE 1
#define ASIO_NO_EXCEPTIONS 1

namespace asio
{
  namespace detail
  {
    template <typename Exception>
    void throw_exception(const Exception &e)
    {
      printf("Exception thrown: %s.\n", e.what());
    }
  }
}

#ifdef _MSC_VER
#pragma warning (push, 0)
#endif
#include "crow.h"
#include "crow/middlewares/cors.h"
#ifdef _MSC_VER
#pragma warning (pop)
#endif

#ifdef _WIN32
#include <Windows.h>
#else
#endif

#include "zydec.h"

#include "execution-flow.h"

//////////////////////////////////////////////////////////////////////////

enum CompilerType
{
  CT_GCCpp,
  CT_Clangpp,
};

crow::response handle_compile(const crow::request &req, const CompilerType type, const size_t version);
crow::response handle_zydec(const crow::request &req);
crow::response handle_execution_flow(const crow::request &req);

//////////////////////////////////////////////////////////////////////////

int32_t main(void)
{
  crow::App<crow::CORSHandler> app;

  auto &cors = app.get_middleware<crow::CORSHandler>();
  cors.global().origin("http://localhost");

  CROW_ROUTE(app, "/g++-x64-11").methods(crow::HTTPMethod::POST)([](const crow::request &req) { return handle_compile(req, CT_GCCpp, 11); });
  CROW_ROUTE(app, "/g++-x64-12").methods(crow::HTTPMethod::POST)([](const crow::request &req) { return handle_compile(req, CT_GCCpp, 12); });
  CROW_ROUTE(app, "/g++-x64-13").methods(crow::HTTPMethod::POST)([](const crow::request &req) { return handle_compile(req, CT_GCCpp, 13); });
  CROW_ROUTE(app, "/clang++-x64-14").methods(crow::HTTPMethod::POST)([](const crow::request &req) { return handle_compile(req, CT_Clangpp, 14); });
  CROW_ROUTE(app, "/clang++-x64-15").methods(crow::HTTPMethod::POST)([](const crow::request &req) { return handle_compile(req, CT_Clangpp, 15); });
  CROW_ROUTE(app, "/clang++-x64-16").methods(crow::HTTPMethod::POST)([](const crow::request &req) { return handle_compile(req, CT_Clangpp, 16); });
  CROW_ROUTE(app, "/clang++-x64-17").methods(crow::HTTPMethod::POST)([](const crow::request &req) { return handle_compile(req, CT_Clangpp, 17); });

  CROW_ROUTE(app, "/zydec").methods(crow::HTTPMethod::POST)([](const crow::request &req) { return handle_zydec(req); });
  CROW_ROUTE(app, "/execution_flow").methods(crow::HTTPMethod::POST)([](const crow::request &req) { return handle_execution_flow(req); });

  app.port(61919).multithreaded().run();
}

//////////////////////////////////////////////////////////////////////////

static std::atomic<size_t> _RequestID = 0;

crow::response handle_compile(const crow::request &req, const CompilerType type, const size_t version)
{
  auto x = crow::json::load(req.body);

  if (!x || !x.has("src") || !x.has("opt") || !x.has("march"))
    return crow::response(crow::status::BAD_REQUEST);

  const auto &opt = x["opt"].s();
  const auto &march = x["march"].s();
  const auto &src = x["src"].s();

  if (src.size() > 1024 * 8)
  {
    printf("Rejecting input, as src size: %" PRIu64 "\n", src.size());
    return crow::response(crow::status::PAYLOAD_TOO_LARGE);
  }

  std::string compileCmd;
  std::string dumpCmd = "objdump -M intel-mnemonic -j .text -C -d --insn-width=15 ";
  std::string objFileName;

  switch (type)
  {
  case CT_GCCpp:
    compileCmd.append("g++-");
    compileCmd.append(std::to_string(version));
    break;

  case CT_Clangpp:
    compileCmd.append("clang++-");
    compileCmd.append(std::to_string(version));
    break;

  default:
    return crow::response(crow::status::INTERNAL_SERVER_ERROR);
  }

  const size_t requestId = _RequestID++;

  objFileName.append(std::to_string(requestId));
  objFileName.append(".o");

  compileCmd.append(" -c -o ");
  compileCmd.append(objFileName);

  dumpCmd.append(std::to_string(requestId));
  dumpCmd.append(".o");
  dumpCmd.append("  2>&1");

  if (opt == "O0")
    compileCmd.append(" -O0");
  else if (opt == "O1")
    compileCmd.append(" -O1");
  else if (opt == "O2")
    compileCmd.append(" -O2");
  else if (opt == "O3")
    compileCmd.append(" -O3");
  else if (opt == "O4")
    compileCmd.append(" -O4");
  else if (opt == "Of")
    compileCmd.append(" -Ofast");
  else if (opt == "Os")
    compileCmd.append(" -Os");
  else if (opt == "Oz")
    compileCmd.append(" -Oz");
  else if (opt == "Og")
    compileCmd.append(" -Og");
  else
    return crow::response(crow::status::BAD_REQUEST);

  if (march == "adl") compileCmd.append(" -march=alderlake");
  else if (march == "brw") compileCmd.append(" -march=broadwell");
  else if (march == "cnl") compileCmd.append(" -march=cannonlake");
  else if (march == "ccl") compileCmd.append(" -march=cascadelake");
  else if (march == "cpl") compileCmd.append(" -march=cooperlake");
  else if (march == "elr") compileCmd.append(" -march=emeraldrapids");
  else if (march == "gmt") compileCmd.append(" -march=goldmont");
  else if (march == "gmp") compileCmd.append(" -march=goldmont_plus");
  else if (march == "gdr") compileCmd.append(" -march=grandridge");
  else if (march == "gtr") compileCmd.append(" -march=graniterapids");
  else if (march == "hsw") compileCmd.append(" -march=haswell");
  else if (march == "ilc") compileCmd.append(" -march=icelake_client");
  else if (march == "lcs") compileCmd.append(" -march=icelake_server");
  else if (march == "ivb") compileCmd.append(" -march=ivybridge");
  else if (march == "mtl") compileCmd.append(" -march=meteorlake");
  else if (march == "rtl") compileCmd.append(" -march=raptorlake");
  else if (march == "rkl") compileCmd.append(" -march=rocketlake");
  else if (march == "sdb") compileCmd.append(" -march=sandybridge");
  else if (march == "spr") compileCmd.append(" -march=sapphirerapids");
  else if (march == "srf") compileCmd.append(" -march=sierraforest");
  else if (march == "svm") compileCmd.append(" -march=silvermont");
  else if (march == "slc") compileCmd.append(" -march=skylake");
  else if (march == "slx") compileCmd.append(" -march=skx");
  else if (march == "sls") compileCmd.append(" -march=skylake_avx512");
  else if (march == "tgl") compileCmd.append(" -march=tigerlake");
  else if (march == "trm") compileCmd.append(" -march=tremont");
  else if (march == "zn1") compileCmd.append(" -march=znver1");
  else if (march == "zn2") compileCmd.append(" -march=znver2");
  else if (march == "zn3") compileCmd.append(" -march=znver3");
  else if (march == "zn4") compileCmd.append(" -march=znver4");

  std::string sourceName;
  sourceName.append(std::to_string(requestId));
  sourceName.append(".cpp");

  compileCmd.append(" ");
  compileCmd.append(sourceName);
  compileCmd.append("  2>&1");
  
  std::string compilerOut;

#ifdef _MSC_VER
  crow::json::wvalue ret; // only for testing.
  ret["compileCmd"] = compileCmd;

  crow::response res(crow::status::OK, ret);
  
  return res;
#else
  // Write Source File.
  {
    printf("trying to write file '%s'.\n", sourceName.c_str());
    FILE *pSource = fopen(sourceName.c_str(), "wb");

    if (pSource == nullptr)
    {
      remove(objFileName.c_str());
      return crow::response(crow::status::INTERNAL_SERVER_ERROR);
    }

    if (src.size() != fwrite(src.s_, 1, src.size(), pSource))
    {
      remove(objFileName.c_str());
      return crow::response(crow::status::INTERNAL_SERVER_ERROR);
    }

    fclose(pSource);
  }

  char line[1024];

  // Run Compiler.
  {
    printf("trying to call '%s'.\n", compileCmd.c_str());
    FILE *pCompilerOutput = popen(compileCmd.c_str(), "r");

    if (pCompilerOutput == nullptr)
    {
      remove(sourceName.c_str());
      remove(objFileName.c_str());
      return crow::response(crow::status::INTERNAL_SERVER_ERROR);
    }

    while (fgets(line, sizeof(line), pCompilerOutput) != nullptr)
      compilerOut.append(line);

    pclose(pCompilerOutput);
    remove(sourceName.c_str());
  }

  std::string objdumpOut;

  // Run Objdump.
  {
    printf("trying to call '%s'.\n", dumpCmd.c_str());
    FILE *pObjDumpOutput = popen(dumpCmd.c_str(), "r");

    if (pObjDumpOutput == nullptr)
    {
      remove(objFileName.c_str());
      return crow::response(crow::status::INTERNAL_SERVER_ERROR);
    }

    // Skip initial information.
    while (fgets(line, 2, pObjDumpOutput) != nullptr)
    {
      if (line[0] == '<')
      {
        objdumpOut = "<";
        break;
      }
    }

    while (fgets(line, sizeof(line), pObjDumpOutput) != nullptr)
      objdumpOut.append(line);

    pclose(pObjDumpOutput);
    remove(objFileName.c_str());
  }

  crow::json::wvalue ret;
  ret["compiler"] = compilerOut.c_str();
  ret["objdump"] = objdumpOut.c_str();

  return crow::response(crow::status::OK, ret);
#endif
}

//////////////////////////////////////////////////////////////////////////

crow::response handle_zydec(const crow::request &req)
{
  auto x = crow::json::load(req.body);

  if (!x || !x.has("bytes") || !x.has("addr"))
    return crow::response(crow::status::BAD_REQUEST);

  const auto &bytesBase64 = x["bytes"].s();
  const uint64_t addressDisplayOffset = x["addr"].i();

  if (bytesBase64.size() > 1024 * 2)
  {
    printf("Rejecting zydec, as bytesBase64 size: %" PRIu64 "\n", bytesBase64.size());
    return crow::response(crow::status::PAYLOAD_TOO_LARGE);
  }

  const auto &bytes = crow::utility::base64decode(bytesBase64);

  ZydisDecoder decoder;
  ZydisFormatter formatter;

  if (!ZYAN_SUCCESS(ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64)) || !ZYAN_SUCCESS(ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL)) || !ZYAN_SUCCESS(ZydisFormatterSetProperty(&formatter, ZYDIS_FORMATTER_PROP_FORCE_SEGMENT, ZYAN_TRUE)) || !ZYAN_SUCCESS(ZydisFormatterSetProperty(&formatter, ZYDIS_FORMATTER_PROP_FORCE_SIZE, ZYAN_TRUE)))
  {
    puts("Failed to init zydis decoder, formatter.");
    return crow::response(crow::status::INTERNAL_SERVER_ERROR);
  }

  ZydisDecodedInstruction instruction;
  ZydisDecodedOperand operands[10];
  ZydecFormattingInfo info;
  ZydecLinearContext linearContext;

  std::string zydecOut;

  info.afterCallRegisterRetentionMode = ZydecFormattingInfo::AfterCallRegisterRetentionMode::Linux;

  char decompBuffer[1024] = "";

  size_t virtualAddress = 0;
  const size_t fileSize = bytes.size();
  const uint8_t *pData = reinterpret_cast<const uint8_t *>(bytes.c_str());

  while (virtualAddress < fileSize)
  {
    if (!(ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, pData + virtualAddress, fileSize - virtualAddress, &instruction, operands))))
    {
      puts("Failed to zydis decode bytes.");
      return crow::response(crow::status::BAD_REQUEST);
    }

    bool hasTranslation = false;

    if (!zydec_TranslateInstructionWithLinearContext(&linearContext, &instruction, operands, sizeof(operands) / sizeof(operands[0]), virtualAddress + addressDisplayOffset, decompBuffer, sizeof(decompBuffer), &hasTranslation, &info) || !hasTranslation)
    {
      decompBuffer[0] = '/';
      decompBuffer[1] = '/';
      decompBuffer[2] = ' ';
      decompBuffer[3] = '\0';

      if (!(ZYAN_SUCCESS(ZydisFormatterFormatInstruction(&formatter, &instruction, operands, sizeof(operands) / sizeof(operands[0]), decompBuffer + 3, sizeof(decompBuffer) - 3, virtualAddress + addressDisplayOffset, nullptr))))
      {
        puts("Failed to zydis format instruction.");
        return crow::response(crow::status::BAD_REQUEST);
      }
    }

    zydecOut.append(std::to_string(virtualAddress + addressDisplayOffset));
    zydecOut.append("\t");
    zydecOut.append(decompBuffer);
    zydecOut.append("\t");

    const char *isaSet = ZydisISASetGetString(instruction.meta.isa_set);

    if (isaSet)
      zydecOut.append(isaSet);

    zydecOut.append("\n");

    if (instruction.length == 0)
    {
      puts("Invalid zydis instruction length.");
      return crow::response(crow::status::BAD_REQUEST);
    }

    virtualAddress += instruction.length;
  }

  crow::json::wvalue ret;
  ret["zydec"] = zydecOut.c_str();

  return crow::response(crow::status::OK, ret);
}

//////////////////////////////////////////////////////////////////////////

crow::response handle_execution_flow(const crow::request &req)
{
  auto x = crow::json::load(req.body);

  if (!x || !x.has("bytes") || !x.has("addr") || !x.has("march"))
    return crow::response(crow::status::BAD_REQUEST);

  const auto &bytesBase64 = x["bytes"].s();
  const uint64_t startAddress = x["addr"].i();
  const auto &march = x["march"].s();

  CoreArchitecture arch;

  if (march == "adl")      arch = CoreArchitecture::Alderlake;
  else if (march == "brw") arch = CoreArchitecture::Broadwell;
  else if (march == "cnl") arch = CoreArchitecture::Cannonlake;
  else if (march == "ccl") arch = CoreArchitecture::Cascadelake;
  else if (march == "cpl") arch = CoreArchitecture::Cooperlake;
  else if (march == "elr") arch = CoreArchitecture::EmeraldRapids;
  else if (march == "gmt") arch = CoreArchitecture::Goldmont;
  else if (march == "gmp") arch = CoreArchitecture::GoldmontPlus;
  else if (march == "gdr") arch = CoreArchitecture::GrandRidge;
  else if (march == "gtr") arch = CoreArchitecture::GraniteRapids;
  else if (march == "hsw") arch = CoreArchitecture::Haswell;
  else if (march == "ilc") arch = CoreArchitecture::IcelakeClient;
  else if (march == "lcs") arch = CoreArchitecture::IcelakeServer;
  else if (march == "ivb") arch = CoreArchitecture::IvyBridge;
  else if (march == "mtl") arch = CoreArchitecture::Meteorlake;
  else if (march == "rtl") arch = CoreArchitecture::Raptorlake;
  else if (march == "rkl") arch = CoreArchitecture::Rocketlake;
  else if (march == "sdb") arch = CoreArchitecture::Sandybridge;
  else if (march == "spr") arch = CoreArchitecture::SapphireRapids;
  else if (march == "srf") arch = CoreArchitecture::Sierraforest;
  else if (march == "svm") arch = CoreArchitecture::Silvermont;
  else if (march == "slc") arch = CoreArchitecture::SkylakeClient;
  else if (march == "slx") arch = CoreArchitecture::SkylakeX;
  else if (march == "sls") arch = CoreArchitecture::SkylakeServer;
  else if (march == "tgl") arch = CoreArchitecture::Tigerlake;
  else if (march == "trm") arch = CoreArchitecture::Tremont;
  else if (march == "zn1") arch = CoreArchitecture::Zen1;
  else if (march == "zn2") arch = CoreArchitecture::Zen2;
  else if (march == "zn3") arch = CoreArchitecture::Zen3;
  else if (march == "zn4") arch = CoreArchitecture::Zen4;
  else return crow::response(crow::status::BAD_REQUEST);

  if (bytesBase64.size() > 1024 * 2)
  {
    printf("Rejecting zydec, as bytesBase64 size: %" PRIu64 "\n", bytesBase64.size());
    return crow::response(crow::status::PAYLOAD_TOO_LARGE);
  }

  const auto &bytes = crow::utility::base64decode(bytesBase64);

  const size_t iterations = 8;
  PortUsageFlow flow;

  if (!execution_flow_create(bytes.c_str(), bytes.size(), &flow, arch, iterations, 0))
  {
    puts("execution_flow_create failed");
    return crow::response(crow::status::INTERNAL_SERVER_ERROR);
  }

  crow::json::wvalue ret;

  // Ports.
  {
    crow::json::wvalue ports;
    size_t i = 0;

    for (const auto &_port : flow.ports)
      ports[i++] = _port.name;

    ret["ports"] = std::move(ports);
  }

  size_t earliestDispatch = (size_t)-1;
  size_t lastRetire = 0;
  size_t earliestIssued = (size_t)-1;
  size_t lastExecuted = 0;

  // Instructions.
  {
    crow::json::wvalue instructions;
    size_t instructionIndex = (size_t)-1;

    for (const auto &instructionInfo : flow.instructionExecutionInfo)
    {
      ++instructionIndex;

      crow::json::wvalue instruction;
      instruction["addr"] = instructionInfo.instructionByteOffset + startAddress;

      // Stalls.
      if (instructionInfo.stallInfo.size())
      {
        crow::json::wvalue stalls;
        size_t stallCount = 0;

        for (const auto &_b : instructionInfo.stallInfo)
          stalls[stallCount++] = _b;

        instruction["stalls"] = std::move(stalls);
      }

      size_t dispatched = 0;
      size_t pending = 0;
      size_t ready = 0;
      size_t executing = 0;
      size_t retiring = 0;
      size_t uops = 0;

      const size_t iterations = instructionInfo.perIteration.size();

      // Add Dependencies & Calculate Average Clocks.
      {
        crow::json::wvalue pressure;
        size_t pressureCount = 0;

        crow::json::wvalue itsInfo;

        for (size_t iteration = 0; iteration < iterations; iteration++)
        {
          const auto &it = instructionInfo.perIteration[iteration];

          dispatched += it.clockPending - it.clockDispatched;
          pending += it.clockReady - it.clockPending;
          ready += it.clockIssued - it.clockReady;
          executing += it.clockExecuted - it.clockIssued;
          retiring += it.clockRetired - it.clockExecuted;
          uops += it.uOps;

          earliestDispatch = std::min(earliestDispatch, it.clockDispatched);
          lastRetire = std::max(lastRetire, it.clockRetired);
          earliestIssued = std::min(earliestIssued, it.clockIssued);
          lastExecuted = std::max(lastExecuted, it.clockExecuted);

          crow::json::wvalue itInfo;
          itInfo["issued"] = it.clockIssued;
          itInfo["executed"] = it.clockExecuted;

          if (it.usage.size())
          {
            crow::json::wvalue ports;

            for (size_t i = 0; i < it.usage.size(); i++)
              ports[i] = it.usage[i].resourceIndex;

            itInfo["ports"] = std::move(ports);
          }

          itsInfo[iteration] = std::move(itInfo);

          const auto &regP = instructionInfo.perIteration[iteration].registerPressure;

          if (regP.selfPressureCycles > 0 && regP.origin.has_value() && regP.origin.value().iterationIndex != (size_t)-1)
          {
            crow::json::wvalue p;
            p["type"] = "reg";
            p["cycles"] = regP.selfPressureCycles;
            p["name"] = regP.registerName;
            p["it"] = iteration;
            p["origin_it"] = regP.origin.value().iterationIndex;
            p["origin_offset"] = (int64_t)regP.origin.value().instructionIndex - (int64_t)instructionInfo.instructionIndex;
            p["origin_addr"] = flow.instructionExecutionInfo[regP.origin.value().instructionIndex].instructionByteOffset + startAddress;

            pressure[pressureCount++] = std::move(p);
          }

          const auto &memP = instructionInfo.perIteration[iteration].memoryPressure;

          if (memP.selfPressureCycles > 0 && memP.origin.has_value() && memP.origin.value().iterationIndex != (size_t)-1)
          {
            crow::json::wvalue p;
            p["type"] = "mem";
            p["cycles"] = memP.selfPressureCycles;
            p["it"] = iteration;
            p["origin_it"] = memP.origin.value().iterationIndex;
            p["origin_offset"] = (int64_t)memP.origin.value().instructionIndex - (int64_t)instructionInfo.instructionIndex;
            p["origin_addr"] = flow.instructionExecutionInfo[memP.origin.value().instructionIndex].instructionByteOffset + startAddress;

            pressure[pressureCount++] = std::move(p);
          }

          const auto &rsrcP = instructionInfo.perIteration[iteration].resourcePressure;

          for (const auto &_port : rsrcP.associatedResources)
          {
            if (_port.pressureCycles > 0 && _port.origin.has_value())
            {
              crow::json::wvalue p;
              p["type"] = "rsrc";
              p["cycles"] = _port.pressureCycles;
              p["it"] = iteration;
              p["origin_it"] = _port.origin.value().iterationIndex;
              p["origin_offset"] = (int64_t)_port.origin.value().instructionIndex - (int64_t)instructionInfo.instructionIndex;
              p["origin_addr"] = flow.instructionExecutionInfo[_port.origin.value().instructionIndex].instructionByteOffset + startAddress;
              p["port_name"] = _port.resourceName;

              pressure[pressureCount++] = std::move(p);
            }
          }
        }

        instruction["iterations"] = std::move(itsInfo);
        instruction["pressure"] = std::move(pressure);
      }

      instruction["dispatched"] = dispatched / (double)iterations;
      instruction["pending"] = pending / (double)iterations;
      instruction["ready"] = ready / (double)iterations;
      instruction["executing"] = executing / (double)iterations;
      instruction["retiring"] = retiring / (double)iterations;
      instruction["uops"] = uops / (double)iterations;

      instructions[instructionIndex] = std::move(instruction);
    }

    ret["instructions"] = std::move(instructions);
  }

  // Overall Stats.
  {
    std::vector<size_t> perPortUsage(flow.ports.size(), 0);
    std::vector<bool> portUsed(flow.ports.size(), false);

    // Check Utilization in Bounds.
    for (size_t cycle = earliestIssued; cycle < lastExecuted; cycle++)
    {
      for (size_t port = 0; port < portUsed.size(); port++)
        portUsed[port] = false;

      for (const auto &_instruction : flow.instructionExecutionInfo)
      {
        for (const auto &it : _instruction.perIteration)
        {
          // If active: Mark port as used.
          if (it.clockIssued <= cycle && it.clockExecuted > cycle)
            for (const auto &_port : _instruction.usage)
              portUsed[_port.resourceIndex] = true;
        }
      }

      for (size_t port = 0; port < portUsed.size(); port++)
        if (portUsed[port])
          perPortUsage[port]++;
    }

    enum ExecutionState
    {
      ES_Dispatched,
      ES_Pending,
      ES_Ready,
      ES_Executing,
      ES_Retiring,

      _ES_Count
    };

    bool stateInUse[_ES_Count];
    size_t stateCyclesInUse[_ES_Count] = {};

    // Check States in Bounds.
    for (size_t cycle = earliestDispatch; cycle < lastRetire; cycle++)
    {
      for (size_t s = 0; s < _ES_Count; s++)
        stateInUse[s] = false;

      for (const auto &_instruction : flow.instructionExecutionInfo)
      {
        // If active: Mark state as used.
        for (const auto &it : _instruction.perIteration)
        {
          if (it.clockDispatched <= cycle && it.clockPending > cycle)
            stateInUse[ES_Dispatched] = true;

          if (it.clockPending <= cycle && it.clockReady > cycle)
            stateInUse[ES_Pending] = true;

          if (it.clockReady <= cycle && it.clockIssued > cycle)
            stateInUse[ES_Ready] = true;

          if (it.clockIssued <= cycle && it.clockExecuted > cycle)
            stateInUse[ES_Executing] = true;

          if (it.clockExecuted <= cycle && it.clockRetired > cycle)
            stateInUse[ES_Retiring] = true;
        }
      }

      for (size_t s = 0; s < _ES_Count; s++)
        if (stateInUse[s])
          stateCyclesInUse[s]++;
    }

    crow::json::wvalue stats;

    const double invLoopItsF = 1.0 / (double)iterations;

    stats["cycles"] = (lastRetire - earliestDispatch) * invLoopItsF;
    stats["cycles_total"] = lastRetire - earliestDispatch;
    stats["execution_cycles"] = (lastExecuted - earliestIssued) * invLoopItsF;
    stats["execution_cycles_total"] = lastExecuted - earliestIssued;

    stats["iterations"] = iterations;

    stats["dispatched"] = stateCyclesInUse[ES_Dispatched];
    stats["pending"] = stateCyclesInUse[ES_Pending];
    stats["ready"] = stateCyclesInUse[ES_Ready];
    stats["executing"] = stateCyclesInUse[ES_Executing];
    stats["retiring"] = stateCyclesInUse[ES_Retiring];

    crow::json::wvalue ports;

    for (size_t i = 0; i < perPortUsage.size(); i++)
      ports[i] = (double)perPortUsage[i] / (lastExecuted - earliestIssued);

    stats["ports"] = std::move(ports);
    ret["stats"] = std::move(stats);
  }

  (void)startAddress;

  return crow::response(crow::status::OK, ret);
}
