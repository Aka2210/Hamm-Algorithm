#!/bin/bash

mkdir -p data_raw tools results

echo "Compiling C++ code..."
if [ -f "src/Hamm.cpp" ]; then
    g++ -O3 -o tools/hamm src/Hamm.cpp
    if [ $? -eq 0 ]; then
        echo "✅ Compilation successful: tools/hamm"
    else
        echo "❌ Compilation failed."
        exit 1
    fi
else
    echo "❌ Error: src/Hamm.cpp not found."
    exit 1
fi

echo "Setting up Python virtual environment..."
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements.txt

echo "Setup complete. Please place your datasets in 'data_raw/'."