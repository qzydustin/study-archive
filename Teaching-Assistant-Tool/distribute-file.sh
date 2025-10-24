#!/bin/bash

# Distribute a file to all subfolders under a target path

# Get the target path
read -p "Please enter the target path: " target_path

# Check if the target path exists
if [ ! -d "$target_path" ]; then
  echo "The target path does not exist. Please enter a valid path."
  exit 1
fi

while true; do
  # Ask for the file to distribute
  read -p "Please enter the file to distribute or type 'E' to exit: " file_to_distribute

  # If the input is 'E', exit the loop
  if [ "$file_to_distribute" == "E" ]; then
    break
  fi

  # Check if the file exists
  if [ ! -f "$file_to_distribute" ]; then
    echo "File does not exist. Please enter a valid filename."
  else
    # Distribute the file to all subfolders under the target path
    for dir in "$target_path"/*; do
      if [ -d "$dir" ]; then
        cp "$file_to_distribute" "$dir/"
      fi
    done
    echo "File has been distribute to all subfolders."
  fi
done

echo "Exiting the program."
