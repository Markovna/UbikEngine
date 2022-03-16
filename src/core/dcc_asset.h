#pragma once

#include "platform/file_system.h"

class asset_id;
class asset;
class asset_repository;
class assets_filesystem;
class world;
class entity;
class interface_registry;
class renderer;

enum class dcc_asset_texture_type {
  RAW = 0,
  PNG,
  JPEG,
  TIFF,
  TGA,

  UNKNOWN
};

asset_id create_dcc_asset(const fs::path& path, asset_repository&);
asset_id create_entity_from_dcc_asset(const asset& asset, asset_repository& rep, assets_filesystem& filesystem);

