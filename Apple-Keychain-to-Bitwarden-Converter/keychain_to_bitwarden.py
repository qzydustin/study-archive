import csv
import json
import ipaddress
from urllib.parse import urlparse
from typing import Dict, Tuple, List, Any

def extract_domain(url: str, etld_plus_one: bool = False) -> str:
    """Extract domain from URL or hostname string.

    - Accepts inputs without scheme (e.g., example.com)
    - Normalizes case, strips trailing dot, removes leading www.
    - Returns IP address as-is when host is IPv4/IPv6
    - For eTLD+1, uses a small heuristic (no external dependency)
    """
    if not url:
        return ""

    candidate = url.strip()
    if "://" not in candidate:
        candidate = "https://" + candidate

    try:
        parsed = urlparse(candidate)
    except Exception:
        return ""

    host = (parsed.hostname or "").strip(".").lower()
    if not host:
        return ""

    # IP address (IPv4/IPv6)
    try:
        ipaddress.ip_address(host)
        return host
    except ValueError:
        pass

    if host.startswith("www."):
        host = host[4:]

    if not etld_plus_one:
        return host

    # Extract eTLD+1 (heuristic)
    parts = host.split(".")
    if len(parts) < 2:
        return host

    two_part_tlds = {"co", "com", "net", "org", "edu", "gov", "ac"}
    country_tlds = {"uk", "au", "cn", "jp", "sg", "hk"}

    # Handle eTLD+1 for common two-part country TLDs like co.uk, com.au
    if len(parts) >= 3 and parts[-2] in two_part_tlds and parts[-1] in country_tlds:
        return ".".join(parts[-3:])

    # Default eTLD+1
    if len(parts) >= 2:
        return ".".join(parts[-2:])
    return host

def process_apple_keychain_csv(input_file: str, output_file: str, verbose: bool = True) -> None:
    """Convert Apple Keychain CSV to Bitwarden JSON format.

    Merge logic: only merge when domain_key, username, password, notes, and TOTP are identical.
    Merged items use domain_key as name; separate items use full domain as name.
    """
    # entries keyed by merge_key -> aggregated fields
    entries: Dict[Tuple[str, str, str, str, str], Dict[str, Any]] = {}

    try:
        with open(input_file, newline='', encoding="utf-8") as csvfile:
            reader = csv.DictReader(csvfile)

            for row in reader:
                url = (row.get("URL") or "").strip()
                username = (row.get("Username") or "").strip()
                password = row.get("Password") or ""
                notes = row.get("Notes") or ""
                otpauth = row.get("OTPAuth") or ""

                if not url or not username or not password:
                    if verbose:
                        print(f"Skipping empty entry: {row}")
                    continue  # Skip empty entries

                # Extract domain information
                domain_key = extract_domain(url, etld_plus_one=True)
                full_domain = extract_domain(url, etld_plus_one=False)
                normalized_notes = notes
                normalized_totp = otpauth

                # Use exact combination as key for merging
                merge_key: Tuple[str, str, str, str, str] = (
                    domain_key, username, password, normalized_notes, normalized_totp
                )

                entry = entries.setdefault(
                    merge_key,
                    {
                        "domain_key": domain_key,
                        "full_domain": full_domain,
                        "username": username,
                        "password": password,
                        "notes": normalized_notes,
                        "totp": normalized_totp,
                        "uris": [],
                    },
                )

                # Append URL if new (preserve first-seen order)
                if url not in entry["uris"]:
                    entry["uris"].append(url)

        # Convert to Bitwarden JSON format
        items = []
        for entry in entries.values():
            name = entry["domain_key"] if len(entry["uris"]) > 1 else entry["full_domain"]
            items.append(
                {
                    "name": name,
                    "login": {
                        "username": entry["username"],
                        "password": entry["password"],
                        "uris": ([{"uri": u} for u in entry["uris"]] if entry["uris"] else []),
                        "totp": entry["totp"] or "",
                    },
                    "notes": entry["notes"] or "",
                    "type": 1,
                }
            )

        bitwarden_data = {"items": items}

        with open(output_file, "w", encoding="utf-8") as jsonfile:
            json.dump(bitwarden_data, jsonfile, indent=4)
        if verbose:
            print(f"Conversion successful! Output saved to {output_file}")

    except FileNotFoundError:
        print(f"Error: The file {input_file} was not found.")
    except Exception as e:
        print(f"An error occurred: {e}")

# Usage example
if __name__ == "__main__":
    process_apple_keychain_csv("apple_keychain.csv", "bitwarden.json")
