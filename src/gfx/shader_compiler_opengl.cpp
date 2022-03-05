#include "shader_compiler_opengl.h"

#include "spirv_glsl.hpp"
#include "shaderc/shaderc.hpp"
#include "base/log.h"


static bool compile_to_spirv(std::string_view source, shader_stage::type stage, std::vector<uint32_t>& spirv) {
  constexpr static const shaderc_shader_kind stage_map[] = {
    shaderc_glsl_vertex_shader,
    shaderc_glsl_fragment_shader,
    shaderc_glsl_compute_shader,
    shaderc_glsl_geometry_shader,
    shaderc_glsl_tess_control_shader,
    shaderc_glsl_default_tess_evaluation_shader
  };
  static_assert(sizeof(stage_map) == sizeof(shaderc_shader_kind) * shader_stage::COUNT,
      "Mismatch stage_map and shader_stage sizes.");

  shaderc::Compiler compiler;
  shaderc::CompileOptions options;
  options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
  options.SetOptimizationLevel(shaderc_optimization_level_zero);

//  options.SetAutoBindUniforms(true);
//  options.SetBindingBase(shaderc_uniform_kind_image, 0);
//  options.SetBindingBase(shaderc_uniform_kind_sampler, 0);
//  options.SetBindingBase(shaderc_uniform_kind_buffer, 0);

  shaderc::SpvCompilationResult module =
      compiler.CompileGlslToSpv(source.data(), source.size(), stage_map[stage], "opengl", options);
  if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
    logger::core::Error("GLSL source to binary SPIR-V compilation failed: \n{}\n{}", module.GetErrorMessage(), source);
    return false;
  }

  std::copy(module.cbegin(), module.cend(), std::back_inserter(spirv));
  return true;
}

shader_compile_result shader_compiler_opengl::compile(std::string_view source, shader_stage::type stage) {
  shader_compile_result result { };
  std::vector<uint32_t> spirv;
  if (!compile_to_spirv(source, stage, spirv)) {
    result.success = false;
    return result;
  }

  // TODO: if OpenGL backend version < 420
  const bool unbind_uniforms = true;
  const bool unbind_images = true;

  spirv_cross::CompilerGLSL compiler(std::move(spirv));

  spirv_cross::ShaderResources resources = compiler.get_shader_resources();
  for (auto &resource : resources.uniform_buffers) {
    auto& uniform = result.uniforms.emplace_back();
    uniform.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
    uniform.name = resource.name;

    const auto& type = compiler.get_type(resource.base_type_id);
    uint32_t size = type.member_types.size();
    for (uint32_t i = 0; i < size; i++) {
      auto& member = uniform.members.emplace_back();
      member.name = compiler.get_member_name(resource.base_type_id, i);
      member.offset = compiler.get_member_decoration(resource.base_type_id, i, spv::DecorationOffset);
    }

    if (unbind_uniforms) {
      compiler.unset_decoration(resource.id, spv::DecorationBinding);
    }
  }

  for (auto& resource : resources.sampled_images) {
    auto& image = result.images.emplace_back();
    image.name = resource.name;
    image.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

    if (unbind_images) {
      compiler.unset_decoration(resource.id, spv::DecorationBinding);
    }
  }

//  for (auto& input : resources.stage_inputs) {
//  }

  spirv_cross::CompilerGLSL::Options options;
  options.version = 410;
  options.es = false;
//  options.emit_uniform_buffer_as_plain_uniforms = true;

  compiler.set_common_options(options);

  result.source = compiler.compile();
  result.success = true;

  return result;
}

std::unique_ptr<shader_compiler_opengl> shader_compiler_opengl::create() {
  return std::make_unique<shader_compiler_opengl>();
}
