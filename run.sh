echo "[run.sh@Hephaestus]: Building and running Hephaestus with run.sh v1.0.0"

#export VK_ICD_FILENAMES="/Users/25granty/Desktop/CLion/Hephaestus/vendor/vulkansdk-macos-1.1.126.0/macOS/etc/vulkan/icd.d/MoltenVK_icd.json"
#export VK_LAYER_PATH="/Users/25granty/Desktop/CLion/Hephaestus/vendor/vulkansdk-macos-1.1.126.0/macOS/etc/vulkan/explicit_layer.d"

export VK_ICD_FILENAMES="/Users/25granty/Desktop/CLion/Mason/Hephaestus/lib/MoltenVK_icd.json"
export VK_LAYER_PATH="/Users/25granty/Desktop/CLion/Mason/Hephaestus/lib/validation_layers.d"

if (( ${EUID:-0} || $(id -u) )) ; then
  echo "[run.sh@Hephaestus]: Not running as root! Things may not work as intented. Try running with sudo if things break."
fi

for ARG in "$@"
do
  if [ "$ARG" == "-c" ] || [ "$ARG" == "--clean-build" ] ; then
      echo "[run.sh@Hephaestus]: --clean-build option active; removing existing binaries and building from scratch"
      rm -rf build   # if -c flag is supplied, rebuild entire thing from scratch.
      rm -f run-env/HephaestusSandbox
  fi
done

mkdir -p build
cd build
cmake ..

make  # cannot use async make bc sandbox target depends on HP target being built.

echo "[run.sh@Hephaestus]: Build successful! Copying binaries...."

for ARG in "$@"
do
  if [ "$ARG" == "-nr" ] || [ "$ARG" == "--no-run" ]; then
      echo "[run.sh@Hephaestus]: --no-run option is active; \`sudo chown\`ing and exiting!"
      sudo chown -R 25granty .
      exit
  fi
done

cp sandbox/HephaestusSandbox ../run-env/HephaestusSandbox
cd ../run-env

echo "[run.sh@Hephaestus]: Running HephaestusSandbox!"

./HephaestusSandbox

cd ..

echo "[run.sh@Hephaestus]: HephaestusSandbox terminated successfully; \`sudo chown\`ing and exiting!"

sudo chown -R 25granty .
