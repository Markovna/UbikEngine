#include <base/log.h>
#include "texture.h"
#include "core/assets_filesystem.h"
#include "core/asset_repository.h"
#include "gfx/command_buffers.h"

texture load_texture(const asset &texture,
                     asset_repository *repository,
                     resource_command_buffer *command_buf) {

  buffer_id buf_id = texture.at("buffer");
  auto buffer = repository->load_buffer(buf_id);

  texture_data_desc desc { };
  std::memcpy(&desc, buffer.data(), sizeof(desc));

  memory mem;
  auto tex_handle = command_buf->create_texture({
        .data = desc
      },
      mem
    );
  std::memcpy(mem.data, buffer.data() + sizeof(desc), mem.size);

  return { tex_handle };
}
