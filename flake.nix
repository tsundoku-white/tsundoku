{
  description = "tsundoku-flake";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "tsundoku-flake";
          version = "0.1.0";

          src = ./.;

          nativeBuildInputs = with pkgs; [
            clang
            clang-tools
            cmake
            pkg-config
            shaderc
          ];

          buildInputs = with pkgs; [
            glfw
            vulkan-headers
            vulkan-loader
            vulkan-validation-layers
            glm
            freetype
            libx11
            libxrandr
            libxinerama
            libxcursor
            libxi
          ];

          cmakeFlags = [
            "-DCMAKE_BUILD_TYPE=Debug"
            "-DCMAKE_CXX_COMPILER=${pkgs.clang}/bin/clang++"
          ];

          installPhase = ''
            mkdir -p $out/bin
            cp app $out/bin/
            if [ -d "shaders" ]; then
              mkdir -p $out/share/shaders
              cp -r shaders/* $out/share/shaders/
            fi
          '';
        };

        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            clang
            clang-tools
            cmake
            pkg-config
            glfw
            vulkan-headers
            vulkan-loader
            vulkan-validation-layers
            vulkan-tools
            shaderc
            glm
            freetype
            libx11
            libxrandr
            libxinerama
            libxcursor
            libxi
            nixpkgs-fmt
            prettier
          ];

          CPATH = "${pkgs.glfw}/include:${pkgs.vulkan-headers}/include:${pkgs.glm}/include:${pkgs.freetype}/include/freetype2";
          VK_LAYER_PATH = "${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d";
          LD_LIBRARY_PATH = "${pkgs.vulkan-loader}/lib:${pkgs.vulkan-validation-layers}/lib:${pkgs.freetype}/lib";
        };
      }
    );
}

