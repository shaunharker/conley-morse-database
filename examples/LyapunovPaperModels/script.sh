# This script is located in ./examples/LyapunovPaperModels
cd ../..
make clean
make SingleCMG MODELDIR=./examples/LyapunovPaperModels/Model1Pointer
make clean
make SingleCMG MODELDIR=./examples/LyapunovPaperModels/Model1Succinct 
make clean
make SingleCMG MODELDIR=./examples/LyapunovPaperModels/Model2Pointer 
make clean
make SingleCMG MODELDIR=./examples/LyapunovPaperModels/Model2Succinct

cd extras/Lyapunov
make clean
make MODELDIR=../../examples/LyapunovPaperModels/Model1Pointer
make clean
make MODELDIR=../../examples/LyapunovPaperModels/Model1Succinct
make clean
make MODELDIR=../../examples/LyapunovPaperModels/Model2Pointer
make clean
make MODELDIR=../../examples/LyapunovPaperModels/Model2Succinct

cd ../..
cd examples/LyapunovPaperModels/

cd Model1Pointer
./SingleCMG ./ 20.0 20.0 > cmg.log
./computeLyapunov data.cmg lyapunov.txt ./ 20.0 20.0 > lyapunov.log
cd ..

cd Model1Succinct
./SingleCMG ./ 20.0 20.0 > cmg.log
./computeLyapunov data.cmg lyapunov.txt	./ 20.0	20.0 > lyapunov.log
cd ..

cd Model2Pointer
./SingleCMG ./ 5.0 5.0 0.1 0.11 0.12 0.1 0.8 > cmg.log
./computeLyapunov data.cmg lyapunov.txt ./ 5.0 5.0 0.1 0.11 0.12 0.1 0.8 > lyapunov.log
cd ..

cd Model2Succinct
./SingleCMG ./ 5.0 5.0 0.1 0.11 0.12 0.1 0.8 > cmg.log
./computeLyapunov data.cmg lyapunov.txt ./ 5.0 5.0 0.1 0.11 0.12 0.1 0.8 > lyapunov.log
cd ..

cd ../..
