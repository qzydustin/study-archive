import asyncio
import aiodns
import time
import argparse
import statistics
import csv

async def fetch_dns(domain, resolver, semaphore, results, progress):
    async with semaphore:
        start_time = time.time()
        try:
            await resolver.query(domain, 'A')
            duration = time.time() - start_time
            results.append((domain, True, duration))
        except Exception as e:
            duration = time.time() - start_time
            results.append((domain, False, duration))
        finally:
            progress[0] += 1
            print(f"Progress: {progress[0]}/{progress[1]}", end='\r')

async def main(concurrent_requests, dns_server, csv_file):
    if not csv_file:
        raise ValueError("CSV file path must be specified.")

    resolver = aiodns.DNSResolver(nameservers=[dns_server])
    semaphore = asyncio.Semaphore(concurrent_requests)
    results = []

    with open(csv_file, 'r') as file:
        domains = [row['domain'] for row in csv.DictReader(file)]

    total_domains = len(domains)
    progress = [0, total_domains]

    tasks = [asyncio.create_task(fetch_dns(domain, resolver, semaphore, results, progress)) for domain in domains]

    for task in asyncio.as_completed(tasks):
        await task

    total_requests = len(results)
    successful_requests = sum(1 for _, success, _ in results if success)
    failed_requests = total_requests - successful_requests
    durations = [duration for _, success, duration in results if success]

    if durations:
        min_time = min(durations)
        max_time = max(durations)
        average_time = sum(durations) / len(durations)
        median_time = statistics.median(durations)
    else:
        min_time = max_time = average_time = median_time = 0

    print("\n\nTest Report")
    print("-----------")
    print(f"Total Requests: {total_requests}")
    print(f"Successful Requests: {successful_requests}")
    print(f"Failed Requests: {failed_requests}")
    print(f"Min Response Time: {min_time:.2f}s")
    print(f"Max Response Time: {max_time:.2f}s")
    print(f"Average Response Time: {average_time:.2f}s")
    print(f"Median Response Time: {median_time:.2f}s")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="DNS Stress Test")
    parser.add_argument('--concurrent', type=int, default=100, help='Number of concurrent requests')
    parser.add_argument('--dns', type=str, default='8.8.8.8', help='DNS server address')
    parser.add_argument('--csv', type=str, required=True, help='Path to the CSV file containing domains')
    args = parser.parse_args()

    asyncio.run(main(args.concurrent, args.dns, args.csv))