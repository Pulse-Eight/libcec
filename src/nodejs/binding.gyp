{
  "targets": [
    {
      "target_name": "cec_native",
      "sources": [ "src/addon.cc" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include_dir\")"
      ],
      "defines": [ "NAPI_VERSION=8" ],
      "cflags_cc": [ "-std=c++17", "-fexceptions", "<!@(pkg-config --cflags libcec)" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "libraries": [ "<!@(pkg-config --libs libcec)" ],
      "conditions": [
        [ "OS==\"mac\"", {
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "CLANG_CXX_LANGUAGE_STANDARD": "c++17",
            "OTHER_CFLAGS": [ "<!@(pkg-config --cflags libcec)" ]
          }
        } ]
      ]
    }
  ]
}
