#!/bin/bash

# Usage: ./replace_string.sh "old_string" "new_string" /path/to/directory

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <old_string> <new_string> <directory>"
    exit 1
fi

OLD="$1"
NEW="$2"
DIR="$3"

# Recursively find all files (excluding directories) and replace
find "$DIR" -type f -print0 | while IFS= read -r -d '' file; do
    # Use sed to do in-place replacement safely
    sed -i.bak "s/${OLD//\//\\/}/${NEW//\//\\/}/g" "$file"
done

echo "Replacement complete. Backup files have .bak extension."
