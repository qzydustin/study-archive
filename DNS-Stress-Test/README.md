# DNS Stress Test

This performs a DNS stress test by querying a list of domains concurrently.

## Features

- Concurrent DNS queries using asyncio and aiodns
- Reads domain list from a user-specified CSV file
- Generates a detailed report with request statistics

## Dependencies

- Python 3.7+
- `aiodns`
- `requests`

You can install the required packages using pip:

```bash
pip install aiodns requests
```

## Usage

To run the script with default settings:

```bash
python dns-stress-test.py --csv path/to/your/domains.csv
```

## Command-line Arguments

- `--concurrent`: Number of concurrent DNS requests (default: 100)
- `--dns`: DNS server address to use for queries (default: 8.8.8.8)
- `--csv`: Path to the CSV file containing domains (required)

## Domain List

You can obtain a list of top domains from [Cloudflare Radar](https://radar.cloudflare.com/domains).

## Output

This generates a report with the following information:

- Total number of requests
- Number of successful and failed requests
- Statistics on response times (min, max, average, median)
