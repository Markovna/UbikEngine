#include "asset_repository.h"

#include "base/log.h"

#include <iostream>

const asset_value &asset_array::operator[](size_t index) const {
  return values_[index];
}

const asset_value &asset_array::at(size_t index) const {
  return values_.at(index);
}

void asset_array::push_back(asset_value val) { values_.push_back(std::move(val)); }

asset_array::iterator asset_array::insert(asset_array::const_iterator it, asset_value val) {
  return values_.insert(it, std::move(val));
}

nlohmann::json asset_to_json(const asset_value& val, asset_repository& rep, std::vector<buffer_id>& buffers) {
  switch ((asset_value::type) val) {
    case asset_value::type::BOOLEAN: {
      return static_cast<bool>(val);
    }
    case asset_value::type::INTEGER: {
      return static_cast<int64_t>(val);
    }
    case asset_value::type::UNSIGNED: {
      return static_cast<uint64_t>(val);
    }
    case asset_value::type::FLOAT: {
      return static_cast<float>(val);
    }
    case asset_value::type::STRING: {
      return static_cast<const std::string&>(val);
    }
    case asset_value::type::ARRAY: {
      nlohmann::json j = nlohmann::json::array();
      auto& array = static_cast<const asset_array&>(val);
      for (auto& sub : array) {
        j.push_back(asset_to_json(sub, rep, buffers));
      }
      return j;
    }
    case asset_value::type::OBJECT: {
      nlohmann::json j = nlohmann::json::object();
      auto& obj = static_cast<const asset&>(val);
      for (auto& [name, sub] : obj) {
        j[name] = asset_to_json(sub, rep, buffers);
      }
      j["__guid"] = rep.get_guid(obj);
      return j;
    }
    case asset_value::type::BUFFER: {
      buffers.push_back(val);
      return { { "__buffer_hash", std::to_string(rep.buffer_hash(val)) } };
    }
    case asset_value::type::NONE:
      break;
  }
  return {};
}

asset_value parse_json(nlohmann::json& j, asset_repository& rep, const fs::path& buffers_path) {
  switch (j.type()) {
    case nlohmann::detail::value_t::number_float: {
      return j.get<float>();
    }
    case nlohmann::detail::value_t::number_integer: {
      return j.get<int64_t>();
    }
    case nlohmann::detail::value_t::number_unsigned: {
      return j.get<uint64_t>();
    }
    case nlohmann::detail::value_t::boolean: {
      return j.get<bool>();
    }
    case nlohmann::detail::value_t::string: {
      return j.get_ref<std::string&>();
    }
    case nlohmann::detail::value_t::object: {
      if (j.contains("__buffer_hash")) {
        fs::path buffer_path = fs::append(buffers_path, j.at("__buffer_hash"));
        if (!fs::exists(buffer_path)) {
          logger::core::Warning("Couldn't load buffer {}", buffer_path.c_str());
          return rep.create_buffer(0);
        }

        return rep.create_buffer_from_file(buffer_path, 0, 0);
      }

      asset* asset = nullptr;
      if (j.contains("__guid")) {
        asset = &rep.create_asset(guid::from_string(j.at("__guid")));
      } else {
        asset = &rep.create_asset();
      }

      for (auto& [name, sub] : j.items()) {
        rep.set_value(*asset, name, parse_json(sub, rep, buffers_path));
      }

      return *asset;
    }
    case nlohmann::detail::value_t::array: {
      asset_array& array = rep.create_array();
      for (auto& sub : j) {
        rep.push_back(array, parse_json(sub, rep, buffers_path));
      }
      return array;
    }
    case nlohmann::detail::value_t::discarded:
    case nlohmann::detail::value_t::null:
    case nlohmann::detail::value_t::binary:
    default: {
      break;
    }
  }
  return nullptr;
}