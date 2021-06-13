#include "sandbox_plugin.h"
#include "spin_plugin.h"
#include "core/engine.h"
#include "core/world.h"
#include "base/log.h"
#include "core/components/mesh_component.h"
#include "core/components/camera_component.h"

#include "core/meta/registration.h"

const char* scene_str = "{\n"
                        "  \"__guid\": \"28dd8e81bca34d4a886cec47cfd63823\",\n"
                        "  \"__type\": \"entity\",\n"
                        "  \"children\": [\n"
                        "    {\n"
                        "      \"__guid\": \"eb30433ec9c64a558654d900010f2717\",\n"
                        "      \"__type\": \"entity\",\n"
                        "      \"children\": [\n"
                        "        {\n"
                        "          \"__guid\": \"1007fb8ad7c547e1a9bf7d7579a55e73\",\n"
                        "          \"__type\": \"entity\",\n"
                        "          \"components\": [\n"
                        "            {\n"
                        "              \"__guid\": \"a816c15a1a304366891a60a0455995be\",\n"
                        "              \"__type\": \"transform_component\",\n"
                        "              \"position\": {\n"
                        "                \"x\": 0.0,\n"
                        "                \"y\": 0.0,\n"
                        "                \"z\": 0.0\n"
                        "              },\n"
                        "              \"rotation\": {\n"
                        "                \"w\": 1.0,\n"
                        "                \"x\": 0.0,\n"
                        "                \"y\": 0.0,\n"
                        "                \"z\": 0.0\n"
                        "              },\n"
                        "              \"scale\": {\n"
                        "                \"x\": 1.0,\n"
                        "                \"y\": 1.0,\n"
                        "                \"z\": 1.0\n"
                        "              }\n"
                        "            },\n"
                        "            {\n"
                        "              \"__guid\": \"a818b718d9fe46f394390dcb50bf7a82\",\n"
                        "              \"__type\": \"mesh_component\",\n"
                        "              \"color\": {\n"
                        "                \"a\": 1.0,\n"
                        "                \"b\": 1.0,\n"
                        "                \"g\": 1.0,\n"
                        "                \"r\": 1.0\n"
                        "              }\n"
                        "            },\n"
                        "            {\n"
                        "              \"__guid\": \"92d1af8c646a4f13a381451c51d502f3\",\n"
                        "              \"__type\": \"custom_component\",\n"
                        "              \"a\": 42,\n"
                        "              \"b\": 0.0,\n"
                        "              \"c\": {\n"
                        "                \"a\": 0.0\n"
                        "              }\n"
                        "            }\n"
                        "          ]\n"
                        "        }\n"
                        "      ],\n"
                        "      \"components\": [\n"
                        "        {\n"
                        "          \"__guid\": \"8f3a03a3aeb04ecf9858973aa919a602\",\n"
                        "          \"__type\": \"transform_component\",\n"
                        "          \"position\": {\n"
                        "            \"x\": 0.0,\n"
                        "            \"y\": 0.0,\n"
                        "            \"z\": 0.0\n"
                        "          },\n"
                        "          \"rotation\": {\n"
                        "            \"w\": 1.0,\n"
                        "            \"x\": 0.0,\n"
                        "            \"y\": 0.0,\n"
                        "            \"z\": 0.0\n"
                        "          },\n"
                        "          \"scale\": {\n"
                        "            \"x\": 1.0,\n"
                        "            \"y\": 1.0,\n"
                        "            \"z\": 1.0\n"
                        "          }\n"
                        "        }\n"
                        "      ]\n"
                        "    },\n"
                        "    {\n"
                        "      \"__guid\": \"3e74dc2e939c42deb1a2b9604f0a32fe\",\n"
                        "      \"__type\": \"entity\",\n"
                        "      \"components\": [\n"
                        "        {\n"
                        "          \"__guid\": \"9bb8346cf176481f9b0ac99bdb611fb3\",\n"
                        "          \"__type\": \"transform_component\",\n"
                        "          \"position\": {\n"
                        "            \"x\": 0.0,\n"
                        "            \"y\": -2.0,\n"
                        "            \"z\": -3.0\n"
                        "          },\n"
                        "          \"rotation\": {\n"
                        "            \"w\": 1.0,\n"
                        "            \"x\": 0.0,\n"
                        "            \"y\": 0.0,\n"
                        "            \"z\": 0.0\n"
                        "          },\n"
                        "          \"scale\": {\n"
                        "            \"x\": 1.0,\n"
                        "            \"y\": 1.0,\n"
                        "            \"z\": 1.0\n"
                        "          }\n"
                        "        },\n"
                        "        {\n"
                        "          \"__guid\": \"8fc9351ed4e348b5840d91831e2056de\",\n"
                        "          \"__type\": \"camera_component\",\n"
                        "          \"clear_flags\": 6,\n"
                        "          \"color\": {\n"
                        "            \"a\": 1.0,\n"
                        "            \"b\": 0.0,\n"
                        "            \"g\": 0.0,\n"
                        "            \"r\": 0.0\n"
                        "          },\n"
                        "          \"far\": 100.0,\n"
                        "          \"fov\": 60.0,\n"
                        "          \"near\": 0.10000000149011612,\n"
                        "          \"normalized_rect\": {\n"
                        "            \"w\": 1.0,\n"
                        "            \"x\": 0.0,\n"
                        "            \"y\": 0.0,\n"
                        "            \"z\": 1.0\n"
                        "          },\n"
                        "          \"orthogonal_size\": 1.0\n"
                        "        }\n"
                        "      ]\n"
                        "    }\n"
                        "  ],\n"
                        "  \"components\": [\n"
                        "    {\n"
                        "      \"__guid\": \"feb3f81bfd8148a6b575e9710c161396\",\n"
                        "      \"__type\": \"transform_component\",\n"
                        "      \"position\": {\n"
                        "        \"x\": 0.0,\n"
                        "        \"y\": 0.0,\n"
                        "        \"z\": 0.0\n"
                        "      },\n"
                        "      \"rotation\": {\n"
                        "        \"w\": 1.0,\n"
                        "        \"x\": 0.0,\n"
                        "        \"y\": 0.0,\n"
                        "        \"z\": 0.0\n"
                        "      },\n"
                        "      \"scale\": {\n"
                        "        \"x\": 1.0,\n"
                        "        \"y\": 1.0,\n"
                        "        \"z\": 1.0\n"
                        "      }\n"
                        "    }\n"
                        "  ]\n"
                        "}";

class test : public plugin_base {
 public:
  int count = 0;
  void stop(engine *e) override {}
  void start(engine *e) override {
    logger::core::Info("test::start  ");

    asset scene_asset = asset::parse(scene_str);
    e->world->load_from_asset(scene_asset);

    /*
    auto root_ent = e->world->create_entity();
    auto& ent0_tr = e->world->component<transform_component>(root_ent);
    auto& ent0_cc = e->world->add_component<custom_component>(root_ent);
    ent0_cc.a = 42;

    auto mesh_ent = e->world->create_entity({}, root_ent);
    auto cam_ent = e->world->create_entity({});

    e->world->add_component<mesh_component>(mesh_ent);

//    for (int i = 1; i < 2; i++) {
//      {
//        vec3 pos = (i * 2.0f + 1) * vec3::forward() + (i * 0.7f) * vec3::right();
//        auto ent = e->world->create_entity({}, root_ent);
//        e->world->add_component<mesh_component>(ent);
//        e->world->set_local_transform(ent, transform::from_matrix(mat4::translation(pos)));
//      }
//      {
//        vec3 pos = (i * 2.0f + 1) * vec3::forward() + (i * 0.7f) * -vec3::right();
//        auto ent = e->world->create_entity({}, root_ent);
//        e->world->add_component<mesh_component>(ent);
//        e->world->set_local_transform(ent, transform::from_matrix(mat4::translation(pos)));
//        e->world->add_component<custom_component>(ent).a = 42;
//      }
//    }

    auto& cam = e->world->add_component<camera_component>(cam_ent);
    e->world->set_local_transform(cam_ent, transform::from_matrix(mat4::translation({0,-2.0f,-3.0f})));

    /**/
  }

  void update(engine *e) override {
    auto mesh_view = e->world->view<mesh_component, custom_component>();
    for (ecs::entity id : mesh_view) {
      mesh_view.get<mesh_component>(id).set_color(color::green());
    }
  }
};


void load_sandbox_plugin(engine* engine) {

  register_type(custom_component);

  logger::core::Info("load_sandbox_plugin");
  engine->plugins->add_plugin<test>("test");
}


void unload_sandbox_plugin(engine* engine) {

  logger::core::Info("unload_sandbox_plugin");
  engine->plugins->remove_plugin("test");

}

void serialization<custom_component>::from_asset(const asset &asset, custom_component *comp) {
  comp->a = asset["a"];
  comp->b = asset["b"];
  comp->c.a = asset["c"]["a"];
}

void serialization<custom_component>::to_asset(asset &asset, const custom_component *comp) {
  asset["a"] = comp->a;
  asset["b"] = comp->b;
  asset["c"]["a"] = comp->c.a;
}