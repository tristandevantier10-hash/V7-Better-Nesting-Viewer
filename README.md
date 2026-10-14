# Signage Costing

## Build on Windows

1. Clone the repository and initialize its pinned vcpkg submodule:

   ```powershell
   git clone --recurse-submodules <repository-url>
   ```

   For an existing clone, run `git submodule update --init --recursive` from the repository root.

2. Open `SignageCosting.sln` in Visual Studio with the C++ desktop development workload installed.
3. Select the **x64** platform and build. The project uses the repository's vcpkg submodule and manifest to install the pinned dependency set for the selected configuration.

The manifest and registry baseline are in `vcpkg.json` and `vcpkg-configuration.json`. Do not replace the submodule with a separately installed vcpkg checkout when building this project.

`SignageCosting/Makefile.win` is a legacy Dev-C++ makefile for an older 32-bit source and dependency layout. It is unsupported and does not build the current application; use the Visual Studio solution above.
