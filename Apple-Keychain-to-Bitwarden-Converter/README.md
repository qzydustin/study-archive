# Apple Keychain to Bitwarden Converter

This project provides a Python script that converts an Apple Keychain CSV file to a Bitwarden-compatible JSON format. The script processes your CSV export by grouping entries that share the same username and password, merging related website URLs, and standardizing the entry titles.

## What It Converts

- **Logins**: Username, Password, and associated Website URIs
- **Notes**: Preserved from the Apple Keychain `Notes` column
- **TOTP**: Preserved from the Apple Keychain `OTPAuth` column (Bitwarden `totp` field)

## How It Works

- **CSV Parsing and Data Extraction**  
  The script reads your Apple Keychain CSV using Python’s CSV library. It expects columns: `URL`, `Username`, `Password`, optionally `Notes` and `OTPAuth`. Rows missing `URL`, `Username`, or `Password` are skipped.

- **Domain Extraction (built-in)**  
  URLs are parsed with Python’s `urllib.parse` and normalized (case, `www.` removal). eTLD+1 is derived heuristically without external dependencies. IP addresses are preserved as-is.

- **Merging Strategy**  
  Entries are merged only when the tuple `(eTLD+1 domain, username, password, notes, totp)` is identical. Merged entries keep multiple URIs. If an item has multiple URIs, its name uses the eTLD+1; otherwise it uses the full domain.

- **Conversion to Bitwarden Format**  
  Outputs Bitwarden’s JSON import format. Each item includes: `name`, `login.username`, `login.password`, `login.uris`, `login.totp`, optional `notes`, and `type = 1` (login).

## Prerequisites

- Python 3.x (no third-party dependencies)

## Installation

1. Clone or download this repository.
2. Ensure that Python 3 is installed on your system.

## Usage

1. Place your Apple Keychain CSV file (e.g., `apple_keychain.csv`) in the same directory as the script.
2. Run the script using Python:

       python keychain_to_bitwarden.py

   By default, the script reads `apple_keychain.csv` and writes `bitwarden.json`.

3. The output JSON file will be created and can be imported directly into Bitwarden.

