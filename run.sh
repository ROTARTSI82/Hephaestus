echo "[run.sh@Hephaestus]: Building and running Hephaestus with run.sh (Version d2019.11.30)"

export VK_ICD_FILENAMES="/Users/25granty/Desktop/CLion/Hephaestus/vendor/lunarg-vulkan-sdk/macOS/etc/vulkan/icd.d/MoltenVK_icd.json"
export VK_LAYER_PATH="/Users/25granty/Desktop/CLion/Hephaestus/vendor/lunarg-vulkan-sdk/macOS/etc/vulkan/explicit_layer.d"

for ARG in "$@"
do
  if [ "$ARG" == "-c" ] || [ "$ARG" == "--clean-build" ] ; then
      echo "[run.sh@Hephaestus]: --clean-build option active; removing existing binaries and building from scratch"
      rm -rf cmake-build-debug   # if -c flag is supplied, rebuild entire thing from scratch.
      rm -f run-env/HephaestusSandbox
  fi
done

for ARG in "$@"
do
  if [ "$ARG" == "-d" ] || [ "$ARG" == "--build-docs" ] ; then
      echo "[run.sh@Hephaestus]: Building documentation using doxygen!"
      rm -rf docs
      doxygen
  fi
done

mkdir -p cmake-build-debug
cd cmake-build-debug

DO_BUILD=1

for ARG in "$@"
do
  if [ "$ARG" == "-nb" ] || [ "$ARG" == "--no-build" ] ; then
    echo "[run.sh@Hephaestus]: --no-build option active; skipping build!"
    DO_BUILD=0
  fi
done

if [ "$DO_BUILD" == "1" ] ; then
  cmake ..
  make  # cannot use async make bc examples target depends on HP target being built.
fi 

cp examples/HephaestusSandbox ../run-env/HephaestusSandbox
#cp libHephaestusShared.dylib ../run-env/libHephaestusShared.dylib

#sudo chown -R 25granty .

for ARG in "$@"
do
  if [ "$ARG" == "-nr" ] || [ "$ARG" == "--no-run" ]; then
      echo "[run.sh@Hephaestus]: --no-run option is active; Exiting!"
      exit
  fi
done

echo "[run.sh@Hephaestus]: Successfully built Hephaestus project. Executing the sandbox application..."

cd ../run-env/shader_pack/
rm frag.spv
rm vert.spv

glslc frag.frag -o frag.spv
glslc vert.vert -o vert.spv

cd ..

./HephaestusSandbox
cd ..

echo "[run.sh@Hephaestus]: Hephaestus built and ran successfully"
