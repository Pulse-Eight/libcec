{
  "targets": [
    {
      "target_name": "cec_native",
      "sources": [ "src/addon.cc" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "defines": [ "NAPI_VERSION=8" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "conditions": [
        [ "OS==\"linux\"", {
          "cflags_cc": [ "-std=c++17", "-fexceptions", "<!@(pkg-config --cflags libcec)" ],
          "libraries": [ "<!@(pkg-config --libs libcec)" ]
        } ],
        [ "OS==\"mac\"", {
          "libraries": [ "<!@(pkg-config --libs libcec)" ],
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "CLANG_CXX_LANGUAGE_STANDARD": "c++17",
            "OTHER_CFLAGS": [ "<!@(pkg-config --cflags libcec)" ]
          }
        } ],
        [ "OS==\"win\"", {
          # Windows has no pkg-config. Point the compiler at libCEC's (flat)
          # headers and the cec.lib import library. Override the two locations
          # with the LIBCEC_INCLUDE_DIR / LIBCEC_LIB_DIR environment variables
          # (a repo build sets them to include\ and build\<cfg>\<arch>);
          # they default to the installed Pulse-Eight USB-CEC Adapter SDK.
          "include_dirs": [
            "<!(node -p \"process.env.LIBCEC_INCLUDE_DIR || String.raw`C:\\Program Files\\Pulse-Eight\\USB-CEC Adapter\\include`\")"
          ],
          "libraries": [
            "<!(node -p \"require('path').join(process.env.LIBCEC_LIB_DIR || String.raw`C:\\Program Files\\Pulse-Eight\\USB-CEC Adapter`, 'cec.lib')\")"
          ],
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1,
              "AdditionalOptions": [ "/std:c++17" ]
            }
          }
        } ]
      ]
    }
  ]
}
