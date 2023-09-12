#!/bin/bash

# The main function of this script is to find all direct subdirectories under a specified path and get their modification dates. 
# It then compares these dates to the dates entered by the user and calculates the number of days each subdirectory is delayed relative to the entered date (if applicable).
# Example: ./submission-report.sh /local/cs460/fall23/turnin/cs460p1 "2023-09-07 15:45:00"
# /local/cs460/fall23/turnin/cs460p1 This folder contains subfolders of student submissions named by student name.
# "2023-09-07 15:45:00" is the deadline for the assignment.

# Check if the correct number of arguments are provided
if [ "$#" -ne 2 ]; then
  echo "Usage: $0 path date"
  exit 1
fi

# Assign the arguments to variables
input_path="$1"
input_date="$2"

# Check if the path exists
if [ ! -d "$input_path" ]; then
  echo "The path does not exist or is not a directory."
  exit 1
fi

# Convert the input date to seconds since 1970-01-01 00:00:00 UTC
input_date_seconds=$(date -d"$input_date" +%s)

# Generate a user-friendly timestamp for the output filename
filename=$(date +"submit-%Y%m%d-%H%M%S")
output_file="$filename.txt"

# Use the find command to locate all direct subdirectories in the specified path and retrieve their modification dates
find "$input_path" -mindepth 1 -maxdepth 1 -type d -exec bash -c '
dir_seconds=$(date -d@"$(stat -c %Y "$1")" +%s)
let "late_days=(dir_seconds-'$input_date_seconds'+(60*60*12))/(60*60*24)"
if (( late_days > 0 )); then
  printf "%-20s %-20s %s\n" "${1#'"$input_path"'/}" "$(date -d@"$(stat -c %Y "$1")" +"%Y-%m-%d-%H:%M:%S")" "Late by $late_days day(s)"
else
  printf "%-20s %s\n" "${1#'"$input_path"'/}" "$(date -d@"$(stat -c %Y "$1")" +"%Y-%m-%d-%H:%M:%S")"
fi' _ {} \; > "$output_file"

echo "Submission report has been saved to $output_file."