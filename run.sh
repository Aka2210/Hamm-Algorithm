#!/bin/bash

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <min_support> <input_file> <answer_file>"
    echo "Example: $0 0.2 sample.txt sample_out.txt"
    exit 1
fi

MIN_SUP=$1
INPUT_FILE=$2
ANSWER_FILE=$3

SOURCE_CODE="Hamm.cpp"
EXECUTABLE="hamm"
MY_OUTPUT="temp_raw_output.txt"
DIFF_FILE="error_diff.txt"

rm -f $EXECUTABLE $MY_OUTPUT $DIFF_FILE

echo "🔨 [1/3] Compiling..."
g++ -O3 -o $EXECUTABLE $SOURCE_CODE

if [ $? -ne 0 ]; then
    echo "❌ Compilation Failed!"
    exit 1
fi

echo "🚀 [2/3] Running FP-Growth..."
./$EXECUTABLE $MIN_SUP $INPUT_FILE $MY_OUTPUT

if [ $? -ne 0 ]; then
    echo "❌ Execution Failed!"
    rm -f $MY_OUTPUT
    exit 1
fi

echo "⚖️  [3/3] Comparing..."

if [ ! -f "$ANSWER_FILE" ]; then
    echo "⚠️ Warning: Answer file not found. Output saved to $MY_OUTPUT"
    exit 0
fi

diff -w <(sort "$MY_OUTPUT") <(sort "$ANSWER_FILE") > "$DIFF_FILE"
DIFF_STATUS=$?

if [ $DIFF_STATUS -eq 0 ]; then
    echo "========================================"
    echo "🎉  PASS! Your output matches the answer."
    echo "========================================"
    rm -f $MY_OUTPUT $DIFF_FILE 
else
    echo "========================================"
    echo "❌  FAIL! Differences detected."
    echo "    Check content in: $DIFF_FILE"
    echo "========================================"
    echo "--- Preview of Differences ---"
    head -n 10 "$DIFF_FILE"
    echo "------------------------------"

    rm -f $MY_OUTPUT
fi