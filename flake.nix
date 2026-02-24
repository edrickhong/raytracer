{
  description = "Cu_std dev env";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
  let
    system = "x86_64-linux";
    pkgs = import nixpkgs { inherit system; };
		build_tools = with pkgs; [
			clang
			cmake
			pkg-config
			bash
			gnumake
		];


		build_deps = with pkgs; [
			dbus
			libffi
			wayland
			wayland-protocols
			wayland-scanner

			vulkan-loader
			vulkan-headers

			alsa-lib

			libX11
			libxkbcommon
		];

  in {


#---------------
#DEV
    devShells.${system}.default = (pkgs.mkShell.override { stdenv = pkgs.clangStdenv; }) {
      packages = with pkgs; ([
#add dev packages here
      ] ++ build_tools) ++ build_deps;

      CC = "${pkgs.clang}/bin/clang";
      CXX = "${pkgs.clang}/bin/clang++";

      shellHook = ''
      PS1="(dev) \u@\h:\w$ "
      echo ">>> cu_std dev shell: CC=$CC CXX=$CXX"
      '';
    };
#---------------
#BUILD
		packages.${system}.default = pkgs.clangStdenv.mkDerivation {
			pname = "cu_std";
			version = "0.1.0";

			src = self;

			nativeBuildInputs = build_tools;
			buildInputs = build_deps;

			configurePhase = ''
				runHook preConfigure

				cmake -S . -B ./build/

				runHook postConfigure
				'';

			buildPhase = ''
				runHook preBuild

				cmake --build ./build/ -j $NIX_BUILD_CORES

				runHook postBuild
				'';

			installPhase = ''
				runHook preInstall

				mkdir -p $out/
				cp -a ./bin/* $out/

				runHook postInstall
				'';
											};	
#---------------
  };
}
