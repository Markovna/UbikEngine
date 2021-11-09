
#include "shader_compiler_opengl.h"

#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"
#include "SPIRV/Logger.h"
#include "SPIRV/SpvTools.h"
#include "SPIRV/disassemble.h"

namespace experimental {

const TBuiltInResource DefaultTBuiltInResource = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .maxMeshOutputVerticesNV = */ 256,
    /* .maxMeshOutputPrimitivesNV = */ 512,
    /* .maxMeshWorkGroupSizeX_NV = */ 32,
    /* .maxMeshWorkGroupSizeY_NV = */ 1,
    /* .maxMeshWorkGroupSizeZ_NV = */ 1,
    /* .maxTaskWorkGroupSizeX_NV = */ 32,
    /* .maxTaskWorkGroupSizeY_NV = */ 1,
    /* .maxTaskWorkGroupSizeZ_NV = */ 1,
    /* .maxMeshViewCountNV = */ 4,
    /* .maxDualSourceDrawBuffersEXT = */ 1,

    /* .limits = */ {
                           /* .nonInductiveForLoops = */ 1,
                           /* .whileLoops = */ 1,
                           /* .doWhileLoops = */ 1,
                           /* .generalUniformIndexing = */ 1,
                           /* .generalAttributeMatrixVectorIndexing = */ 1,
                           /* .generalVaryingIndexing = */ 1,
                           /* .generalSamplerIndexing = */ 1,
                           /* .generalVariableIndexing = */ 1,
                           /* .generalConstantMatrixVectorIndexing = */ 1,
                       }};

static bool compile_to_spirv(std::string_view source, shader_stage::type stage, std::vector<uint32_t>& spirv) {
  constexpr static const EShLanguage stage_map[] = {
      EShLanguage::EShLangVertex, EShLanguage::EShLangFragment
  };
  static_assert(sizeof(stage_map) == sizeof(EShLanguage) * shader_stage::COUNT, "Mismatch stage_map and shader_stage sizes.");

  glslang::InitializeProcess();
  {
    TBuiltInResource resources = DefaultTBuiltInResource;

    glslang::TShader shader(stage_map[stage]);

    // TODO: add #define OPENGL
    const char* data[] = { source.data() };
    const int size[] = { (int) source.size() };
    shader.setStringsWithLengths(data, size, 1);
    shader.setEntryPoint("main");

    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
    shader.setEnvInput(glslang::EShSourceGlsl, stage_map[stage], glslang::EShClientVulkan, 420);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_0);

    if (!shader.parse(&resources, 110, EProfile::ECoreProfile, EShMessages::EShMsgDebugInfo)) {
      logger::core::Error("Shader GLSL parsing failed with error: \n {}", shader.getInfoLog());
      return false;
    }

    spv::SpvBuildLogger logger;
    glslang::SpvOptions spvOptions;

    const bool debug = true;
    const bool strip_debug = false;
    const bool optimize = true;
    const bool optimize_size = true;
    if (debug) spvOptions.generateDebugInfo = true;
    else if (strip_debug) spvOptions.stripDebugInfo = true;

    spvOptions.disableOptimizer = !optimize;
    spvOptions.optimizeSize = optimize_size;
    spvOptions.disassemble = true;
    spvOptions.validate = true;

    glslang::GlslangToSpv(*shader.getIntermediate(), spirv, &logger, &spvOptions);
    if (spirv.empty()) {
      logger::core::Error("Shader compilation to SPIR-V failed: \n {}", logger.getAllMessages());
    }

    spv::Disassemble(std::cout, spirv);
  }

  glslang::FinalizeProcess();
  return true;
}

bool shader_compiler_opengl::compile(std::string_view source, shader_stage::type stage, shader_reflection& reflection, std::ostream& out) {
  std::vector<uint32_t> spirv;
  if (!compile_to_spirv(source, stage, spirv))
    return false;


}

std::unique_ptr<shader_compiler_opengl> shader_compiler_opengl::create() {
  return std::make_unique<shader_compiler_opengl>();
}

}