#!/bin/bash

SUBMISSION_DIR="submissions"
BACKUP_DIR="unique_backup"
REPORT_FILE="report.txt"
ERROR_FILE="errors.log"
HASH_FILE="processed_hashes.tmp"

processed=0
duplicates=0
backed_up=0

# Clear files from previous runs.
> "$REPORT_FILE"
> "$ERROR_FILE"
> "$HASH_FILE"

# Check whether the submissions directory exists.
if [ ! -d "$SUBMISSION_DIR" ]; then
    echo "Error: submissions directory does not exist." >> "$ERROR_FILE"
    exit 1
fi

# Create the backup directory if it does not exist.
mkdir -p "$BACKUP_DIR" 2>> "$ERROR_FILE"

# Remove backups from an earlier run.
rm -f "$BACKUP_DIR"/* 2>> "$ERROR_FILE"

# Process every regular file in the submissions directory.
for file in "$SUBMISSION_DIR"/*; do

    if [ ! -f "$file" ]; then
        continue
    fi

    processed=$((processed + 1))

    hash=$(sha256sum "$file" 2>> "$ERROR_FILE" | awk '{print $1}')

    if [ -z "$hash" ]; then
        echo "Failed to calculate checksum for $file" >> "$ERROR_FILE"
        continue
    fi

    if grep -q "^$hash " "$HASH_FILE"; then
        duplicates=$((duplicates + 1))
        original=$(grep "^$hash " "$HASH_FILE" | head -n 1 | cut -d' ' -f2-)

        echo "Duplicate found: $file"
        echo "Original file: $original"
    else
        echo "$hash $file" >> "$HASH_FILE"

        if cp "$file" "$BACKUP_DIR/" 2>> "$ERROR_FILE"; then
            backed_up=$((backed_up + 1))
            echo "Unique file backed up: $file"
        else
            echo "Backup failed for $file" >> "$ERROR_FILE"
        fi
    fi

done

{
    echo "Submission Processing Report"
    echo "============================"
    echo "Files processed: $processed"
    echo "Duplicate files: $duplicates"
    echo "Unique files backed up: $backed_up"
    echo "Backup directory: $BACKUP_DIR"
    echo "Error log: $ERROR_FILE"
} > "$REPORT_FILE"

rm -f "$HASH_FILE"

echo
cat "$REPORT_FILE"
